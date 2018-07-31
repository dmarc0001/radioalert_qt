#ifndef NOAVAILIBLESOUNDDEVICEEXCEPTION_HPP
#define NOAVAILIBLESOUNDDEVICEEXCEPTION_HPP

#include <qglobal.h>
#include <QException>

namespace radioalert
{
  class NoAvailibleSoundDeviceException : public QException
  {
    public:
    NoAvailibleSoundDeviceException();

    private:
    QString message;

    public:
    NoAvailibleSoundDeviceException( QString const &message );
    virtual ~NoAvailibleSoundDeviceException(){};
    QString getMessage( void ) const;
    NoAvailibleSoundDeviceException *clone() const;
    void raise() const;
  };
};  // namespace radioalert

#endif  // NOAVAILIBLESOUNDDEVICEEXCEPTION_HPP
