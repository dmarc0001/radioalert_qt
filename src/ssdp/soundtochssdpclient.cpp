#include "soundtochssdpclient.hpp"

namespace radio
{
  SoundTochSSDPClient::SoundTochSSDPClient( QObject *parent ) : QObject( parent ), running( false )
  {
  }

  SoundTochSSDPClient::~SoundTochSSDPClient()
  {
    if ( isRunning() )
      stop();

    delete this->udpListener;
  }

  void SoundTochSSDPClient::start()
  {
    if ( !isRunning() )
    {
      this->udpListener = new SoundTouchUDPListener( "239.255.255.250", 1900, "SoundTochSSDPClient" );
      connect( this->udpListener, QOverload<>::of( &SoundTouchUDPListener::readyRead ), this, &SoundTochSSDPClient::datagramReceived );
      this->udpListener->start();
      running = true;
    }
    else
    {
      qDebug() << "SoundTochSSDPClient::start: Already running!";
    }
  }

  void SoundTochSSDPClient::stop()
  {
    if ( isRunning() )
    {
      this->udpListener->disconnectFromHost();
      ;
      running = false;
    }
    else
    {
      qDebug() << "SoundTochSSDPClient::start: Already stopped!";
    }
  }

  bool SoundTochSSDPClient::isRunning() const
  {
    return running;
  }

  void SoundTochSSDPClient::datagramReceived()
  {
    while ( this->udpListener->hasPendingDatagrams() )
    {
      QByteArray *datagram = new QByteArray();

      datagram->resize( udpListener->pendingDatagramSize() );
      udpListener->readDatagram( datagram->data(), datagram->size() );

      QString temp( datagram->data() );
      QHttpRequestHeader *parser = new QHttpRequestHeader( temp );

      notifyReceived( parser );

      delete datagram;
      delete parser;
    }
  }

  void SoundTochSSDPClient::notifyReceived( QNetworkReply *datagram )
  {
    QVariant ntsKEy = datagram->attribute( "nts" );

    if ( !datagram->hasKey( "nts" ) )
      return;

    if ( datagram->value( "nts" ) == "ssdp:alive" )
    {
      emit newDeviceEvent( datagram->value( "usn" ), datagram->value( "location" ), datagram->value( "nt" ), datagram->value( "ext" ),
                           datagram->value( "server" ), datagram->value( "cacheControl" ) );
      qDebug() << "Brisa SSDP Client: Received alive from " << datagram->value( "usn" ) << "";
    }
    else if ( datagram->value( "nts" ) == "ssdp:byebye" )
    {
      emit removedDeviceEvent( datagram->value( "usn" ) );
      qDebug() << "Brisa SSDP Client: Received byebye from " << datagram->value( "usn" ) << "";
    }
    else
    {
      qDebug() << "Brisa SSDP Client: Received unknown subtype: " << datagram->value( "nts" ) << "";
    }
  }
}  // namespace radio
