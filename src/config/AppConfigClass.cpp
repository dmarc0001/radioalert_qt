#include <QCoreApplication>
#include <QDebug>
#include <QSettings>

#include "AppConfigClass.hpp"

namespace radioalert
{
  const QString AppConfigClass::constNoData = "-";
  const QString AppConfigClass::constAppGroupName = "application";

  /**
   * @brief LoggerClass::LoggerClass Der Konstruktor mit Name der Konfigdatei im Programmverzeichnis
   * @param cfg
   */
  AppConfigClass::AppConfigClass( void ) : configFileName( QCoreApplication::applicationName() + ".ini" )
  {
  }

  AppConfigClass::AppConfigClass( const QString &configFileName ) : configFileName( configFileName )
  {
  }

  AppConfigClass::~AppConfigClass( void )
  {
    qDebug().noquote() << "...";
    saveSettings();
  }

  QString AppConfigClass::getLogfileName( void ) const
  {
    return ( configFileName );
  }

  void AppConfigClass::setConfigFileName( const QString &fileName )
  {
    configFileName = fileName;
    loadSettings();
  }

  /**
   * @brief LoggerClass::loadSettings Lade Einstellungen aus der Datei
   * @return
   */
  bool AppConfigClass::loadSettings( void )
  {
    qDebug().noquote() << "CONFIG: <" + configFileName + ">";
    QSettings settings( configFileName, QSettings::IniFormat );
    if ( !globalConfig.loadSettings( settings ) )
      globalConfig.makeDefaultSettings( settings );

    //
    if ( !loadAlertSettings( settings ) )
    {
      makeAlertDefaultSettings( settings );
    }
    return ( true );
  }

  /**
   * @brief AppConfigClass::loadSettings Lade Einstellungen aus CONFIG Datei
   * @param cFile
   * @return
   */
  bool AppConfigClass::loadSettings( QString &cFile )
  {
    qDebug().noquote() << "load settings from" << cFile;
    configFileName = cFile;
    QSettings settings( configFileName, QSettings::IniFormat );
    return loadSettings();
  }

  /**
   * @brief LoggerClass::saveSettings sichere Einstellunge in Datei
   * @return
   */
  bool AppConfigClass::saveSettings( void )
  {
    qDebug().noquote() << "...";
    bool retVal = true;
    qDebug().noquote() << "save to <" + configFile + ">";
    // QSettings settings(configFile, QSettings::NativeFormat);
    QSettings settings( configFile, QSettings::IniFormat );
    //
    // die Logeinstellungen sichern
    //
    if ( !saveGlobalSettings( settings ) )
    {
      retVal = false;
    }
    if ( !saveAlertSettings( settings ) )
    {
      retVal = false;
    }
    return ( retVal );
  }

  /**
   * @brief AppConfigClass::loadLogSettings Private Funktion zu m laden der Logeinstelungen
   * @param settings
   * @return
   */
  bool AppConfigClass::loadGlobalSettings( QSettings &settings )
  {
    bool retval = true;
    qDebug().noquote() << "";
    //
    // Öffne die Gruppe Logeinstellungen als allererstes
    //
    settings.beginGroup( constGlobalGroupName );
    //
    // Lese den Dateinamen aus
    //
    logfileName = settings.value( constGlobalLogFileKey, AppConfigClass::constNoData ).toString();
    if ( QString::compare( logfileName, constNoData ) == 0 )
    {
      // Nicht gefunden -> Fehler markieren
      retval = false;
    }
    qDebug().noquote().nospace() << "Logfile: <" + logfileName + ">";
    //
    // Lese die Loggerstufe
    //
    logThreshold = LoggingUtils::thresholdNames.value( settings.value( constGlobalLogThresholdKey, "warning" ).toString().toLower() );
    qDebug().noquote().nospace() << "logThreshold: <" << LoggingUtils::thresholdNames.key( logThreshold ) << ">";
    //
    // loggen zur Konsole
    //
    logToConsole = settings.value( constGlobalLogToConsoleKey, "true" ).toBool();
    qDebug().noquote().nospace() << "logToConsole: <" << logToConsole << ">";
    //
    // Ende der Gruppe
    //
    settings.endGroup();
    //
    // Ergebnis kommunizieren
    //
    return ( retval );
  }

  /**
   * @brief AppConfigClass::makeDefaultLogSettings Erzeuge VORGABE Einstellungen für LOGGING
   * @param settings
   */
  void AppConfigClass::makeDefaultGlobalSettings( QSettings &settings )
  {
    qDebug().noquote() << "AppConfigClass::makeDefaultLogSettings()";
    //
    // Öffne die Gruppe Logeinstellungen
    //
    settings.beginGroup( constGlobalGroupName );
    //
    // defaultwerte setzten
    //
    logfileName = QCoreApplication::applicationName() + ".log";
    settings.setValue( constGlobalLogFileKey, logfileName );
    settings.setValue( constGlobalLogToConsoleKey, true );
    //
    // Ende der Gruppe
    //
    settings.endGroup();
  }

  /**
   * @brief AppConfigClass::saveLogSettings Sichere Einstellungen
   * @param settings
   * @return
   */
  bool AppConfigClass::saveGlobalSettings( QSettings &settings )
  {
    qDebug().noquote().nospace() << "save to: <" + configFile + ">";
    //
    // Öffne die Gruppe Logeinstellungen
    //
    settings.beginGroup( constGlobalGroupName );
    //
    settings.setValue( constGlobalLogFileKey, logfileName );
    settings.setValue( constGlobalLogToConsoleKey, logToConsole );
    //
    // Ende der Gruppe
    //
    settings.endGroup();
    return ( true );
  }

  bool AppConfigClass::getLogToConsole() const
  {
    return logToConsole;
  }

  void AppConfigClass::setLogToConsole( bool value )
  {
    logToConsole = value;
  }

  /*###########################################################################
   ############################################################################
   #### Application settings                                               ####
   ############################################################################
  ###########################################################################*/

  void AppConfigClass::setLogThreshold( LoggingThreshold th )
  {
    logThreshold = th;
  }

  LoggingThreshold AppConfigClass::getLogTreshold( void )
  {
    return ( logThreshold );
  }

  bool AppConfigClass::loadAlertSettings( QSettings &settings )
  {
    bool retval = true;
    qDebug().noquote() << "";
    //
    // Öffne die Gruppe app
    //
    settings.beginGroup( constAppGroupName );
    //
    // Ende der Gruppe
    //
    settings.endGroup();
    //
    // Ergebnis kommunizieren
    //
    return ( retval );
  }

  void AppConfigClass::makeAlertDefaultSettings( QSettings &settings )
  {
    qDebug().noquote() << "";
    //
    // Öffne die Gruppe App
    //
    settings.beginGroup( constAppGroupName );
    //
    // defaultwerte setzten
    //
    logThreshold = defaultAppThreshold;
    settings.setValue( constGlobalLogThresholdKey, LoggingUtils::thresholdNames.key( logThreshold ) );
    //
    // Ende der Gruppe
    //
    settings.endGroup();
  }

  bool AppConfigClass::saveAlertSettings( QSettings &settings )
  {
    //
    // Öffne die Gruppe Logeinstellungen
    //
    settings.beginGroup( constAppGroupName );
    //
    qDebug().noquote().nospace() << "threshold: <" << LoggingUtils::thresholdNames.key( logThreshold ) << ">";
    settings.setValue( constGlobalLogThresholdKey, LoggingUtils::thresholdNames.key( logThreshold ) );
    //
    // Ende der Gruppe
    //
    settings.endGroup();
    return ( true );
  }

}  // namespace radioalert
