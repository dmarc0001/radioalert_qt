#ifndef GLOBAL_CONFIG_HPP
#define GLOBAL_CONFIG_HPP

#define LGDEBUG( val ) \
  if ( lg->isDebug() ) \
  {                    \
    lg->debug( val );  \
  }

//#define LGDEBUG( val ) lg->debug( val )
#define LGINFO( val ) lg->info( val )
#define LGWARN( val ) lg->warn( val )
#define LGCRIT( val ) lg->crit( val )

constexpr char ConfigLockFile[]{"alert_daemon.lck"};
#endif  // GLOBAL_CONFIG_HPP
