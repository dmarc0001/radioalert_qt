#include "udpcontrolprocess.hpp"

namespace radioalert
{
  const QString UdpControlProcess::alertRootStr{"alert-"};
  const QRegExp UdpControlProcess::alertExpr{"^alert-\\d{2}$"};
  const QString UdpControlProcess::allAlertExpr{"all"};
  const QRegExp UdpControlProcess::noAlertExpr{"^(config-id|config|devices|new)$"};
  const QString UdpControlProcess::commandGet{"get"};
  const QString UdpControlProcess::commandConfigId{"config-id"};
  const QString UdpControlProcess::answerConfigVersion{"version"};
  const QString UdpControlProcess::commandConfig{"config"};
  const QString UdpControlProcess::commandDevices{"devices"};

  UdpControlProcess::UdpControlProcess( std::shared_ptr< Logger > logger, std::shared_ptr< AppConfigClass > config, QObject *parent )
      : QObject( parent )
      , lg( logger )
      , cf( config )
      , udpSocket( std::unique_ptr< QUdpSocket >( new QUdpSocket( this ) ) )
      , isRunning( true )
  {
    LGINFO( QString( "UdpControlProcess::UdpControlProcess: bind on udp <%1:%2>" )
                .arg( cf->getGlobalConfig().getServerAddr().toString() )
                .arg( cf->getGlobalConfig().getServerPort() ) );
    udpSocket->bind( cf->getGlobalConfig().getServerAddr(), cf->getGlobalConfig().getServerPort() );
    LGINFO( "UdpControlProcess::UdpControlProcess: bind on udp OK" );
  }

  void UdpControlProcess::init( void )
  {
    LGDEBUG( "UdpControlProcess::start: connect signals..." );
    connect( udpSocket.get(), &QUdpSocket::readyRead, this, &UdpControlProcess::slotReadPendingDatagrams );
    LGDEBUG( "UdpControlProcess::start: OK" );
  }

  void UdpControlProcess::requestQuit( void )
  {
    LGINFO( "UdpControlProcess::requestQuit..." );
    //
    // Alle eventuell vorhandenen Alarme beenden
    //
    disconnect( udpSocket.get(), 0, 0, 0 );
    udpSocket->close();
    //
    // Aufräumen dem System/ der runtime überlassen :-)
    //
    LGINFO( "UdpControlProcess::requestQuit...OK" );
    emit closeUdpServer();
  }

  void UdpControlProcess::slotReadPendingDatagrams( void )
  {
    LGDEBUG( "UdpControlProcess::slotRreadPendingDatagrams..." );
    while ( udpSocket->hasPendingDatagrams() )
    {
      QNetworkDatagram datagram = udpSocket->receiveDatagram();
      if ( !datagram.isNull() && datagram.isValid() )
      {
        QJsonParseError err;
        LGDEBUG( "UdpControlProcess::slotRreadPendingDatagrams -> create QJsonObject..." );
        QJsonDocument bDocument( QJsonDocument::fromJson( datagram.data(), &err ) );
        LGDEBUG( QString( "DATA: ####<%1>####" ).arg( QString( datagram.data() ) ) );
        if ( bDocument.isObject() )
        {
          QJsonObject bObject( bDocument.object() );
          QStringList keys = bObject.keys();
          //
          // für alle keys machen
          //
          for ( QStringList::iterator strId = keys.begin(); strId != keys.end(); strId++ )
          {
            LGDEBUG( QString( "UdpControlProcess::slotRreadPendingDatagrams -> key <%1> in json document..." ).arg( *strId ) );
            //
            // welches Kommando ist hier angekommen?
            //
            if ( commandGet.compare( *strId ) == 0 )
            {
              //
              // Kommando GET (für einen /alle Alarme
              //
              QByteArray sendDatagram( computeGetCommand( bObject.value( *strId ) ) );
              //
              // return to sender
              //
              if ( sendDatagram.count() > 0 )
              {
                LGDEBUG( "UdpControlProcess::slotRreadPendingDatagrams -> send answer for alert condig request..." );
                udpSocket->writeDatagram( sendDatagram, datagram.senderAddress(), datagram.senderPort() );
              }
              return;
            }
          }
        }
      }
    }
  }

  QByteArray UdpControlProcess::computeGetCommand( QJsonValue jsonVal ) const
  {
    //
    // ich erwarte ein array
    // Parameter können sein
    // [alert-\d\d|config-id|config|all|devices|new]
    //
    if ( jsonVal.isArray() )
    {
      QStringList alerts;
      //
      // was will ich antworten?
      //
      QJsonObject answerObject;
      //
      LGDEBUG( "UdpControlProcess::computeGetCommand -> read param array..." );
      QJsonArray arr( jsonVal.toArray() );
      QStringList commandList;
      for ( QJsonArray::Iterator it = arr.begin(); it != arr.end(); it++ )
      {
        commandList.append( ( *it ).toString() );
      }
      //
      // ist eine "all" Anforderung für alle Alarme?
      // wenn nicht, schaue ob einzelne alarme angefordert wurden
      //
      if ( commandList.contains( allAlertExpr ) )
      {
        //
        // Alle Alarme finden
        //
        alerts = cf->getAlertList().keys();
      }
      else
      {
        //
        // durchsuche erst mal ob Alarme drin sind
        // die können mehrfach auftauchen
        //
        alerts = commandList.filter( alertExpr );
      }
      //
      // alle anderen
      //
      QStringList others( commandList.filter( noAlertExpr ) );
      //
      // alarme, wenn vorhanden
      //
      if ( alerts.count() > 0 )
      {
        return ( computeGetAlerts( alerts ) );
      }
      else
      {
        for ( QStringList::iterator otIter = others.begin(); otIter != others.end(); otIter++ )
        {
          // config-id|config|devices|new
          QString cmd( *otIter );
          LGDEBUG( QString( "UdpControlProcess::computeGetCommand -> get cmd: <%1>..." ).arg( cmd ) );
          //
          // config-version?
          //
          if ( commandConfigId.compare( cmd ) == 0 )
          {
            //
            // TODO: alarme, welche arbeiten in die Liste aufnehmen
            //
            LGDEBUG( "UdpControlProcess::computeGetCommand -> answer config version hash..." );
            answerObject.insert( "al_working", "none" );
            answerObject.insert( answerConfigVersion, QString( cf->getConfigHashLoad() ) );
            QJsonDocument answerDoc( answerObject );
            QByteArray retArr( answerDoc.toJson() );
            return ( retArr );
          }
          //
          // config
          //
          else if ( commandConfig.compare( cmd ) )
          {
            return ( computeGetGlobalConfig() );
          }
          //
          // devices (available)
          //
          else if ( commandDevices.compare( cmd ) )
          {
            return ( computeGetDevices() );
          }
        }
      }
    }  // ende isArray
    else
    {
      LGWARN( "UdpControlProcess::computeGetCommand: params are not an array, ABORT parsing..." );
    }
    //
    // leetes Array zurück...
    //
    return ( QByteArray() );
  }

  QByteArray UdpControlProcess::computeGetAlerts( QStringList &alerts ) const
  {
    //
    // was will ich antworten?
    //
    QJsonObject answerObject;
    //
    // alle angeforderten Alarme bearbeiten
    //
    for ( QStringList::iterator alIter = alerts.begin(); alIter != alerts.end(); alIter++ )
    {
      QString alertName( *alIter );
      int alertNumber = alertName.remove( alertRootStr ).toInt();
      alertName = *alIter;
      LGDEBUG( QString( "UdpControlProcess::computeGetCommand -> get alert: <%1>..." ).arg( *alIter ) );
      LGDEBUG( QString( "UdpControlProcess::computeGetCommand -> is alert: <%1>..." ).arg( alertNumber, 2, 10, QChar( '0' ) ) );
      if ( cf->getAlertList().contains( alertName ) )
      {
        LGDEBUG( QString( "UdpControlProcess::computeGetCommand -> construct data for alert: <%1>..." ).arg( alertName ) );
        //
        // den alarm gab es in der Config
        //
        SingleAlertConfig currAlert( *( cf->getAlertList().find( alertName ) ) );
        // instanziere ein lokales Objekt
        QJsonObject ajObj;
        ajObj.insert( AlertConfig::dateKey, currAlert.getAlertDate().isValid() ? currAlert.getAlertDate().toString() : "" );
        ajObj.insert( AlertConfig::daysKey, currAlert.getAlertDays().empty() ? "" : currAlert.getAlertDays().join( "," ) );
        ajObj.insert( AlertConfig::raiseVolKey, currAlert.getAlertRaiseVol() ? "true" : "false" );
        ajObj.insert( AlertConfig::devicesKey, currAlert.getAlertDevices().empty() ? "" : currAlert.getAlertDevices().join( "," ) );
        ajObj.insert( AlertConfig::durationKey, QString( "%1" ).arg( currAlert.getAlertDuration() ) );
        ajObj.insert( AlertConfig::locationKey, currAlert.getAlertLocation() );
        ajObj.insert( AlertConfig::alertEnableKey, currAlert.getAlertEnable() ? "true" : "false" );
        ajObj.insert( AlertConfig::typeKey, currAlert.getAlertType() );
        ajObj.insert( AlertConfig::volumeKey, QString( "%1" ).arg( static_cast< int >( currAlert.getAlertVolume() ) ) );
        ajObj.insert( AlertConfig::noteKey, currAlert.getAlertNote() );
        ajObj.insert( AlertConfig::timeKey, currAlert.getAlertTime().toString( AlertConfig::timeFormatToken ) );
        ajObj.insert( AlertConfig::sourceKey, currAlert.getAlertSource() );
        ajObj.insert( AlertConfig::sourceAccountKey, currAlert.getAlertSourceAccount() );
        // das Objekt in das answer Objekt kopieren
        answerObject.insert( alertName, ajObj );
        LGDEBUG( QString( "UdpControlProcess::computeGetCommand -> construct data for alert: <%1>...OK" ).arg( alertName ) );
      }
      else
      {
        LGWARN( QString( "UdpControlProcess::computeGetCommand -> alert: <%1> not found in alert config list..." ).arg( alertName ) );
      }
    }  // ende for schleife
    LGDEBUG( "UdpControlProcess::computeGetCommand -> alerts config ready to send back..." );
    QJsonDocument answerDoc( answerObject );
    QByteArray retArr( answerDoc.toJson() );
    // keine weiteren anforderungen bearbeiten, Antwort zurück senden
    return ( retArr );
  }

  QByteArray UdpControlProcess::computeGetGlobalConfig( void ) const
  {
    //
    // was will ich antworten?
    //
    QJsonObject answerObject;
    // instanziere ein lokales Objekt
    QJsonObject ajObj;
    //
    // alle CONFIG Variablen einfügen
    //
    ajObj.insert( GlobalConfig::logFileKey, cf->getGlobalConfig().getLogFile() );
    ajObj.insert( GlobalConfig::logToConsoleKey, cf->getGlobalConfig().getLogToConsole() ? "true" : "false" );
    ajObj.insert( GlobalConfig::serverPortKey, QString( "%1" ).arg( static_cast< int >( cf->getGlobalConfig().getServerPort() ) ) );
    ajObj.insert( GlobalConfig::serverAddrKey, cf->getGlobalConfig().getServerAddr().toString() );
    ajObj.insert( GlobalConfig::timeZoneKey, QString( cf->getGlobalConfig().getTimeZone().id() ) );
    ajObj.insert( GlobalConfig::raiseVolKey, cf->getGlobalConfig().getRaiseVol() ? "true" : "false" );
    ajObj.insert( GlobalConfig::networkTimeoutKey, cf->getGlobalConfig().getNetworkTimeout() );
    ajObj.insert( GlobalConfig::autorefreshKey, QString( "%1" ).arg( static_cast< int >( cf->getGlobalConfig().getAutorefresh() ) ) );
    ajObj.insert( GlobalConfig::guiExtraBottom1Key, cf->getGlobalConfig().getGuiExtraBottom1() );
    ajObj.insert( GlobalConfig::guiExtraBottom2Key, cf->getGlobalConfig().getGuiExtraBottom2() );
    ajObj.insert( GlobalConfig::loglevelKey, LoggingUtils::thresholdNames.key( cf->getGlobalConfig().getLoglevel() ) );
    ajObj.insert( GlobalConfig::guiHeaderKey, cf->getGlobalConfig().getGuiHeader() );
    ajObj.insert( GlobalConfig::devicesFileKey, cf->getGlobalConfig().getDevicesFile() );
    ajObj.insert( GlobalConfig::guiThemeKey, cf->getGlobalConfig().getGuiTheme() );
    // das in das Antwortobjekt inbringen
    answerObject.insert( GlobalConfig::groupGlobalName, ajObj );
    QJsonDocument answerDoc( answerObject );
    QByteArray retArr( answerDoc.toJson() );
    return ( retArr );
  }

  QByteArray UdpControlProcess::computeGetDevices( void ) const
  {
    AvailableDevices tempDev;
    try
    {
      //
      // versuche einzulesen
      //
      LGDEBUG( "MainDaemon::readAvailDevices" );
      tempDev.loadSettings( cf->getGlobalConfig().getDevicesFile() );
      //
      // Objekt kopieren
      //
      avStDevices = tempDev.getDevicesList();
    }
    catch ( ConfigfileNotExistException ex )
    {
      //
      // Fehlermeldung loggen!
      //
      LGCRIT( QString( "Cant read devices from file: " ).append( ex.getMessage() ) );
    }
  }
}  // namespace radioalert
