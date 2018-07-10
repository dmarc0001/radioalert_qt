#include "configfilenotexistexception.hpp"

namespace radioalert
{
  ConfigfileNotExistException::ConfigfileNotExistException( QString const &message ) : message( message )
  {
  }

  void ConfigfileNotExistException::raise() const
  {
    throw *this;
  }

  ConfigfileNotExistException *ConfigfileNotExistException::clone() const
  {
    return new ConfigfileNotExistException( *this );
  }

  QString ConfigfileNotExistException::getMessage( void ) const
  {
    return ( message );
  }

}  // namespace radioalert
