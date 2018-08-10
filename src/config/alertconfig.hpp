#ifndef ALERTCONFIG_HPP
#define ALERTCONFIG_HPP

#include <qglobal.h>
#include <QMap>
#include <QObject>
#include <QRegExp>
#include <QSettings>
#include <QString>
#include "../global_config.hpp"
#include "singlealertconfig.hpp"

namespace radioalert
{
  class AlertConfig
  {
    public:
    static const QString dateKey;
    static const QString timeKey;
    static const QString raiseVolKey;
    static const QString volumeKey;
    static const QString devicesKey;
    static const QString sourceAccountKey;
    static const QString noteKey;
    static const QString alertEnableKey;
    static const QString sourceKey;
    static const QString durationKey;
    static const QString typeKey;
    static const QString daysKey;
    static const QString locationKey;
    static const QRegExp alertGroupToken;
    // static const QString alertTemplate;
    static const QString dateFormatToken;
    static const QString timeFormatToken;

    public:
    explicit AlertConfig( void );
    //
    bool loadSettings( QSettings &settings, RadioAlertList &alerts );
    bool saveSettings( QSettings &settings, RadioAlertList &alerts );
    bool makeDefaultSettings( QSettings &settings, RadioAlertList *alerts );
  };
}  // namespace radioalert
#endif  // ALERTCONFIG_HPP
