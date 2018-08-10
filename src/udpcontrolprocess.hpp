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
    static const QString alertRootStr;
    static const QRegExp alertExpr;
    static const QString allAlertExpr;
    static const QRegExp noAlertExpr;
    static const QString commandGet;
    static const QString commandConfigId;
    static const QString answerConfigVersion;
    static const QString commandConfig;
    static const QString commandDevices;
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

    signals:
    void closeUdpServer( void );
  };
}  // namespace radioalert
