#ifndef IPUTILITYS_HPP
#define IPUTILITYS_HPP

#include <QtCore/qglobal.h>
#include <QHostAddress>
#include <QNetworkInterface>

namespace radio
{
  class IpUtilitys
  {
    public:
    IpUtilitys();
    static bool isLoopbackIPv4Address( QString address );
    static bool isLoopbackIPv6Address( QString address );
    static bool isPromiscuousIPv4Address( QString address );
    static bool isPromiscuousIPv6Address( QString address );
    static QString getValidIP( void );
  };
}  // namespace radio
#endif  // IPUTILITYS_HPP
