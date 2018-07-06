#include "radioalertthread.hpp"

qint16 radioalert::RadioAlertThread::threadCounter = 0;

namespace radioalert
{
  RadioAlertThread::RadioAlertThread( std::shared_ptr< Logger > logger, SingleAlertConfig &alert, QObject *parent )
      : QThread( parent ), lg( logger ), localAlertConfig( alert ), alertTimer( this )
  {
    alertTimer.setInterval( 1000 );
    threadCounter++;
  }

  RadioAlertThread::~RadioAlertThread()
  {
    threadCounter--;
  }

  void RadioAlertThread::run( void )
  {
    lg->debug( QString( "RadioAlertThread::run: thread %1 is starting..." ).arg( threadCounter, 3, 10, QChar( '0' ) ) );
    connect( &alertTimer, &QTimer::timeout, this, &RadioAlertThread::slotOnTimer );
    exec();
    emit sigAlertFinished( this );
    lg->debug( QString( "RadioAlertThread::run: thread %1 is ending..." ).arg( threadCounter, 3, 10, QChar( '0' ) ) );
  }

  void RadioAlertThread::startTimer( int interval )
  {
    alertTimer.start( interval );
  }

  void RadioAlertThread::slotOnTimer( void )
  {
    lg->debug( QString( "RadioAlertThread::slotOnTimer: thread: %1 counter: %2" )
                   .arg( threadCounter, 3, 10, QChar( '0' ) )
                   .arg( timerCounter, 3, 10, QChar( '0' ) ) );
    if ( --timerCounter < 1 )
      this->quit();
  }

  void RadioAlertThread::cancelThread( void )
  {
    // den Thread abwürgen...
    lg->info( "RadioAlertThread::cancelThread..." );
    this->quit();
  }

}  // namespace radioalert
