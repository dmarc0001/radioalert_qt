#ifndef MAINDAEMON_HPP
#define MAINDAEMON_HPP

#include <qglobal.h>
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QString>
#include <QTime>
#include <QTimer>
#include <QVector>
#include <memory>
#include "config/appconfigclass.hpp"
#include "logging/Logger.hpp"
#include "radioalertthread.hpp"
#include "utils/configfilenotexistexception.hpp"

namespace radioalert
{
  //! Zeitintervall für checks ob eine alarmzeit erreicht wurde
  static const int mainTimerDelay = 1000 * 10;
  //! Zeitintervall für test ob die Konfiguration verändert wurde
  static const int checkConfigTime = 1000 * 45;
  //! maximaler Abstand des Alarm in die Vergangenheit
  static const int timeDiffToPast = -10;
  //! maximaler Abstand des Alarm in die Zukunfst
  static const int timeDiffToFuture = 30;

  class MainDaemon : public QObject
  {
    Q_OBJECT
    private:
    QString configFile;
    QFileInfo configFileInfo;
    bool isDebugOverride;
    QTimer zyclon;
    QTimer configZyclon;
    std::shared_ptr< AppConfigClass > appConfig;
    static const QString version;
    std::shared_ptr< Logger > lg;
    QDateTime lastModifiedConfig;
    QVector< RadioAlertThread * > activeThreads;

    public:
    explicit MainDaemon( QString const &_configFile, bool _isOverrideDebug, QObject *parent = nullptr );
    ~MainDaemon() override;             //! Destruktor
    void init( void );                  //! initialisiere das Programm
    void reReadConfigFromFile( void );  //! Lese config von Konfigurationsdatei
    void requestQuit( void );           //! Fordere Ende an

    private:
    bool isAlertDateToday( QDate aData );
    qint16 howFarIsAlert( QTime aTime );

    signals:
    void close( void );

    private slots:
    void slotZyclonTimer( void );
    void slotConfigZyclonTimer( void );
    void slotAlertFinished( RadioAlertThread *theTread );

    public slots:
  };

}  // namespace radioalert

#endif  // MAINDAEMON_HPP
