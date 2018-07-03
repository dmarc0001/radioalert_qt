#include "alertconfig.hpp"
#include <QDate>
#include <QDebug>
#include <QList>
#include <QStringList>
#include <QStringListIterator>
#include <QTime>

namespace radioalert
{
  const QString AlertConfig::dateKey( "date" );
  const QString AlertConfig::timeKey( "time" );
  const QString AlertConfig::raiseVolKey( "raise_vol" );
  const QString AlertConfig::volumeKey( "volume" );
  const QString AlertConfig::devicesKey( "devices" );
  const QString AlertConfig::sourceAccountKey( "source_account" );
  const QString AlertConfig::noteKey( "note" );
  const QString AlertConfig::alertEnableKey( "enable" );
  const QString AlertConfig::sourceKey( "source" );
  const QString AlertConfig::durationKey( "duration" );
  const QString AlertConfig::typeKey( "type" );
  const QString AlertConfig::daysKey( "days" );
  const QString AlertConfig::locationKey( "location" );
  const QRegExp AlertConfig::alertGroupToken( QString( "alert-\\d{2}" ) );

  AlertConfig::AlertConfig( void )
  {
  }

  bool AlertConfig::loadSettings( QSettings &settings, QMap< qint8, SingleAlertConfig > *alerts )
  {
    QString tempStr;
    qDebug() << "";
    //
    // alle child groups durchsuchen, da ich in der obersten ebene bin
    // findet das alle Gruppen, welche meinem Muster entsprechen
    //
    QStringList groups = settings.childGroups().filter( alertGroupToken );
    QList< QString >::iterator sli;
    for ( sli = groups.begin(); sli != groups.end(); sli++ )
    {
      qDebug() << "load " << *sli;
      settings.beginGroup( *sli );
      SingleAlertConfig currAlert;
      //
      // Datum, falls vorhanden
      //
      tempStr = settings.value( dateKey, "" ).toString();
      if ( tempStr.isEmpty() || tempStr.isNull() )
      {
        //
        // Nulldatum ==> Kein Datum
        //
        qDebug() << "no date...";
        currAlert.setAlertDate( QDate() );
      }
      else
      {
        //
        // Datum aus dem String erzeugen, Muster JAHR-Monat-Tag (ISO Format)
        //
        qDebug() << "date for alert <" << settings.value( dateKey ).toString() << ">";
        currAlert.setAlertDate( QDate::fromString( settings.value( dateKey ).toString(), "yyyy-MM-dd" ) );
      }
      //
      // Zeit, zwingend...
      //
      qDebug() << "time for alert: <" << settings.value( timeKey, "06:00" ).toString() << ">";
      currAlert.setAlertTime( QTime::fromString( settings.value( timeKey, "06:00" ).toString(), "hh:mm" ) );
      //
      // ansteigende Lautstärke?
      //
      qDebug() << "raising volume for alert: <" << settings.value( raiseVolKey, "false" ).toString() << ">";
      currAlert.setAlertRaiseVol( settings.value( raiseVolKey, "false" ).toBool() );
      //
      // Lautstärke
      //
      qDebug() << "alert volume: <" << settings.value( volumeKey, "10" ).toString() << ">";
      currAlert.setAlertVolume( static_cast< qint8 >( settings.value( volumeKey, "10" ).toInt() & 0xff ) );
      //
      // Geräte
      //
      tempStr = settings.value( devicesKey, "" ).toString();
      qDebug() << "devices (name) for alert: <" << tempStr << ">";
      currAlert.setAlertDevices( QStringList( tempStr.split( "," ) ) );
      //
      // source account, falls vorhanden
      //
      qDebug() << "source account (if availible): <" << settings.value( sourceAccountKey, "" ).toString() << ">";
      currAlert.setSourceAccount( settings.value( sourceAccountKey, "" ).toString() );
      //
      // note (bemerkungen)
      //
      qDebug() << "note for alert: <" << settings.value( noteKey, "" ).toString() << ">";
      currAlert.setAlertNote( settings.value( noteKey, "" ).toString() );
      //
      // enable alert
      //
      qDebug() << "alert is enabled: <" << settings.value( alertEnableKey, "true" ).toString() << ">";
      currAlert.setAlertEnable( settings.value( alertEnableKey, "true" ).toBool() );
      //
      // Absplielquelle
      //
      qDebug() << "alert radio source <" << settings.value( sourceKey, "PRESET_1" ).toString() << ">";
      currAlert.setAlertSource( settings.value( sourceKey, "PRESET_1" ).toString() );
      //
      // Weckzeit Länge in Sekunden
      //
      qDebug() << "alert duration: <" << settings.value( durationKey, "600" ).toString() << ">";
      currAlert.setAlertDuration( settings.value( durationKey, "600" ).toInt() );
      //
      // Typ der Quelle
      //
      qDebug() << "type of source for radio: <" << settings.value( typeKey, "" ).toString() << ">";
      currAlert.setAlertType( settings.value( typeKey, "" ).toString() );
      //
      // tage, an denen er weckt. Leer ==> jeden Tag
      //
      tempStr = settings.value( daysKey, "" ).toString();
      qDebug() << "days to alert (empty if every day): <" << tempStr << ">";
      currAlert.setAlertDays( QStringList( tempStr.split( "," ) ) );
      //
      // location speichern
      //
      currAlert.setAlertLocation( settings.value( locationKey, "" ).toString() );
      //
      // FERTIG
      //
      settings.endGroup();
    }
    return ( true );
  }

  bool AlertConfig::saveSettings( QSettings &settings, QMap< qint8, SingleAlertConfig > *alerts )
  {
  }

}  // namespace radioalert
