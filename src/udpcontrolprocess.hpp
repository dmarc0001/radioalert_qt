#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkDatagram>
#include <QObject>
#include <QRegExp>
#include <QThread>
#include <QUdpSocket>
#include <memory>
#include "config/availabledevices.hpp"
#include "logging/logger.hpp"
#include "utils/configfilenotexistexception.hpp"

namespace radioalert
{
  class UdpControlProcess : public QObject
  {
    Q_OBJECT
    private:
    static const QString okString;             //! antwort bei set für OK
    static const QString errString;            //! antwort bei Fehlern
    static const QString alertRootStr;         //! basisstring für alert
    static const QRegExp alertExpr;            //! regex zum Finden eines alert in der Konfiguration
    static const QRegExp noAlertExpr;          //! Alles was nicht alert ist
    static const QString cmdGet;               //! Kommando GET
    static const QString cmdGetAllAlerts;      //! unterkommando zum erfragen aller Alarme
    static const QString cmdGetConfigId;       //! unterkommando für Konfigurations-Id
    static const QString cmdGetConfigVersion;  //! unterkommando für Version der Config
    static const QString cmdGetConfig;         //! unterkommando für Programmkonfiguration
    static const QString cmdGetDevices;        //! unterkommando für vorhandene Geräte
    static const QString cmdGetNew;            //! unterkommando zum erzeuen und herausgeben eines neuen, leerren alarms
    static const QString cmdSet;               //! Kommando SET
    static const QString cmdSetAlertName;      //! unterkommando Name des zu bearbeitenden Alarms
    //
    std::shared_ptr< Logger > lg;             //! der Logfer
    std::shared_ptr< AppConfigClass > cf;     //! App Config
    std::unique_ptr< QUdpSocket > udpSocket;  //! der Socket zum Empfangen
    volatile bool isRunning;                  //! Marker für weiterlaufen

    public:
    explicit UdpControlProcess( std::shared_ptr< Logger > logger,
                                std::shared_ptr< AppConfigClass > config,
                                QObject *parent = nullptr );
    void init( void );
    void requestQuit( void );  //! Fordere Ende an

    private:
    void slotReadPendingDatagrams( void );                     //! empfängt Daten
    QByteArray computeGetCommand( QJsonValue jsonVal ) const;  //! wertet GET aus
    QByteArray computeGetAlerts( QStringList &alerts ) const;  //! bearbeitet alarm rquests
    QByteArray computeGetGlobalConfig( void ) const;           //! globale Onfiguration erstellen
    QByteArray computeGetDevices( void ) const;                //! verfügbare Geräte zusammenstellen
    QByteArray computeGetNew( void ) const;                    //! bearbeite die Anforderung nach einem neuen alarm
    QByteArray computeSetCommand( QJsonValue jsonVal ) const;  //! wertet SET aus

    signals:
    void closeUdpServer( void );
  };
}  // namespace radioalert
