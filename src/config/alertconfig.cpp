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
  // const QString AlertConfig::alertTemplate( "alert-%1" );
  const QString AlertConfig::dateFormatToken( "yyyy-MM-dd" );
  const QString AlertConfig::timeFormatToken( "hh:mm" );

  AlertConfig::AlertConfig( void )
  {
  }

  bool AlertConfig::loadSettings( QSettings &settings, RadioAlertList &alerts )
  {
    QString tempStr;
    qDebug() << "";
    qDebug() << QLatin1String( "try to load alert settings..." );
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
      qDebug().nospace().noquote() << endl << QLatin1String( "load " ) << *sli;
      settings.beginGroup( *sli );
      SingleAlertConfig currAlert;
      //
      // Name des Alarms
      //
      qDebug().noquote().nospace() << QLatin1String( "alert name: " ) << *sli;
      currAlert.setAlertName( *sli );
      //
      // Datum, falls vorhanden
      //
      tempStr = settings.value( dateKey, QLatin1String( "" ) ).toString();
      if ( tempStr.isEmpty() || tempStr.isNull() )
      {
        //
        // Nulldatum ==> Kein Datum
        //
        qDebug().nospace().noquote() << QLatin1String( "no date..." );
        currAlert.setAlertDate( QDate() );
      }
      else
      {
        //
        // Datum aus dem String erzeugen, Muster JAHR-Monat-Tag (ISO Format)
        //
        qDebug().nospace().noquote() << QLatin1String( "date for alert <" ) << settings.value( dateKey ).toString()
                                     << QLatin1String( ">" );
        currAlert.setAlertDate( QDate::fromString( settings.value( dateKey ).toString(), dateFormatToken ) );
      }
      //
      // Zeit, zwingend...
      //
#ifdef DEBUG
      qDebug().nospace().noquote() << QLatin1String( "time for alert: <" ) << settings.value( timeKey, QLatin1String( "" ) ).toString()
                                   << QLatin1String( ">" );
      if ( settings.value( timeKey, QLatin1String( "" ) ).toString() == QLatin1String( "now" ) )
      {
        //
        // Leeres, ungültiges Objekt als Kennzeichnung erstellen
        //
        currAlert.setAlertTime( QTime() );
      }
      else
      {
        currAlert.setAlertTime( QTime::fromString( settings.value( timeKey, QLatin1String( "06:00" ) ).toString(), timeFormatToken ) );
      }
#else
      qDebug().nospace().noquote() << QLatin1String( "time for alert: <" )
                                   << settings.value( timeKey, QLatin1String( "06:00" ) ).toString() << QLatin1String( ">" );
      currAlert.setAlertTime( QTime::fromString( settings.value( timeKey, QLatin1String( "06:00" ) ).toString(), timeFormatToken ) );
      if ( !currAlert.getAlertTime().isValid() )
      {
        //
        // FEHLER, jetzt Notmaßnahme ergreifen
        //
        qWarning() << QLatin1String( "alert time in config is not valid! reset to 06:00am..." );
        currAlert.setAlertTime( QTime::fromString( QLatin1String( "06:00" ), timeFormatToken ) );
      }
#endif
      //
      // ansteigende Lautstärke?
      //
      qDebug().nospace().noquote() << QLatin1String( "raising volume for alert: <" )
                                   << settings.value( raiseVolKey, QLatin1String( "false" ) ).toString() << QLatin1String( ">" );
      currAlert.setAlertRaiseVol( settings.value( raiseVolKey, "false" ).toBool() );
      //
      // Lautstärke
      //
      qDebug().nospace().noquote() << QLatin1String( "alert volume: <" )
                                   << settings.value( volumeKey, QLatin1String( "10" ) ).toString() << QLatin1String( ">" );
      currAlert.setAlertVolume( static_cast< qint8 >( settings.value( volumeKey, QLatin1String( "10" ) ).toInt() & 0xff ) );
      //
      // Geräte
      //
      tempStr = settings.value( devicesKey, QLatin1String( "" ) ).toString();
      qDebug().nospace().noquote() << QLatin1String( "devices (name) for alert: <" ) << tempStr << QLatin1String( ">" );
      currAlert.setAlertDevices( QStringList( tempStr.split( QLatin1String( "," ) ) ) );
      //
      // source account, falls vorhanden
      //
      qDebug().nospace().noquote() << QLatin1String( "source account (if availible): <" )
                                   << settings.value( sourceAccountKey, QLatin1String( "" ) ).toString() << QLatin1String( ">" );
      currAlert.setSourceAccount( settings.value( sourceAccountKey, QLatin1String( "" ) ).toString() );
      //
      // note (bemerkungen)
      //
      qDebug().nospace().noquote() << QLatin1String( "note for alert: <" ) << settings.value( noteKey, QLatin1String( "" ) ).toString()
                                   << QLatin1String( ">" );
      currAlert.setAlertNote( settings.value( noteKey, "" ).toString() );
      //
      // enable alert
      //
      qDebug().nospace().noquote() << QLatin1String( "alert is enabled: <" )
                                   << settings.value( alertEnableKey, QLatin1String( "true" ) ).toString() << QLatin1String( ">" );
      currAlert.setAlertEnable( settings.value( alertEnableKey, QLatin1String( "true" ) ).toBool() );
      //
      // Absplielquelle
      //
      qDebug().nospace().noquote() << QLatin1String( "alert radio source <" )
                                   << settings.value( sourceKey, QLatin1String( "PRESET_1" ) ).toString() << QLatin1String( ">" );
      currAlert.setAlertSource( settings.value( sourceKey, QLatin1String( "PRESET_1" ) ).toString() );
      //
      // Weckzeit Länge in Sekunden
      //
      qDebug().nospace().noquote() << QLatin1String( "alert duration: <" )
                                   << settings.value( durationKey, QLatin1String( "600" ) ).toString() << QLatin1String( ">" );
      currAlert.setAlertDuration( settings.value( durationKey, "600" ).toInt() );
      //
      // Typ der Quelle
      //
      qDebug().nospace().noquote() << QLatin1String( "type of source for radio: <" )
                                   << settings.value( typeKey, QLatin1String( "" ) ).toString() << QLatin1String( ">" );
      currAlert.setAlertType( settings.value( typeKey, QLatin1String( "" ) ).toString() );
      //
      // tage, an denen er weckt. Leer ==> jeden Tag
      //
      tempStr = settings.value( daysKey, QLatin1String( "" ) ).toString();
      qDebug().nospace().noquote() << QLatin1String( "days to alert (empty if every day): <" ) << tempStr << QLatin1String( ">" );
      currAlert.setAlertDays( QStringList( tempStr.split( QLatin1String( "," ) ) ) );
      //
      // location speichern
      //
      currAlert.setAlertLocation( settings.value( locationKey, QLatin1String( "" ) ).toString() );
      //
      // FERTIG
      // einfügen in die nummerierte Liste
      //
      alerts.insert( *sli, currAlert );
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
      qDebug().nospace().noquote() << endl << QLatin1String( "save " ) << ral.key();
      settings.beginGroup( ral.key() );
      //
      // Datum des Alarms oder leer
      //
      if ( ral->getAlertDate().isValid() )
      {
        qDebug().nospace().noquote() << QLatin1String( "save date for alert: <" ) << ral->getAlertDate().toString( dateFormatToken )
                                     << QLatin1String( ">" );
        settings.setValue( dateKey, ral->getAlertDate().toString( dateFormatToken ) );
      }
      else
      {
        qDebug() << QLatin1String( "save empty alert date..." );
        settings.setValue( dateKey, "" );
      }
      //
      // Zeit, zwingend...
      //
      qDebug().nospace().noquote() << QLatin1String( "save time for alert: <" ) << ral->getAlertTime().toString( timeFormatToken )
                                   << QLatin1String( ">" );
#ifdef DEBUG
      if ( !ral->getAlertTime().isValid() )
      {
        settings.setValue( timeKey, QLatin1String( "now" ) );
      }
      else
      {
        settings.setValue( timeKey, ral->getAlertTime().toString( timeFormatToken ) );
      }
#else
      settings.setValue( timeKey, ral->getAlertTime().toString( timeFormatToken ) );
#endif
      //
      // ansteigende Lautstärke?
      //
      qDebug().nospace().noquote() << QLatin1String( "save raising volume for alert: <" ) << ral->getAlertRaiseVol()
                                   << QLatin1String( ">" );
      settings.setValue( raiseVolKey, ( ral->getAlertRaiseVol() ) ? QLatin1String( "true" ) : QLatin1String( "false" ) );
      //
      // Lautstärke
      //
      qDebug().nospace().noquote() << QLatin1String( "save alert volume: <" ) << ral->getAlertVolume() << QLatin1String( ">" );
      settings.setValue( volumeKey, ral->getAlertVolume() );
      //
      // Geräte
      //
      qDebug().nospace().noquote() << QLatin1String( "save alert devices: <" ) << ral->getAlertDevices().join( QLatin1String( "," ) )
                                   << QLatin1String( ">" );
      settings.setValue( devicesKey, ral->getAlertDevices().join( QLatin1String( "," ) ) );
      //
      // source account, falls vorhanden
      //
      qDebug().nospace().noquote() << QLatin1String( "save source account (if availible): <" ) << ral->getSourceAccount()
                                   << QLatin1String( ">" );
      settings.setValue( sourceAccountKey, ral->getSourceAccount() );
      //
      // note (bemerkungen)
      //
      qDebug().nospace().noquote() << QLatin1String( "save note for alert (if availible): <" ) << ral->getAlertNote()
                                   << QLatin1String( ">" );
      settings.setValue( noteKey, ral->getAlertNote() );
      //
      // enable alert
      //
      qDebug().nospace().noquote() << QLatin1String( "save alert is enabled: <" )
                                   << ( ral->getAlertEnable() ? QLatin1String( "true" ) : QLatin1String( "false" ) ) << ">";
      settings.setValue( alertEnableKey, ral->getAlertEnable() ? QLatin1String( "true" ) : QLatin1String( "false" ) );
      //
      // Absplielquelle
      //
      qDebug().nospace().noquote() << QLatin1String( "save alert radio source <" ) << ral->getAlertSource() << QLatin1String( ">" );
      settings.setValue( sourceKey, ral->getAlertSource() );
      //
      // Weckzeit Länge in Sekunden
      //
      qDebug().nospace().noquote() << QLatin1String( "save alert duration: <" ) << ral->getAlertDuration() << QLatin1String( ">" );
      settings.setValue( durationKey, ral->getAlertDuration() );
      //
      // Typ der Quelle
      //
      qDebug().nospace().noquote() << QLatin1String( "save type of source for radio: <" ) << ral->getAlertType()
                                   << QLatin1String( ">" );
      settings.setValue( typeKey, ral->getAlertType() );
      //
      // tage, an denen er weckt. Leer ==> jeden Tag
      //
      qDebug().nospace().noquote() << QLatin1String( "save days to alert (empty if every day) <" )
                                   << ral->getAlertDays().join( QLatin1String( "," ) ) << QLatin1String( ">" );
      settings.setValue( daysKey, ral->getAlertDays().join( QLatin1String( "," ) ) );
      //
      // location speichern
      //
      qDebug().nospace().noquote() << QLatin1String( "save location: <" ) << ral->getAlertLocation() << QLatin1String( ">" );
      settings.setValue( locationKey, ral->getAlertLocation() );
      // Gruppe verlassen
      settings.endGroup();
    }
    return ( true );
  }

}  // namespace radioalert
