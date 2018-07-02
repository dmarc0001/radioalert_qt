#include "soundtouchudplistener.hpp"

#if defined( Q_OS_UNIX ) || defined( Q_OS_ANDROID )
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <cstring>
#define MAX_LEN 1024 /* maximum receive string size */

namespace radio
{
  SoundTouchUDPListener::SoundTouchUDPListener( QString address, quint32 port, QString objectName, QObject *parent )
      : QUdpSocket( parent )
  {
    this->port = port;
    this->objectName = objectName;
    this->address = address;
  }

  SoundTouchUDPListener::~SoundTouchUDPListener()
  {
  }

  void SoundTouchUDPListener::start()
  {
#if defined( Q_WS_X11 ) || defined( Q_OS_ANDROID )
    if ( !this->bind( QHostAddress( this->address ), this->port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint ) )
    {
      qWarning() << this->objectName << ": failure to bind interface.";
    }
#endif
    //
    // Socket descriptor vorbereiten
    //
    int fd;
    fd = this->socketDescriptor();
    struct ip_mreq mreq;
    memset( &mreq, 0, sizeof( ip_mreq ) );
    mreq.imr_multiaddr.s_addr = inet_addr( this->address.toUtf8() );
    bool boolean = true;
    //
    // gültige Adresse besorgen
    //
    QString ip = IpUtilitys::getValidIP();
    if ( IpUtilitys::isLoopbackIPv4Address( ip ) )
    {
      mreq.imr_interface.s_addr = inet_addr( ip.toUtf8() );
    }
    else
    {
      mreq.imr_interface.s_addr = htons( INADDR_ANY );
    }
#if defined( Q_OS_WIN )
    qWarning() << "windows procedure is running...";
    WSADATA wsaData;            /* Windows socket DLL structure */
    struct sockaddr_in mc_addr; /* socket address structure */

    /* Load Winsock 2.0 DLL */
    if ( WSAStartup( MAKEWORD( 2, 0 ), &wsaData ) != 0 )
    {
      fprintf( stderr, "WSAStartup() failed" );
      exit( 1 );
    }

    /* create socket to join multicast group on */
    if ( ( fd = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 )
    {
      qWarning() << "socket() failed";
      exit( 1 );
    }

    /* set reuse port to on to allow multiple binds per host */
    if ( ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, ( char * ) &boolean, sizeof( boolean ) ) ) < 0 )
    {
      qWarning() << "setsockopt() failed";
      exit( 1 );
    }

    /* construct a multicast address structure */
    memset( &mc_addr, 0, sizeof( mc_addr ) );
    mc_addr.sin_family = AF_INET;
    mc_addr.sin_addr.s_addr = htonl( INADDR_ANY );
    mc_addr.sin_port = htons( this->port );

    /* bind to multicast address to socket */
    if ( (::bind( fd, ( struct sockaddr * ) &mc_addr, sizeof( mc_addr ) ) ) < 0 )
    {
      qWarning() << "bind() failed";
      exit( 1 );
    }

    /* construct an IGMP join request structure */
    mreq.imr_multiaddr.s_addr = inet_addr( address.toAscii() );
    mreq.imr_interface.s_addr = htonl( INADDR_ANY );

    /* send an ADD MEMBERSHIP message via setsockopt */
    if ( ( setsockopt( fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, ( char * ) &mreq, sizeof( mreq ) ) ) < 0 )
    {
      qWarning() << this->objectName << ": could not join MULTICAST group.";

      exit( 1 );
    }
    this->setSocketDescriptor( fd, QUdpSocket::BoundState, QUdpSocket::ReadOnly );
#endif

#if defined( Q_WS_X11 ) || defined( Q_OS_ANDROID ) || defined( Q_OS_LINUX ) || defined( Q_OS_UNIX )
    if ( setsockopt( fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof( mreq ) ) < 0 ||
         setsockopt( fd, IPPROTO_IP, IP_MULTICAST_LOOP, &boolean, sizeof( boolean ) ) < 0 )
    {
      qWarning() << this->objectName << ": could not join MULTICAST group.";
    }
#endif
  }
}  // namespace radio
