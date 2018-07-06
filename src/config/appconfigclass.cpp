#include <QCoreApplication>
#include <QDebug>
#include <QFile>
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

  /**
   * @brief AppConfigClass::~AppConfigClass
   */
  AppConfigClass::~AppConfigClass( void )
  {
    qDebug().noquote() << "...";
    saveSettings();
  }

  /**
   * @brief AppConfigClass::getLogfileName
   * @return
   */
  QString AppConfigClass::getLogfileName( void ) const
  {
    return ( globalConfig.getLogFile() );
  }

  /**
   * @brief AppConfigClass::setConfigFileName
   * @param fileName
   */
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
    // Locken während die config geladen wird
    QMutexLocker locker( &configLocker );

    qDebug().noquote() << "CONFIG: <" + configFileName + ">";
    QSettings settings( configFileName, QSettings::IniFormat );
    qDebug().noquote() << "load settings from" << configFileName;

    qDebug() << "global settings load...";
    if ( !globalConfig.loadSettings( settings ) )
      globalConfig.makeDefaultSettings( settings );
    qDebug() << "global settings load...OK";
    //
    // Alarme auslesen mit lokalem Objekt
    //
    AlertConfig alConfig;
    qDebug() << "alert settings load...";
    alConfig.loadSettings( settings, alerts );
    qDebug() << "alert settings load...OK";
    configHashLoad = makeConfigHash();
    configFileHash = makeConfigfileHash();
    return ( true );
  }

  /**
   * @brief AppConfigClass::loadSettings Lade Einstellungen aus CONFIG Datei
   * @param cFile
   * @return
   */
  bool AppConfigClass::loadSettings( QString &cFile )
  {
    configFileName = cFile;
    return loadSettings();
  }

  /**
   * @brief LoggerClass::saveSettings sichere Einstellunge in Datei
   * @return
   */
  bool AppConfigClass::saveSettings( void )
  {
    // Locken während die config gesichert wird
    QMutexLocker locker( &configLocker );

    qDebug().noquote() << "...";
    bool retVal = true;
    qDebug().noquote() << "save to <" + configFileName + ">";
    QSettings settings( configFileName, QSettings::IniFormat );
    //
    // die globalen sichern
    //
    if ( !globalConfig.saveSettings( settings ) )
    {
      retVal = false;
    }
    AlertConfig alConfig;
    alConfig.saveSettings( settings, alerts );
    settings.sync();
    configHashLoad = makeConfigHash();
    configFileHash = makeConfigfileHash();
    return ( retVal );
  }

  /**
   * @brief AppConfigClass::isConfigChanged
   * @return
   */
  bool AppConfigClass::isConfigChanged( void )
  {
    if ( configHashLoad == makeConfigHash() )
      return ( false );
    return ( true );
  }

  /**
   * @brief AppConfigClass::isConfigFileChanged
   * @return
   */
  bool AppConfigClass::isConfigFileChanged( void )
  {
    if ( configFileHash == makeConfigfileHash() )
      return ( false );
    return ( true );
  }

  /**
   * @brief AppConfigClass::makeConfigHash
   * @return
   */
  QByteArray AppConfigClass::makeConfigHash( void )
  {
    // config während Ausführung sperren
    // QMutexLocker locker( &configLocker );

    QByteArray serialized;
    RadioAlertList::Iterator ral;
    //
    qDebug().nospace().noquote() << "AppConfigClass::makeConfigHash: clobalConfig...";
    serialized.append( globalConfig.serialize() );
    //
    for ( ral = alerts.begin(); ral != alerts.end(); ++ral )
    {
      serialized.append( ral->serialize() );
    }
    QCryptographicHash qhash( QCryptographicHash::Md5 );
    qhash.reset();
    qhash.addData( serialized );
    qDebug().nospace().noquote() << "AppConfigClass::makeConfigHash: clobalConfig...OK";
    return ( qhash.result() );
  }

  /**
   * @brief AppConfigClass::getConfigfileHash
   * @return
   */
  QByteArray AppConfigClass::makeConfigfileHash( void )
  {
    // config während ausführung sperren
    // QMutexLocker locker( &configLocker );

    qDebug().noquote().nospace() << "AppConfigClass::makeConfigfileHash: calculate hash for config file...";
    QCryptographicHash crypto( QCryptographicHash::Md5 );
    QFile file( configFileName );
    file.open( QFile::ReadOnly );
    while ( !file.atEnd() )
    {
      crypto.addData( file.read( 8192 ) );
    }
    qDebug().noquote().nospace() << "AppConfigClass::makeConfigfileHash: calculate hash for config file...OK";
    return ( crypto.result() );
  }

  /**
   * @brief AppConfigClass::getGlobalConfig
   * @return
   */
  GlobalConfig &AppConfigClass::getGlobalConfig( void )
  {
    // sperren bis abgeschlossen
    QMutexLocker locker( &configLocker );
    return ( globalConfig );
  }

  /**
   * @brief AppConfigClass::getAlertList
   * @return
   */
  RadioAlertList &AppConfigClass::getAlertList( void )
  {
    // sperren bis abgeschlossen
    QMutexLocker locker( &configLocker );
    return ( alerts );
  }

  /**
   * @brief AppConfigClass::getLockMutexPtr
   * @return
   */
  QMutex *AppConfigClass::getLockMutexPtr( void )
  {
    return ( &configLocker );
  }

}  // namespace radioalert
