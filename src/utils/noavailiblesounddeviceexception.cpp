#include "noavailiblesounddeviceexception.hpp"

namespace radioalert
{
  NoAvailibleSoundDeviceException::NoAvailibleSoundDeviceException( QString const &message ) : message( message )
  {
  }

  void NoAvailibleSoundDeviceException::raise() const
  {
    throw *this;
  }

  NoAvailibleSoundDeviceException *NoAvailibleSoundDeviceException::clone() const
  {
    return new NoAvailibleSoundDeviceException( *this );
  }

  QString NoAvailibleSoundDeviceException::getMessage( void ) const
  {
    return ( message );
  }

}  // namespace radioalert
