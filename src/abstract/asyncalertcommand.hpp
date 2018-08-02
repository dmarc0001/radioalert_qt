#ifndef ASYNCALERTCOMMAND_HPP
#define ASYNCALERTCOMMAND_HPP

#include <qglobal.h>
#include <QObject>
#include <bsoundtouchdevice.hpp>
#include <memory>
#include "../logging/logger.hpp"
#include "xmlparser/httpresponse/httpnowplayingobject.hpp"
#include "xmlparser/wscallback/wsnowplayingupdate.hpp"

namespace radioalert
{
  using namespace bose_soundtoch_lib;
  using SharedResponsePtr = std::shared_ptr< IResponseObject >;
  using BoseDevice = BSoundTouchDevice;

  class AsyncAlertCommand : public QObject
  {
    Q_OBJECT
    private:
    std::shared_ptr< Logger > lg;

    public:
    explicit AsyncAlertCommand( std::shared_ptr< Logger > logger, QObject *parent = nullptr );
    ~AsyncAlertCommand();
    void checkIfDeviceInStandby( BoseDevice *masterDevice );
    void switchDeviceToSource( BoseDevice *masterDevice, const QString source );

    signals:
    void sigDeviceIsStandby( bool isStandby );
    void sigDeviceIsSwitchedToSource( bool isSwitched );

    public slots:
  };
}  // namespace radioalert
#endif  // ASYNCALERTCOMMAND_HPP
