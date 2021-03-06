#include "asyncalertcommand.hpp"

namespace radioalert
{
  AsyncAlertCommand::AsyncAlertCommand( std::shared_ptr< Logger > logger, QObject *parent )
      : QObject( parent ), lg( logger ), keyresponseCounter( 0 )
  {
  }

  void AsyncAlertCommand::checkIfDeviceInStandby( BoseDevice *masterDevice )
  {
    //
    // conn ist ein Zeiger auf eine Verbindung
    // und wird dann meiner Verbindung zugewiesen (für disconnect)
    //
    auto conn = std::make_shared< QMetaObject::Connection >();
    //
    // Callback einrichten
    //
    *conn = connect( masterDevice, &BoseDevice::sigOnRequestAnswer, [=]( SharedResponsePtr response ) {
      //
      // Kurze Lambda Funktion reicht hier
      // ich interessiere mich nur für eine Antort vom Typ R_NOW_PLAYING
      //
      if ( response->getResultType() == ResultobjectType::R_NOW_PLAYING )
      {
        HttpNowPlayingObject *nPlayObj = static_cast< HttpNowPlayingObject * >( response.get() );
        disconnect( *conn );
        if ( ( nPlayObj->getSource() ).contains( QLatin1String( "STANDBY" ) ) )
        {
          // Ja, ich habe STANDBY
          LGDEBUG( "AsyncAlertCommand::checkIfDeviceInStandby: device is in state STANDBY..." );
          emit sigDeviceIsStandby( true );
        }
        else
        {
          emit sigDeviceIsStandby( false );
        }
      }
    } );
    //
    // und nun nachfragen!
    //
    LGDEBUG( "AsyncAlertCommand::checkIfDeviceInStandby: ask master device..." );
    masterDevice->getNowPlaying();
  }

  void AsyncAlertCommand::switchDeviceToSource( BoseDevice *masterDevice, const QString source )
  {
    //
    // Die Funktion setzt voraus, dass eine Winsocket Verbindung
    // besteht oder hergestellt werden kann, sonst kann der Callback nicht funktionieren
    // conn ist ein Zeiger auf eine Verbindung
    // und wird dann meiner Verbindung zugewiesen (für disconnect)
    //
    masterDevice->connectWs();
    //
    auto conn = std::make_shared< QMetaObject::Connection >();
    //
    // Callback einrichten
    //
    // NOW Playing info
    *conn = connect( masterDevice, &BoseDevice::sigOnNowPlayingUpdated, [=]( SharedResponsePtr response ) {
      //
      // Kurze Lambda Funktion reicht hier
      //
      WsNowPlayingUpdate *nowPlayObj = static_cast< WsNowPlayingUpdate * >( response.get() );
      //
      // Puffer Status?
      //
      if ( nowPlayObj->getPlayStatus().contains( masterDevice->getPlayStateName( BoseDevice::bose_playstate::BUFFERING_STATE ) ) )
      {
        // Puffern?
        LGDEBUG( "device is buffering..." );
        emit sigDeviceIsSwitchedToSource( false );
      }
      //
      // Abspielstatus?
      //
      else if ( nowPlayObj->getPlayStatus().contains( masterDevice->getPlayStateName( BoseDevice::bose_playstate::PLAY_STATE ) ) )
      {
        disconnect( *conn );
        LGDEBUG( "AsyncAlertCommand::switchDeviceToSource: device is PLAY_STATE" );
        emit sigDeviceIsSwitchedToSource( true );
      }
    } );
    //
    // Schalten!
    //
    // Radio einschalten
    // TODO: gilt nur für PRESETS -> Anpassung erforderlich wenn erweitert
    //
    BoseDevice::bose_key preset = masterDevice->getKeyForName( source );
    if ( preset == BoseDevice::bose_key::KEY_UNKNOWN )
    {
      LGWARN( QString( "SingleRadioAlert::switchMasterDeviceToSource: alert source unknown: " ).append( source ) );
      disconnect( *conn );
      emit sigDeviceIsSwitchedToSource( false );
      return;
    }
    LGDEBUG( QString( "SingleRadioAlert::switchMasterDeviceToSource: now switch to source " ).append( source ) );
    masterDevice->setKey( preset, BoseDevice::bose_keystate::KEY_RELEASED );
  }

  void AsyncAlertCommand::askForVolume( BoseDevice *masterDevice )
  {
    //
    // Die Funktion setzt voraus, dass eine Winsocket Verbindung
    // besteht oder hergestellt werden kann, sonst kann der Callback nicht funktionieren
    // conn ist ein Zeiger auf eine Verbindung
    // und wird dann meiner Verbindung zugewiesen (für disconnect)
    //
    masterDevice->connectWs();
    //
    auto conn = std::make_shared< QMetaObject::Connection >();
    //
    // Callback einrichten
    //
    // request answer volume
    *conn = connect( masterDevice, &BoseDevice::sigOnRequestAnswer, [=]( SharedResponsePtr response ) {
      //*conn = connect( masterDevice, &BoseDevice::sigOnVolumeUpdated, [=]( SharedResponsePtr response ) {
      if ( response->getResultType() == ResultobjectType::R_VOLUME )
      {
        HttpVolumeObject *volObj = static_cast< HttpVolumeObject * >( response.get() );
        //
        // Aktuelle Lautstärke lesen
        //
        emit sigDeviceVolume( volObj->getActualVolume() );
      }
    } );
    masterDevice->getVolume();
  }

  void AsyncAlertCommand::switchPowerOff( BoseDevice *masterDevice )
  {
    switchPowerTo( false, masterDevice );
  }

  void AsyncAlertCommand::switchPowerOn( BoseDevice *masterDevice )
  {
    switchPowerTo( true, masterDevice );
  }

  void AsyncAlertCommand::switchPowerTo( bool toOn, BoseDevice *masterDevice )
  {
    //
    // Schritt 1 feststelen welcher Zustand aktuell ist
    //
    // conn ist ein Zeiger auf eine Verbindung
    // und wird dann meiner Verbindung zugewiesen (für disconnect)
    //
    auto conn = std::make_shared< QMetaObject::Connection >();
    //
    // Callback einrichten
    //
    *conn = connect( masterDevice, &BoseDevice::sigOnRequestAnswer, [=]( SharedResponsePtr response ) {
      //
      // Kurze Lambda Funktion reicht hier
      // ich interessiere mich nur für eine Antort vom Typ R_NOW_PLAYING
      //
      if ( response->getResultType() == ResultobjectType::R_NOW_PLAYING )
      {
        HttpNowPlayingObject *nPlayObj = static_cast< HttpNowPlayingObject * >( response.get() );
        disconnect( *conn );
        if ( ( nPlayObj->getSource() ).contains( QLatin1String( "STANDBY" ) ) )
        {
          //
          // ich habe STANDBY
          //
          LGDEBUG( "AsyncAlertCommand::switchPowerTo: device is in state STANDBY..." );
          if ( !toOn )
          {
            // der gewünschte zustand ist schon errreicht
            emit sigDeviceIsPoweredOFF();
            // hier ist das schon zuende
            return;
          }
        }
        else
        {
          //
          // ich hab PLAY, PAUSE oder STOP
          //
          LGDEBUG( "AsyncAlertCommand::switchPowerTo: device is not state STANDBY..." );
          if ( toOn )
          {
            // der gewünschte Status ist schon da
            emit sigDeviceIsPoweredON();
            // hier ist das schon zuende
            return;
          }
        }
        //
        // nicht der gewünschte status empfangen...
        // Gerät schalten und prüfen
        //
        togglePowerKey( toOn, masterDevice );
      }
    } );
    //
    // und nun nachfragen!
    //
    LGDEBUG( "AsyncAlertCommand::checkIfDeviceInStandby: ask master device..." );
    masterDevice->getNowPlaying();
  }

  void AsyncAlertCommand::togglePowerKey( bool toOn, BoseDevice *masterDevice )
  {
    bool shouldSwitchTo = toOn;
    //
    // Schritt 02
    // wenn dann ein/aus geschaktet werden muss, dann hier
    //
    LGDEBUG( QString( "AsyncAlertCommand::togglePowerKey: device switch power %1..." ).arg( shouldSwitchTo ) );
    //
    // conn ist ein Zeiger auf eine Verbindung
    // und wird dann meiner Verbindung zugewiesen (für disconnect)
    //
    auto conn = std::make_shared< QMetaObject::Connection >();
    //
    // Callback einrichten
    //
    *conn = connect( masterDevice, &BoseDevice::sigOnRequestAnswer, [=]( SharedResponsePtr response ) {
      //
      // Kurze Lambda Funktion reicht hier
      //
      bool isPowered = true;
      //
      // ich interessiere mich für eine Antort vom Typ R_NOW_PLAYING
      // um zu testen ob das gerät aus geschaltet wurde
      //
      if ( response->getResultType() == ResultobjectType::R_NOW_PLAYING )
      {
        HttpNowPlayingObject *nPlayObj = static_cast< HttpNowPlayingObject * >( response.get() );
        if ( ( nPlayObj->getSource() ).contains( QLatin1String( "STANDBY" ) ) )
        {
          isPowered = false;
          // ich habe STANDBY
          LGDEBUG( "AsyncAlertCommand::togglePowerKey: device is in state STANDBY..." );
        }
        else
        {
          isPowered = true;
          LGDEBUG( QString( "AsyncAlertCommand::togglePowerKey: device playing source %1..." ).arg( nPlayObj->getSource() ) );
        }
        //
        // wenn das das gewünschte Ergebnis war, Signal und schluss
        //
        if ( isPowered == shouldSwitchTo )
        {
          if ( isPowered )
            emit sigDeviceIsPoweredON();
          else
            emit sigDeviceIsPoweredOFF();
          disconnect( *conn );
          return;
        }
        //
        // wenn das NICHT das gewünschte Ergebnis war
        // dann erneut fragten (bis von ausserhalb der gesetzte Timeout zuschlägt)
        //
        QThread::yieldCurrentThread();
        QThread::msleep( 1200 );
        LGDEBUG( "AsyncAlertCommand::togglePowerKey: ask again for now playing..." );
        masterDevice->getNowPlaying();
      }
      //
      // ich interessiere mich auch für eine Antort vom Typ R_OK
      //
      else if ( response->getResultType() == ResultobjectType::R_OK )
      {
        //
        // wenn die power taste gedrückt und gelöst wird
        //
        HttpResultOkObject *nResultObj = static_cast< HttpResultOkObject * >( response.get() );
        LGDEBUG( QString( "AsyncAlertCommand::togglePowerKey: result response: %1" ).arg( nResultObj->getStatus() ) );
        keyresponseCounter++;
        if ( keyresponseCounter == 2 )
        {
          //
          // und nun nachfragen ob die Kiste aus ist
          //
          LGDEBUG( "AsyncAlertCommand::checkIfDeviceInStandby: ask master device..." );
          masterDevice->getNowPlaying();
        }
      }
    } );
    //
    // Schalten!
    //
    keyresponseCounter = 0;
    LGDEBUG( QString( "AsyncAlertCommand::togglePowerKey: now switch device power %1" ).arg( shouldSwitchTo ) );
    //
    // toggle key mit callback, dort wird dann now playing erfragt und geschaut
    // ob das Gerät auch ausgeschaltet wurde
    //
    masterDevice->setKey( BoseDevice::bose_key::KEY_POWER, BoseDevice::bose_keystate::KEY_TOGGLE );
  }
}  // namespace radioalert
