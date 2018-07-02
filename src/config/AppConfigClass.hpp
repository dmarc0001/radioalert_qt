#ifndef SRC_CONFIG_APPCONFIGCLASS_HPP
#define SRC_CONFIG_APPCONFIGCLASS_HPP

#include <QHash>
#include <QSettings>
#include <QString>
#include <QVector>
#include "../logging/loggingthreshold.hpp"

namespace radioalert
{
  class AppConfigClass
  {
    private:
    static const QString constLogGroupName;             //! Gruppenname Logeinstellungen
    static const QString constLogFileKey;               //! Einstellung für Logdatei
    static const QString constLogToConsoleKey;          //! Einstellung für loggen zur konsole
    static const QString constNoData;                   //! Kennzeichner für keine Daten
    static const QString constAppGroupName;             //! Gruppenname für App Einstellungen
    static const QString constAppTimeoutKey;            //! Einstellung für Watchdog
    static const QString constAppThresholdKey;          //! Einstellung für Logebene
    static const LoggingThreshold defaultAppThreshold;  //! defaultwert für Loggingebene
    // ab hier die Konfiguration lagern
    QString configFile;             //! wie nennt sich die Konfigurationsdatei
    QString logfileName;            //! Wie heisst das Logfile
    LoggingThreshold logThreshold;  //! welche Loggerstufe hat die App
    bool logToConsole;              //! Einstellung ob logging zur Konsole

    public:
    AppConfigClass( void );                           //! Konstruktor
    AppConfigClass( const QString &configFileName );  //! Konstruktor
    virtual ~AppConfigClass();                        //! Destruktor
    bool loadSettings( void );                        //! lade Einstellungen aus default Konfigdatei
    bool loadSettings( QString &configFile );         //! lade Einstellungen aus benannter Konfigdatei
    bool saveSettings( void );                        //! sichere Einstellungen
    QString getConfigFile( void ) const;              //! Name der Konfigdatei ausgeben
    QString getLogfileName( void ) const;             //! Name der Logdatei ausgeben
    void setLogfileName( const QString &value );      //! Name der Logdatei setzten
    void setLogThreshold( LoggingThreshold th );      //! setzte Loggingstufe in Config
    LoggingThreshold getLogTreshold( void );          //! hole Loggingstufe aus config
    bool getLogToConsole() const;                     //! setze den Wert ob zu Konsole geloggt werden darf
    void setLogToConsole( bool value );               //! lese den Wert ob zu Konsole geloggt werden darf

    private:
    // Logeinstellungen
    bool loadLogSettings( QSettings &settings );
    void makeDefaultLogSettings( QSettings &settings );
    bool saveLogSettings( QSettings &settings );
    // allg. Programmeinstellungen
    bool loadAppSettings( QSettings &settings );
    void makeAppDefaultSettings( QSettings &settings );
    bool saveAppSettings( QSettings &settings );
  };
}  // namespace radioalert
#endif  // LOGGERCLASS_HPP
