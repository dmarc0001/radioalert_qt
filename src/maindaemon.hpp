#ifndef MAINDAEMON_HPP
#define MAINDAEMON_HPP

#include <qglobal.h>
#include <QDebug>
#include <QObject>
#include <QString>
#include <QTimer>
#include <memory>
#include "config/AppConfigClass.hpp"
#include "logging/Logger.hpp"

namespace radioalert
{
  class MainDaemon : public QObject
  {
    Q_OBJECT
    private:
    QString configFile;
    bool isDebugOverride;
    QTimer zyclon;
    std::shared_ptr< AppConfigClass > appConfig;
    qint16 programTimeoutDebugging;
    static const QString version;
    std::shared_ptr< Logger > lg;

    public:
    explicit MainDaemon( QString const &_configFile, bool _isOverrideDebug, QObject *parent = nullptr );
    ~MainDaemon() override;
    void init( void );

    private:
    signals:
    void close( void );

    private slots:
    void slotZyclonTimer( void );

    public slots:
  };

}  // namespace radioalert

#endif  // MAINDAEMON_HPP
