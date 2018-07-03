#ifndef ALERTCONFIG_HPP
#define ALERTCONFIG_HPP

#include <qglobal.h>
#include <QMap>
#include <QObject>
#include <QRegExp>
#include <QSettings>
#include <QString>
#include "singlealertconfig.hpp"

namespace radioalert
{
  class AlertConfig
  {
    private:
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

    public:
    explicit AlertConfig( void );
    //
    bool loadSettings( QSettings &settings, QMap< qint8, SingleAlertConfig > *alerts );
    bool saveSettings( QSettings &settings, QMap< qint8, SingleAlertConfig > *alerts );
    bool makeDefaultSettings( QSettings &settings, QMap< qint8, SingleAlertConfig > *alerts );
  };
}  // namespace radioalert
#endif  // ALERTCONFIG_HPP
