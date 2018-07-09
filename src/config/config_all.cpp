#include "globalconfig.hpp"
#include <QCoreApplication>
#include <QtDebug>

namespace radioalert
{
  const QString GlobalConfig::groupName( "global" );
  const QString GlobalConfig::logFileKey( "logfile" );
  const QString GlobalConfig::logToConsoleKey( "log_to_console" );
  const QString GlobalConfig::serverPortKey( "server_port" );
  const QString GlobalConfig::serverAddrKey( "server_addr" );
  const QString GlobalConfig::timeZoneKey( "timezone" );
  const QString GlobalConfig::raiseVolKey( "raise_vol" );
  const QString GlobalConfig::networkTimeoutKey( "network_timeout" );
  const QString GlobalConfig::autorefreshKey( "autorefresh" );
  const QString GlobalConfig::guiExtraBottom1Key( "gui_extra_bottom1" );
  const QString GlobalConfig::guiExtraBottom2Key( "gui_extra_bottom2" );
  const QString GlobalConfig::loglevelKey( "loglevel" );
  const QString GlobalConfig::path1Key( "path1" );
  const QString GlobalConfig::path2Key( "path2" );
  const QString GlobalConfig::guiHeaderKey( "gui_header" );
  const QString GlobalConfig::devicesFileKey( "devices_file" );
  const QString GlobalConfig::guiThemeKey( "gui_theme" );

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
    qDebug().noquote() << "";
    //
    // Öffne die Gruppe Logeinstellungen als allererstes
    //
    settings.beginGroup( groupName );
    //
    // Lese den Dateinamen for das Logfile
    //
    logFile = settings.value( logFileKey, "" ).toString();
    if ( logFile.isEmpty() || logFile.isNull() )
    {
      settings.endGroup();
      return ( false );
    }
    qDebug().noquote().nospace() << "log file: <" << logFile << ">";
    //
    // Lese die Loggerstufe
    //
    loglevel = LoggingUtils::thresholdNames.value( settings.value( loglevelKey, "warning" ).toString().toLower() );
    qDebug().noquote().nospace() << "log level: <" << LoggingUtils::thresholdNames.key( loglevel ) << ">";
    //
    // loggen zur Konsole
    //
    logToConsole = settings.value( logToConsoleKey, "true" ).toBool();
    qDebug().noquote().nospace() << "log to console: <" << logToConsole << ">";
    //
    // server port
    //
    serverPort = static_cast< qint16 >( settings.value( serverPortKey, "26106" ).toInt() & 0xffff );
    qDebug().noquote().nospace() << "server port: <" << serverPort << ">";
    //
    // server Hostaddr
    //
    serverAddr = QHostAddress( settings.value( serverAddrKey, "localhost" ).toString() );
    qDebug().noquote().nospace() << "server host addr: <" << serverAddr.toString() << ">";
    //
    // Time zone
    //
    timeZone = QTimeZone( settings.value( timeZoneKey, "UTC+01:00" ).toByteArray() );
    qDebug().noquote().nospace() << "time zone is: <" << timeZone.id() << ">";
    //
    // lautstärke sanft als default?
    //
    raiseVol = settings.value( raiseVolKey, "false" ).toBool();
    qDebug().noquote().nospace() << "raise vol default is: <" << raiseVol << ">";
    //
    // network timeout
    //
    networkTimeout = static_cast< qint16 >( settings.value( networkTimeoutKey, "10" ).toInt() & 0xffff );
    qDebug().noquote().nospace() << "network timeout is: <" << networkTimeout << ">";
    //
    // autorefresh
    //
    autorefresh = static_cast< qint16 >( settings.value( autorefreshKey, "5" ).toInt() & 0xffff );
    qDebug().noquote().nospace() << "auto refresh is: <" << autorefresh << ">";
    //
    // gui extra header
    //
    guiHeader = settings.value( guiHeaderKey, "Radio Alert" ).toString();
    qDebug().noquote().nospace() << "gui extra header: <" << guiHeader << ">";
    //
    // gui extra bottom 1 und 2
    //
    guiExtraBottom1 = settings.value( guiExtraBottom1Key, "RADIOALERT" ).toString();
    qDebug().noquote().nospace() << "gui extra bottom line 1: <" << guiExtraBottom1 << ">";
    guiExtraBottom2 = settings.value( guiExtraBottom2Key, "for Soundtouch devices" ).toString();
    qDebug().noquote().nospace() << "gui extra bottom line 2: <" << guiExtraBottom2 << ">";
    //
    // path 1 und 2
    //
    path1 = settings.value( path1Key, "./" ).toString();
    qDebug().noquote().nospace() << "path 1: <" << path1 << ">";
    path2 = settings.value( path2Key, "./" ).toString();
    qDebug().noquote().nospace() << "path 2: <" << path2 << ">";
    //
    // Gui devices file (discovered devices)
    //
    devicesFile = settings.value( devicesFileKey, "." ).toString();
    qDebug().noquote().nospace() << "devices file: <" << devicesFile << ">";
    //
    // gui theme
    //
    guiTheme = settings.value( guiThemeKey, "a" ).toString();
    qDebug().noquote().nospace() << "gui theme: <" << guiTheme << ">";
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
    qDebug().noquote() << "";
    qDebug().noquote() << "save globel settings....";
    settings.beginGroup( groupName );
    settings.setValue( logFileKey, logFile );
    settings.setValue( logToConsoleKey, logToConsole );
    settings.setValue( serverPortKey, serverPort );
    settings.setValue( serverAddrKey, serverAddr.toString() );
    settings.setValue( timeZoneKey, timeZone.id() );
    settings.setValue( raiseVolKey, raiseVol );
    settings.setValue( networkTimeoutKey, networkTimeout );
    settings.setValue( autorefreshKey, autorefresh );
    settings.setValue( guiExtraBottom1Key, guiExtraBottom1 );
    settings.setValue( guiExtraBottom2Key, guiExtraBottom2 );
    settings.setValue( loglevelKey, LoggingUtils::thresholdNames.key( loglevel ) );
    settings.setValue( path1Key, path1 );
    settings.setValue( path2Key, path2 );
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
    qDebug().noquote() << "remove old settings, if there one...";
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
    settings.remove( path1Key );
    settings.remove( path2Key );
    settings.remove( guiHeaderKey );
    settings.remove( devicesFileKey );
    settings.remove( guiThemeKey );
    //
    // neues Logfile setzen
    //
    qDebug().noquote() << "set an value for log file...";
    logFile = QCoreApplication::applicationName().append( ".log" );
    settings.setValue( logFileKey, logFile );
    settings.endGroup();
    return ( loadSettings( settings ) );
  }

  QByteArray GlobalConfig::serialize( void )
  {
    QString serializeStr( QString( "%1" ).arg( serverPort, 5, 10, QChar( '0' ) ) );
    serializeStr += serverAddr.toString();
    serializeStr += logToConsole ? "true" : "false";
    serializeStr += timeZone.id();
    serializeStr += raiseVol ? "true" : "false";
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
