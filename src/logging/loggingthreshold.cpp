#include "loggingthreshold.hpp"

namespace radioalert
{
  const QHash< QString, LoggingThreshold > LoggingUtils::thresholdNames = {
      {"none", LoggingThreshold::LG_NONE},       {"debug", LoggingThreshold::LG_DEBUG},       {"info", LoggingThreshold::LG_INFO},
      {"warning", LoggingThreshold::LG_WARNING}, {"critical", LoggingThreshold::LG_CRITICAL}, {"fatal", LoggingThreshold::LG_FATAL}};

}  // namespace radioalert
