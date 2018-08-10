#ifndef SRC_CONFIG_APPCONFIGCLASS_HPP
#define SRC_CONFIG_APPCONFIGCLASS_HPP

#include <QCryptographicHash>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QSettings>
#include <QString>
#include <QVector>
#include "../global_config.hpp"
#include "../logging/loggingthreshold.hpp"
#include "alertconfig.hpp"
#include "config_all.hpp"
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
    QMutex configLocker;                      //! verhindert gleichzeitige verändernde zugriffe auf config
    QByteArray configHashLoad;                //! nach dem Laden odder speichern hash berechnet
    QByteArray configFileHash;                //! der Hash über die datei (veränderung feststellen)

    public:
    AppConfigClass( void );                             //! Konstruktor
    AppConfigClass( const QString &configFileName );    //! Konstruktor
    virtual ~AppConfigClass();                          //! Destruktor
    QByteArray makeConfigfileHash( void );              //! errechne den Hashwert der Konfigurationsdatei
    bool loadSettings( void );                          //! lade Einstellungen aus default Konfigdatei
    bool loadSettings( QString &configFile );           //! lade Einstellungen aus benannter Konfigdatei
    bool saveSettings( void );                          //! sichere Einstellungen
    QString getLogfileName( void ) const;               //! Name der configdatei ausgeben
    void setConfigFileName( const QString &fileName );  //! name der configfdatei setzten
    GlobalConfig &getGlobalConfig( void );              //! Referenz auf die globale Konfiguration
    RadioAlertList &getAlertList( void );               //! Referenz auf die Liste mit Alarmen
    bool isConfigChanged( void );                       //! errechne anhand des crypto-Hashwertes Veränderung
    bool isConfigFileChanged( void );                   //! wurde die datei verändert?
    QMutex *getLockMutexPtr( void );                    //! gib den zeiger für den Mutex für externe Sperrung
    QByteArray getConfigHashLoad() const;               //! den aktuellen Hashwert ausgeben

    private:
    QByteArray makeConfigHash( void );  //! erzeuge einen crypto hash für die Daten
  };
}  // namespace radioalert
#endif  // LOGGERCLASS_HPP
