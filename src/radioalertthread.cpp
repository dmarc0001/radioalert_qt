#include "radioalertthread.hpp"
#include "xmlparser/httpresponse/httpvolumeobject.hpp"

namespace radioalert
{
  /**
   * @brief RadioAlertThread::RadioAlertThread
   * @param logger
   * @param alert
   * @param parent
   */
  RadioAlertThread::RadioAlertThread( std::shared_ptr< Logger > logger,
                                      SingleAlertConfig &alert,
                                      StDevicesHashList &devices,
                                      QObject *parent )
      : QThread( parent ), lg( logger ), localAlertConfig( alert ), avStDevices( devices ), alertTimer( this )
  {
    alertTimer.setInterval( 1000 );
  }

  /**
   * @brief RadioAlertThread::~RadioAlertThread
   */
  RadioAlertThread::~RadioAlertThread()
  {
  }

  /**
   * @brief RadioAlertThread::run
   */
  void RadioAlertThread::run( void )
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
    // das Master device ist dann das erst in der liste
    masterDeviceName = *( realDevices.keyBegin() );
    SoundTouchDeviceData masterDeviceData( realDevices.value( masterDeviceName ) );
    LGDEBUG( "RadioAlertThread::run: create BSoundTouchDevice..." );
    masterDevice = std::unique_ptr< bose_soundtoch_lib::BSoundTouchDevice >(
        new bose_soundtoch_lib::BSoundTouchDevice( masterDeviceData.hostName, masterDeviceData.wsPort, masterDeviceData.httpPort ) );
    connect( masterDevice.get(), &bose_soundtoch_lib::BSoundTouchDevice::sigOnRequestAnswer, this,
             &RadioAlertThread::slotOnRequestAnswer );
    LGDEBUG( "RadioAlertThread::run: create BSoundTouchDevice...OK" );
    //
    // ist das master device frei (also im standby)?
    //
    if ( !checkIfDeviceIsInStandby( masterDevice.get() ) )
    {
      throw NoAvailibleSoundDeviceException( QLatin1String( "master device is not available for alert (busy)!" ) );
    }

    //
    // Gerät(e) verbinden
    //

    //
    // verbinde den Timer für zyklische Kontrolle
    //
    connect( &alertTimer, &QTimer::timeout, this, &RadioAlertThread::slotOnTimer );
    exec();
    LGINFO( QString( "radio alert %1 finished." ).arg( localAlertConfig.getAlertName() ) );
    emit sigAlertFinished( this );
    LGDEBUG( QString( "RadioAlertThread::run: thread %1 is ending..." ).arg( localAlertConfig.getAlertName() ) );
  }

  /**
   * @brief RadioAlertThread::getAlertName
   * @return
   */
  QString RadioAlertThread::getAlertName( void ) const
  {
    return ( localAlertConfig.getAlertName() );
  }

  /**
   * @brief RadioAlertThread::startTimer
   * @param interval
   */
  void RadioAlertThread::startTimer( int interval )
  {
    //
    // der Timer muss vom aufrufenden Thread gestartet werden :-(
    //
    LGDEBUG( QLatin1String( "RadioAlertThread::startTimer..." ) );
    alertTimer.start( interval );
  }

  /**
   * @brief RadioAlertThread::slotOnTimer
   */
  void RadioAlertThread::slotOnTimer( void )
  {
    LGDEBUG( QString( "RadioAlertThread::slotOnTimer: thread: %1 counter: %2" )
                 .arg( localAlertConfig.getAlertName() )
                 .arg( timerCounter, 3, 10, QChar( '0' ) ) );
    if ( --timerCounter < 1 )
      this->quit();
  }

  void RadioAlertThread::slotOnRequestAnswer( std::shared_ptr< bose_soundtoch_lib::IResponseObject > response )
  {
    switch ( response->getResultType() )
    {
      default:
        break;

      case bose_soundtoch_lib::ResultobjectType::R_VOLUME:
        bose_soundtoch_lib::HttpVolumeObject *volObj = static_cast< bose_soundtoch_lib::HttpVolumeObject * >( response.get() );
        LGDEBUG( QString( "RadioAlertThread::slotOnRequestAnswer: recived type: VOLUME <%1>" ).arg( volObj->getActualVolume() ) );
        break;
    }
  }

  /**
   * @brief RadioAlertThread::cancelThread
   */
  void RadioAlertThread::cancelThread( void )
  {
    // den Thread abwürgen...
    LGINFO( QString( "RadioAlertThread::cancelThread %1..." ).arg( localAlertConfig.getAlertName() ) );
    this->quit();
  }

  /**
   * @brief RadioAlertThread::checkIfDevicesAvailible
   * @return
   */
  bool RadioAlertThread::checkIfDevicesAvailible( void )
  {
    LGDEBUG( "RadioAlertThread::checkIfDevicesAvailible: check if devices availible..." );
    // Iterator für alarm Devices
    QList< QString >::Iterator alDevice;
    for ( alDevice = localAlertConfig.getAlertDevices().begin(); alDevice != localAlertConfig.getAlertDevices().end(); alDevice++ )
    {
      qDebug() << "RadioAlertThread::checkIfDevicesAvailible: ";
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

  bool RadioAlertThread::checkIfDeviceIsInStandby( bose_soundtoch_lib::BSoundTouchDevice *device )
  {
    device->getNowPlaying();
    //
    return ( true );
  }
}  // namespace radioalert
