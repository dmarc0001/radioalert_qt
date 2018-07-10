#ifndef CONFIGFILENOTEXISTEXCEPTION_HPP
#define CONFIGFILENOTEXISTEXCEPTION_HPP

#include <qglobal.h>
#include <QException>

namespace radioalert
{
  class ConfigfileNotExistException : public QException
  {
    private:
    QString message;

    public:
    ConfigfileNotExistException( QString const &message );
    virtual ~ConfigfileNotExistException(){};
    QString getMessage( void ) const;
    ConfigfileNotExistException *clone() const;
    void raise() const;
  };
}  // namespace radioalert
#endif  // CONFIGFILENOTEXISTEXCEPTION_HPP
