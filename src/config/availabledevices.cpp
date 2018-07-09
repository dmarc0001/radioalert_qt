#include "availabledevices.hpp"
#include <QFileInfo>
#include <QList>
#include <QStringList>
#include "../utils/configfilenotexistexception.hpp"

namespace radioalert
{
  const QString AvailableDevices::hostName( "host" );
  const QString AvailableDevices::deviceType( "type" );
  const QString AvailableDevices::hostPort( "port" );
  const QString AvailableDevices::deviceName( "name" );

  AvailableDevices::AvailableDevices( void )
  {
  }

  /**
   * @brief AvailablaDevices::loadSettings
   * @param fileName
   * @return
   * throws exception if no file is availible
   */
  bool AvailableDevices::loadSettings( const QString &fileName )
  {
    // Config locken, während der aktion
    QMutexLocker locking( &configLocker );
    //
    stDevices.clear();
    // Locken während die config geladen wird
    // QMutexLocker locker( &configLocker );

    qDebug().noquote() << "Devices file: <" + fileName + ">";
    QFileInfo configFileInfo( fileName );
    if ( !configFileInfo.exists() )
    {
      throw ConfigfileNotExistException( QString( "device configfile %1 not exist" ).arg( fileName ) );
    }
    QSettings settings( fileName, QSettings::IniFormat );
    qDebug().noquote() << "load settings from" << fileName;
    //
    // alle Gruppen (devices) laden
    //
    QStringList groups( settings.childGroups() );
    QList< QString >::iterator sli;
    //
    // Alle Gruppen abarbeiten
    //
    for ( sli = groups.begin(); sli != groups.end(); sli++ )
    {
      qDebug().nospace().noquote() << endl << "load device: " << *sli;
      //
      // Gruppe eröffnen
      //
      settings.beginGroup( *sli );
      SoundTouchDeviceData stDevice;
      //
      // Hostnamen erfragen
      //
      stDevice.hostName = settings.value( hostName, "localhost" ).toString();
      qDebug().nospace().noquote() << "AvailablaDevices::loadSettings: " << hostName << ": " << stDevice.hostName;
      //
      // device name erfragen
      //
      stDevice.deviceName = settings.value( deviceName, "no name" ).toString();
      qDebug().nospace().noquote() << "AvailablaDevices::loadSettings: " << deviceName << ": " << stDevice.deviceName;
      //
      // Host type erfragen
      //
      stDevice.deviceTyype = settings.value( deviceType, "unknown" ).toString();
      qDebug().nospace().noquote() << "AvailablaDevices::loadSettings: " << deviceType << ": " << stDevice.deviceTyype;
      //
      // Host port erfragen
      //
      stDevice.hostPort = static_cast< qint16 >( settings.value( hostPort, "8090" ).toString().toInt() );
      qDebug().nospace().noquote() << "AvailablaDevices::loadSettings: " << hostPort << ": " << stDevice.hostPort;
      //
      settings.endGroup();
      stDevices.insert( stDevice.hostName, stDevice );
    }
    return ( true );
  }

  /**
   * @brief AvailableDevices::getDevicesList
   * @return
   */
  StDevicesHashList &AvailableDevices::getDevicesList( void )
  {
    // Config locken, während der aktion
    QMutexLocker locker( &configLocker );
    // und eine Referenz für eine Kopie zurück geben
    return ( stDevices );
  }

}  // namespace radioalert
