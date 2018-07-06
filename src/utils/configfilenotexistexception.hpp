#ifndef CONFIGFILENOTEXISTEXCEPTION_HPP
#define CONFIGFILENOTEXISTEXCEPTION_HPP

#include <qglobal.h>
#include <QException>

namespace radioalert
{
  class ConfigfileNotExistException : public QException
  {
    public:
    ConfigfileNotExistException *clone() const;
    void raise() const;
  };
}  // namespace radioalert
#endif  // CONFIGFILENOTEXISTEXCEPTION_HPP
