#include "main.hpp"
#include <QObject>
#include <QRegExp>
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
  QString appName = QCoreApplication::applicationName().remove( QRegExp( "_d$" ) ).append( ".ini" );
  QString appDir = QCoreApplication::applicationDirPath();
  QString lockDir = appDir;
  QString configName;

  parser.addHelpOption();
  //
  // debug an oder aus
  //
  QCommandLineOption dbgOption( {"d", "debug"}, "Debug override config [default: off]." );
  QCommandLineOption configFileOption( {"c", "config"}, QString( "special config file [default: %1]" ).arg( appName ), "config",
                                       appName );
  QCommandLineOption configLockFileDir( {"l", "lockdir"}, QString( "directory for config lockfile [default: %1]" ).arg( appDir ),
                                        "config_lock", appDir );
  parser.addOption( dbgOption );
  parser.addOption( configFileOption );
  parser.addOption( configLockFileDir );
  parser.process( a );
  bool debug = parser.isSet( dbgOption );
  configName = parser.value( configFileOption );
  lockDir = parser.value( configLockFileDir );
  //
  // das Hauptobjekt erzeugen
  //
  radioalert::MainDaemon daemon( configName, lockDir, debug );
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
  std::signal( SIGHUP, signalHandler );
  std::signal( SIGINT, signalHandler );
  //
  // Schliessen der App an QCoreApplication mitteilen
  //
  QObject::connect( &daemon, &radioalert::MainDaemon::close, &a, &QCoreApplication::quit );
  return ( a.exec() );
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
}
