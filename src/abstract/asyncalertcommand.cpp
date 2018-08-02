#include "asyncalertcommand.hpp"

namespace radioalert
{
  AsyncAlertCommand::AsyncAlertCommand( std::shared_ptr< Logger > logger, QObject *parent ) : QObject( parent ), lg( logger )
  {
  }

  AsyncAlertCommand::~AsyncAlertCommand()
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
        if ( ( nPlayObj->getSource() ).compare( QLatin1String( "STANDBY" ) ) == 0 )
        {
          // Ja, ich habe STANDBY
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
    masterDevice->getNowPlaying();
  }

  void AsyncAlertCommand::switchDeviceToSource( BoseDevice *masterDevice, const QString source )
  {
    //
    // Die Funktion setzt voraus, dass eine Winsocket Verbindung bereits
    // besteht, sonst kann der Callback nicht funktionieren
    // conn ist ein Zeiger auf eine Verbindung
    // und wird dann meiner Verbindung zugewiesen (für disconnect)
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

}  // namespace radioalert
