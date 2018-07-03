#include "singlealertconfig.hpp"

namespace radioalert
{
  SingleAlertConfig::SingleAlertConfig( void )
  {
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

}  // namespace radioalert
