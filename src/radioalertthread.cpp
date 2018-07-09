#include "radioalertthread.hpp"

qint16 radioalert::RadioAlertThread::threadCounter = 0;
qint16 radioalert::RadioAlertThread::threadNumbers = 0;

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
    threadCounter++;
    threadNumber = threadNumbers++;
  }

  /**
   * @brief RadioAlertThread::~RadioAlertThread
   */
  RadioAlertThread::~RadioAlertThread()
  {
    threadCounter--;
  }

  /**
   * @brief RadioAlertThread::run
   */
  void RadioAlertThread::run( void )
  {
    LGDEBUG( QString( "RadioAlertThread::run: thread %1 (total: %2) is starting..." )
                 .arg( threadNumber, 3, QChar( '0' ) )
                 .arg( threadCounter, 3, 10, QChar( '0' ) ) );
    LGINFO( QString( "start radio alert %1 at %2" )
                .arg( localAlertConfig.getAlertName() )
                .arg( localAlertConfig.getAlertDate().toString( "hh:mm" ) ) );
    connect( &alertTimer, &QTimer::timeout, this, &RadioAlertThread::slotOnTimer );
    //
    // Voraussetzungen prüfen
    //

    //
    // Geräte verbinden
    //

    exec();
    LGINFO( QString( "radio alert %1 finished." ).arg( localAlertConfig.getAlertName() ) );
    emit sigAlertFinished( this );
    LGDEBUG( QString( "RadioAlertThread::run: thread %1 is ending..." ).arg( threadNumber, 3, 10, QChar( '0' ) ) );
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
    alertTimer.start( interval );
  }

  /**
   * @brief RadioAlertThread::slotOnTimer
   */
  void RadioAlertThread::slotOnTimer( void )
  {
    LGDEBUG( QString( "RadioAlertThread::slotOnTimer: thread: %1 counter: %2" )
                 .arg( threadNumber, 3, 10, QChar( '0' ) )
                 .arg( timerCounter, 3, 10, QChar( '0' ) ) );
    if ( --timerCounter < 1 )
      this->quit();
  }

  /**
   * @brief RadioAlertThread::cancelThread
   */
  void RadioAlertThread::cancelThread( void )
  {
    // den Thread abwürgen...
    LGINFO( QString( "RadioAlertThread::cancelThread %1..." ).arg( threadNumber, 2, 10, QChar( '0' ) ) );
    this->quit();
  }

}  // namespace radioalert
