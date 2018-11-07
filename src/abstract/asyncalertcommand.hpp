#ifndef ASYNCALERTCOMMAND_HPP
#define ASYNCALERTCOMMAND_HPP

#include <qglobal.h>
#include <QObject>
#include <QThread>
#include <bsoundtouchdevice.hpp>
#include <memory>
#include "../logging/logger.hpp"
#include "xmlparser/httpresponse/httpnowplayingobject.hpp"
#include "xmlparser/httpresponse/httpresultokobject.hpp"
#include "xmlparser/httpresponse/httpvolumeobject.hpp"
#include "xmlparser/wscallback/wsnowplayingupdate.hpp"
#include "xmlparser/wscallback/wsvolumeupdated.hpp"

namespace radioalert
{
  using namespace bose_soundtoch_lib;
  using SharedResponsePtr = std::shared_ptr< IResponseObject >;
  using BoseDevice = BSoundTouchDevice;

  class AsyncAlertCommand : public QObject
  {
    Q_OBJECT
    private:
    std::shared_ptr< Logger > lg;  //! der Zeiger auf den Logger
    qint8 keyresponseCounter;      //! zähle bei powerToggle mit, ob zwei mal OK kam

    public:
    explicit AsyncAlertCommand( std::shared_ptr< Logger > logger, QObject *parent = nullptr );
    ~AsyncAlertCommand() = default;
    void checkIfDeviceInStandby( BoseDevice *masterDevice );                      //! prüft, ob das Gerät im STANDBY ist
    void switchDeviceToSource( BoseDevice *masterDevice, const QString source );  //! Schaltet das Gerät zu einer Quelle (PRESET)
    void askForVolume( BoseDevice *masterDevice );                                //! erfragt Lautstärke des Gerätes
    void switchPowerOff( BoseDevice *masterDevice );                              //! schalte das Gerät AUS
    void switchPowerOn( BoseDevice *masterDevice );                               //! Schalte das Gerät AN

    private:
    void switchPowerTo( bool toOn, BoseDevice *masterDevice );   //! schalte das Gerät zum Status isOn
    void togglePowerKey( bool toOn, BoseDevice *masterDevice );  //! klöppel mit der POWER Taste

    signals:
    void sigDeviceIsPoweredON();                          //! callback, Gerät ist an
    void sigDeviceIsPoweredOFF();                         //! callback, Gerät ist aus
    void sigDeviceIsStandby( bool isStandby );            //! Callback über Standby oder nicht
    void sigDeviceIsSwitchedToSource( bool isSwitched );  //! Callback über Erfolg der Umschaltung
    void sigDeviceVolume( int currVol );                  //! Callback informiert über Lautstärke des Gerätes

    public slots:
    private slots:
  };
}  // namespace radioalert
#endif  // ASYNCALERTCOMMAND_HPP
