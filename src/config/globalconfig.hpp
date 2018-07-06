#ifndef SRC_CONFIG_GLOBALCONFIG_HPP
#define SRC_CONFIG_GLOBALCONFIG_HPP

#include <qglobal.h>
#include <QHostAddress>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QTimeZone>
#include "../logging/loggingthreshold.hpp"

namespace radioalert
{
  class GlobalConfig
  {
    private:
    static const QString groupName;           //! Gruppenname Logeinstellungen
    static const QString logFileKey;          //! Einstellung für Logdatei
    static const QString logToConsoleKey;     //! Einstellung für loggen zur konsole
    static const QString serverPortKey;       //! Port an dem der daemon lauscht
    static const QString serverAddrKey;       //! auf welcher Adresse lauscht der Server
    static const QString timeZoneKey;         //! Welche Zeitzohne? (default die vom System)
    static const QString raiseVolKey;         //! als Voreinstellung: soll die Lautstärke sanft anschwellen?
    static const QString networkTimeoutKey;   //! Timeout des Servers
    static const QString autorefreshKey;      //! GUI autorefresh?
    static const QString guiExtraBottom1Key;  //! Extra Fusszeile
    static const QString guiExtraBottom2Key;  //! Extra 2. Fusszeile
    static const QString loglevelKey;         //! loglevel?
    static const QString path1Key;            //! Pfad nummer 1
    static const QString path2Key;            //! Pfad nummer 2
    static const QString guiHeaderKey;        //! benutzerdefinierte Titelzeile
    static const QString devicesFileKey;      //! evtl extern discoverte Soundtouch Devices
    static const QString guiThemeKey;         //! welches Thema soll die GUI haben
    // Members:
    qint16 serverPort;
    QHostAddress serverAddr;
    bool logToConsole;
    QTimeZone timeZone;
    bool raiseVol;
    qint16 networkTimeout;
    qint16 autorefresh;
    QString guiExtraBottom1;
    QString guiExtraBottom2;
    QString logFile;
    LoggingThreshold loglevel;
    QString path1;
    QString path2;
    QString guiHeader;
    QString devicesFile;
    QString guiTheme;

    public:
    explicit GlobalConfig( void );
    bool loadSettings( QSettings &settings );
    bool saveSettings( QSettings &settings );
    bool makeDefaultSettings( QSettings &settings );
    //
    // GETTER/SETTER
    //
    qint16 getServerPort() const;
    void setServerPort( const qint16 &value );
    QString getGuiExtraBottom1() const;
    void setGuiExtraBottom1( const QString &value );
    QString getGuiExtraBottom2() const;
    void setGuiExtraBottom2( const QString &value );
    QString getLogFile() const;
    void setLogFile( const QString &value );
    bool getLogToConsole() const;
    void setLogToConsole( bool value );
    LoggingThreshold getLoglevel() const;
    void setLoglevel( const LoggingThreshold &value );
    QString getPath1() const;
    void setPath1( const QString &value );
    QString getPath2() const;
    void setPath2( const QString &value );
    QTimeZone getTimeZone() const;
    void setTimeZone( const QTimeZone &value );
    bool getRaiseVol() const;
    void setRaiseVol( bool value );
    QString getGuiHeader() const;
    void setGuiHeader( const QString &value );
    qint16 getNetworkTimeout() const;
    void setNetworkTimeout( const qint16 &value );
    qint16 getAutorefresh() const;
    void setAutorefresh( const qint16 &value );
    QHostAddress getServerAddr() const;
    void setServerAddr( const QHostAddress &value );
    QString getDevicesFile() const;
    void setDevicesFile( const QString &value );
    QString getGuiTheme() const;
    void setGuiTheme( const QString &value );
    QByteArray serialize( void );
  };
}  // namespace radioalert
#endif  // GLOBALCONFIG_HPP
