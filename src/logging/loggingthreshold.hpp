#ifndef SRC_LOGGING_LOGGINGTHRESHOLD_HPP
#define SRC_LOGGING_LOGGINGTHRESHOLD_HPP

#include <qglobal.h>
#include <QHash>
#include <QString>

namespace radioalert
{
  enum class LoggingThreshold : quint8
  {
    LG_NONE,
    LG_DEBUG,
    LG_INFO,
    LG_WARNING,
    LG_CRITICAL,
    LG_FATAL
  };

  class LoggingUtils
  {
    public:
    static const QHash< QString, LoggingThreshold > thresholdNames;  //! Mapping Name => Wert
  };
}  // namespace radioalert

#endif  // LOGGINGTHRESHOLD_HPP
