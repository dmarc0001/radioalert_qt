#include "singleradioalert.hpp"

namespace radioalert
{
  using namespace bose_soundtoch_lib;

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
    waitForTimer.setInterval( 400 );
  }

  SingleRadioAlert::~SingleRadioAlert()
  {
    LGDEBUG( "SingleRadioAlert::~SingleRadioAlert..." );
    disconnect( masterDevice.get(), 0, 0, 0 );
    // emit sigAlertFinished( this );
  }

  /**
   * @brief SingleRadioAlert::start
   */
  void SingleRadioAlert::start( void )
  {
    lastError.clear();
    // das Master device ist dann das erst in der liste
    masterDeviceName = *( realDevices.keyBegin() );
    SoundTouchDeviceData masterDeviceData( realDevices.value( masterDeviceName ) );
    LGDEBUG( "SingleRadioAlert::SingleRadioAlert: create BSoundTouchDevice..." );
    masterDevice = std::unique_ptr< BoseDevice >(
        new BoseDevice( masterDeviceData.hostName, masterDeviceData.wsPort, masterDeviceData.httpPort ) );
    connect( masterDevice.get(), &BoseDevice::sigOnRequestAnswer, this, &SingleRadioAlert::slotOnRequestAnswer );
    LGDEBUG( "SingleRadioAlert::SingleRadioAlert: create BSoundTouchDevice...OK" );
    //
    // 01:
    // anfrage senden: ist das master device frei (also im standby)?
    //
    if ( !checkIfDeviceIsInStandby( masterDevice.get() ) )
    {
      throw NoAvailibleSoundDeviceException( QLatin1String( "master device is not available for alert (busy)!" ) );
    }
    // wie lange geht der Spass?
    alertDuration = localAlertConfig.getAlertDuration();
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
   * @brief SingleRadioAlert::checkIfDeviceIsInStandby
   * @param device
   * @return
   */
  bool SingleRadioAlert::checkIfDeviceIsInStandby( BoseDevice *device )
  {
    LGDEBUG( "RadioAlertThread::checkIfDeviceIsInStandby: get now playing..." );
    device->getNowPlaying();
    LGDEBUG( "RadioAlertThread::checkIfDeviceIsInStandby: get now playing...OK" );
    //
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

    /*
    LGDEBUG( "RadioAlertThread::checkIfDeviceIsInStandby: select medium..." );
    if ( localAlertConfig.getAlertSource().startsWith( PRESETPATTERN ) )
    {
      //
      // momentan nur PRESET für den Wecker
      //
    }
    else
    {
      LGWARN( QString( "SingleRadioAlert::connectCallbacksforDevice: alert source % not implemented." )
                  .arg( localAlertConfig.getAlertSource() ) );
      cancelAlert( "not implemented play type yet" );
    }
    */
  }

  /**
   * @brief SingleRadioAlert::slotOnZyclonTimer
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
    if ( isActive && alertDuration-- > 0 )
    {
      //
      // TODO: hier den alarm bearbeiten
      //
    }
    else
    {
      //
      // TODO: Radios abschalten?
      //

      //
      // beenden
      //
      LGDEBUG( "SingleRadioAlert::slotOnZyclonTimer: alert duration reached or deactivated..." );
      disconnect( masterDevice.get(), 0, 0, 0 );
      emit sigAlertFinished( this );
    }
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
    if ( nowPlayObj->getPlayStatus().contains( masterDevice->getPlayStateName( BoseDevice::bose_playstate::BUFFERING_STATE ) ) )
    {
      // Puffern?
      // schritt 03a
      // warten auf PLAY
      masterDeviceStat = deviceStatus::BUFFERING;
    }
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
      }
      else
      {
        // TODO: was sinnvolles machen
      }
    }
    else
    {
      // TODO: was mache ich hier? (PAUSE oder STOP)
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
    LGDEBUG( QString( "SingleRadioAlert::computeVolumeMsg: recived type: VOLUME <%1>" ).arg( volObj->getActualVolume() ) );
  }

  void SingleRadioAlert::computeNowPlayingMsg( SharedResponsePtr response )
  {
    HttpNowPlayingObject *nPlayObj = static_cast< HttpNowPlayingObject * >( response.get() );
    LGDEBUG( QString( "SingleRadioAlert::computeNowPlayingMsg: recived type: SOURCE <%1>" ).arg( nPlayObj->getSource() ) );
    switch ( masterDeviceStat )
    {
      case deviceStatus::NONE:
        // da sollte dann die Meldung kommen, das Gerät ist da und AUS
        switchMasterDeviceToSource( nPlayObj );
        break;

      default:
        LGDEBUG( "SingleRadioAlert::computeNowPlayingMsg: master device status is NOT expected/supported" );
    }
  }

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
