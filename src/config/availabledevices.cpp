#include "availabledevices.hpp"
#include <QFileInfo>
#include <QList>
#include <QStringList>
#include "../utils/configfilenotexistexception.hpp"

namespace radioalert
{
  const QString AvailableDevices::hostName{"host"};
  const QString AvailableDevices::deviceType{"type"};
  const QString AvailableDevices::httpPort{"port"};
  const QString AvailableDevices::wsPort{"wsport"};
  const QString AvailableDevices::deviceName{"name"};
  const QString AvailableDevices::deviceId{"id"};

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

    qDebug().noquote() << QLatin1String( "Devices file: <" ) + fileName + QLatin1String( ">" );
    QFileInfo configFileInfo( fileName );
    if ( !configFileInfo.exists() )
    {
      throw ConfigfileNotExistException( QString( "device configfile %1 not exist" ).arg( fileName ) );
    }
    QSettings settings( fileName, QSettings::IniFormat );
    qDebug().noquote() << QLatin1String( "load settings from" ) << fileName;
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
      qDebug().nospace().noquote() << endl << QLatin1String( "load device: " ) << *sli;
      //
      // Gruppe eröffnen
      //
      settings.beginGroup( *sli );
      SoundTouchDeviceData stDevice;
      //
      // Hostnamen erfragen
      //
      stDevice.hostName = settings.value( hostName, QLatin1String( "localhost" ) ).toString();
      qDebug().nospace().noquote() << "AvailablaDevices::loadSettings: " << hostName << QLatin1String( ": " ) << stDevice.hostName;
      //
      // device name erfragen
      //
      stDevice.deviceName = settings.value( deviceName, QLatin1String( "no name" ) ).toString();
      qDebug().nospace().noquote() << QLatin1String( "AvailablaDevices::loadSettings: " ) << deviceName << QLatin1String( ": " )
                                   << stDevice.deviceName;
      //
      // Host type erfragen
      //
      stDevice.deviceTyype = settings.value( deviceType, QLatin1String( "unknown" ) ).toString();
      qDebug().nospace().noquote() << QLatin1String( "AvailablaDevices::loadSettings: " ) << deviceType << QLatin1String( ": " )
                                   << stDevice.deviceTyype;
      //
      // Host http port erfragen
      //
      stDevice.httpPort = static_cast< qint16 >( settings.value( httpPort, QLatin1String( "8090" ) ).toString().toInt() );
      qDebug().nospace().noquote() << QLatin1String( "AvailablaDevices::loadSettings: " ) << httpPort << QLatin1String( ": " )
                                   << stDevice.httpPort;
      //
      // Host ws port erfragen
      //
      stDevice.wsPort = static_cast< qint16 >( settings.value( wsPort, QLatin1String( "8080" ) ).toString().toInt() );
      qDebug().nospace().noquote() << QLatin1String( "AvailablaDevices::loadSettings: " ) << wsPort << QLatin1String( ": " )
                                   << stDevice.wsPort;
      //
      // Device Id erfragen
      //
      stDevice.deviceId = settings.value( deviceId, QLatin1String( "00000000" ) ).toString().trimmed();
      qDebug().nospace().noquote() << QLatin1String( "AvailablaDevices::loadSettings: " ) << deviceId << QLatin1String( ": " )
                                   << stDevice.deviceId;
      //
      settings.endGroup();
      stDevices.insert( stDevice.deviceName, stDevice );
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
