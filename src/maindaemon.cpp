#include "maindaemon.hpp"
#include <QDateTime>

// using bose_soundtoch_lib;

namespace radioalert
{
  const QString MainDaemon::version = QString( "%1.%2.%3" ).arg( VMAJOR ).arg( VMINOR ).arg( VPATCH );

  MainDaemon::MainDaemon( QString const &_configFile, bool _isOverrideDebug, QObject *parent )
      : QObject( parent )
      , configFile( _configFile )
      , isDebugOverride( _isOverrideDebug )
      , zyclon( this )
      , appConfig( std::shared_ptr< AppConfigClass >( new AppConfigClass( _configFile ) ) )
      , programTimeoutDebugging( 600 )
  {
    appConfig->loadSettings();
    if ( isDebugOverride )
      ( appConfig->getGlobalConfig() ).setLoglevel( LoggingThreshold::LG_DEBUG );
    lg = std::shared_ptr< Logger >( new Logger( appConfig ) );
    lg->startLogging();
  }

  MainDaemon::~MainDaemon()
  {
    lg->shutdown();
    zyclon.stop();
  }

  void MainDaemon::init( void )
  {
    lg->info( QString( "alert daemon version: %1 started." ).arg( MainDaemon::version ) );
    lg->debug( QString( "override debug: %1" ).arg( isDebugOverride ) );
    lg->debug( QString( "config file: %1" ).arg( configFile ) );
    //
    // hier werden alle Callbacks initialisiert und Vorbereitugnen für den Daemon getroffen
    //
    connect( &zyclon, &QTimer::timeout, this, &MainDaemon::slotZyclonTimer );
    zyclon.setInterval( 500 );
    zyclon.start( 1000 );
  }

  void MainDaemon::slotZyclonTimer( void )
  {
    //
    lg->debug( QString( "TICK...<%1>" ).arg( programTimeoutDebugging ) );
    programTimeoutDebugging--;
    if ( programTimeoutDebugging <= 0 )
      emit close();
  }

}  // namespace radioalert
