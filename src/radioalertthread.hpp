#ifndef TIMESERVICE_MAINTIMERTHREAD_HPP
#define TIMESERVICE_MAINTIMERTHREAD_HPP

#include <qglobal.h>
#include <QObject>
#include <QThread>
#include <QTimer>
#include "config/singlealertconfig.hpp"
#include "logging/Logger.hpp"

namespace radioalert
{
  class RadioAlertThread : public QThread
  {
    Q_OBJECT

    private:
    std::shared_ptr< Logger > lg;
    const SingleAlertConfig localAlertConfig;  //! lokale kopie der Konfiguration
    QTimer alertTimer;
    qint32 timerCounter = 20;
    qint16 threadNumber;
    static qint16 threadCounter;
    static qint16 threadNumbers;

    public:
    explicit RadioAlertThread( std::shared_ptr< Logger > logger, SingleAlertConfig &alert, QObject *parent = nullptr );
    ~RadioAlertThread() override;
    void run( void ) override;
    void startTimer( int interval );
    void cancelThread( void );

    signals:
    void sigAlertFinished( RadioAlertThread *theTread );  //! signalisiert, dass der Thread fertig ist und aus der Liste raus kann

    private slots:
    void slotOnTimer( void );
  };
}  // namespace radioalert

#endif  // MAINTIMERTHREAD_HPP
