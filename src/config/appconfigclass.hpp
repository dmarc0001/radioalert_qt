#ifndef SRC_CONFIG_APPCONFIGCLASS_HPP
#define SRC_CONFIG_APPCONFIGCLASS_HPP

#include <QHash>
#include <QSettings>
#include <QString>
#include <QVector>
#include "../logging/loggingthreshold.hpp"
#include "alertconfig.hpp"
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
    QString configFileName;                   //! Name der Konfigurationsdatei
    GlobalConfig globalConfig;                //! globale Konfiguration als Objekt
    RadioAlertList alerts;                    //! Liste mit Alarmen aus der Konfig

    public:
    AppConfigClass( void );                             //! Konstruktor
    AppConfigClass( const QString &configFileName );    //! Konstruktor
    virtual ~AppConfigClass();                          //! Destruktor
    bool loadSettings( void );                          //! lade Einstellungen aus default Konfigdatei
    bool loadSettings( QString &configFile );           //! lade Einstellungen aus benannter Konfigdatei
    bool saveSettings( void );                          //! sichere Einstellungen
    QString getLogfileName( void ) const;               //! Name der configdatei ausgeben
    void setConfigFileName( const QString &fileName );  //! name der configfdatei setzten
    GlobalConfig &getGlobalConfig( void );              //! Referenz auf die globale Konfiguration
    RadioAlertList &getAlertList( void );               //! Referenz auf die Liste mit Alarmen

    private:
  };
}  // namespace radioalert
#endif  // LOGGERCLASS_HPP
