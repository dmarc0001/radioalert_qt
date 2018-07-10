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
    qint16 hostPort;
    QString deviceTyype;
    QString deviceName;
  };

  class AvailableDevices
  {
    private:
    static const QString hostName;    //! Adresse/Hostname des Gerätes
    static const QString hostPort;    //! Port für Web
    static const QString deviceType;  //! Typenbezeichnung
    static const QString deviceName;  //! Menschenlesbarer Name (für Anzeigen)
    QMutex configLocker;              //! verhindert gleichzeitige verändernde zugriffe auf config
    StDevicesHashList stDevices;      //! Liste mit Geräten

    public:
    explicit AvailableDevices( void );
    bool loadSettings( const QString &fileName );
    StDevicesHashList &getDevicesList( void );
  };
}  // namespace radioalert
#endif  // AVAILABLADEVICES_HPP
