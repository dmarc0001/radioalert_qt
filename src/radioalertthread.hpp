#ifndef TIMESERVICE_MAINTIMERTHREAD_HPP
#define TIMESERVICE_MAINTIMERTHREAD_HPP

#include <qglobal.h>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <bsoundtouchdevice.hpp>
#include "config/availabledevices.hpp"
#include "config/singlealertconfig.hpp"
#include "global_config.hpp"
#include "logging/logger.hpp"
#include "utils/noavailiblesounddeviceexception.hpp"

namespace radioalert
{
  class RadioAlertThread : public QThread
  {
    Q_OBJECT

    private:
    std::shared_ptr< Logger > lg;              //! der Logger
    const SingleAlertConfig localAlertConfig;  //! lokale kopie der Konfiguration
    const StDevicesHashList avStDevices;       //! lokale kopie der verfügbaren Geräte
    QTimer alertTimer;                         //! der timer, welcher kontinuierlich die Funktion überwacht
    std::unique_ptr< bose_soundtoch_lib::BSoundTouchDevice > masterDevice;  //! das Soundtoch Masterdevice
    StDevicesHashList realDevices;                                          //! devices, welceh angefordert und auch vorhanden sind
    QString masterDeviceName;                                               //! Name des Master Device (kann auch das einzige sein)
    // fürs debugging
    qint32 timerCounter = 20;

    public:
    explicit RadioAlertThread( std::shared_ptr< Logger > logger,
                               SingleAlertConfig &alert,
                               StDevicesHashList &devices,
                               QObject *parent = nullptr );
    ~RadioAlertThread() override;
    void run( void ) override;
    QString getAlertName( void ) const;
    void startTimer( int interval );
    void cancelThread( void );

    private:
    bool checkIfDevicesAvailible( void );
    bool checkIfDeviceIsInStandby( bose_soundtoch_lib::BSoundTouchDevice *device );

    signals:
    void sigAlertFinished( RadioAlertThread *theTread );  //! signalisiert, dass der Thread fertig ist und aus der Liste raus kann

    private slots:
    void slotOnTimer( void );
    void slotOnRequestAnswer( std::shared_ptr< bose_soundtoch_lib::IResponseObject > response );
  };
}  // namespace radioalert

#endif  // MAINTIMERTHREAD_HPP
