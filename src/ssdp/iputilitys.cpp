#include "iputilitys.hpp"

namespace radio
{
  IpUtilitys::IpUtilitys()
  {
  }

  bool IpUtilitys::isLoopbackIPv4Address( QString address )
  {
    return bool( !address.compare( "127.0.0.1" ) );
  }

  bool IpUtilitys::isLoopbackIPv6Address( QString address )
  {
    return bool( !address.compare( "0:0:0:0:0:0:0:1" ) );
  }

  bool IpUtilitys::isPromiscuousIPv4Address( QString address )
  {
    return bool( !address.compare( "0.0.0.0" ) );
  }

  bool IpUtilitys::isPromiscuousIPv6Address( QString address )
  {
    return bool( !address.compare( "0:0:0:0:0:0:0:0" ) | !address.compare( "::" ) );
  }

  QString IpUtilitys::getValidIP( void )
  {
    /*#if defined(Q_OS_UNIX) || defined(Q_OS_ANDROID)
        BrisaConfigurationManager *config = BrisaConfigurationManager::getInstance();
        QString interfaceName = config->getParameter("network", "interface");
        QString ip = getIp(interfaceName);
        if (ip.isEmpty()) {
            ip = config->getParameter("network", "ip");
        }
        if (ip.isEmpty()) {
            ip = QHostAddress(QHostAddress::Any).toString();
        }
        return ip;
    #else*/
    foreach ( QHostAddress addressEntry, QNetworkInterface::allAddresses() )
    {
      QString address = addressEntry.toString();
      if ( !( IpUtilitys::isLoopbackIPv4Address( address ) ) && !( IpUtilitys::isLoopbackIPv6Address( address ) ) &&
           !( IpUtilitys::isPromiscuousIPv4Address( address ) ) && !( IpUtilitys::isPromiscuousIPv6Address( address ) ) )
      {
        return address;
      }
    }
    qWarning() << "Couldn't acquire a non loopback IP  address,returning 127.0.0.1.";
    return "127.0.0.1";
    //#endif
  }
}  // namespace radio
