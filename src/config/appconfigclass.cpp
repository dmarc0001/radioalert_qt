﻿#include "appconfigclass.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QSettings>
#include <QThread>

namespace radioalert
{
  const QString AppConfigClass::constNoData{"-"};
  const QString AppConfigClass::constAppGroupName{"application"};

  /**
   * @brief LoggerClass::LoggerClass Der Konstruktor mit Name der Konfigdatei im Programmverzeichnis
   * @param cfg
   */
  AppConfigClass::AppConfigClass()
      : configFileName( QCoreApplication::applicationName().append( QLatin1String( ".ini" ) ) ), configLockFile( ConfigLockFile )
  {
  }

  AppConfigClass::AppConfigClass( const QString &_configFileName )
      : configFileName( _configFileName ), configLockFile( ConfigLockFile )
  {
  }

  AppConfigClass::AppConfigClass( const QString &_configFileName, const QString &_configLockFile )
      : configFileName( _configFileName ), configLockFile( _configLockFile )
  {
  }

  /**
   * @brief AppConfigClass::~AppConfigClass
   */
  AppConfigClass::~AppConfigClass()
  {
    qDebug().noquote() << QLatin1String( "..." );
    saveSettings();
  }

  /**
   * @brief AppConfigClass::getLogfileName
   * @return
   */
  QString AppConfigClass::getLogfileName() const
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
  bool AppConfigClass::loadSettings()
  {
    qDebug().noquote() << QLatin1String( "AppConfigClass::loadSettings..." );
    QLockFile configLockFile( ConfigLockFile );
    if ( configLockFile.isLocked() )
    {
      qWarning().noquote() << QLatin1String( "config file ist locking by another process, wait..." );
      for ( auto i = 0; i < 10; i++ )
      {
        QThread::sleep( 1 );
        if ( configLockFile.isLocked() )
        {
          qWarning().noquote() << QLatin1String( "config file ist locking by another process, wait..." );
        }
        else
        {
          break;
        }
      }
      qCritical().noquote() << QLatin1String( "can't load config, file is locked!" );
      throw ConfigfileNotExistException(
          QString( "AppConfigClass::loadSettings -> configfile %1 locked!" ).arg( QString( ConfigLockFile ) ) );
    }
    //
    // OKAY nicht gelockt
    //
    qDebug().noquote() << QLatin1String( "AppConfigClass::loadSettings -> lock config file..." );
    if ( configLockFile.lock() )
    {
      // config file ist gelockt...
      // config Locken während die config geladen wird
      QMutexLocker locker( &configLocker );

      qDebug().noquote() << QLatin1String( "CONFIG: <" ) << configFileName << QLatin1String( ">" );
      QSettings settings( configFileName, QSettings::IniFormat );
      qDebug().noquote() << QLatin1String( "load settings from" ) << configFileName;

      qDebug() << QLatin1String( "global settings load..." );
      if ( !globalConfig.loadSettings( settings ) )
        globalConfig.makeDefaultSettings( settings );
      qDebug() << QLatin1String( "global settings load...OK" );
      //
      // Alarme auslesen mit lokalem Objekt
      //
      AlertConfig alConfig;
      qDebug() << QLatin1String( "alert settings load..." );
      alConfig.loadSettings( settings, alerts );
      qDebug() << QLatin1String( "alert settings load...OK" );
      //
      // Datei entsperren
      //
      qDebug().noquote() << QLatin1String( "AppConfigClass::loadSettings -> unlock config file..." );
      configLockFile.unlock();
    }
    else
    {
      qCritical().noquote() << QLatin1String( "can't load config, file can't lock!" );
      throw ConfigfileNotExistException(
          QString( "AppConfigClass::loadSettings -> configfile %1 can't lock!" ).arg( QString( ConfigLockFile ) ) );
    }
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
  bool AppConfigClass::saveSettings()
  {
    // Locken während die config gesichert wird
    QMutexLocker locker( &configLocker );

    qDebug().noquote() << QLatin1String( "..." );
    bool retVal = true;
    qDebug().noquote() << QLatin1String( "save to <" ) << configFileName << QLatin1String( ">" );
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
  bool AppConfigClass::isConfigChanged()
  {
    if ( configHashLoad == makeConfigHash() )
      return ( false );
    return ( true );
  }

  /**
   * @brief AppConfigClass::isConfigFileChanged
   * @return
   */
  bool AppConfigClass::isConfigFileChanged()
  {
    if ( configFileHash == makeConfigfileHash() )
      return ( false );
    return ( true );
  }

  /**
   * @brief AppConfigClass::makeConfigHash
   * @return
   */
  QByteArray AppConfigClass::makeConfigHash()
  {
    // config während Ausführung sperren
    // QMutexLocker locker( &configLocker );

    QByteArray serialized;
    RadioAlertList::Iterator ral;
    //
    qDebug().nospace().noquote() << QLatin1String( "AppConfigClass::makeConfigHash: clobalConfig..." );
    serialized.append( globalConfig.serialize() );
    //
    for ( ral = alerts.begin(); ral != alerts.end(); ++ral )
    {
      serialized.append( ral->serialize() );
    }
    QCryptographicHash qhash( QCryptographicHash::Md5 );
    qhash.reset();
    qhash.addData( serialized );
    qDebug().nospace().noquote() << QLatin1String( "AppConfigClass::makeConfigHash: clobalConfig...OK" );
    return ( qhash.result() );
  }

  /**
   * @brief AppConfigClass::getConfigfileHash
   * @return
   */
  QByteArray AppConfigClass::makeConfigfileHash()
  {
    // config während ausführung sperren
    // QMutexLocker locker( &configLocker );

    qDebug().noquote().nospace() << QLatin1String( "AppConfigClass::makeConfigfileHash: calculate hash for config file..." );
    QCryptographicHash crypto( QCryptographicHash::Md5 );
    QFile file( configFileName );
    file.open( QFile::ReadOnly );
    while ( !file.atEnd() )
    {
      crypto.addData( file.read( 8192 ) );
    }
    qDebug().noquote().nospace() << QLatin1String( "AppConfigClass::makeConfigfileHash: calculate hash for config file...OK" );
    return ( crypto.result() );
  }

  /**
   * @brief AppConfigClass::getGlobalConfig
   * @return
   */
  GlobalConfig &AppConfigClass::getGlobalConfig()
  {
    // sperren bis abgeschlossen
    QMutexLocker locker( &configLocker );
    return ( globalConfig );
  }

  /**
   * @brief AppConfigClass::getAlertList
   * @return
   */
  RadioAlertList &AppConfigClass::getAlertList()
  {
    // sperren bis abgeschlossen
    QMutexLocker locker( &configLocker );
    return ( alerts );
  }

  /**
   * @brief AppConfigClass::getLockMutexPtr
   * @return
   */
  QMutex *AppConfigClass::getLockMutexPtr()
  {
    return ( &configLocker );
  }

  QByteArray AppConfigClass::getConfigHashLoad() const
  {
    return configHashLoad.toHex();
  }

}  // namespace radioalert
