#ifndef AVAILABLADEVICES_HPP
#define AVAILABLADEVICES_HPP

#include <qglobal.h>
#include <QMutex>
#include <QMutexLocker>
#include <QSettings>
#include <QtDebug>

namespace radioalert
{
  // vorwärts deklaration
  class SoundTouchDeviceData;

  //! lesbarer machen der Sourcen
  using StDevicesHashList = QHash< QString, SoundTouchDeviceData >;

  class SoundTouchDeviceData
  {
    public:
    QString hostName;
    qint16 httpPort;
    qint16 wsPort;
    QString deviceTyype;
    QString deviceName;
    QString deviceId;
  };

  class AvailableDevices
  {
    private:
    static const QString hostName;    //! Adresse/Hostname des Gerätes
    static const QString httpPort;    //! Port für Web
    static const QString wsPort;      //! Port für Websocket
    static const QString deviceType;  //! Typenbezeichnung
    static const QString deviceName;  //! Menschenlesbarer Name (für Anzeigen)
    static const QString deviceId;    //! ID(Mac) des Gerätes
    QMutex configLocker;              //! verhindert gleichzeitige verändernde zugriffe auf config
    StDevicesHashList stDevices;      //! Liste mit Geräten

    public:
    explicit AvailableDevices( void );
    bool loadSettings( const QString &fileName );
    StDevicesHashList &getDevicesList( void );
  };
}  // namespace radioalert
#endif  // AVAILABLADEVICES_HPP
