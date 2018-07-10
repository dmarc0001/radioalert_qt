#ifndef GLOBAL_CONFIG_HPP
#define GLOBAL_CONFIG_HPP

#define LGDEBUG( val ) \
  if ( lg->isDebug() ) \
  {                    \
    lg->debug( val );  \
  }

#define LGINFO( val ) lg->info( val )
#define LGWARN( val ) lg->warn( val )
#define LGCRIT( val ) lg->crit( val )

#endif  // GLOBAL_CONFIG_HPP
