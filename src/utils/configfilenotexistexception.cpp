#include "configfilenotexistexception.hpp"

namespace radioalert
{
  void ConfigfileNotExistException::raise() const
  {
    throw *this;
  }

  ConfigfileNotExistException *ConfigfileNotExistException::clone() const
  {
    return new ConfigfileNotExistException( *this );
  }
}  // namespace radioalert
