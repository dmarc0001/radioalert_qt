#include "singleradioalert.hpp"
#include <QThread>

namespace radioalert
{
  using namespace bose_soundtoch_lib;

  /**
   * @brief SingleRadioAlert::SingleRadioAlert
   * @param logger Logger vom Haptprozess
   * @param alert Konfiguration des aktell zu bearbeitenden alarms
   * @param devices List mit Geräten
   * @param parent QObject Parent Widget
   */
  SingleRadioAlert::SingleRadioAlert( std::shared_ptr< Logger > logger,
                                      SingleAlertConfig &alert,
                                      StDevicesHashList &devices,
                                      QObject *parent )
      : QObject( parent )
      , lg( logger )
      , localAlertConfig( alert )
      , avStDevices( devices )
      , alertLoopCounter( 0 )
      , masterDeviceStat( deviceStatus::NONE )
      , isActive( true )
      , callBackWsConnected( false )
      , connectWsTrysCount( 0 )
      , oldVolume( -1 )
  {
    LGINFO( QString( "start radio alert %1 at %2" )
                .arg( localAlertConfig.getAlertName() )
                .arg( localAlertConfig.getAlertDate().toString( "hh:mm" ) ) );
    //
    // Voraussetzungen prüfen
    // sind das/die angeorderten gerät(e) verfügbar?
    //
    if ( !checkIfDevicesAvailible() )
    {
      throw NoAvailibleSoundDeviceException( QLatin1String( "no available devices for alert!" ) );
    }
  }

  /**
   * @brief SingleRadioAlert::~SingleRadioAlert
   */
  SingleRadioAlert::~SingleRadioAlert()
  {
    LGDEBUG( "SingleRadioAlert::~SingleRadioAlert..." );
    waitForTimer.stop();
    disconnect( &waitForTimer, 0, 0, 0 );
    disconnect( masterDevice.get(), 0, 0, 0 );
    // emit sigAlertFinished( this );
  }

  /**
   * @brief SingleRadioAlert::slotOnZyclonTimer
   * Zyklisch vom Daemon aufgerufen
   */
  void SingleRadioAlert::slotOnZyclonTimer( void )
  {
    // TODO: Überwachung des Ablaufes...
    alertLoopCounter++;
    LGINFO( QString( "SingleRadioAlert::slotOnZyclonTimer: alert %1 loop <%2> duration <%3> sec, active: %4" )
                .arg( localAlertConfig.getAlertName() )
                .arg( alertLoopCounter, 8, 10, QChar( '0' ) )
                .arg( alertDuration, 4, 10, QChar( '0' ) )
                .arg( isActive ) );
    //
    // läuft der Alarm noch?
    //
    if ( isActive )
    {
      //
      // Alarm ist noch aktiv, läuft die Zeit noch?
      //
      if ( alertDuration-- > 0 )
      {
        // läuft noch, nix machen
      }
      else
      {
        //
        // Alarmzeit abgelaufen
        // Kontrollieren, damit das nur ein mal aufgerufen wird
        //
        if ( masterDeviceStat == deviceStatus::PLAY_ALERT )
        {
          if ( localAlertConfig.getAlertRaiseVol() && isActive )
          {
            //
            // ok sanft ausdimmen, Kennzeichne das mit ENDING_ALERT
            //
            masterDeviceStat = deviceStatus::ENDING_ALERT;
            waitForTimer.setSingleShot( false );
            sendVolume = currentVolume;
            connect( &waitForTimer, &QTimer::timeout, [=]() {
              // runterdimmen
              masterDevice->setVolume( sendVolume-- );
              if ( sendVolume <= 0 )
              {
                connect( &waitForTimer, 0, 0, 0 );
                waitForTimer.stop();
                masterDeviceStat = deviceStatus::ALERT_FINISH;
              }
            } );
            waitForTimer.start( DIMMERTIMEVELUE );
          }
          else
          {
            masterDeviceStat = deviceStatus::ALERT_FINISH;
          }
        }  // ende if PLAY_ALERT
        else if ( masterDeviceStat == deviceStatus::ENDING_ALERT )
        {
          // hier geht es lang, wenn ich noch dimme
          LGDEBUG(
              QString( "SingleRadioAlert::slotOnZyclonTimer: alert %1 wait for end dimming" ).arg( localAlertConfig.getAlertName() ) );
        }
        else
        {
          //
          // wenn nicht PLAY_ALERT und nicht ENDING_ALERT dann ist das wohl das Ende
          //
          LGDEBUG( "SingleRadioAlert::slotOnZyclonTimer: alert regulay endet" );
          masterDeviceStat = deviceStatus::ALERT_FINISH;
          masterDevice->setKey( BoseDevice::bose_key::KEY_POWER, BoseDevice::bose_keystate::KEY_TOGGLE );
          QThread::sleep( 200 );
          masterDevice->setVolume( oldVolume );
          isActive = false;
          disconnect( masterDevice.get(), 0, 0, 0 );
          emit sigAlertFinished( this );
          return;
        }
      }  // ende duration < 0
    }    // ende isActive
    else
    {
      //
      // active ist false -> abbrechen
      //
      LGDEBUG( "SingleRadioAlert::slotOnZyclonTimer: alert deactivated..." );
      disconnect( masterDevice.get(), 0, 0, 0 );
      emit sigAlertFinished( this );
    }
  }

  /**
   * @brief SingleRadioAlert::start
   */
  void SingleRadioAlert::start( void )
  {
    lastError.clear();
    // das Master device ist dann das erst in der liste
    masterDeviceName = *( realDevices.keyBegin() );
    //
    // entferne Master aus der Liste
    //
    masterDeviceData = realDevices.take( masterDeviceName );
    LGDEBUG( "SingleRadioAlert::SingleRadioAlert: create BSoundTouchDevice, masterr device is %1..." );
    masterDevice = std::unique_ptr< BoseDevice >(
        new BoseDevice( masterDeviceData.hostName, masterDeviceData.wsPort, masterDeviceData.httpPort ) );
    connect( masterDevice.get(), &BoseDevice::sigOnRequestAnswer, this, &SingleRadioAlert::slotOnRequestAnswer );
    LGDEBUG( "SingleRadioAlert::SingleRadioAlert: create BSoundTouchDevice...OK" );
    alertDuration = localAlertConfig.getAlertDuration();
    //
    // 01:
    // anfrage senden: ist das master device frei (also im standby)?
    //
    masterDeviceStat = deviceStatus::NONE;
    masterDevice->getNowPlaying();
    // wie lange geht der Spass?
    //
    // jetzt warten wir auf now playing == "STANDBY"
    // wenn das nicht innerhalb der TIMEOUT zeit passiert oder das Gerät
    // etwas abspielt, vergessen wir das...
    //
    waitForTimer.setSingleShot( true );
    connect( &waitForTimer, &QTimer::timeout, [=]() {
      this->cancelAlert( QString( "timeout while waiting for now playing in alert %1" ).arg( localAlertConfig.getAlertName() ) );
    } );
    //
    // um den Erfolg abzuwenden müsste man den timer stoppen
    // der wird nur gestoppt, wenn nowPlaying STANDBY empfängt und der Status des Gerätes NONE ist
    //
    waitForTimer.start( RESPONSETIMEOUT );
  }

  /**
   * @brief SingleRadioAlert::cancelAlert
   */
  void SingleRadioAlert::cancelAlert( void )
  {
    cancelAlert( "alert canceled!" );
  }

  /**
   * @brief SingleRadioAlert::cancelAlert
   * @param msg
   */
  void SingleRadioAlert::cancelAlert( QString msg )
  {
    // TODO: alarm beenden, Radio evtl abschalten
    qWarning().nospace().noquote() << "################ cancel alert: <" << msg << "> ##################";
    isActive = false;
    masterDevice.get()->disconnect();
    emit sigAlertResultError( msg );
  }

  /**
   * @brief SingleRadioAlert::getAlertName
   * @return
   */
  QString SingleRadioAlert::getAlertName( void )
  {
    return ( localAlertConfig.getAlertName() );
  }

  /**
   * @brief SingleRadioAlert::checkIfDevicesAvailible
   * @return
   */
  bool SingleRadioAlert::checkIfDevicesAvailible( void )
  {
    LGDEBUG( "SingleRadioAlert::checkIfDevicesAvailible: check if devices availible..." );
    // Iterator für alarm Devices
    QStringList::Iterator alDevice;
    // QList< QString >::Iterator alDevice;
    for ( alDevice = localAlertConfig.getAlertDevices().begin(); alDevice != localAlertConfig.getAlertDevices().end(); alDevice++ )
    {
      if ( avStDevices.contains( *alDevice ) )
      {
        //
        // der realen Liste zufügen
        //
        realDevices.insert( *alDevice, avStDevices.value( *alDevice ) );
      }
      else
      {
        LGWARN(
            QString( "RadioAlertThread::checkIfDevicesAvailible: device %1 was not discovered in the network..." ).arg( *alDevice ) );
      }
    }
    LGDEBUG( "SingleRadioAlert::checkIfDevicesAvailible: done." );
    //
    // Liste mit geforderten devices, die auch im Netzwerk vorhanden sind
    //
    if ( realDevices.count() == 0 )
    {
      //
      // ALARM: Keine Geräte gefunden
      //
      LGWARN( QString( "there are none availible soundtouch devices for this request (%1:%2) in the network" )
                  .arg( localAlertConfig.getAlertName() )
                  .arg( localAlertConfig.getAlertDevices().join( "," ) ) );
      return ( false );
    }
    LGDEBUG( "RadioAlertThread::checkIfDevicesAvailible: check if devices availible...OK" );
    return ( true );
  }

  /**
   * @brief SingleRadioAlert::connectCallbacksforDevice
   * @param device
   */
  void SingleRadioAlert::connectCallbacksforDevice( void )
  {
    if ( connectWsTrysCount++ > MAXCONNECTTRYS || !isActive )
    {
      cancelAlert( "to many unsuccesfull connect trys to device! CANCEL Alert" );
      return;
    }
    // ########################################################################
    // CALLBACKS erstellen
    // ########################################################################
    //
    // besteht eine Verbindung, Marker setzen
    connect( masterDevice.get(), &BoseDevice::sigOnWSConnected, [=]() {
      callBackWsConnected = true;
      LGDEBUG( "SingleRadioAlert::connectCallbacksforDevice: device callback estabished..." );
    } );
    // geht Verbindung verloren, neu versuchen
    connect( masterDevice.get(), &BoseDevice::sigOnWSDisConnected, [=]() {
      callBackWsConnected = false;
      LGDEBUG( "SingleRadioAlert::connectCallbacksforDevice: device callback connection lost..." );
      // Signale wieder trennen (ergänzen, wenn mehr verbunden werden)
      disconnect( masterDevice.get(), &BoseDevice::sigOnWSConnected, 0, 0 );
      disconnect( masterDevice.get(), &BoseDevice::sigOnWSDisConnected, 0, 0 );
      disconnect( masterDevice.get(), &BoseDevice::sigOnVolumeUpdated, 0, 0 );
      disconnect( masterDevice.get(), &BoseDevice::sigOnPresetSelectionUpdated, 0, 0 );
      disconnect( masterDevice.get(), &BoseDevice::sigOnErrorUpdated, 0, 0 );
      disconnect( masterDevice.get(), &BoseDevice::sigOnNowPlayingUpdated, 0, 0 );
      this->connectCallbacksforDevice();
    } );
    // Lautstärke Callback
    connect( masterDevice.get(), &BoseDevice::sigOnVolumeUpdated, [=]( SharedResponsePtr respObj ) {
      WsVolumeUpdated *volObj = static_cast< WsVolumeUpdated * >( respObj.get() );
      currentVolume = volObj->getActualVolume();
      if ( oldVolume == -1 )
        oldVolume = currentVolume;
      LGDEBUG( QString( "SingleRadioAlert::connectCallbacksforDevice: volume callback. volume: %1" )
                   .arg( currentVolume, 3, 10, QChar( '0' ) ) );
    } );
    // Sender gewechselt
    connect( masterDevice.get(), &BoseDevice::sigOnPresetSelectionUpdated, [=]( SharedResponsePtr respObj ) {
      WsNowSelectionUpdated *selObj = static_cast< WsNowSelectionUpdated * >( respObj.get() );
      // TODO: gilt nur für PRESETS (anpassen bei Erweiterung)
      if ( QString( PRESETPATTERN ).append( selObj->getDevicePresets().id ) != localAlertConfig.getAlertSource() )
      {
        this->cancelAlert( "device play selection was changed, alert cancel!" );
        LGWARN( "SingleRadioAlert::connectCallbacksforDevice: devices selection was changed, cancel alert!" );
        return;
      }
    } );
    // NOW Playing info
    connect( masterDevice.get(), &BoseDevice::sigOnNowPlayingUpdated, this, &SingleRadioAlert::slotOnNowPlayingUpdate );
    // ERROR
    connect( masterDevice.get(), &BoseDevice::sigOnErrorUpdated, [=]( SharedResponsePtr respObj ) {
      bose_soundtoch_lib::WsErrorUpdated *errObj = static_cast< WsErrorUpdated * >( respObj.get() );
      lastError = errObj->getError().text;
    } );
    //
    // device verbinden
    //
    masterDevice->connectWs();
  }

  void SingleRadioAlert::slotOnRequestAnswer( SharedResponsePtr response )
  {
    //
    LGDEBUG( "RadioAlertThread::slotOnRequestAnswer..." );
    switch ( response->getResultType() )
    {
      default:
        LGWARN( QString( "RadioAlertThread::slotOnRequestAnswer: %1" ).arg( static_cast< int >( response->getResultType() ) ) );
        break;

      case ResultobjectType::R_OK:
        LGDEBUG( "RadioAlertThread::slotOnRequestAnswer: status" );
        computeStausMsg( response );
        break;

      case ResultobjectType::R_VOLUME:
        LGDEBUG( "RadioAlertThread::slotOnRequestAnswer: volume" );
        computeVolumeMsg( response );
        break;

      case ResultobjectType::R_NOW_PLAYING:
        LGDEBUG( "RadioAlertThread::slotOnRequestAnswer: now playing" );
        computeNowPlayingMsg( response );
        break;
    }
  }

  void SingleRadioAlert::slotOnNowPlayingUpdate( SharedResponsePtr respObj )
  {
    WsNowPlayingUpdate *nowPlayObj = static_cast< WsNowPlayingUpdate * >( respObj.get() );
    //
    // Puffer Status?
    //
    if ( nowPlayObj->getPlayStatus().contains( masterDevice->getPlayStateName( BoseDevice::bose_playstate::BUFFERING_STATE ) ) )
    {
      // Puffern?
      // schritt 03a
      // warten auf PLAY
      if ( masterDeviceStat == deviceStatus::NONE )
      {
        //
        // erst mal auf LEISE
        //
        masterDevice->setVolume( 0 );
      }
      masterDeviceStat = deviceStatus::BUFFERING;
    }
    //
    // Abspielstatus?
    //
    else if ( nowPlayObj->getPlayStatus().contains( masterDevice->getPlayStateName( BoseDevice::bose_playstate::PLAY_STATE ) ) )
    {
      if ( masterDeviceStat == deviceStatus::NONE || masterDeviceStat == deviceStatus::BUFFERING )
      {
        //
        // 04, das Gerät spielt
        // OK Timer deaktivieren
        //
        LGDEBUG( "SingleRadioAlert::slotOnNowPlayingUpdate: timeout stopped..." );
        disconnect( &waitForTimer, 0, 0, 0 );
        waitForTimer.stop();
        LGDEBUG( "SingleRadioAlert::slotOnNowPlayingUpdate: set master device status to PLAYING..." );
        masterDeviceStat = deviceStatus::PLAYING;
        //
        // wenn in der Liste noch Geräte vorhanden sind, dann sind das SKLAVEN
        //
        if ( realDevices.count() > 0 )
        {
          masterDeviceStat = deviceStatus::INIT_GROUP;
          LGDEBUG( "SingleRadioAlert::slotOnNowPlayingUpdate: build an zone for play..." );
          //
          // versuche 4a, Gruppenbildung...
          // Gruppe bilden
          //
          slaveList.clear();
          for ( auto it = realDevices.keyBegin(); it != realDevices.keyEnd(); it++ )
          {
            LGDEBUG( QString( "SingleRadioAlert::slotOnNowPlayingUpdate: add slave: %1" ).arg( realDevices.value( *it ).hostName ) );
            slaveList.append( SoundTouchMemberObject( realDevices.value( *it ).hostName, realDevices.value( *it ).deviceId ) );
          }
          masterDevice->setZone( masterDeviceData.deviceId, slaveList );
          QThread::sleep( 400 );
          //
          // TODO: evtl noch überwachen, dass die zohne eingerichtet wurde
          //
        }
        masterDeviceStat = deviceStatus::PLAY_ALERT;
        //#####################################################################
        // ab hier ist der Alarm im Gange
        //#####################################################################
        if ( localAlertConfig.getAlertRaiseVol() )
        {
          LGDEBUG( "raise volume: started!" );
          waitForTimer.setSingleShot( false );
          sendVolume = 0;
          connect( &waitForTimer, &QTimer::timeout, [=]() {
            // Lambda zum hochdimmen
            masterDevice->setVolume( sendVolume++ );
            if ( sendVolume >= localAlertConfig.getAlertVolume() )
            {
              disconnect( &waitForTimer, 0, 0, 0 );
              LGDEBUG( "raise volume: reached!" );
              waitForTimer.stop();
            }
          } );
          waitForTimer.start( DIMMERTIMEVELUE );
        }
        else
        {
          masterDevice->setVolume( localAlertConfig.getAlertVolume() );
        }
      }
      else
      {
        // TODO: was sinnvolles machen, momentan einfach nix
      }
    }
    else
    {
      //
      // da hat jemand eingegriffen, beende alle Aktionen
      //
      LGWARN( QString( "SingleRadioAlert::slotOnNowPlayingUpdate: device going to state: %1" ).arg( nowPlayObj->getPlayStatus() ) );
      cancelAlert( "device going in state stop/pause, cancel alert!" );
    }
  }

  void SingleRadioAlert::computeStausMsg( SharedResponsePtr response )
  {
    HttpResultOkObject *okObj = static_cast< HttpResultOkObject * >( response.get() );
    LGDEBUG( QString( "SingleRadioAlert::computeStausMsg: status for action recived, status: %1" ).arg( okObj->getStatus() ) );
  }

  void SingleRadioAlert::computeVolumeMsg( SharedResponsePtr response )
  {
    HttpVolumeObject *volObj = static_cast< HttpVolumeObject * >( response.get() );
    if ( oldVolume == -1 )
    {
      oldVolume = volObj->getActualVolume();
    }
    currentVolume = volObj->getActualVolume();
    LGDEBUG( QString( "SingleRadioAlert::computeVolumeMsg: recived type: VOLUME <%1>" ).arg( volObj->getActualVolume() ) );
  }

  /**
   * @brief SingleRadioAlert::computeNowPlayingMsg
   * @param response
   * Was spielt das Gerät zur Zeit?
   */
  void SingleRadioAlert::computeNowPlayingMsg( SharedResponsePtr response )
  {
    HttpNowPlayingObject *nPlayObj = static_cast< HttpNowPlayingObject * >( response.get() );
    LGDEBUG( QString( "SingleRadioAlert::computeNowPlayingMsg: recived type: SOURCE <%1>" ).arg( nPlayObj->getSource() ) );
    switch ( masterDeviceStat )
    {
      case deviceStatus::NONE:
        //
        // wenn das Gerät noch keinen Status hat, war das die Frage ob
        // es im STANDBY ist, da sollte dann die Meldung kommen, das Gerät ist da und AUS
        //
        switchMasterDeviceToSource( nPlayObj );
        break;

      default:
        LGDEBUG( "SingleRadioAlert::computeNowPlayingMsg: master device status is NOT expected/supported" );
    }
  }

  /**
   * @brief SingleRadioAlert::switchMasterDeviceToSource
   * @param nPlayObj
   * schalte das Masterdevice zur konfigurierten Quelle
   */
  void SingleRadioAlert::switchMasterDeviceToSource( HttpNowPlayingObject *nPlayObj )
  {
    LGDEBUG( "SingleRadioAlert::switchMasterDeviceToSource: master device status is NONE" );
    if ( ( nPlayObj->getSource() ).compare( QLatin1String( "STANDBY" ) ) == 0 )
    {
      // die Timeoutfuntion löschen
      LGDEBUG( "SingleRadioAlert::switchMasterDeviceToSource: timeout stopped..." );
      disconnect( &waitForTimer, 0, 0, 0 );
      waitForTimer.stop();
      LGDEBUG( "SingleRadioAlert::switchMasterDeviceToSource: set master device status to STANDBY" );
      masterDeviceStat = deviceStatus::STANDBY;
      //
      // 02:
      // Callbacks einrichten, Websocket verbinden
      //
      LGDEBUG( "SingleRadioAlert::switchMasterDeviceToSource: connect ws callbacks..." );
      connectCallbacksforDevice();
      LGDEBUG( "SingleRadioAlert::switchMasterDeviceToSource: connect ws callbacks...OK" );
      //
      // und Lautstärke erfragen (zum wieder einstellen nach dem Wecker)
      // wird über Callback dann ausgelesen, asyncron
      //
      masterDevice->getVolume();
      //
      // 03:
      // Radio einschalten
      // TODO: gilt nur für PRESETS -> Anpassung erforderlich wenn erweitert
      //
      BoseDevice::bose_key preset = masterDevice->getKeyForName( localAlertConfig.getAlertSource() );
      if ( preset == BoseDevice::bose_key::KEY_UNKNOWN )
      {
        LGWARN( QString( "SingleRadioAlert::switchMasterDeviceToSource: alert source unknown: " )
                    .append( localAlertConfig.getAlertSource() ) );
        cancelAlert( QString( "alert source %1 unknown / not supported" ).arg( localAlertConfig.getAlertSource() ) );
        return;
      }
      LGDEBUG( QString( "SingleRadioAlert::switchMasterDeviceToSource: now switch to source " )
                   .append( localAlertConfig.getAlertSource() ) );
      masterDevice->setKey( preset, BoseDevice::bose_keystate::KEY_RELEASED );
      //
      // warte bis das Gerät eingeschaltet ist und BUFFERING oder PLAY_STATE gemeldet wird
      // während der Status noch STANDBY ist
      // dann weiter mit 03a (warten währen buffering) oder 04 (playstatus)
      //
      waitForTimer.setSingleShot( true );
      connect( &waitForTimer, &QTimer::timeout, [=]() {
        this->cancelAlert(
            QString( "timeout while waiting for switch to source in alert %1" ).arg( localAlertConfig.getAlertName() ) );
      } );
      //
      // um den Erfolg abzuwenden müsste man den timer stoppen
      waitForTimer.start( RESPONSETIMEOUT );
      return;
    }
    else
    {
      LGWARN( "SingleRadioAlert::computeNowPlayingMsg: NOT STANDBY == alert ignore!" );
      cancelAlert( "device is NOT in STANDBY == alert ignore!" );
    }
  }

}  // namespace radioalert
