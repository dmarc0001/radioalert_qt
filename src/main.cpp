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
  QString appName = QCoreApplication::applicationName().append( ( ".ini" ) );
  QString configName;

  parser.setApplicationDescription( "radioalert qt" );
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
  mSignalHahdler = std::bind( &radioalert::MainDaemon::reReadConfigFromFile, &daemon );
  daemon.init();
  //
  // Signalhandler installieren
  //
  std::signal( SIGHUP, signalHandler );
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
  if ( signal == SIGHUP )
  {
    //
    // Sicherheitsabfrage, sonst würde das mit SIFGAULT enden...
    //
    if ( mSignalHahdler != nullptr )
    {
      //
      // Funktionsobjekt, zeigt auf die Instanz/memberfunktion
      //
      mSignalHahdler();
    }
  }
}
