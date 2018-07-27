#include "config_all.hpp"
#include <QCoreApplication>
#include <QtDebug>

namespace radioalert
{
  const QString GlobalConfig::groupName{"global"};
  const QString GlobalConfig::logFileKey{"logfile"};
  const QString GlobalConfig::logToConsoleKey{"log_to_console"};
  const QString GlobalConfig::serverPortKey{"server_port"};
  const QString GlobalConfig::serverAddrKey{"server_addr"};
  const QString GlobalConfig::timeZoneKey{"timezone"};
  const QString GlobalConfig::raiseVolKey{"raise_vol"};
  const QString GlobalConfig::networkTimeoutKey{"network_timeout"};
  const QString GlobalConfig::autorefreshKey{"autorefresh"};
  const QString GlobalConfig::guiExtraBottom1Key{"gui_extra_bottom1"};
  const QString GlobalConfig::guiExtraBottom2Key{"gui_extra_bottom2"};
  const QString GlobalConfig::loglevelKey{"loglevel"};
  const QString GlobalConfig::guiHeaderKey{"gui_header"};
  const QString GlobalConfig::devicesFileKey{"devices_file"};
  const QString GlobalConfig::guiThemeKey{"gui_theme"};

  GlobalConfig::GlobalConfig( void )
      : serverPort( 26106 )
      , serverAddr( QHostAddress( "localhost" ) )
      , logToConsole( true )
      , timeZone( QTimeZone::systemTimeZone() )
      , raiseVol( false )
      , networkTimeout( 10 )
      , autorefresh( 5 )
  {
  }

  bool GlobalConfig::loadSettings( QSettings &settings )
  {
    bool retval = true;
    qDebug().noquote() << QLatin1String( "" );
    //
    // Öffne die Gruppe Logeinstellungen als allererstes
    //
    settings.beginGroup( groupName );
    //
    // Lese den Dateinamen for das Logfile
    //
    logFile = settings.value( logFileKey, QLatin1String( "" ) ).toString();
    if ( logFile.isEmpty() || logFile.isNull() )
    {
      settings.endGroup();
      return ( false );
    }
    qDebug().noquote().nospace() << QLatin1String( "log file: <" ) << logFile << ">";
    //
    // Lese die Loggerstufe
    //
    loglevel = LoggingUtils::thresholdNames.value( settings.value( loglevelKey, QLatin1String( "warning" ) ).toString().toLower() );
    qDebug().noquote().nospace() << QLatin1String( "log level: <" ) << LoggingUtils::thresholdNames.key( loglevel )
                                 << QLatin1String( ">" );
    //
    // loggen zur Konsole
    //
    logToConsole = settings.value( logToConsoleKey, QLatin1String( "true" ) ).toBool();
    qDebug().noquote().nospace() << QLatin1String( "log to console: <" ) << logToConsole << QLatin1String( ">" );
    //
    // server port
    //
    serverPort = static_cast< qint16 >( settings.value( serverPortKey, QLatin1String( "26106" ) ).toInt() & 0xffff );
    qDebug().noquote().nospace() << QLatin1String( "server port: <" ) << serverPort << QLatin1String( ">" );
    //
    // server Hostaddr
    //
    serverAddr = QHostAddress( settings.value( serverAddrKey, QLatin1String( "localhost" ) ).toString() );
    qDebug().noquote().nospace() << QLatin1String( "server host addr: <" ) << serverAddr.toString() << QLatin1String( ">" );
    //
    // Time zone
    //
    timeZone = QTimeZone( settings.value( timeZoneKey, QLatin1String( "UTC+01:00" ) ).toByteArray() );
    qDebug().noquote().nospace() << QLatin1String( "time zone is: <" ) << timeZone.id() << QLatin1String( ">" );
    //
    // lautstärke sanft als default?
    //
    raiseVol = settings.value( raiseVolKey, QLatin1String( "false" ) ).toBool();
    qDebug().noquote().nospace() << QLatin1String( "raise vol default is: <" ) << raiseVol << QLatin1String( ">" );
    //
    // network timeout
    //
    networkTimeout = static_cast< qint16 >( settings.value( networkTimeoutKey, QLatin1String( "10" ) ).toInt() & 0xffff );
    qDebug().noquote().nospace() << QLatin1String( "network timeout is: <" ) << networkTimeout << QLatin1String( ">" );
    //
    // autorefresh
    //
    autorefresh = static_cast< qint16 >( settings.value( autorefreshKey, QLatin1String( "5" ) ).toInt() & 0xffff );
    qDebug().noquote().nospace() << QLatin1String( "auto refresh is: <" ) << autorefresh << QLatin1String( ">" );
    //
    // gui extra header
    //
    guiHeader = settings.value( guiHeaderKey, QLatin1String( "Radio Alert" ) ).toString();
    qDebug().noquote().nospace() << QLatin1String( "gui extra header: <" ) << guiHeader << QLatin1String( ">" );
    //
    // gui extra bottom 1 und 2
    //
    guiExtraBottom1 = settings.value( guiExtraBottom1Key, QLatin1String( "RADIOALERT" ) ).toString();
    qDebug().noquote().nospace() << QLatin1String( "gui extra bottom line 1: <" ) << guiExtraBottom1 << QLatin1String( ">" );
    guiExtraBottom2 = settings.value( guiExtraBottom2Key, QLatin1String( "for Soundtouch devices" ) ).toString();
    qDebug().noquote().nospace() << QLatin1String( "gui extra bottom line 2: <" ) << guiExtraBottom2 << QLatin1String( ">" );
    //
    // Gui devices file (discovered devices)
    //
    devicesFile = settings.value( devicesFileKey, QLatin1String( "." ) ).toString();
    qDebug().noquote().nospace() << QLatin1String( "devices file: <" ) << devicesFile << QLatin1String( ">" );
    //
    // gui theme
    //
    guiTheme = settings.value( guiThemeKey, QLatin1String( "a" ) ).toString();
    qDebug().noquote().nospace() << QLatin1String( "gui theme: <" ) << guiTheme << QLatin1String( ">" );
    //
    // Ende der Gruppe
    //
    settings.endGroup();
    //
    // Ergebnis kommunizieren
    //
    return ( retval );
  }

  bool GlobalConfig::saveSettings( QSettings &settings )
  {
    qDebug().noquote() << QLatin1String( "" );
    qDebug().noquote() << QLatin1String( "save globel settings...." );
    settings.beginGroup( groupName );
    settings.setValue( logFileKey, logFile );
    settings.setValue( logToConsoleKey, logToConsole );
    settings.setValue( serverPortKey, serverPort );
    settings.setValue( serverAddrKey, serverAddr.toString() );
    settings.setValue( timeZoneKey, QString( timeZone.id() ) );
    settings.setValue( raiseVolKey, raiseVol );
    settings.setValue( networkTimeoutKey, networkTimeout );
    settings.setValue( autorefreshKey, autorefresh );
    settings.setValue( guiExtraBottom1Key, guiExtraBottom1 );
    settings.setValue( guiExtraBottom2Key, guiExtraBottom2 );
    settings.setValue( loglevelKey, LoggingUtils::thresholdNames.key( loglevel ) );
    settings.setValue( guiHeaderKey, guiHeader );
    settings.setValue( devicesFileKey, devicesFile );
    settings.setValue( guiThemeKey, guiTheme );
    settings.endGroup();
    return ( true );
  }

  bool GlobalConfig::makeDefaultSettings( QSettings &settings )
  {
    qDebug().noquote() << "";
    settings.beginGroup( groupName );
    //
    // alle Einstellungen leeren
    //
    qDebug().noquote() << QLatin1String( "remove old settings, if there one present..." );
    settings.remove( logFileKey );
    settings.remove( logToConsoleKey );
    settings.remove( serverPortKey );
    settings.remove( serverAddrKey );
    settings.remove( timeZoneKey );
    settings.remove( raiseVolKey );
    settings.remove( networkTimeoutKey );
    settings.remove( autorefreshKey );
    settings.remove( guiExtraBottom1Key );
    settings.remove( guiExtraBottom2Key );
    settings.remove( loglevelKey );
    settings.remove( guiHeaderKey );
    settings.remove( devicesFileKey );
    settings.remove( guiThemeKey );
    //
    // neues Logfile setzen
    //
    qDebug().noquote() << QLatin1String( "set an value for log file..." );
    logFile = QCoreApplication::applicationName().append( QLatin1String( ".log" ) );
    settings.setValue( logFileKey, logFile );
    settings.endGroup();
    return ( loadSettings( settings ) );
  }

  QByteArray GlobalConfig::serialize( void )
  {
    QString serializeStr( QString( "%1" ).arg( serverPort, 5, 10, QChar( '0' ) ) );
    serializeStr += serverAddr.toString();
    serializeStr += logToConsole ? QLatin1String( "true" ) : QLatin1String( "false" );
    serializeStr += timeZone.id();
    serializeStr += raiseVol ? QLatin1String( "true" ) : QLatin1String( "false" );
    serializeStr += QString( "%1" ).arg( networkTimeout, 5, 10, QChar( '0' ) );
    serializeStr += QString( "%1" ).arg( autorefresh, 5, 10, QChar( '0' ) );
    serializeStr += guiExtraBottom1 + guiExtraBottom2 + logFile;
    serializeStr += QString( "%1" ).arg( static_cast< qint8 >( loglevel ), 2, 10, QChar( '0' ) );
    serializeStr += path1 + path2 + guiHeader + devicesFile + guiTheme;
    // Rückmeldung
    return ( serializeStr.toUtf8() );
  }

  qint16 GlobalConfig::getServerPort() const
  {
    return serverPort;
  }

  void GlobalConfig::setServerPort( const qint16 &value )
  {
    serverPort = value;
  }

  QString GlobalConfig::getGuiExtraBottom1() const
  {
    return guiExtraBottom1;
  }

  void GlobalConfig::setGuiExtraBottom1( const QString &value )
  {
    guiExtraBottom1 = value;
  }

  QString GlobalConfig::getGuiExtraBottom2() const
  {
    return guiExtraBottom2;
  }

  void GlobalConfig::setGuiExtraBottom2( const QString &value )
  {
    guiExtraBottom2 = value;
  }

  QString GlobalConfig::getLogFile() const
  {
    return logFile;
  }

  void GlobalConfig::setLogFile( const QString &value )
  {
    logFile = value;
  }

  bool GlobalConfig::getLogToConsole() const
  {
    return logToConsole;
  }

  void GlobalConfig::setLogToConsole( bool value )
  {
    logToConsole = value;
  }

  LoggingThreshold GlobalConfig::getLoglevel() const
  {
    return loglevel;
  }

  void GlobalConfig::setLoglevel( const LoggingThreshold &value )
  {
    loglevel = value;
  }

  QString GlobalConfig::getPath1() const
  {
    return path1;
  }

  void GlobalConfig::setPath1( const QString &value )
  {
    path1 = value;
  }

  QString GlobalConfig::getPath2() const
  {
    return path2;
  }

  void GlobalConfig::setPath2( const QString &value )
  {
    path2 = value;
  }

  QTimeZone GlobalConfig::getTimeZone() const
  {
    return timeZone;
  }

  void GlobalConfig::setTimeZone( const QTimeZone &value )
  {
    timeZone = value;
  }

  bool GlobalConfig::getRaiseVol() const
  {
    return raiseVol;
  }

  void GlobalConfig::setRaiseVol( bool value )
  {
    raiseVol = value;
  }

  QString GlobalConfig::getGuiHeader() const
  {
    return guiHeader;
  }

  void GlobalConfig::setGuiHeader( const QString &value )
  {
    guiHeader = value;
  }

  qint16 GlobalConfig::getNetworkTimeout() const
  {
    return networkTimeout;
  }

  void GlobalConfig::setNetworkTimeout( const qint16 &value )
  {
    networkTimeout = value;
  }

  qint16 GlobalConfig::getAutorefresh() const
  {
    return autorefresh;
  }

  void GlobalConfig::setAutorefresh( const qint16 &value )
  {
    autorefresh = value;
  }

  QHostAddress GlobalConfig::getServerAddr() const
  {
    return serverAddr;
  }

  void GlobalConfig::setServerAddr( const QHostAddress &value )
  {
    serverAddr = value;
  }

  QString GlobalConfig::getDevicesFile() const
  {
    return devicesFile;
  }

  void GlobalConfig::setDevicesFile( const QString &value )
  {
    devicesFile = value;
  }

  QString GlobalConfig::getGuiTheme() const
  {
    return guiTheme;
  }

  void GlobalConfig::setGuiTheme( const QString &value )
  {
    guiTheme = value;
  }

}  // namespace radioalert
