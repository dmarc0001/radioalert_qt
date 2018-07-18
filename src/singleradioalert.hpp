#ifndef SINGLERADIOALERT_HPP
#define SINGLERADIOALERT_HPP

#include <qglobal.h>
#include <QObject>
#include <bsoundtouchdevice.hpp>
#include <memory>
#include "config/availabledevices.hpp"
#include "config/singlealertconfig.hpp"
#include "global_config.hpp"
#include "logging/logger.hpp"
#include "utils/noavailiblesounddeviceexception.hpp"

namespace radioalert
{
  //
  // Lesbasrkeit vereinfachen
  //
  constexpr qint32 RESPONSETIMEOUT = 20;
  using SharedResponsePtr = std::shared_ptr< bose_soundtoch_lib::IResponseObject >;
  //
  enum class deviceStatus : quint8
  {
    NONE,
    STANDBY,
    READY,
    BUFFERING,
    PLAYING,
    ERROR
  };

  class SingleRadioAlert : public QObject
  {
    Q_OBJECT

    private:
    std::shared_ptr< Logger > lg;                                           //! der Logger
    const SingleAlertConfig localAlertConfig;                               //! lokale kopie der Konfiguration
    const StDevicesHashList avStDevices;                                    //! lokale kopie der verfügbaren Geräte
    std::unique_ptr< bose_soundtoch_lib::BSoundTouchDevice > masterDevice;  //! das Soundtoch Masterdevice
    StDevicesHashList realDevices;                                          //! devices, welceh angefordert und auch vorhanden sind
    qint32 alertLoopCounter;                                                //! anzahl der timerdurchläufe zählen
    deviceStatus masterDeviceStat;                                          //! wenn ich auf das Masterdevice warte
    bool isActive;                                                          //! ist dieser alarm aktiv?
    QString masterDeviceName;                                               //! Name des Master Device (kann auch das einzige sein)
    qint16 alertDuration;                                                   //! zeit, die der alarm noch läuft

    public:
    SingleRadioAlert( std::shared_ptr< Logger > logger, SingleAlertConfig &alert, StDevicesHashList &devices, QObject *parent );
    ~SingleRadioAlert();
    void start( void );               //! starte die Verarbeitung
    void cancelAlert( void );         //! beende Alarm
    void cancelAlert( QString msg );  //! beende Alarm
    QString getAlertName( void );     //! welchen namen hat der alarm in der config

    private:
    bool checkIfDevicesAvailible( void );                                            //! sind Geräte für diesen alarm verfügbar?
    bool checkIfDeviceIsInStandby( bose_soundtoch_lib::BSoundTouchDevice *device );  //! Ist das Gerät im standby?
    void computeVolumeMsg( SharedResponsePtr response );                             //! Lautstärke Nachricht verarbeiten
    void computeNowPlayingMsg( SharedResponsePtr response );                         //! Now Playing Nachricht verarbeiten

    signals:
    void sigAlertFinished( SingleRadioAlert *theAlert );  //! sende Signal wenn der Alarm regilär beendet wurde
    void sigAlertResultError( const QString &errMsg );    //! sende Signal mit Fehlertext ei Fehler

    public slots:
    void slotOnZyclonTimer( void );

    private slots:
    void slotOnRequestAnswer( SharedResponsePtr response );
  };
}  // namespace radioalert
#endif  // SINGLERADIOALERT_HPP
