#ifndef SINGLERADIOALERT_HPP
#define SINGLERADIOALERT_HPP

#include <qglobal.h>
#include <QHash>
#include <QObject>
#include <QTimer>
#include <bsoundtouchdevice.hpp>
#include <memory>
#include "config/availabledevices.hpp"
#include "config/singlealertconfig.hpp"
#include "global_config.hpp"
#include "logging/logger.hpp"
#include "utils/noavailiblesounddeviceexception.hpp"
#include "xmlparser/httpresponse/httpnowplayingobject.hpp"
#include "xmlparser/httpresponse/httpresultokobject.hpp"
#include "xmlparser/httpresponse/httpvolumeobject.hpp"
#include "xmlparser/wscallback/wserrorupdated.hpp"
#include "xmlparser/wscallback/wsnowplayingupdate.hpp"
#include "xmlparser/wscallback/wsnowselectionupdated.hpp"
#include "xmlparser/wscallback/wsvolumeupdated.hpp"

namespace radioalert
{
  using namespace bose_soundtoch_lib;
  //
  // Lesbarkeit vereinfachen
  //
  constexpr qint32 RESPONSETIMEOUT = 20 * 1000;    //! Timeout in ms
  constexpr qint8 MAXCONNECTTRYS = 5;              //! maximale Anzahl von Verbindungsversuchen für Websocket
  constexpr qint16 DIMMERTIMEVELUE = 160;          //! timer intervall zum dimmen der Lautstärke
  static const QString PRESETPATTERN = "PRESET_";  //! wenn ein PRESET ausgewählt wurde
  //
  using SharedResponsePtr = std::shared_ptr< IResponseObject >;
  using BoseDevice = BSoundTouchDevice;
  //
  enum class deviceStatus : quint8
  {
    NONE,
    STANDBY,
    BUFFERING,
    PLAYING,
    INIT_GROUP,
    PLAY_ALERT,
    ENDING_ALERT,
    ALERT_FINISH,
    ERROR
  };

  class SingleRadioAlert : public QObject
  {
    Q_OBJECT

    private:
    std::shared_ptr< Logger > lg;                //! der Logger
    const SingleAlertConfig localAlertConfig;    //! lokale kopie der Konfiguration
    const StDevicesHashList avStDevices;         //! lokale kopie der verfügbaren Geräte
    std::unique_ptr< BoseDevice > masterDevice;  //! das Soundtoch Masterdevice
    SoundTouchDeviceData masterDeviceData;       //! config daten des master device
    StDevicesHashList realDevices;               //! devices, welceh angefordert und auch vorhanden sind
    SoundTouchMemberList slaveList;              //! Liste der Sklaven, wenn vorhanden
    qint32 alertLoopCounter;                     //! anzahl der timerdurchläufe zählen
    deviceStatus masterDeviceStat;               //! wenn ich auf das Masterdevice warte
    bool isActive;                               //! ist dieser alarm aktiv?
    bool callBackWsConnected;                    //! zeigt ob die Websocketverbindung zum Gerät besteht
    qint8 connectWsTrysCount;                    //! anzahl der Verbindungsversuche für Websocket
    QString masterDeviceName;                    //! Name des Master Device (kann auch das einzige sein)
    qint16 alertDuration;                        //! zeit, die der alarm noch läuft
    QTimer waitForTimer;                         //! Timer zum awrten auf erfolg einer Aktion
    int currentVolume;                           //! aktuelle Lautstärke des Gerätes
    int sendVolume;                              //! zum gerät beim dimmen gesendete Lautstärke
    int oldVolume;                               //! beim Einschalten gefundene Lautstärke
    QString lastError;                           //! kam ein Fehler vom Gerät, hier der letzte Fehler vorgehalten

    public:
    SingleRadioAlert( std::shared_ptr< Logger > logger, SingleAlertConfig &alert, StDevicesHashList &devices, QObject *parent );
    ~SingleRadioAlert();
    void start( void );               //! starte die Verarbeitung
    void cancelAlert( void );         //! beende Alarm
    void cancelAlert( QString msg );  //! beende Alarm
    QString getAlertName( void );     //! welchen namen hat der alarm in der config

    private:
    bool checkIfDevicesAvailible( void );                               //! sind Geräte für diesen alarm verfügbar?
    void connectCallbacksforDevice( void );                             //! verbinde die Callbacks mit dem Gerät
    void computeStausMsg( SharedResponsePtr response );                 //! Bestätigung für eine GET Anforderung
    void computeVolumeMsg( SharedResponsePtr response );                //! Lautstärke Nachricht verarbeiten
    void computeNowPlayingMsg( SharedResponsePtr response );            //! Now Playing Nachricht verarbeiten
    void switchMasterDeviceToSource( HttpNowPlayingObject *nPlayObj );  //! schalte das Gerät zur Quelle im aktuellen alarm

    signals:
    void sigAlertFinished( SingleRadioAlert *theAlert );  //! sende Signal wenn der Alarm regilär beendet wurde
    void sigAlertResultError( const QString &errMsg );    //! sende Signal mit Fehlertext ei Fehler

    public slots:
    void slotOnZyclonTimer( void );  //! der zyklische Timer

    private slots:
    void slotOnRequestAnswer( SharedResponsePtr response );    //! wenn eine Antwort auf http request rein kommt
    void slotOnNowPlayingUpdate( SharedResponsePtr respObj );  //! wenn der Spielstatus sich ändert...
  };
}  // namespace radioalert
#endif  // SINGLERADIOALERT_HPP
