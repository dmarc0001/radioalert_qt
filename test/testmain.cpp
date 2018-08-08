#include <QObject>
#include <QRegExp>
#include <QThread>
#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <csignal>
#include "../src/config/alertconfig.hpp"
#include "../src/logging/logger.hpp"
#include "../src/udpcontrolprocess.hpp"

using namespace radioalert;

void signalHandler( int signal );
static std::function< void() > mIntSignalHahdler;

int main( int argc, char *argv[] )
{
  QCoreApplication a( argc, argv );
  QCommandLineParser parser;
  QString configName = QCoreApplication::applicationName().remove( QRegExp( "_d$" ) ).append( ".ini" );
  std::shared_ptr< AppConfigClass > appConfig( new AppConfigClass( configName ) );
  appConfig->loadSettings();
  std::shared_ptr< Logger > lg( new Logger( appConfig ) );
  lg->startLogging();
  UdpControlProcess udpControl( lg, appConfig );
  udpControl.init();
  //
  // Signalhandler installieren
  //
  mIntSignalHahdler = std::bind( &UdpControlProcess::requestQuit, &udpControl );
  std::signal( SIGINT, signalHandler );
  //
  // Schliessen der App an QCoreApplication mitteilen
  //
  QObject::connect( &udpControl, &UdpControlProcess::closeUdpServer, &a, &QCoreApplication::quit );
  return a.exec();
}

void signalHandler( int signal )
{
  //
  // nur bei dem gewünschten Signal
  //
  if ( signal == SIGINT )
  {
    if ( mIntSignalHahdler != nullptr )
    {
      //
      // Funktionsobjekt, zeigt auf die Instanz/memberfunktion
      //
      mIntSignalHahdler();
    }
  }
}
