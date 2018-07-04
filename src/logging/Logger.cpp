#include "Logger.hpp"
#include <QCoreApplication>
#include <QDateTime>
#include <QFileInfo>

namespace radioalert
{
  const QString Logger::dateTimeFormat = "[yyyy-MM-dd hh:mm:ss.z] ";
  /**
   * @brief Logger::Logger Konstruktor mit Konfigurationsdatei Übergabe
   * @param lFile
   */
  Logger::Logger( const std::shared_ptr< AppConfigClass > _config )
      : threshold( LoggingThreshold::LG_WARNING )
      , logFile( nullptr )
      , textStream( nullptr )
      , configClass( _config )
      , isFileOpen( false )
      , logToConsole( true )
  {
    //
    // einen eigenen Handler für debug ausgaben einbinden
    //
    // qInstallMessageHandler( &Logger::messageOutput );
  }

  Logger::Logger() : threshold( LoggingThreshold::LG_WARNING ), configClass( nullptr ), isFileOpen( false )
  {
    //
    // einen eigenen Handler für debug ausgaben einbinden
    //
    // qInstallMessageHandler( &Logger::messageOutput );
  }

  Logger::~Logger()
  {
    shutdown();
  }

  /**
   * @brief Logger::startLogging begine Logging, öffne Logfile
   * @return
   */
  int Logger::startLogging( LoggingThreshold th )
  {
    threshold = th;
    return ( startLogging() );
  }

  int Logger::startLogging( void )
  {
    //
    // gibt es eine Einstellung?
    //
    if ( configClass )
    {
      if ( configClass->getLogfileName().length() > 4 )
      {
        // aus dem Objekt erfragen ob konsole geloggt werden soll
        logToConsole = ( configClass->getGlobalConfig() ).getLogToConsole();
        // Super, das Logfile ist benannt!
        if ( logToConsole )
          qDebug().noquote().nospace() << "START LOGGING...";
        logFile = std::unique_ptr< QFile >( new QFile( configClass->getLogfileName() ) );
        logFile->open( QIODevice::WriteOnly | QIODevice::Append );
        textStream = std::unique_ptr< QTextStream >( new QTextStream( logFile.get() ) );
        *textStream << getDateString() << "START LOGGING" << endl;
        if ( logFile.get() && logFile->isOpen() && textStream.get() )
        {
          isFileOpen = true;
          if ( logToConsole )
            qDebug().noquote().nospace() << "START LOGGING...OK";
          return ( 1 );
        }
      }
    }
    //
    // Oh, da lief was falsch, LOGGEN nicht möglich
    //
    qDebug().noquote().nospace() << "START LOGGING...FAILED";
    return ( 0 );
  }

  /**
   * @brief Logger::setThreshold
   * @param th
   */
  /*
  void Logger::setThreshold( LoggingThreshold th )
  {
    threshold = th;
  }
  */

  /**
   * @brief Logger::getThreshold
   * @return
   */
  /*
  LoggingThreshold Logger::getThreshold( void )
  {
    return ( threshold );
  }
  */

  /**
   * @brief Logger::warn Ausgabe(n) für WARNUNG
   * @param msg
   */
  void Logger::warn( const QString &msg )
  {
    if ( logToConsole )
      qWarning().noquote().nospace() << msg;
    if ( isFileOpen && threshold >= LoggingThreshold::LG_WARNING )
    {
      *textStream << getDateString() << LoggingUtils::thresholdNames.key( LoggingThreshold::LG_WARNING ) << msg << endl;
    }
  }

  void Logger::warn( const char *msg )
  {
    if ( logToConsole )
      qWarning().noquote().nospace() << msg;
    if ( textStream && threshold >= LoggingThreshold::LG_WARNING )
    {
      *textStream << getDateString() << LoggingUtils::thresholdNames.key( LoggingThreshold::LG_WARNING ) << msg << endl;
    }
  }

  void Logger::warn( const std::string &msg )
  {
    if ( logToConsole )
      qWarning().noquote().nospace() << msg.c_str();
    if ( textStream && threshold >= LoggingThreshold::LG_WARNING )
    {
      *textStream << getDateString() << LoggingUtils::thresholdNames.key( LoggingThreshold::LG_WARNING ) << msg.c_str() << endl;
    }
  }

  /**
   * @brief Logger::info Ausgabe(n) für Info
   * @param msg
   */
  void Logger::info( const QString &msg )
  {
    if ( logToConsole )
      qInfo().noquote().nospace() << msg;
    if ( textStream && threshold >= LoggingThreshold::LG_INFO )
    {
      *textStream << getDateString() << LoggingUtils::thresholdNames.key( LoggingThreshold::LG_INFO ) << msg << endl;
    }
  }

  void Logger::info( const char *msg )
  {
    if ( logToConsole )
      qInfo().noquote().nospace() << msg;
    if ( textStream && threshold >= LoggingThreshold::LG_INFO )
    {
      *textStream << getDateString() << LoggingUtils::thresholdNames.key( LoggingThreshold::LG_INFO ) << msg << endl;
    }
  }
  void Logger::info( const std::string &msg )
  {
    if ( logToConsole )
      qInfo().noquote().nospace() << msg.c_str();
    if ( textStream && threshold >= LoggingThreshold::LG_INFO )
    {
      *textStream << getDateString() << LoggingUtils::thresholdNames.key( LoggingThreshold::LG_INFO ) << msg.c_str() << endl;
    }
  }

  /**
   * @brief Logger::debug Ausgaben für Debugging
   * @param msg
   */
  void Logger::debug( const QString &msg )
  {
    if ( logToConsole )
      qDebug().noquote().nospace() << msg;
    if ( textStream && threshold >= LoggingThreshold::LG_DEBUG )
    {
      *textStream << getDateString() << LoggingUtils::thresholdNames.key( LoggingThreshold::LG_DEBUG ) << msg << endl;
    }
  }

  void Logger::debug( const char *msg )
  {
    if ( logToConsole )
      qDebug().noquote().nospace() << msg;
    if ( textStream && threshold >= LoggingThreshold::LG_DEBUG )
    {
      *textStream << getDateString() << LoggingUtils::thresholdNames.key( LoggingThreshold::LG_DEBUG ) << msg << endl;
    }
  }

  void Logger::debug( const std::string &msg )
  {
    if ( logToConsole )
      qDebug().noquote().nospace() << msg.c_str();
    if ( textStream && threshold >= LoggingThreshold::LG_DEBUG )
    {
      *textStream << getDateString() << LoggingUtils::thresholdNames.key( LoggingThreshold::LG_DEBUG ) << msg.c_str() << endl;
    }
  }

  /**
   * @brief Logger::crit Ausgaben für Kritische Fehler
   * @param msg
   */
  void Logger::crit( const QString &msg )
  {
    if ( logToConsole )
      qCritical().noquote().nospace() << msg;
    if ( textStream && threshold >= LoggingThreshold::LG_CRITICAL )
    {
      *textStream << getDateString() << LoggingUtils::thresholdNames.key( LoggingThreshold::LG_CRITICAL ) << msg << endl;
    }
  }

  void Logger::crit( const char *msg )
  {
    if ( logToConsole )
      qCritical().noquote().nospace() << msg;
    if ( textStream && threshold >= LoggingThreshold::LG_CRITICAL )
    {
      *textStream << getDateString() << LoggingUtils::thresholdNames.key( LoggingThreshold::LG_CRITICAL ) << msg << endl;
    }
  }

  void Logger::crit( const std::string &msg )
  {
    if ( logToConsole )
      qCritical().noquote().nospace() << msg.c_str();
    if ( textStream && threshold >= LoggingThreshold::LG_CRITICAL )
    {
      *textStream << getDateString() << LoggingUtils::thresholdNames.key( LoggingThreshold::LG_CRITICAL ) << msg.c_str() << endl;
    }
  }

  void Logger::shutdown()
  {
    if ( textStream != nullptr )
    {
      textStream->flush();
    }
    if ( logFile != nullptr )
    {
      logFile->flush();
      logFile->close();
    }
  }

  QString Logger::getDateString()
  {
    dateTime = QDateTime::currentDateTime();
    return ( dateTime.toString( dateTimeFormat ) );
  }

  /*
  void Logger::messageOutput( QtMsgType type, const QMessageLogContext &context, const QString &msg )
  {
    std::string timeString = QDateTime( QDateTime::currentDateTime() ).toString( "[yyyy-MM-dd HH:mm:ss zzz]" ).toStdString();
    QByteArray localMsg = msg.toLocal8Bit();
    QStringList functParts;
    QString func;
    //
    switch ( type )
    {
      case QtDebugMsg:
        //
        // versuche nur objekt/function
        //
        functParts = QString( context.function ).split( "::" );
        if ( functParts.count() > 2 )
        {
          func = QString( functParts.value( functParts.count() - 2 ) )
                     .append( "::" )
                     .append( functParts.value( functParts.count() - 1 ) );
        }
        else
          func = context.function;

        fprintf( stdout, "%s DEBUG %s: %s\n", timeString.c_str(), func.toStdString().c_str(), localMsg.constData() );
        fflush( stdout );
        break;
      case QtInfoMsg:
        fprintf( stdout, "%s INFO %s\n", timeString.c_str(), localMsg.constData() );
        break;
      case QtWarningMsg:
        fprintf( stdout, "%s WARNING %s\n", timeString.c_str(), localMsg.constData() );
        break;
      case QtCriticalMsg:
        fprintf( stderr, "%s CRITICAL %s (%s:%u)\n", timeString.c_str(), localMsg.constData(), context.file, context.line );
        fflush( stderr );
        break;
      case QtFatalMsg:
        fprintf( stderr, "%s FATAL %s (%s:%u)\n", timeString.c_str(), localMsg.constData(), context.file, context.line );
        fflush( stderr );
        break;
    }
  }
  */

}  // namespace radioalert
