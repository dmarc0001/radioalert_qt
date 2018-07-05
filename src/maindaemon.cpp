#include "maindaemon.hpp"
#include <QDateTime>

// using bose_soundtoch_lib;

namespace radioalert
{
  const QString MainDaemon::version = QString( "%1.%2.%3" ).arg( VMAJOR ).arg( VMINOR ).arg( VPATCH );

  MainDaemon::MainDaemon( QString const &_configFile, bool _isOverrideDebug, QObject *parent )
      : QObject( parent )
      , configFile( _configFile )
      , configFileInfo( configFile )
      , isDebugOverride( _isOverrideDebug )
      , zyclon( this )
      , configZyclon( this )
      , appConfig( std::shared_ptr< AppConfigClass >( new AppConfigClass( _configFile ) ) )
  {
    if ( !configFileInfo.exists() )
    {
      throw ConfigfileNotExistException();
    }
    appConfig->loadSettings();
    // kein Caching, sonst merkt er nicht wenn es veränderungen gibt
    configFileInfo.setCaching( false );
    lastModifiedConfig = configFileInfo.lastModified();
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

  void MainDaemon::reReadConfigFromFile( void )
  {
    lg->debug( "MainDaemon::reReadConfigFromFile..." );
    //
    // Konfiguration neu laden
    //
    if ( !configFileInfo.exists() )
    {
      throw ConfigfileNotExistException();
    }
    appConfig->loadSettings();
    lastModifiedConfig = configFileInfo.lastModified();
    if ( isDebugOverride )
      ( appConfig->getGlobalConfig() ).setLoglevel( LoggingThreshold::LG_DEBUG );
    //
    // Konfigobjekt ist als Zeiger im Logger vorhanden,
    // also reicht ein Neustart des Loggers
    //
    lg->shutdown();
    lg->startLogging();
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
    connect( &configZyclon, &QTimer::timeout, this, &MainDaemon::slotConfigZyclonTimer );
    //
    // der startet verzögert
    //
    zyclon.setInterval( 800 );
    configZyclon.setInterval( 1600 );
    //
    // Timer intervall hart kodiert in SEKUNDEN
    //
    zyclon.start( mainTimerDelay );
    configZyclon.start( checkConfigTime );
  }

  /**
   * @brief MainDaemon::slotZyclonTimer zyklische Abfrage der Alarme
   */
  void MainDaemon::slotZyclonTimer( void )
  {
    static qint32 loopcounter = 0;
    qint16 timeDiff = 0;
    //
    lg->debug( QString( "zycon loop...<%1>" ).arg( loopcounter++ ) );
    //
    // lies die Timer und stelle fest ob ein Alarm fällig ist
    // kopie der Liste machen
    //
    RadioAlertList alertList = appConfig->getAlertList();
    //
    // jetzt feststellen, ob ein Alarm bevorsteht
    //
    RadioAlertList::Iterator ali;
    for ( ali = alertList.begin(); ali != alertList.end(); ++ali )
    {
      // Voreinstellung
      timeDiff = 0;
      // ist er Alarm an?
      if ( !ali->getAlertEnable() )
      {
        // Alarm aus, ignorieren
        continue;
      }
      // ist ein datum gesetzt, und ist es heute?
      if ( ali->getAlertDate().isValid() && isAlertDateToday( ali->getAlertDate() ) )
      {
        // es ist heute! Teste ob die Zeit hinkommt
        timeDiff = howFarIsAlert( ali->getAlertTime() );
      }
      else if ( ali->getAlertDate().isValid() )
      {
        // Datum Gültig, aber nicht heute, ignorieren
        continue;
      }
      else
      {
        // Datum nicht angegeben, also schau mal nach der zeit
        timeDiff = howFarIsAlert( ali->getAlertTime() );
      }
      //
      // ist der alarm nicht länger als
      // timeDiffToPast in die Vergangenheit und
      // timeDiffToFuture in die Zukunft
      //
      if ( timeDiffToPast < timeDiff && timeDiff < timeDiffToFuture )
      {
        // TODO: sperre config-reload, starte alarm, gebe config frei
      }
      else
      {
        // ist der Alarm ausserhalb des Fensters, lösche wenigstens den Marker
        // für "in Arbeit"
        ali->setAlertIsBusy( false );
      }
    }
    // emit close();
  }

  bool MainDaemon::isAlertDateToday( QDate aData )
  {
    if ( aData.isValid() && aData == QDate::currentDate() )
    {
      return ( true );
    }
    return ( false );
  }

  qint16 MainDaemon::howFarIsAlert( QTime aTime )
  {
    //
    // wie viele Sekunden bis zum Alarm
    // negativ heisst in der Vergangenheit
    // 86400 sekunden hat ein Tag, Ergebnis liegt zwischen -86400  und  86400
    //
    if ( aTime.isValid() )
    {
      if ( aTime == QTime::currentTime() )
      {
        // gerade JETZT!
        return ( 1 );
      }
      return ( QTime::currentTime().secsTo( aTime ) );
    }
    return ( 0 );
  }

  void MainDaemon::slotConfigZyclonTimer( void )
  {
    lg->debug( "MainDaemon::slotConfigZyclonTimer: check if config changes..." );
    //
    // Konfiguration neu laden
    //
    if ( !configFileInfo.exists() )
    {
      throw ConfigfileNotExistException();
    }
    //
    // Änderung nach dem letzten Einlesen in der Konfiguration?
    //
    if ( appConfig->isConfigChanged() )
    {
      lg->info( "configuration was changed, write to file...(TODO)" );
      return;
    }

    QDateTime currentModificationTime = configFileInfo.lastModified();
    if ( currentModificationTime != lastModifiedConfig )
    {
      lg->debug(
          QString( "MainDaemon::slotConfigZyclonTimer: current: %1" ).arg( configFileInfo.lastModified().toString( "hh:mm:ss" ) ) );
      lg->info( "config has changed, re-read config file..." );
      //
      // TODO: checksumme prüfen
      //
      appConfig->loadSettings();
      if ( isDebugOverride )
        ( appConfig->getGlobalConfig() ).setLoglevel( LoggingThreshold::LG_DEBUG );
      //
      // Konfigobjekt ist als Zeiger im Logger vorhanden,
      // also reicht ein Neustart des Loggers
      //
      lg->shutdown();
      lg->startLogging();
      lastModifiedConfig = configFileInfo.lastModified();
      lg->info( "config has changed, re-read config file...done" );
    }
    lg->debug( "MainDaemon::slotConfigZyclonTimer: check if config changes...OK" );
  }

}  // namespace radioalert
