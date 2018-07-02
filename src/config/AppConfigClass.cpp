#include <QCoreApplication>
#include <QDebug>
#include <QSettings>

#include "AppConfigClass.hpp"

namespace radioalert
{
  const QString AppConfigClass::constLogGroupName = "logger";
  const QString AppConfigClass::constLogFileKey = "logFileName";
  const QString AppConfigClass::constLogToConsoleKey = "logToConsole";
  const QString AppConfigClass::constNoData = "-";
  const QString AppConfigClass::constAppGroupName = "application";
  const QString AppConfigClass::constAppThresholdKey = "threshold";
  const LoggingThreshold AppConfigClass::defaultAppThreshold = LoggingThreshold::LG_WARNING;

  /**
   * @brief LoggerClass::LoggerClass Der Konstruktor mit Name der Konfigdatei im Programmverzeichnis
   * @param cfg
   */
  AppConfigClass::AppConfigClass( void ) : configFile( QCoreApplication::applicationName() + ".ini" ), logToConsole( true )
  {
  }

  AppConfigClass::AppConfigClass( const QString &configFileName ) : configFile( configFileName ), logToConsole( true )
  {
  }

  AppConfigClass::~AppConfigClass( void )
  {
    qDebug().noquote() << "...";
    saveSettings();
  }

  /**
   * @brief AppConfigClass::getConfigFile Gibt das konfigurierte CONFIG File zurück (aus loadSettings)
   * @return
   */
  QString AppConfigClass::getConfigFile( void ) const
  {
    return ( configFile );
  }

  /**
   * @brief LoggerClass::loadSettings Lade Einstellungen aus der Datei
   * @return
   */
  bool AppConfigClass::loadSettings( void )
  {
    qDebug().noquote() << "CONFIG: <" + configFile + ">";
    QSettings settings( configFile, QSettings::IniFormat );
    if ( !loadLogSettings( settings ) )
    {
      makeDefaultLogSettings( settings );
    }
    if ( !loadAppSettings( settings ) )
    {
      makeAppDefaultSettings( settings );
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
    configFile = cFile;
    QSettings settings( configFile, QSettings::IniFormat );
    if ( !loadLogSettings( settings ) )
    {
      makeDefaultLogSettings( settings );
    }
    if ( !loadAppSettings( settings ) )
    {
      makeAppDefaultSettings( settings );
    }
    return ( true );
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
    if ( !saveLogSettings( settings ) )
    {
      retVal = false;
    }
    if ( !saveAppSettings( settings ) )
    {
      retVal = false;
    }
    return ( retVal );
  }

  /*###########################################################################
   ############################################################################
   #### Logger Einstellungen                                               ####
   ############################################################################
  ###########################################################################*/

  /**
   * @brief AppConfigClass::getLogfileName Gib den Namen der LOGDATEI zurück
   * @return
   */
  QString AppConfigClass::getLogfileName( void ) const
  {
    return logfileName;
  }

  /**
   * @brief AppConfigClass::setLogfileName Setze den Dateinamen des LOGFILES
   * @param value
   */
  void AppConfigClass::setLogfileName( const QString &value )
  {
    logfileName = value;
  }

  /**
   * @brief AppConfigClass::loadLogSettings Private Funktion zu m laden der Logeinstelungen
   * @param settings
   * @return
   */
  bool AppConfigClass::loadLogSettings( QSettings &settings )
  {
    bool retval = true;
    qDebug().noquote() << "";
    //
    // Öffne die Gruppe Logeinstellungen als allererstes
    //
    settings.beginGroup( constLogGroupName );
    //
    // Lese den Dateinamen aus
    //
    logfileName = settings.value( constLogFileKey, AppConfigClass::constNoData ).toString();
    if ( QString::compare( logfileName, constNoData ) == 0 )
    {
      // Nicht gefunden -> Fehler markieren
      retval = false;
    }
    qDebug().noquote().nospace() << "Logfile: <" + logfileName + ">";
    //
    // Lese die Loggerstufe
    //
    logThreshold = LoggingUtils::thresholdNames.value( settings.value( constAppThresholdKey, "warning" ).toString().toLower() );
    qDebug().noquote().nospace() << "logThreshold: <" << LoggingUtils::thresholdNames.key( logThreshold ) << ">";
    //
    // loggen zur Konsole
    //
    logToConsole = settings.value( constLogToConsoleKey, "true" ).toBool();
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
  void AppConfigClass::makeDefaultLogSettings( QSettings &settings )
  {
    qDebug().noquote() << "AppConfigClass::makeDefaultLogSettings()";
    //
    // Öffne die Gruppe Logeinstellungen
    //
    settings.beginGroup( constLogGroupName );
    //
    // defaultwerte setzten
    //
    logfileName = QCoreApplication::applicationName() + ".log";
    settings.setValue( constLogFileKey, logfileName );
    settings.setValue( constLogToConsoleKey, true );
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
  bool AppConfigClass::saveLogSettings( QSettings &settings )
  {
    qDebug().noquote().nospace() << "save to: <" + configFile + ">";
    //
    // Öffne die Gruppe Logeinstellungen
    //
    settings.beginGroup( constLogGroupName );
    //
    settings.setValue( constLogFileKey, logfileName );
    settings.setValue( constLogToConsoleKey, logToConsole );
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

  bool AppConfigClass::loadAppSettings( QSettings &settings )
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

  void AppConfigClass::makeAppDefaultSettings( QSettings &settings )
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
    settings.setValue( constAppThresholdKey, LoggingUtils::thresholdNames.key( logThreshold ) );
    //
    // Ende der Gruppe
    //
    settings.endGroup();
  }

  bool AppConfigClass::saveAppSettings( QSettings &settings )
  {
    //
    // Öffne die Gruppe Logeinstellungen
    //
    settings.beginGroup( constAppGroupName );
    //
    qDebug().noquote().nospace() << "threshold: <" << LoggingUtils::thresholdNames.key( logThreshold ) << ">";
    settings.setValue( constAppThresholdKey, LoggingUtils::thresholdNames.key( logThreshold ) );
    //
    // Ende der Gruppe
    //
    settings.endGroup();
    return ( true );
  }

}  // namespace radioalert
