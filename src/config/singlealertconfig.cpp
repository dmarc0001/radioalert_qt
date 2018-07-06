#include "singlealertconfig.hpp"

namespace radioalert
{
  SingleAlertConfig::SingleAlertConfig( void ) : alertIsBusy( false )
  {
  }

  QByteArray SingleAlertConfig::serialize( void )
  {
    QLatin1String tr( "true" );
    QLatin1String fa( "false" );
    //
    QString serStr = alertName;
    serStr += alertDate.toString( "yyyy-MM-dd" );
    serStr += alertTime.toString( "hh_mm" );
    serStr += alertRaiseVol ? tr : fa;
    serStr += QString( "%1" ).arg( alertVolume, 3, 10, QChar( '0' ) );
    serStr += alertDevices.join( '-' ).append( sourceAccount ).append( alertNote );
    serStr += alertEnable ? tr : fa;
    serStr += alertSource;
    serStr += QString( "%1" )
                  .arg( alertDuration, 5, 10, QChar( '0' ) )
                  .append( alertType )
                  .append( alertDays.join( '-' ) )
                  .append( alertLocation );
    return ( serStr.toUtf8() );
  }

  QDate SingleAlertConfig::getAlertDate() const
  {
    return alertDate;
  }

  void SingleAlertConfig::setAlertDate( const QDate &value )
  {
    alertDate = value;
  }

  QTime SingleAlertConfig::getAlertTime() const
  {
    return alertTime;
  }

  void SingleAlertConfig::setAlertTime( const QTime &value )
  {
    alertTime = value;
  }

  bool SingleAlertConfig::getAlertRaiseVol() const
  {
    return alertRaiseVol;
  }

  void SingleAlertConfig::setAlertRaiseVol( bool value )
  {
    alertRaiseVol = value;
  }

  qint8 SingleAlertConfig::getAlertVolume() const
  {
    return alertVolume;
  }

  void SingleAlertConfig::setAlertVolume( const qint8 &value )
  {
    alertVolume = value;
  }

  QStringList SingleAlertConfig::getAlertDevices() const
  {
    return alertDevices;
  }

  void SingleAlertConfig::setAlertDevices( const QStringList &value )
  {
    alertDevices = value;
  }

  QString SingleAlertConfig::getSourceAccount() const
  {
    return sourceAccount;
  }

  void SingleAlertConfig::setSourceAccount( const QString &value )
  {
    sourceAccount = value;
  }

  QString SingleAlertConfig::getAlertNote() const
  {
    return alertNote;
  }

  void SingleAlertConfig::setAlertNote( const QString &value )
  {
    alertNote = value;
  }

  bool SingleAlertConfig::getAlertEnable() const
  {
    return alertEnable;
  }

  void SingleAlertConfig::setAlertEnable( bool value )
  {
    alertEnable = value;
  }

  QString SingleAlertConfig::getAlertSource() const
  {
    return alertSource;
  }

  void SingleAlertConfig::setAlertSource( const QString &value )
  {
    alertSource = value;
  }

  qint16 SingleAlertConfig::getAlertDuration() const
  {
    return alertDuration;
  }

  void SingleAlertConfig::setAlertDuration( const qint16 &value )
  {
    alertDuration = value;
  }

  QString SingleAlertConfig::getAlertType() const
  {
    return alertType;
  }

  void SingleAlertConfig::setAlertType( const QString &value )
  {
    alertType = value;
  }

  QStringList SingleAlertConfig::getAlertDays() const
  {
    return alertDays;
  }

  void SingleAlertConfig::setAlertDays( const QStringList &value )
  {
    alertDays = value;
  }

  QString SingleAlertConfig::getAlertLocation() const
  {
    return alertLocation;
  }

  void SingleAlertConfig::setAlertLocation( const QString &value )
  {
    alertLocation = value;
  }

  bool SingleAlertConfig::getAlertIsBusy() const
  {
    return alertIsBusy;
  }

  void SingleAlertConfig::setAlertIsBusy( bool value )
  {
    alertIsBusy = value;
  }

  QString SingleAlertConfig::getAlertName() const
  {
    return alertName;
  }

  void SingleAlertConfig::setAlertName( const QString &value )
  {
    alertName = value;
  }

}  // namespace radioalert
