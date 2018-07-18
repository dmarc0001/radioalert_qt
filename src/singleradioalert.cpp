#include "singleradioalert.hpp"
#include "xmlparser/httpresponse/httpnowplayingobject.hpp"
#include "xmlparser/httpresponse/httpvolumeobject.hpp"

namespace radioalert
{
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

  SingleRadioAlert::~SingleRadioAlert()
  {
    LGDEBUG( "SingleRadioAlert::~SingleRadioAlert..." );
    disconnect( masterDevice.get(), &bose_soundtoch_lib::BSoundTouchDevice::sigOnRequestAnswer, this,
                &SingleRadioAlert::slotOnRequestAnswer );
    // emit sigAlertFinished( this );
  }

  /**
   * @brief SingleRadioAlert::start
   */
  void SingleRadioAlert::start( void )
  {
    // das Master device ist dann das erst in der liste
    masterDeviceName = *( realDevices.keyBegin() );
    SoundTouchDeviceData masterDeviceData( realDevices.value( masterDeviceName ) );
    LGDEBUG( "SingleRadioAlert::SingleRadioAlert: create BSoundTouchDevice..." );
    masterDevice = std::unique_ptr< bose_soundtoch_lib::BSoundTouchDevice >(
        new bose_soundtoch_lib::BSoundTouchDevice( masterDeviceData.hostName, masterDeviceData.wsPort, masterDeviceData.httpPort ) );
    connect( masterDevice.get(), &bose_soundtoch_lib::BSoundTouchDevice::sigOnRequestAnswer, this,
             &SingleRadioAlert::slotOnRequestAnswer );
    LGDEBUG( "SingleRadioAlert::SingleRadioAlert: create BSoundTouchDevice...OK" );
    //
    // anfrage senden: ist das master device frei (also im standby)?
    //
    if ( !checkIfDeviceIsInStandby( masterDevice.get() ) )
    {
      throw NoAvailibleSoundDeviceException( QLatin1String( "master device is not available for alert (busy)!" ) );
    }
    // wiel lange geht der Spass?
    alertDuration = localAlertConfig.getAlertDuration();
    //
    // jetzt warten wir auf now playing == "STANDBY"
    // wenn das nciht innerhalb der TIMEOUT zeit passiert oder das Gerät
    // etwas abspielt, vergessen wir das...
    //
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
    isActive = false;
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

  bool SingleRadioAlert::checkIfDeviceIsInStandby( bose_soundtoch_lib::BSoundTouchDevice *device )
  {
    LGDEBUG( "RadioAlertThread::checkIfDeviceIsInStandby: get now playing..." );
    device->getNowPlaying();
    LGDEBUG( "RadioAlertThread::checkIfDeviceIsInStandby: get now playing...OK" );
    //
    return ( true );
  }

  /**
   * @brief SingleRadioAlert::slotOnZyclonTimer
   */
  void SingleRadioAlert::slotOnZyclonTimer( void )
  {
    // TODO: Überwachung des Ablaufes...
    alertLoopCounter++;
    LGINFO( QString( "SingleRadioAlert::slotOnZyclonTimer: alert %1 loop <%2>" )
                .arg( localAlertConfig.getAlertName() )
                .arg( alertLoopCounter, 8, 10, QChar( '0' ) ) );
    //
    // läuft der Alarm noch?
    //
    if ( isActive && alertDuration-- > 0 )
    {
      //
      // ist das gerät bereit?
      //
      if ( masterDeviceStat == deviceStatus::NONE )
      {
        if ( alertLoopCounter > RESPONSETIMEOUT )
        {
          LGWARN( "SingleRadioAlert::slotOnZyclonTimer: timeout while wait for response from master device. CANCEL Alert." );
          cancelAlert( "timeout while wait for response from master device" );
        }
        //
        // Radio ist noch nicht soweit, dann ist hier erst mal schluss
        //
        return;
      }
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
      disconnect( masterDevice.get(), &bose_soundtoch_lib::BSoundTouchDevice::sigOnRequestAnswer, this,
                  &SingleRadioAlert::slotOnRequestAnswer );
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
        break;

      case bose_soundtoch_lib::ResultobjectType::R_VOLUME:
        LGDEBUG( "RadioAlertThread::slotOnRequestAnswer: volume" );
        computeVolumeMsg( response );
        break;

      case bose_soundtoch_lib::ResultobjectType::R_NOW_PLAYING:
        LGDEBUG( "RadioAlertThread::slotOnRequestAnswer: now playing" );
        computeNowPlayingMsg( response );
        break;
    }
  }

  void SingleRadioAlert::computeVolumeMsg( SharedResponsePtr response )
  {
    bose_soundtoch_lib::HttpVolumeObject *volObj = static_cast< bose_soundtoch_lib::HttpVolumeObject * >( response.get() );
    LGDEBUG( QString( "SingleRadioAlert::computeVolumeMsg: recived type: VOLUME <%1>" ).arg( volObj->getActualVolume() ) );
  }

  void SingleRadioAlert::computeNowPlayingMsg( SharedResponsePtr response )
  {
    bose_soundtoch_lib::HttpNowPlayingObject *nPlayObj = static_cast< bose_soundtoch_lib::HttpNowPlayingObject * >( response.get() );
    LGDEBUG( QString( "SingleRadioAlert::computeNowPlayingMsg: recived type: SOURCE <%1>" ).arg( nPlayObj->getSource() ) );
    if ( masterDeviceStat == deviceStatus::NONE )
    {
      if ( ( nPlayObj->getSource() ).compare( QLatin1String( "STANDBY" ) ) == 0 )
      {
        LGDEBUG( "SingleRadioAlert::computeNowPlayingMsg: set master device status to STANDBY" );
        masterDeviceStat = deviceStatus::STANDBY;
      }
      else
      {
        LGWARN( "SingleRadioAlert::computeNowPlayingMsg: NOT STANDBY == alert ignore!" );
        cancelAlert( "device is NOT in STANDBY == alert ignore!" );
      }
    }
  }
}  // namespace radioalert
