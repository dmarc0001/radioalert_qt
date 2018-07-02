#ifndef SRC_CONFIG_APPCONFIGCLASS_HPP
#define SRC_CONFIG_APPCONFIGCLASS_HPP

#include <QHash>
#include <QSettings>
#include <QString>
#include <QVector>
#include "../logging/loggingthreshold.hpp"
#include "globalconfig.hpp"
#include "singlealertconfig.hpp"

namespace radioalert
{
  class AppConfigClass
  {
    private:
    static const QString constNoData;         //! Kennzeichner für keine Daten
    static const QString constAppGroupName;   //! Gruppenname für App Einstellungen
    static const QString constAppTimeoutKey;  //! Einstellung für Watchdog
    // ab hier die Konfiguration lagern
    QString configFileName;
    GlobalConfig globalConfig;                //! globale Konfiguration als Objekt
    QMap< qint8, SingleAlertConfig > alerts;  //! Liste mit Alarmen aus der Konfig

    public:
    AppConfigClass( void );                             //! Konstruktor
    AppConfigClass( const QString &configFileName );    //! Konstruktor
    virtual ~AppConfigClass();                          //! Destruktor
    bool loadSettings( void );                          //! lade Einstellungen aus default Konfigdatei
    bool loadSettings( QString &configFile );           //! lade Einstellungen aus benannter Konfigdatei
    bool saveSettings( void );                          //! sichere Einstellungen
    QString getLogfileName( void ) const;               //! Name der configdatei ausgeben
    void setConfigFileName( const QString &fileName );  //! name der configfdatei setzten
    /*
    QString getConfigFile( void ) const;              //! Name der Konfigdatei ausgeben
    QString getLogfileName( void ) const;             //! Name der Logdatei ausgeben
    void setLogfileName( const QString &value );      //! Name der Logdatei setzten
    void setLogThreshold( LoggingThreshold th );      //! setzte Loggingstufe in Config
    LoggingThreshold getLogTreshold( void );          //! hole Loggingstufe aus config
    bool getLogToConsole() const;                     //! setze den Wert ob zu Konsole geloggt werden darf
    void setLogToConsole( bool value );               //! lese den Wert ob zu Konsole geloggt werden darf
    */

    private:
    // Logeinstellungen
    bool loadGlobalSettings( QSettings &settings );
    void makeDefaultGlobalSettings( QSettings &settings );
    bool saveGlobalSettings( QSettings &settings );
    // allg. Programmeinstellungen
    bool loadAlertSettings( QSettings &settings );
    void makeAlertDefaultSettings( QSettings &settings );
    bool saveAlertSettings( QSettings &settings );
  };
}  // namespace radioalert
#endif  // LOGGERCLASS_HPP
