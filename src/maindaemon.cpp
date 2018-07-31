#include "maindaemon.hpp"
#include <QDateTime>

// using bose_soundtoch_lib;

namespace radioalert
{
  const QString MainDaemon::version{QString( "%1.%2.%3" ).arg( VMAJOR ).arg( VMINOR ).arg( VPATCH )};

  /**
   * @brief MainDaemon::MainDaemon
   * @param _configFile
   * @param _isOverrideDebug
   * @param parent
   */
  MainDaemon::MainDaemon( QString const &_configFile, bool _isOverrideDebug, QObject *parent )
      : QObject( parent )
      , configFile( _configFile )
      , configFileInfo( _configFile )
      , isDebugOverride( _isOverrideDebug )
      , zyclon( this )
      , configZyclon( this )
      , availDevicesZyclon( this )
      , appConfig( std::shared_ptr< AppConfigClass >( new AppConfigClass( _configFile ) ) )
  {
    if ( !configFileInfo.exists() )
    {
      throw ConfigfileNotExistException( QString( "configfile %1 not exist" ).arg( configFile ) );
    }
    // kein Caching, sonst merkt er nicht wenn es veränderungen gibt
    configFileInfo.setCaching( false );
    appConfig->loadSettings();
    lastModifiedConfig = configFileInfo.lastModified();
    if ( isDebugOverride )
      ( appConfig->getGlobalConfig() ).setLoglevel( LoggingThreshold::LG_DEBUG );
    lg = std::shared_ptr< Logger >( new Logger( appConfig ) );
    lg->startLogging();
  }

  /**
   * @brief MainDaemon::~MainDaemon
   */
  MainDaemon::~MainDaemon()
  {
    lg->shutdown();
    zyclon.stop();
    configZyclon.stop();
    availDevicesZyclon.stop();
  }

  /**
   * @brief MainDaemon::init
   */
  void MainDaemon::init( void )
  {
    LGINFO( QString( "alert daemon version: %1 started." ).arg( MainDaemon::version ) );
    LGDEBUG( QString( "override debug: %1" ).arg( isDebugOverride ) );
    LGDEBUG( QString( "config file: %1" ).arg( configFile ) );
    //
    // initial Geräte einlesen
    //
    readAvailDevices();
    //
    // hier werden alle Callbacks initialisiert und Vorbereitugnen für den Daemon getroffen
    //
    connect( &zyclon, &QTimer::timeout, this, &MainDaemon::slotZyclonTimer );
    connect( &configZyclon, &QTimer::timeout, this, &MainDaemon::slotConfigZyclonTimer );
    connect( &availDevicesZyclon, &QTimer::timeout, this, &MainDaemon::slotavailDevicesZyclonTimer );
    //
    // Timer intervall hart kodiert in SEKUNDEN
    //
    zyclon.start( mainTimerDelay );
    configZyclon.start( checkConfigTime );
    availDevicesZyclon.start( availDevices );
  }

  /**
   * @brief MainDaemon::reReadConfigFromFile
   */
  void MainDaemon::reReadConfigFromFile( void )
  {
    LGDEBUG( "MainDaemon::reReadConfigFromFile..." );
    //
    // Konfiguration neu laden
    //
    if ( !configFileInfo.exists() )
    {
      throw ConfigfileNotExistException( QString( "configfile %1 not exist" ).arg( configFile ) );
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

  /**
   * @brief MainDaemon::requestQuit
   */
  void MainDaemon::requestQuit( void )
  {
    LGINFO( "MainDaemon::requestQuit..." );
    //
    // Alle eventuell vorhandenen Alarme beenden
    //
    SingleRadioAlertList::Iterator alt;
    for ( alt = activeAlerts.begin(); alt != activeAlerts.end(); alt++ )
    {
      LGINFO( "MainDaemon::requestQuit: kill thread..." );
      // signalisiere sein baldiges Ende
      ( *alt )->cancelAlert();
    }
    //
    // Aufräumen dem System/ der runtime überlassen :-)
    //
    LGINFO( "MainDaemon::requestQuit...OK" );
    emit close();
  }

  /**
   * @brief MainDaemon::readAvailDevices
   * @param fileName
   * @return
   */
  bool MainDaemon::readAvailDevices( void )
  {
    AvailableDevices tempDev;
    try
    {
      //
      // versuche einzulesen
      //
      LGDEBUG( "MainDaemon::readAvailDevices" );
      tempDev.loadSettings( appConfig->getGlobalConfig().getDevicesFile() );
      //
      // Objekt kopieren
      //
      avStDevices = tempDev.getDevicesList();
      return ( true );
    }
    catch ( ConfigfileNotExistException ex )
    {
      //
      // Fehlermeldung loggen!
      //
      LGCRIT( QString( "Cant read devices from file: " ).append( ex.getMessage() ) );
      return ( false );
    }
  }

  /**
   * @brief MainDaemon::slotavailDevicesZyclonTimer
   */
  void MainDaemon::slotavailDevicesZyclonTimer( void )
  {
    //
    // leitet den Aufruf nur weiter....
    //
    readAvailDevices();
  }

  /**
   * @brief MainDaemon::slotZyclonTimer zyklische Abfrage der Alarme
   */
  void MainDaemon::slotZyclonTimer( void )
  {
    static qint32 loopcounter = 0;
    qint16 timeDiff = 0;
    //
    //
    // nur jedes zehnte mal reagieren
    //
    if ( ++loopcounter % 10 != 0 )
      return;
    LGDEBUG( QString( "MainDaemon::slotZyclonTimer: <%1>" ).arg( loopcounter, 8, 10, QChar( '0' ) ) );
    //
    // lies die Timer und stelle fest ob ein Alarm fällig ist
    // Referenz auf die Liste holen
    //
    RadioAlertList &alertList = appConfig->getAlertList();
    //
    // Konfiguration sperren bis der Test durch ist
    //
    QMutexLocker locker( appConfig->getLockMutexPtr() );
    //
    // jetzt feststellen, ob ein Alarm bevorsteht
    // iteriere über die values...
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
#ifdef DEBUG
      if ( !ali->getAlertTime().isValid() )
      {
        //
        // kann nicht vorkommen, ausser im debug buils und time == "now"
        //
        if ( ali->getAlertIsBusy() )
        {
          //
          // BESETZT!
          //
          continue;
        }
        //
        // starte den Alarmthread mit einer Kopie des SigleAlertConfig...
        //
        LGINFO( "MainDaemon::slotZyclonTimer: start alert thread (DEBUG MODE)" );
        // besetzt markieren
        ali->setAlertIsBusy( true );
        SingleAlertConfig alrt( *ali );
        //
        // den Alarm auf genau jetzt setzen (minus 8 Sekunden)
        //
        alrt.setAlertTime( QTime::currentTime().addSecs( -8 ) );
        try
        {
          SingleRadioAlert *newAlert = new SingleRadioAlert( lg, alrt, avStDevices, this );
          // in die Liste der Threads
          activeAlerts.append( newAlert );
          // verbinde die Endemeldung des Thread mit dem Slot
          connect( newAlert, &SingleRadioAlert::sigAlertFinished, this, &MainDaemon::slotAlertFinished );
          // Timer hier auch dran binden
          connect( &zyclon, &QTimer::timeout, newAlert, &SingleRadioAlert::slotOnZyclonTimer );
          newAlert->start();
        }
        catch ( NoAvailibleSoundDeviceException ex )
        {
          LGWARN( "no availible devices for this alert!" );
        }
      }
#endif
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
        if ( ali->getAlertIsBusy() )
        {
          //
          // BESETZT!
          //
          continue;
        }
        //
        // starte den Alarmthread mit einer Kopie des SigleAlertConfig...
        //
        LGINFO( "MainDaemon::slotZyclonTimer: start alert thread" );
        // TODO: implementieren
        /*
        ali->setAlertIsBusy( true );
        RadioAlertThread *newAlert = new RadioAlertThread( lg, *ali, avStDevices, this );
        // in die Liste der Threads
        activeThreads.append( newAlert );
        // verbinde die Endemeldung des Thread mit dem Slot
        connect( newAlert, &RadioAlertThread::sigAlertFinished, this, &MainDaemon::slotAlertFinished );
        // und die Selbstzerstörung einleiten, wenn Thread endet
        connect( newAlert, &RadioAlertThread::finished, newAlert, &RadioAlertThread::deleteLater );
        // Thread starten
        newAlert->start();
        newAlert->startTimer( 1000 );
        */
      }
      else
      {
        // ist der Alarm ausserhalb des Fensters, lösche wenigstens den Marker
        // für "in Arbeit"
        // Rückfallebene falls woanders etwas schief ging
        // LGDEBUG( QString( "MainDaemon::slotZyclonTimer: alert %1: set busy :false" ).arg( ali->getAlertName() ) );
        ali->setAlertIsBusy( false );
      }
    }
  }

  /**
   * @brief MainDaemon::isAlertDateToday
   * @param aData
   * @return
   */
  bool MainDaemon::isAlertDateToday( QDate aData )
  {
    if ( aData.isValid() && aData == QDate::currentDate() )
    {
      return ( true );
    }
    return ( false );
  }

  /**
   * @brief MainDaemon::howFarIsAlert
   * @param aTime
   * @return
   */
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

  /**
   * @brief MainDaemon::slotConfigZyclonTimer
   */
  void MainDaemon::slotConfigZyclonTimer( void )
  {
    LGDEBUG( "MainDaemon::slotConfigZyclonTimer: check if config changes..." );
    //
    // Konfiguration neu laden
    //
    if ( !configFileInfo.exists() )
    {
      throw ConfigfileNotExistException( QString( "configfile %1 not exist" ).arg( configFile ) );
    }
    //
    // Änderung nach dem letzten Einlesen in der Konfiguration?
    //
    if ( appConfig->isConfigChanged() )
    {
      LGINFO( "configuration was changed, write to file..." );
      appConfig->saveSettings();
      lastModifiedConfig = configFileInfo.lastModified();
      return;
    }

    QDateTime currentModificationTime = configFileInfo.lastModified();
    if ( currentModificationTime != lastModifiedConfig )
    {
      LGDEBUG(
          QString( "MainDaemon::slotConfigZyclonTimer: current: %1" ).arg( configFileInfo.lastModified().toString( "hh:mm:ss" ) ) );
      LGINFO( "config timestamp has changed, check config file..." );
      if ( appConfig->isConfigFileChanged() )
      {
        //
        // Die Checksumme hat sich geändert, also neu einlesen
        //
        LGINFO( "config has changed, re-read config file..." );
        reReadConfigFromFile();
        LGINFO( "config has changed, re-read config file...done" );
      }
    }
    LGDEBUG( "MainDaemon::slotConfigZyclonTimer: check if config changes...OK" );
  }

  /**
   * @brief MainDaemon::slotAlertFinished
   * @param theTread
   */
  void MainDaemon::slotAlertFinished( SingleRadioAlert *theAlert )
  {
    //
    // der Alarm beendet sich...
    // den alarm wieder freigeben
    //
    if ( appConfig->getAlertList().contains( theAlert->getAlertName() ) )
    {
      //
      // setzte BUSY auf False
      //
      ( appConfig->getAlertList() )[ theAlert->getAlertName() ].setAlertIsBusy( false );
    }
    //
    // aus der Liste entfernen und freigeben!
    //
    theAlert->deleteLater();
    if ( activeAlerts.contains( theAlert ) )
    {
      activeAlerts.removeOne( theAlert );
    }
  }
}  // namespace radioalert
