#include "udpcontrolprocess.hpp"

namespace radioalert
{
  const QString UdpControlProcess::okString{"ok"};
  const QString UdpControlProcess::errString{"error"};
  const QString UdpControlProcess::alertRootStr{"alert-"};
  const QRegExp UdpControlProcess::alertExpr{"^alert-\\d{2}$"};
  const QRegExp UdpControlProcess::noAlertExpr{"^(config-id|config|devices|new)$"};
  const QString UdpControlProcess::cmdGet{"get"};
  const QString UdpControlProcess::cmdGetAllAlerts{"all"};
  const QString UdpControlProcess::cmdGetConfigId{"config-id"};
  const QString UdpControlProcess::cmdGetConfigVersion{"version"};
  const QString UdpControlProcess::cmdGetConfig{"config"};
  const QString UdpControlProcess::cmdGetDevices{"devices"};
  const QString UdpControlProcess::cmdGetNew{"new"};
  const QString UdpControlProcess::cmdSet{"set"};
  const QString UdpControlProcess::cmdSetAlertName{"alert"};
  const QString UdpControlProcess::cmdAlertDelete{"del"};

  //###########################################################################
  //###########################################################################
  //#### Objektverwaltung
  //###########################################################################
  //###########################################################################

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

  void UdpControlProcess::init()
  {
    LGDEBUG( "UdpControlProcess::start: connect signals..." );
    connect( udpSocket.get(), &QUdpSocket::readyRead, this, &UdpControlProcess::slotReadPendingDatagrams );
    LGDEBUG( "UdpControlProcess::start: OK" );
  }

  void UdpControlProcess::requestQuit()
  {
    LGINFO( "UdpControlProcess::requestQuit..." );
    //
    // Alle eventuell vorhandenen Alarme beenden
    //
    disconnect( udpSocket.get(), nullptr, nullptr, nullptr );
    udpSocket->close();
    //
    // Aufräumen dem System/ der runtime überlassen :-)
    //
    LGINFO( "UdpControlProcess::requestQuit...OK" );
    emit closeUdpServer();
  }

  //###########################################################################
  //#### SLOTS für UDP
  //###########################################################################

  void UdpControlProcess::slotReadPendingDatagrams()
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
          // C++11
          // for ( QStringList::iterator strId = keys.begin(); strId != keys.end(); strId++ )
          for ( auto &strId : keys )
          {
            LGDEBUG( QString( "UdpControlProcess::slotRreadPendingDatagrams -> key <%1> in json document..." ).arg( strId ) );
            //
            // welches Kommando ist hier angekommen?
            //
            if ( cmdGet.compare( strId ) == 0 )
            {
              //
              // Kommando GET (für einen /alle Alarme )
              //
              QByteArray sendDatagram( computeGetCommand( bObject.value( strId ) ) );
              //
              // return to sender
              //
              if ( sendDatagram.count() > 0 )
              {
                LGDEBUG( "UdpControlProcess::slotRreadPendingDatagrams -> send answer for alert get request..." );
                udpSocket->writeDatagram( sendDatagram, datagram.senderAddress(), datagram.senderPort() );
              }
            }
            else if ( cmdSet.compare( strId ) == 0 )
            {
              //
              // Kommando SET (für Alarm(e) )
              //
              QByteArray sendDatagram( computeSetCommand( bObject.value( strId ) ) );
              //
              // return to sender
              //
              if ( sendDatagram.count() > 0 )
              {
                LGDEBUG( "UdpControlProcess::slotRreadPendingDatagrams -> send answer for alert set request..." );
                udpSocket->writeDatagram( sendDatagram, datagram.senderAddress(), datagram.senderPort() );
              }
            }
            else if ( cmdAlertDelete.compare( strId ) == 0 )
            {
              //
              // Kommando DELETE (für Alarm(e)
              //
              QByteArray sendDatagram( computeDelCommand( bObject.value( strId ) ) );
              //
              // return to sender
              //
              if ( sendDatagram.count() > 0 )
              {
                LGDEBUG( "UdpControlProcess::slotRreadPendingDatagrams -> send answer for alert del request..." );
                udpSocket->writeDatagram( sendDatagram, datagram.senderAddress(), datagram.senderPort() );
              }
              return;
            }
          }
        }
      }
    }
  }

  //###########################################################################
  //#### GET Kommandos bearbeiten
  //###########################################################################

  QByteArray UdpControlProcess::computeGetCommand( const QJsonValue &jsonVal ) const
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
      // C++11
      // for ( QJsonArray::Iterator it = arr.begin(); it != arr.end(); it++ )
      for ( auto &&js : arr )
      {
        commandList.append( ( js ).toString() );
      }
      //
      // ist eine "all" Anforderung für alle Alarme?
      // wenn nicht, schaue ob einzelne alarme angefordert wurden
      //
      if ( commandList.contains( cmdGetAllAlerts ) )
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
        // C#++11
        // for ( QStringList::iterator otIter = others.begin(); otIter != others.end(); otIter++ )
        for ( auto &othersCmd : others )
        {
          // config-id|config|devices|new
          // QString cmd( ot );
          LGDEBUG( QString( "UdpControlProcess::computeGetCommand -> get cmd: <%1>..." ).arg( othersCmd ) );
          //
          // config-version?
          //
          if ( cmdGetConfigId.compare( othersCmd ) == 0 )
          {
            //
            // TODO: alarme, welche arbeiten in die Liste aufnehmen
            //
            LGDEBUG( "UdpControlProcess::computeGetCommand -> answer config version hash..." );
            answerObject.insert( "al_working", "none" );
            answerObject.insert( cmdGetConfigVersion, QString( cf->getConfigHashLoad() ) );
            QJsonDocument answerDoc( answerObject );
            QByteArray retArr( answerDoc.toJson() );
            return ( retArr );
          }
          //
          // config
          //
          else if ( cmdGetConfig.compare( othersCmd ) == 0 )
          {
            LGDEBUG( "UdpControlProcess::computeGetCommand -> compute get global request..." );
            return ( computeGetGlobalConfig() );
          }
          //
          // devices (available)
          //
          else if ( cmdGetDevices.compare( othersCmd ) == 0 )
          {
            LGDEBUG( "UdpControlProcess::computeGetCommand -> compute get devices request..." );
            return ( computeGetDevices() );
          }
          else if ( cmdGetNew.compare( othersCmd ) == 0 )
          {
            LGDEBUG( "UdpControlProcess::computeGetCommand -> compute get new alert request..." );
            return ( computeGetNew() );
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
    // C++11
    // for ( QStringList::iterator alIter = alerts.begin(); alIter != alerts.end(); alIter++ )
    for ( auto &alertName : alerts )
    {
      // QString alertName( alIter );
      int alertNumber = alertName.remove( alertRootStr ).toInt();
      LGDEBUG( QString( "UdpControlProcess::computeGetCommand -> get alert: <%1>..." ).arg( alertName ) );
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

  QByteArray UdpControlProcess::computeGetGlobalConfig() const
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
    LGDEBUG( "MainDaemon::computeGetGlobalConfig" );
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

  QByteArray UdpControlProcess::computeGetDevices() const
  {
    AvailableDevices tempDev;
    QJsonObject answerObject;
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
      StDevicesHashList avDevices = tempDev.getDevicesList();
      // C++11
      // for ( StDevicesHashList::iterator dListIt = avDevices.begin(); dListIt != avDevices.end(); dListIt++ )
      for ( auto &avDevice : avDevices )
      {
        // instanziere ein lokales Objekt
        QJsonObject ajObj;
        ajObj.insert( "id", avDevice.deviceId );
        ajObj.insert( "name", avDevice.deviceName );
        ajObj.insert( "type", avDevice.deviceType );
        ajObj.insert( "host", avDevice.hostName );
        ajObj.insert( "httpport", avDevice.httpPort );
        ajObj.insert( "wsport", avDevice.wsPort );
        answerObject.insert( avDevice.deviceName, ajObj );
      }
      QJsonDocument answerDoc( answerObject );
      QByteArray retArr( answerDoc.toJson() );
      return ( retArr );
    }
    catch ( ConfigfileNotExistException &ex )
    {
      //
      // Fehlermeldung loggen!
      //
      LGCRIT( QString( "Cant read devices from file: " ).append( ex.getMessage() ) );
      return ( QByteArray() );
    }
    return ( QByteArray() );
  }

  QByteArray UdpControlProcess::computeGetNew() const
  {
    //
    // suche einen noch freien alarmnamen raus
    //
    QStringList alerts;
    LGDEBUG( "UdpControlProcess::computeGetNew" );
    alerts = cf->getAlertList().keys();
    for ( int alCounter = 1; alCounter < 999; alCounter++ )
    {
      QString alertName = QString( "%1%2" ).arg( alertRootStr ).arg( alCounter, 2, 10, QChar( '0' ) );
      if ( alerts.contains( alertName ) )
      {
        //
        // der alarm existiert, weiter...
        //
        continue;
      }
      //
      // erzeuge einen neuen, leeren Alarm. Standart "disabled"
      // durch das config object wird dann die Speicherung gehandhabt
      //
      QJsonObject answerObject;
      SingleAlertConfig currAlert;
      QJsonObject ajObj;
      currAlert.setAlertName( alertName );
      currAlert.setAlertNote( "new alert, disabled" );
      cf->getAlertList().insert( alertName, currAlert );
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
      LGDEBUG( QString( "UdpControlProcess::computeGetNew -> construct data for alert: <%1>...OK" ).arg( alertName ) );
      QJsonDocument answerDoc( answerObject );
      QByteArray retArr( answerDoc.toJson() );
      return ( retArr );
    }
    return ( QByteArray() );
  }

  //###########################################################################
  //#### SET Kommandos bearbeiten
  //###########################################################################

  QByteArray UdpControlProcess::computeSetCommand( const QJsonValue &jsonVal ) const
  {
    //
    // ich erwarte ein array, editiere einen/mehrere vorhandenen alarm
    // set editert NUR Alarme
    // Parameter können sein
    // [alert-\d\d]
    // {"set":[{"alert":"alert-01", "enable":"true"...},{"alert":"alert-02", "enable":"true"...}]}
    //
    LGDEBUG( "UdpControlProcess::computeSetCommand..." );
    if ( jsonVal.isArray() )
    {
      QJsonObject answerObject;
      bool isOk = true;
      QStringList alerts = cf->getAlertList().keys();  // Liste der konfigurierten Alarme
      QStringList sList;
      //
      // get objekt ist ein Array
      // also caste zum Array
      QJsonArray alertSetArray = jsonVal.toArray();
      // C++11
      // for ( QJsonArray::iterator alIter = alertSetArray.begin(); alIter != alertSetArray.end(); alIter++ )
      for ( auto &&alertSet : alertSetArray )
      {
        if ( ( alertSet ).isObject() )
        {
          //
          // das ist dann (hoffentlich) ein Objekt zum Setzen von Parametern
          //
          QJsonObject alertSetObj = ( alertSet ).toObject();
          if ( alertSetObj.keys().contains( cmdSetAlertName ) )
          {
            QString alertName = alertSetObj.value( cmdSetAlertName ).toString();
            if ( alerts.contains( alertName ) )
            {
              //
              // der Alarm ist vorhanden, nun bearbeite ihn auch mal
              //
              SingleAlertConfig &currAlert = cf->getAlertList()[ alertName ];
              //
              // alle durch...
              //
              // Datum
              if ( alertSetObj.keys().contains( AlertConfig::dateKey ) )
              {
                currAlert.setAlertDate(
                    QDate::fromString( alertSetObj.value( AlertConfig::dateKey ).toString(), AlertConfig::dateFormatToken ) );
              }
              // Tage
              if ( alertSetObj.keys().contains( AlertConfig::daysKey ) )
              {
                sList = alertSetObj.value( AlertConfig::daysKey ).toString().split( ',' );
                currAlert.setAlertDays( sList );
              }
              // raise vol
              if ( alertSetObj.keys().contains( AlertConfig::raiseVolKey ) )
              {
                currAlert.setAlertRaiseVol( alertSetObj.value( AlertConfig::raiseVolKey ).toBool() );
              }
              // devices
              if ( alertSetObj.keys().contains( AlertConfig::devicesKey ) )
              {
                sList = alertSetObj.value( AlertConfig::devicesKey ).toString().split( ',' );
                currAlert.setAlertDays( sList );
              }
              // duration
              if ( alertSetObj.keys().contains( AlertConfig::durationKey ) )
              {
                currAlert.setAlertDuration( static_cast< qint16 >( alertSetObj.value( AlertConfig::durationKey ).toInt() ) );
              }
              // location
              if ( alertSetObj.keys().contains( AlertConfig::locationKey ) )
              {
                currAlert.setAlertLocation( alertSetObj.value( AlertConfig::locationKey ).toString() );
              }
              // alert enable
              if ( alertSetObj.keys().contains( AlertConfig::alertEnableKey ) )
              {
                currAlert.setAlertRaiseVol( alertSetObj.value( AlertConfig::alertEnableKey ).toBool() );
              }
              // Alarm type
              if ( alertSetObj.keys().contains( AlertConfig::typeKey ) )
              {
                currAlert.setAlertType( alertSetObj.value( AlertConfig::typeKey ).toString() );
              }
              // alarm volume
              if ( alertSetObj.keys().contains( AlertConfig::volumeKey ) )
              {
                currAlert.setAlertVolume( static_cast< qint8 >( alertSetObj.value( AlertConfig::volumeKey ).toInt() ) );
              }
              // alarm bemerkung
              if ( alertSetObj.keys().contains( AlertConfig::noteKey ) )
              {
                currAlert.setAlertNote( alertSetObj.value( AlertConfig::noteKey ).toString() );
              }
              // alarm Zeit
              if ( alertSetObj.keys().contains( AlertConfig::timeKey ) )
              {
                currAlert.setAlertTime(
                    QTime::fromString( alertSetObj.value( AlertConfig::timeKey ).toString(), AlertConfig::timeFormatToken ) );
              }
              // alarm radio source
              if ( alertSetObj.keys().contains( AlertConfig::sourceKey ) )
              {
                currAlert.setAlertSource( alertSetObj.value( AlertConfig::sourceKey ).toString() );
              }
              // alarm radio source account (amazon, linkedin etc)
              if ( alertSetObj.keys().contains( AlertConfig::sourceAccountKey ) )
              {
                currAlert.setAlertSourceAccount( alertSetObj.value( AlertConfig::sourceAccountKey ).toString() );
              }
            }
          }
          else
          {
            LGWARN( "UdpControlProcess::computeSetCommand -> found set object is not an alert! Ignore!" );
          }
        }
        else
        {
          LGWARN( "UdpControlProcess::computeSetCommand -> found object is not an alert-set object!" );
          isOk = false;
        }
      }
      //
      // Antwort zusammen bauen
      //
      if ( isOk )
      {
        answerObject.insert( okString, "alert(s) successful set" );
      }
      else
      {
        answerObject.insert( errString, "alert(s) NOT successful set" );
      }
      QJsonDocument answerDoc( answerObject );
      QByteArray retArr( answerDoc.toJson() );
      return ( retArr );
    }
    return ( QByteArray() );
  }

  //###########################################################################
  //#### DELETE Kommandos bearbeiten
  //###########################################################################

  QByteArray UdpControlProcess::computeDelCommand( const QJsonValue &jsonVal ) const
  {
    //
    // ich erwarte ein array, editiere einen/mehrere vorhandenen alarm
    // set editert NUR Alarme
    // Parameter können sein
    // [alert-\d\d]
    // {"del":["alert-01", "alert-02"]}
    //
    LGDEBUG( "UdpControlProcess::computeDelCommand..." );
    if ( jsonVal.isArray() )
    {
      QJsonObject answerObject;
      QStringList alerts = cf->getAlertList().keys();  // Liste der konfigurierten Alarme
      //
      // get objekt ist ein Array
      // also caste zum Array
      //
      QJsonArray alertDelArray = jsonVal.toArray();
      // C++11
      // for ( QJsonArray::iterator alIter = alertDelArray.begin(); alIter != alertDelArray.end(); alIter++ )
      for ( auto &&alIter : alertDelArray )
      {
        if ( ( alIter ).isString() )
        {
          //
          // hier ist der alarmname zum löschen
          //
          QString alertName = ( alIter ).toString();
          if ( alerts.contains( alertName ) )
          {
            LGDEBUG( QString( "UdpControlProcess::computeDelCommand -> remove <%1> from config..." ).arg( alertName ) );
            cf->getAlertList().remove( alertName );
          }
          else
          {
            LGWARN( QString( "UdpControlProcess::computeDelCommand -> can't remove <%1> from config. It's not there..." )
                        .arg( alertName ) );
          }
        }
      }
      answerObject.insert( okString, "alert(s) successful removed" );
      QJsonDocument answerDoc( answerObject );
      QByteArray retArr( answerDoc.toJson() );
      return ( retArr );
    }
    return ( QByteArray() );
  }
}  // namespace radioalert
