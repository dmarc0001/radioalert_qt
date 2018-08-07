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
      , alertCommand( std::unique_ptr< AsyncAlertCommand >( new AsyncAlertCommand( lg, parent ) ) )
      , alertLoopCounter( 0 )
      , masterDeviceStat( deviceStatus::NONE )
      , isActive( true )
      , callBackWsConnected( false )
      , connectWsTrysCount( 0 )
      , oldVolume( -1 )
      , isUserVolumeAction( false )
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
      isActive = false;
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
    disconnect( masterDevice.get() );
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
        if ( masterDeviceStat == deviceStatus::ALERT_IS_PLAYING )
        {
          if ( localAlertConfig.getAlertRaiseVol() && isActive && !isUserVolumeAction )
          {
            //
            // ok sanft ausdimmen, Kennzeichne das mit ENDING_ALERT
            //
            masterDeviceStat = deviceStatus::ALERT_IS_ENDING;
            waitForTimer.setSingleShot( false );
            sendVolume = currentVolume;
            connect( &waitForTimer, &QTimer::timeout, [=]() {
              // runterdimmen ohne callbackkontrolle
              masterDevice->setVolume( --sendVolume );
              if ( sendVolume <= 0 )
              {
                disconnect( &( this->waitForTimer ), 0, 0, 0 );
                this->waitForTimer.stop();
                this->masterDeviceStat = deviceStatus::ALERT_IS_FINISHED;
              }
            } );
            waitForTimer.start( DIMMERTIMEVELUE );
          }
          else
          {
            masterDeviceStat = deviceStatus::ALERT_IS_FINISHED;
          }
        }  // ende if PLAY_ALERT
        else if ( masterDeviceStat == deviceStatus::ALERT_IS_ENDING )
        {
          // hier geht es lang, wenn ich noch dimme
          LGDEBUG(
              QString( "SingleRadioAlert::slotOnZyclonTimer: alert %1 wait for end dimming" ).arg( localAlertConfig.getAlertName() ) );
        }  // ende ENDING_ALERT
        else if ( masterDeviceStat == deviceStatus::ALERT_IS_FINISHED )
        {
          //
          // jetz nur noch ausschalten
          //
          LGDEBUG( "SingleRadioAlert::slotOnZyclonTimer: alert regulay ending..." );
          switchDeviceOff();
          return;
        }
        // ende ALERT_FINISH
        else if ( masterDeviceStat == deviceStatus::DEVICE_IS_SWITCH_OFF )
        {
          //
          // warten auf Abschalten
          //
          LGDEBUG( "SingleRadioAlert::slotOnZyclonTimer: wait for switch off..." );
        }  // ende ALERT_SWITCH_OFF
        else if ( masterDeviceStat == deviceStatus::ALERT_IS_ENDET )
        {
          sendVolume = oldVolume;
          masterDevice->setVolume( sendVolume );
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
    LGDEBUG( QString( "SingleRadioAlert::start: create BSoundTouchDevice, masterr device is %1..." ).arg( masterDeviceName ) );
    masterDevice = std::unique_ptr< BoseDevice >(
        new BoseDevice( masterDeviceData.hostName, masterDeviceData.wsPort, masterDeviceData.httpPort ) );
    connect( masterDevice.get(), &BoseDevice::sigOnRequestAnswer, this, &SingleRadioAlert::slotOnRequestAnswer );
    LGDEBUG( "SingleRadioAlert::start: create BSoundTouchDevice...OK" );
    alertDuration = localAlertConfig.getAlertDuration();
    //
    // 01:
    // anfrage senden: ist das master device frei (also im standby)?
    //
    masterDeviceStat = deviceStatus::NONE;
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
    auto conn = std::make_shared< QMetaObject::Connection >();
    *conn = connect( alertCommand.get(), &AsyncAlertCommand::sigDeviceIsStandby, [=]( bool isStandby ) {
      disconnect( *conn );
      // Antwort ob das Gerät im Standby ist
      if ( isStandby )
      {
        // Alles Gut, Gerät ist im Standby
        // TIMER STOPPEN
        disconnect( &( this->waitForTimer ), 0, 0, 0 );
        this->waitForTimer.stop();
        //
        // und Lautstärke erfragen (zum wieder einstellen nach dem Wecker)
        // wird über Callback dann ausgelesen, asyncron
        //
        this->masterDevice->getVolume();
        // zu Schritt 02
        LGDEBUG( "SingleRadioAlert::start: device is STANDBY, go to set volume" );
        this->getDeviceStartVolume();
      }
      else
      {
        // nix ist gut
        this->cancelAlert( QString( "device is NOT in standby Mode for alert %1" ).arg( localAlertConfig.getAlertName() ) );
      }
    } );
    //
    // um den Erfolg abzuwenden müsste man den timer stoppen
    // der wird nur gestoppt, wenn nowPlaying STANDBY empfängt und der Status des Gerätes NONE ist
    //
    waitForTimer.start( RESPONSETIMEOUT );
    //
    // die Anfrage ausführen
    //
    LGDEBUG( "SingleRadioAlert::start: check if device in Standby..." );
    alertCommand->checkIfDeviceInStandby( masterDevice.get() );
  }

  void SingleRadioAlert::getDeviceStartVolume( void )
  {
    //
    // Schritt 02
    // Lautstärke initial erfragen
    // wenn das nicht innerhalb der TIMEOUT zeit passiert oder das Gerät
    // etwas abspielt, vergessen wir das...
    //
    LGDEBUG( "SingleRadioAlert::getDeviceStartVolume..." );
    waitForTimer.setSingleShot( true );
    connect( &waitForTimer, &QTimer::timeout, [=]() {
      this->cancelAlert( QString( "timeout while waiting for get volume in alert %1" ).arg( localAlertConfig.getAlertName() ) );
    } );
    auto conn = std::make_shared< QMetaObject::Connection >();
    *conn = connect( alertCommand.get(), &AsyncAlertCommand::sigDeviceVolume, [=]( int cVolume ) {
      disconnect( *conn );
      // hier die aktuelle Lautstärke
      currentVolume = cVolume;
      if ( oldVolume < 0 )
      {
        oldVolume = cVolume;
      }
      // Alles Gut, Lautstärke gelesen
      // TIMER STOPPEN
      disconnect( &( this->waitForTimer ), 0, 0, 0 );
      this->waitForTimer.stop();
      //
      // weiter zu Schritt 03
      //
      this->masterDeviceStat = deviceStatus::DEVICE_STANDBY;
      LGDEBUG( "SingleRadioAlert::getDeviceStartVolume: set volume OK, now switch source on device..." );
      this->switchMasterDeviceToSource();
    } );
    //
    // um den Erfolg abzuwenden müsste man den timer stoppen
    // der wird nur gestoppt, wenn nowPlaying STANDBY empfängt und der Status des Gerätes NONE ist
    //
    waitForTimer.start( RESPONSETIMEOUT );
    //
    // die Anfrage ausführen
    //
    alertCommand->askForVolume( masterDevice.get() );
  }

  /**
   * @brief SingleRadioAlert::switchMasterDeviceToSource
   * @param nPlayObj
   * schalte das Masterdevice zur konfigurierten Quelle
   */
  void SingleRadioAlert::switchMasterDeviceToSource( void )
  {
    LGDEBUG( "SingleRadioAlert::switchMasterDeviceToSource..." );
    //
    // erst mal auf LEISE (asyncron)
    //
    sendVolume = 0;
    masterDevice->setVolume( sendVolume );
    //
    // Schritt 03:
    // Radio einschalten
    // TODO: gilt nur für PRESETS -> Anpassung erforderlich wenn erweitert
    // warte bis das Gerät eingeschaltet ist und BUFFERING oder PLAY_STATE gemeldet wird
    // während der Status noch STANDBY ist
    // dann weiter mit 04 (warten während buffering bis status play)
    //
    auto conn = std::make_shared< QMetaObject::Connection >();
    *conn = connect( alertCommand.get(), &AsyncAlertCommand::sigDeviceIsSwitchedToSource, [=]( bool isSwitched ) {
      // Antwort ob das Gerät umgeschaltet hat und läuft
      if ( isSwitched )
      {
        disconnect( *conn );
        // Alles Gut, Gerät spielt
        // TIMER STOPPEN
        disconnect( &( this->waitForTimer ), 0, 0, 0 );
        this->waitForTimer.stop();
        //
        // zu Schritt 04 , Sklaven verbinden
        //
        masterDeviceStat = deviceStatus::DEVICE_PLAYING;
        this->connectDeviceSlaves();
      }
      else
      {
        // nix ist gut
        // gerät ist noch am puffern...
      }
    } );
    //
    // Timeout einschalten
    //
    waitForTimer.setSingleShot( true );
    connect( &waitForTimer, &QTimer::timeout, [=]() {
      this->cancelAlert( QString( "timeout while waiting for switch to source in alert %1" ).arg( localAlertConfig.getAlertName() ) );
    } );
    //
    // um den Erfolg abzuwenden müsste man den timer stoppen
    waitForTimer.start( RESPONSETIMEOUT );
    //
    // Abfrage starten
    //
    alertCommand->switchDeviceToSource( masterDevice.get(), localAlertConfig.getAlertSource() );
  }

  void SingleRadioAlert::connectDeviceSlaves( void )
  {
    //
    // Schritt 04
    // verbinde Sklaven, wenn vorhanden
    //
    if ( realDevices.count() > 0 )
    {
      masterDeviceStat = deviceStatus::DEVICE_INIT_GROUP;
      LGDEBUG( "SingleRadioAlert::connectDeviceSlaves: build an zone for play..." );
      //
      // versuche 04a, Gruppenbildung...
      // Gruppe bilden
      //
      slaveList.clear();
      for ( auto it = realDevices.keyBegin(); it != realDevices.keyEnd(); it++ )
      {
        LGDEBUG( QString( "SingleRadioAlert::connectDeviceSlaves: add slave: %1" ).arg( realDevices.value( *it ).hostName ) );
        slaveList.append( SoundTouchMemberObject( realDevices.value( *it ).hostName, realDevices.value( *it ).deviceId ) );
      }
      masterDevice->setZone( masterDeviceData.deviceId, slaveList );
      QThread::sleep( 400 );
      //
      // TODO: evtl noch überwachen, dass die zohne eingerichtet wurde
      //
    }
    else
    {
      //
      // Gehe zu Schritt 05, dimme oder stelle Lautstärke auf Konfigurierten Wert
      //
      computeVolumeForDevice();
    }
  }

  void SingleRadioAlert::computeVolumeForDevice( void )
  {
    //
    // Schritt 05
    // das Gerät spielt die Source, jetzt Lautstärke einstellen
    //
    masterDeviceStat = deviceStatus::ALERT_IS_PLAYING;
    //#####################################################################
    // ab hier ist der Alarm im Gange
    //#####################################################################
    //
    // Callbacks einrichten, Websocket verbinden
    //
    LGDEBUG( "SingleRadioAlert::computeVolumeForDevice: connect ws callbacks..." );
    connectCallbacksforDevice();
    LGDEBUG( "SingleRadioAlert::computeVolumeForDevice: connect ws callbacks...OK" );
    //
    if ( !isUserVolumeAction )
    {
      //
      // der User hat eingegriffen, also keine Lautstärkeaktion mehr vom Programm
      // sollte eingeltlich hier noch nicht gehen
      //
      if ( localAlertConfig.getAlertRaiseVol() )
      {
        LGDEBUG( "raise volume: started!" );
        waitForTimer.setSingleShot( false );
        sendVolume = 0;
        this->masterDevice->setVolume( ++sendVolume );
        //
        // Missbrauche den Timer zum Dimmen...
        //
        connect( &waitForTimer, &QTimer::timeout, [=]() {
          if ( sendVolume != currentVolume )
            return;
          // Lambda zum hochdimmen
          this->masterDevice->setVolume( ++sendVolume );
          if ( sendVolume >= localAlertConfig.getAlertVolume() )
          {
            disconnect( &( this->waitForTimer ), 0, 0, 0 );
            LGDEBUG( "raise volume: reached!" );
            this->waitForTimer.stop();
            this->disconnectCallbacksforDevice();
          }
        } );
        waitForTimer.start( DIMMERTIMEVELUE );
      }
      else
      {
        sendVolume = localAlertConfig.getAlertVolume();
        masterDevice->setVolume( sendVolume );
      }
    }
    //
    // ###### Ab hier läuft der Alarm #####
    //
  }

  void SingleRadioAlert::switchDeviceOff( void )
  {
    //
    // Gerät ausschalten, mit Timeout
    //
    // markiere ausschalten
    masterDeviceStat = deviceStatus::DEVICE_IS_SWITCH_OFF;
    auto conn = std::make_shared< QMetaObject::Connection >();
    *conn = connect( alertCommand.get(), &AsyncAlertCommand::sigDeviceIsPoweredOFF, [=]() {
      // Antwort das das Gerät AUS ist
      disconnect( *conn );
      disconnect( &( this->waitForTimer ), 0, 0, 0 );
      this->waitForTimer.stop();
      this->isActive = false;
      // markiere FERTICH
      this->masterDeviceStat = deviceStatus::ALERT_IS_ENDET;
    } );
    //
    // Timeout einschalten
    //
    waitForTimer.setSingleShot( true );
    connect( &waitForTimer, &QTimer::timeout, [=]() {
      this->cancelAlert( QString( "timeout while waiting for switch off device alert %1" ).arg( localAlertConfig.getAlertName() ) );
    } );
    //
    // um den Erfolg abzuwenden müsste man den timer stoppen
    waitForTimer.start( RESPONSETIMEOUT );
    //
    // Auftrag starten
    //
    alertCommand->switchPowerOff( masterDevice.get() );
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
            QString( "SingleRadioAlert::checkIfDevicesAvailible: device %1 was not discovered in the network..." ).arg( *alDevice ) );
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
    LGDEBUG( "SingleRadioAlert::checkIfDevicesAvailible: check if devices availible...OK" );
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
      connectWsTrysCount = 0;  // zurücksetzten der Fehlversuche...
      LGDEBUG( "SingleRadioAlert::connectCallbacksforDevice: device callback estabished..." );
    } );
    // geht Verbindung verloren, neu versuchen
    connect( masterDevice.get(), &BoseDevice::sigOnWSDisConnected, [=]() {
      callBackWsConnected = false;
      LGDEBUG( "SingleRadioAlert::connectCallbacksforDevice: device callback connection lost..." );
      // Signale wieder trennen (ergänzen, wenn mehr verbunden werden)
      this->disconnectCallbacksforDevice();
      // neu verbinden!
      this->connectCallbacksforDevice();
    } );
    // Lautstärke Callback
    connect( masterDevice.get(), &BoseDevice::sigOnVolumeUpdated, [=]( SharedResponsePtr respObj ) {
      WsVolumeUpdated *volObj = static_cast< WsVolumeUpdated * >( respObj.get() );
      currentVolume = volObj->getActualVolume();
      if ( oldVolume == -1 )
      {
        oldVolume = currentVolume;
        isUserVolumeAction = false;
      }
      if ( currentVolume != sendVolume )
      {
        isUserVolumeAction = true;
        disconnect( &( this->waitForTimer ), 0, 0, 0 );
        LGINFO( "user has changed the volume, not more touching volume via this alert..." );
        LGDEBUG( QString( "current volume <%1>, sendVolume <%2>..." ).arg( currentVolume ).arg( sendVolume ) );
        this->waitForTimer.stop();
        this->disconnectCallbacksforDevice();
      }
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

  /**
   * @brief SingleRadioAlert::disconnectCallbacksforDevice
   */
  void SingleRadioAlert::disconnectCallbacksforDevice( void )
  {
    // Signale wieder trennen (ergänzen, wenn mehr verbunden werden)
    disconnect( masterDevice.get(), &BoseDevice::sigOnWSConnected, 0, 0 );
    disconnect( masterDevice.get(), &BoseDevice::sigOnWSDisConnected, 0, 0 );
    disconnect( masterDevice.get(), &BoseDevice::sigOnVolumeUpdated, 0, 0 );
    disconnect( masterDevice.get(), &BoseDevice::sigOnPresetSelectionUpdated, 0, 0 );
    disconnect( masterDevice.get(), &BoseDevice::sigOnErrorUpdated, 0, 0 );
    disconnect( masterDevice.get(), &BoseDevice::sigOnNowPlayingUpdated, 0, 0 );
  }

  void SingleRadioAlert::slotOnRequestAnswer( SharedResponsePtr response )
  {
    //
    LGDEBUG( "SingleRadioAlert::slotOnRequestAnswer..." );
    switch ( response->getResultType() )
    {
      default:
        LGWARN( QString( "SingleRadioAlert::slotOnRequestAnswer: %1" ).arg( static_cast< int >( response->getResultType() ) ) );
        break;

      case ResultobjectType::R_OK:
        LGDEBUG( "SingleRadioAlert::slotOnRequestAnswer: status" );
        computeStausMsg( response );
        break;

      case ResultobjectType::R_VOLUME:
        LGDEBUG( "SingleRadioAlert::slotOnRequestAnswer: volume" );
        computeVolumeMsg( response );
        break;

      case ResultobjectType::R_NOW_PLAYING:
        LGDEBUG( "SingleRadioAlert::slotOnRequestAnswer: now playing" );
        computeNowPlayingMsg( response );
        break;
    }
  }

  void SingleRadioAlert::slotOnNowPlayingUpdate( SharedResponsePtr /*respObj*/ )
  {
    // WsNowPlayingUpdate *nowPlayObj = static_cast< WsNowPlayingUpdate * >( respObj.get() );
    //
    // Abspielstatus?
    // TODO: wird umgeschaltet, alarm beenden
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
  }

}  // namespace radioalert
