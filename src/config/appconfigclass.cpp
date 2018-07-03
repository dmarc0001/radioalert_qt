#include <QCoreApplication>
#include <QDebug>
#include <QSettings>

#include "appconfigclass.hpp"

namespace radioalert
{
  const QString AppConfigClass::constNoData( "-" );
  const QString AppConfigClass::constAppGroupName( "application" );

  /**
   * @brief LoggerClass::LoggerClass Der Konstruktor mit Name der Konfigdatei im Programmverzeichnis
   * @param cfg
   */
  AppConfigClass::AppConfigClass( void ) : configFileName( QCoreApplication::applicationName() + ".ini" )
  {
  }

  AppConfigClass::AppConfigClass( const QString &configFileName ) : configFileName( configFileName )
  {
  }

  AppConfigClass::~AppConfigClass( void )
  {
    qDebug().noquote() << "...";
    saveSettings();
  }

  QString AppConfigClass::getLogfileName( void ) const
  {
    return ( configFileName );
  }

  void AppConfigClass::setConfigFileName( const QString &fileName )
  {
    configFileName = fileName;
    loadSettings();
  }

  /**
   * @brief LoggerClass::loadSettings Lade Einstellungen aus der Datei
   * @return
   */
  bool AppConfigClass::loadSettings( void )
  {
    qDebug().noquote() << "CONFIG: <" + configFileName + ">";
    QSettings settings( configFileName, QSettings::IniFormat );
    if ( !globalConfig.loadSettings( settings ) )
      globalConfig.makeDefaultSettings( settings );

    //
    if ( !loadAlertSettings( settings ) )
    {
      makeAlertDefaultSettings( settings );
    }
    return ( true );
  }

  /**
   * @brief AppConfigClass::loadSettings Lade Einstellungen aus CONFIG Datei
   * @param cFile
   * @return
   */
  bool AppConfigClass::loadSettings( QString &cFile )
  {
    qDebug().noquote() << "load settings from" << cFile;
    configFileName = cFile;
    QSettings settings( configFileName, QSettings::IniFormat );
    return loadSettings();
  }

  /**
   * @brief LoggerClass::saveSettings sichere Einstellunge in Datei
   * @return
   */
  bool AppConfigClass::saveSettings( void )
  {
    qDebug().noquote() << "...";
    bool retVal = true;
    qDebug().noquote() << "save to <" + configFileName + ">";
    // QSettings settings(configFile, QSettings::NativeFormat);
    QSettings settings( configFileName, QSettings::IniFormat );
    //
    // die globalen sichern
    //
    if ( !globalConfig.saveSettings( settings ) )
    {
      retVal = false;
    }
    if ( !saveAlertSettings( settings ) )
    {
      retVal = false;
    }
    return ( retVal );
  }

}  // namespace radioalert
