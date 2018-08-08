#include "udpcontrolprocess.hpp"

namespace radioalert
{
  const QString UdpControlProcess::commandGet{"get"};

  UdpControlProcess::UdpControlProcess( std::shared_ptr< Logger > logger, std::shared_ptr< AppConfigClass > config, QObject *parent )
      : QObject( parent )
      , lg( logger )
      , cf( config )
      , udpSocket( std::unique_ptr< QUdpSocket >( new QUdpSocket( this ) ) )
      , isRunning( true )
  {
    LGINFO( QString( "UdpControlProcess::UdpControlProcess: bind on udp <%1:%2>" )
                .arg( cf->getGlobalConfig().getServerAddr().toString() )
                .arg( cf->getGlobalConfig().getServerPort() ) );
    udpSocket->bind( cf->getGlobalConfig().getServerAddr(), cf->getGlobalConfig().getServerPort() );
    LGINFO( "UdpControlProcess::UdpControlProcess: bind on udp OK" );
  }

  void UdpControlProcess::init( void )
  {
    LGDEBUG( "UdpControlProcess::start: connect signals..." );
    connect( udpSocket.get(), &QUdpSocket::readyRead, this, &UdpControlProcess::slotReadPendingDatagrams );
    LGDEBUG( "UdpControlProcess::start: OK" );
  }

  void UdpControlProcess::requestQuit( void )
  {
    LGINFO( "UdpControlProcess::requestQuit..." );
    //
    // Alle eventuell vorhandenen Alarme beenden
    //
    disconnect( udpSocket.get(), 0, 0, 0 );
    udpSocket->close();
    //
    // Aufräumen dem System/ der runtime überlassen :-)
    //
    LGINFO( "UdpControlProcess::requestQuit...OK" );
    emit closeUdpServer();
  }

  void UdpControlProcess::slotReadPendingDatagrams( void )
  {
    LGDEBUG( "UdpControlProcess::slotRreadPendingDatagrams..." );
    while ( udpSocket->hasPendingDatagrams() )
    {
      QNetworkDatagram datagram = udpSocket->receiveDatagram();
      if ( !datagram.isNull() && datagram.isValid() )
      {
        QJsonParseError err;
        LGDEBUG( "UdpControlProcess::slotRreadPendingDatagrams: create QJsonObject..." );
        QJsonDocument bDocument( QJsonDocument::fromJson( datagram.data(), &err ) );
        if ( bDocument.isObject() )
        {
          QJsonObject bObject( bDocument.object() );
          LGDEBUG( "UdpControlProcess::slotRreadPendingDatagrams: read data from json root object..." );
          //
          // Daten auslesen?
          // Datenmuster wert: array
          // {"get": ["alert-01"]}
          //
          for ( QJsonObject::iterator jRoot( bObject.begin() ); jRoot != bObject.end(); jRoot++ )
          {
            if ( jRoot->isObject() )
            {
              // child...
              QJsonObject element( jRoot->toObject() );
              if ( element.contains( commandGet ) && element.value( commandGet ).isArray() )
              {
              }
            }
          }
        }
      }
    }
  }

}  // namespace radioalert
