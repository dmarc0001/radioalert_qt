#include "main.hpp"
#include <QObject>
#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>

/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char *argv[] )
{
  QCoreApplication a( argc, argv );
  QCommandLineParser parser;
  QString appName = QString( "%1.ini" ).arg( QCoreApplication::applicationName() );
  QString configName;

  parser.addHelpOption();
  //
  // debug an oder aus
  //
  QCommandLineOption dbgOption( {"d", "debug"}, "Debug override config [default: off]." );
  QCommandLineOption configFileOption( {"c", "config"}, QString( "special config file [default: %1]" ).arg( appName ), "config",
                                       appName );
  parser.addOption( dbgOption );
  parser.addOption( configFileOption );
  parser.process( a );
  bool debug = parser.isSet( dbgOption );
  configName = parser.value( configFileOption );
  //
  // das Hauptobjekt erzeugen
  //
  radioalert::MainDaemon daemon( configName, debug );
  //
  // Zeiger auf die Reload Funktion zuweisen, damit der Signalhandler
  // dann auch reagieren kann
  //
  mHupSignalHahdler = std::bind( &radioalert::MainDaemon::reReadConfigFromFile, &daemon );
  mIntSignalHahdler = std::bind( &radioalert::MainDaemon::requestQuit, &daemon );
  daemon.init();
  //
  // Signalhandler installieren
  //
#ifndef WIN32
  std::signal( SIGHUP, signalHandler );
#endif
  std::signal( SIGINT, signalHandler );
  //
  // Schliessen der App an QCoreApplication mitteilen
  //
  QObject::connect( &daemon, &radioalert::MainDaemon::close, &a, &QCoreApplication::quit );
  return a.exec();
}

/**
 * @brief signalHandlerSIGINT
 * @param signal
 */
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
#ifndef WIN32
  if ( signal == SIGHUP )
  {
    //
    // Sicherheitsabfrage, sonst würde das mit SIFGAULT enden...
    //
    if ( mHupSignalHahdler != nullptr )
    {
      //
      // Funktionsobjekt, zeigt auf die Instanz/memberfunktion
      //
      mHupSignalHahdler();
    }
  }
#endif
}
