#ifndef GLOBAL_CONFIG_HPP
#define GLOBAL_CONFIG_HPP

#define LGDEBUG( val ) \
  if ( lg->isDebug() ) \
  lg->debug( val )

#define LGINFO lg->info
#define LGWARN lg->warn
#define LGCRIT lg->crit

#endif  // GLOBAL_CONFIG_HPP
