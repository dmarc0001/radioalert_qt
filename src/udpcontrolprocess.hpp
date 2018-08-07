#ifndef UDPCONTROLPROCESS_HPP
#define UDPCONTROLPROCESS_HPP

#include <qglobal.h>
#include <QNetworkDatagram>
#include <QObject>
#include <QThread>
#include <QUdpSocket>
#include <memory>
#include "global_config.hpp"
#include "logging/logger.hpp"

namespace radioalert
{
  class UdpControlProcess : public QThread
  {
    Q_OBJECT
    private:
    std::shared_ptr< Logger > lg;
    std::shared_ptr< AppConfigClass > appConfig;
    std::unique_ptr< QUdpSocket > udpSocket;

    public:
    explicit UdpControlProcess( std::shared_ptr< Logger > logger,
                                std::shared_ptr< AppConfigClass > config,
                                QObject *parent = nullptr );
    void run( void ) override;

    signals:
    void sigReloadConfig( void );

    private slots:
    void slotRreadPendingDatagrams( void );
  };
}  // namespace radioalert
#endif  // UDPCONTROLPROCESS_HPP
