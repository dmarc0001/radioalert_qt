#ifndef TIMESERVICE_MAINTIMERTHREAD_HPP
#define TIMESERVICE_MAINTIMERTHREAD_HPP

#include <qglobal.h>
#include <QObject>
#include <QThread>

namespace radioalert
{
  class CheckForAlertTimerThread : public QThread
  {
    public:
    explicit CheckForAlertTimerThread( QObject *parent = nullptr );
    void run( void ) override;
  };
}  // namespace radioalert

#endif  // MAINTIMERTHREAD_HPP
