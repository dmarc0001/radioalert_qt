#ifndef SRC_LOGGING_LOGGER_HPP_
#define SRC_LOGGING_LOGGER_HPP_
#include <QDateTime>
#include <QFile>
#include <QMap>
#include <QString>
#include <QTextStream>
#include <QtDebug>
#include <QtGlobal>
#include <memory>
#include <string>
#include "../config/appconfigclass.hpp"
#include "loggingthreshold.hpp"

namespace radioalert
{
  // Vorwärts deklaration für den Konstruktor
  class AppConfigClass;

  class Logger
  {
    private:
    LoggingThreshold threshold;                           //! Logstatus
    std::unique_ptr< QFile > logFile;                     //! Zeiger auf das Logdateiobjekt
    std::unique_ptr< QTextStream > textStream;            //! Zeiger auf einen Textstrom
    QDateTime dateTime;                                   //! das lokale Datum/Zeit objekt
    const std::shared_ptr< AppConfigClass > configClass;  //! Zeiger auf die Konfiguration
    static const QString dateTimeFormat;                  //! Format der Zeitausgabe
    bool isFileOpen;                                      //! true, wenn logdatei beschrieben werden kann
    bool logToConsole;                                    //! loggen zur Konsole?
    bool loggerIsDebug;                                   //! ist der Logger im DEBUG Mode?

    public:
    Logger( const std::shared_ptr< AppConfigClass > _config );  //! Konstruktor mit Zeiger auf das Konfig-Objekt
    virtual ~Logger();                                          //! Destruktor
    int startLogging( LoggingThreshold th );                    //! Loggen beginnen
    int startLogging( void );                                   //! loggen beginnen mit festgelegter einstellung
    bool isDebug( void );                                       //! debuggingMode?
    //
    void info( const QString &msg );      //! INFO Ausgaben
    void info( const std::string &msg );  //! INFO Ausgaben
    void info( const char *msg );         //! INFO Ausgaben
    //
    void debug( const QString &msg );      //! DEBUG Ausgaben
    void debug( const std::string &msg );  //! DEBUG Ausgaben
    void debug( const char *msg );         //! DEBUG Ausgaben
    //
    void warn( const QString &msg );      //! WARN Ausgaben
    void warn( const std::string &msg );  //! WARN Ausgaben
    void warn( const char *msg );         //! WARN Ausgaben
    //
    void crit( const QString &msg );      //! CRIT Ausgaben
    void crit( const std::string &msg );  //! CRIT Ausgaben
    void crit( const char *msg );         //! CRIT Ausgaben
    //
    void shutdown();  //! Logger explizit herunterfahren

    private:
    Logger();                 //! den Konstruktor verbergen/sperren
    QString getDateString();  //! interne Funktion für den Datumsstring
    // static void messageOutput( QtMsgType type, const QMessageLogContext &context, const QString &msg );
  };

}  // namespace radioalert

#endif /* SRC_LOGGING_LOGGER_HPP_ */
