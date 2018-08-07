#include "udpcontrolprocess.hpp"

namespace radioalert
{
  UdpControlProcess::UdpControlProcess( std::shared_ptr< Logger > logger, std::shared_ptr< AppConfigClass > config, QObject *parent )
      : QThread( parent ), lg( logger ), appConfig( config ), udpSocket( std::unique_ptr< QUdpSocket >( new QUdpSocket ) )
  {
    udpSocket->bind( appConfig->getGlobalConfig().getServerAddr(), appConfig->getGlobalConfig().getServerPort() );
  }

  void UdpControlProcess::run( void )
  {
    connect( udpSocket.get(), &QUdpSocket::readyRead, this, &UdpControlProcess::slotRreadPendingDatagrams );
  }

  void UdpControlProcess::slotRreadPendingDatagrams( void )
  {
    while ( udpSocket->hasPendingDatagrams() )
    {
      QNetworkDatagram datagram = udpSocket->receiveDatagram();
      // TODO: processTheDatagram( datagram );
    }
  }

}  // namespace radioalert
