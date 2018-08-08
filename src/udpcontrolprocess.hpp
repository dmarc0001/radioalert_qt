#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkDatagram>
#include <QObject>
#include <QThread>
#include <QUdpSocket>
#include <memory>
#include "logging/logger.hpp"

namespace radioalert
{
  class UdpControlProcess : public QObject
  {
    Q_OBJECT
    private:
    static const QString commandGet;
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
    void slotReadPendingDatagrams( void );  //! empfängt Daten

    signals:
    void closeUdpServer( void );
  };
}  // namespace radioalert
