#ifndef NOAVAILIBLESOUNDDEVICEEXCEPTION_HPP
#define NOAVAILIBLESOUNDDEVICEEXCEPTION_HPP

#include <QtCore/qglobal.h>

class NoAvailibleSoundDeviceException : public QException
{
  public:
    NoAvailibleSoundDeviceException();
};

#endif // NOAVAILIBLESOUNDDEVICEEXCEPTION_HPP