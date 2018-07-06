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
  const QString AlertConfig::alertTemplate( "alert-%1" );
  const QString AlertConfig::dateFormatToken( "yyyy-MM-dd" );
  const QString AlertConfig::timeFormatToken( "hh:mm" );

  AlertConfig::AlertConfig( void )
  {
  }

  bool AlertConfig::loadSettings( QSettings &settings, RadioAlertList &alerts )
  {
    QString tempStr;
    qint8 alertCount = 0;
    qDebug() << "";
    qDebug() << "try to load alert settings...";
    //
    // sicherheitshalber alles entfernen
    //
    alerts.clear();
    //
    // alle child groups durchsuchen, da ich in der obersten ebene bin
    // findet das alle Gruppen, welche meinem Muster entsprechen
    //
    QStringList groups = settings.childGroups().filter( alertGroupToken );
    QList< QString >::iterator sli;
    //
    // Alle Gruppen abarbeiten
    //
    for ( sli = groups.begin(); sli != groups.end(); sli++ )
    {
      qDebug().nospace().noquote() << endl << "load " << *sli;
      settings.beginGroup( *sli );
      SingleAlertConfig currAlert;
      //
      // Name des Alarms
      //
      qDebug().noquote().nospace() << "alert name: " << *sli;
      currAlert.setAlertName( *sli );
      //
      // Datum, falls vorhanden
      //
      tempStr = settings.value( dateKey, "" ).toString();
      if ( tempStr.isEmpty() || tempStr.isNull() )
      {
        //
        // Nulldatum ==> Kein Datum
        //
        qDebug().nospace().noquote() << "no date...";
        currAlert.setAlertDate( QDate() );
      }
      else
      {
        //
        // Datum aus dem String erzeugen, Muster JAHR-Monat-Tag (ISO Format)
        //
        qDebug().nospace().noquote() << "date for alert <" << settings.value( dateKey ).toString() << ">";
        currAlert.setAlertDate( QDate::fromString( settings.value( dateKey ).toString(), dateFormatToken ) );
      }
      //
      // Zeit, zwingend...
      //
      qDebug().nospace().noquote() << "time for alert: <" << settings.value( timeKey, "06:00" ).toString() << ">";
      currAlert.setAlertTime( QTime::fromString( settings.value( timeKey, "06:00" ).toString(), timeFormatToken ) );
      //
      // ansteigende Lautstärke?
      //
      qDebug().nospace().noquote() << "raising volume for alert: <" << settings.value( raiseVolKey, "false" ).toString() << ">";
      currAlert.setAlertRaiseVol( settings.value( raiseVolKey, "false" ).toBool() );
      //
      // Lautstärke
      //
      qDebug().nospace().noquote() << "alert volume: <" << settings.value( volumeKey, "10" ).toString() << ">";
      currAlert.setAlertVolume( static_cast< qint8 >( settings.value( volumeKey, "10" ).toInt() & 0xff ) );
      //
      // Geräte
      //
      tempStr = settings.value( devicesKey, "" ).toString();
      qDebug().nospace().noquote() << "devices (name) for alert: <" << tempStr << ">";
      currAlert.setAlertDevices( QStringList( tempStr.split( "," ) ) );
      //
      // source account, falls vorhanden
      //
      qDebug().nospace().noquote() << "source account (if availible): <" << settings.value( sourceAccountKey, "" ).toString() << ">";
      currAlert.setSourceAccount( settings.value( sourceAccountKey, "" ).toString() );
      //
      // note (bemerkungen)
      //
      qDebug().nospace().noquote() << "note for alert: <" << settings.value( noteKey, "" ).toString() << ">";
      currAlert.setAlertNote( settings.value( noteKey, "" ).toString() );
      //
      // enable alert
      //
      qDebug().nospace().noquote() << "alert is enabled: <" << settings.value( alertEnableKey, "true" ).toString() << ">";
      currAlert.setAlertEnable( settings.value( alertEnableKey, "true" ).toBool() );
      //
      // Absplielquelle
      //
      qDebug().nospace().noquote() << "alert radio source <" << settings.value( sourceKey, "PRESET_1" ).toString() << ">";
      currAlert.setAlertSource( settings.value( sourceKey, "PRESET_1" ).toString() );
      //
      // Weckzeit Länge in Sekunden
      //
      qDebug().nospace().noquote() << "alert duration: <" << settings.value( durationKey, "600" ).toString() << ">";
      currAlert.setAlertDuration( settings.value( durationKey, "600" ).toInt() );
      //
      // Typ der Quelle
      //
      qDebug().nospace().noquote() << "type of source for radio: <" << settings.value( typeKey, "" ).toString() << ">";
      currAlert.setAlertType( settings.value( typeKey, "" ).toString() );
      //
      // tage, an denen er weckt. Leer ==> jeden Tag
      //
      tempStr = settings.value( daysKey, "" ).toString();
      qDebug().nospace().noquote() << "days to alert (empty if every day): <" << tempStr << ">";
      currAlert.setAlertDays( QStringList( tempStr.split( "," ) ) );
      //
      // location speichern
      //
      currAlert.setAlertLocation( settings.value( locationKey, "" ).toString() );
      //
      // FERTIG
      // einfügen in die nummerierte Liste
      //
      alerts.insert( alertCount++, currAlert );
      settings.endGroup();
    }  // nächste Gruppe
    return ( true );
  }

  bool AlertConfig::saveSettings( QSettings &settings, RadioAlertList &alerts )
  {
    //
    // Iterator für das Alert-Config-Objekt
    //
    RadioAlertList::iterator ral;
    //
    // Alle alert-Objekte durch
    // settings ist ohne alert objekte beim start
    //
    for ( ral = alerts.begin(); ral != alerts.end(); ral++ )
    {
      //
      // Gruppe öffnen
      //
      qDebug().nospace().noquote() << endl << "save " << ral.key();
      settings.beginGroup( alertTemplate.arg( ral.key(), 2, 10, QLatin1Char( '0' ) ) );
      //
      // Datum des Alarms oder leer
      //
      if ( ral->getAlertDate().isValid() )
      {
        qDebug().nospace().noquote() << "save date for alert: <" << ral->getAlertDate().toString( dateFormatToken ) << ">";
        settings.setValue( dateKey, ral->getAlertDate().toString( dateFormatToken ) );
      }
      else
      {
        qDebug() << "save empty alert date...";
        settings.setValue( dateKey, "" );
      }
      //
      // Zeit, zwingend...
      //
      qDebug().nospace().noquote() << "save time for alert: <" << ral->getAlertTime().toString( timeFormatToken ) << ">";
      settings.setValue( timeKey, ral->getAlertTime().toString( timeFormatToken ) );
      //
      // ansteigende Lautstärke?
      //
      qDebug().nospace().noquote() << "save raising volume for alert: <" << ral->getAlertRaiseVol() << ">";
      settings.setValue( raiseVolKey, ( ral->getAlertRaiseVol() ) ? "true" : "false" );
      //
      // Lautstärke
      //
      qDebug().nospace().noquote() << "save alert volume: <" << ral->getAlertVolume() << ">";
      settings.setValue( volumeKey, ral->getAlertVolume() );
      //
      // Geräte
      //
      qDebug().nospace().noquote() << "save alert devices: <" << ral->getAlertDevices().join( "," ) << ">";
      settings.setValue( devicesKey, ral->getAlertDevices().join( "," ) );
      //
      // source account, falls vorhanden
      //
      qDebug().nospace().noquote() << "save source account (if availible): <" << ral->getSourceAccount() << ">";
      settings.setValue( sourceAccountKey, ral->getSourceAccount() );
      //
      // note (bemerkungen)
      //
      qDebug().nospace().noquote() << "save note for alert (if availible): <" << ral->getAlertNote() << ">";
      settings.setValue( noteKey, ral->getAlertNote() );
      //
      // enable alert
      //
      qDebug().nospace().noquote() << "save alert is enabled: <" << ( ral->getAlertEnable() ? "true" : "false" ) << ">";
      settings.setValue( alertEnableKey, ral->getAlertEnable() ? "true" : "false" );
      //
      // Absplielquelle
      //
      qDebug().nospace().noquote() << "save alert radio source <" << ral->getAlertSource() << ">";
      settings.setValue( sourceKey, ral->getAlertSource() );
      //
      // Weckzeit Länge in Sekunden
      //
      qDebug().nospace().noquote() << "save alert duration: <" << ral->getAlertDuration() << ">";
      settings.setValue( durationKey, ral->getAlertDuration() );
      //
      // Typ der Quelle
      //
      qDebug().nospace().noquote() << "save type of source for radio: <" << ral->getAlertType() << ">";
      settings.setValue( typeKey, ral->getAlertType() );
      //
      // tage, an denen er weckt. Leer ==> jeden Tag
      //
      qDebug().nospace().noquote() << "save days to alert (empty if every day) <" << ral->getAlertDays().join( "," ) << ">";
      settings.setValue( daysKey, ral->getAlertDays().join( "," ) );
      //
      // location speichern
      //
      qDebug().nospace().noquote() << "save location: <" << ral->getAlertLocation() << ">";
      settings.setValue( locationKey, ral->getAlertLocation() );
      // Gruppe verlassen
      settings.endGroup();
    }
    return ( true );
  }

}  // namespace radioalert
