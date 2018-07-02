#include <qglobal.h>
#include <QObject>
#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include "maindaemon.hpp"

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
  radioalert::MainDaemon daemon( configName, debug );
  daemon.init();
  QObject::connect( &daemon, &radioalert::MainDaemon::close, &a, &QCoreApplication::quit );
  return a.exec();
}
