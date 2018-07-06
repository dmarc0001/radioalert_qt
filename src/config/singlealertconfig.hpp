#ifndef SRC_CONFIG_SINGLEALERTCONFIG_HPP
#define SRC_CONFIG_SINGLEALERTCONFIG_HPP

#include <qglobal.h>
#include <QDate>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTime>

namespace radioalert
{
  //
  // vorwärts deklaration für using...
  //
  class SingleAlertConfig;
  //
  // für die Lesbarkeit der Sourcen umdefinieren
  //
  using RadioAlertList = QMap< qint8, SingleAlertConfig >;

  class SingleAlertConfig
  {
    private:
    QString alertName;         //! Name des Alarmes (aus der Konfigurationsdatei)
    QDate alertDate;           //! Datum oder leer
    QTime alertTime;           //! Weckzeit, obligatorisch
    bool alertRaiseVol;        //! Lautstärke langsam steigern?
    qint8 alertVolume;         //! Wecker lautstärke
    QStringList alertDevices;  //! auf welchen Geräten wecken?
    QString sourceAccount;     //! Account bei sourcen mit Benutezraccounts (z.B. AMAZON prime)
    QString alertNote;         //! Bemerkungen (auf der GUI anzeigbar)
    bool alertEnable;          //! Alarm aktiv?
    QString alertSource;       //! Medienquelle
    qint16 alertDuration;      //! Länge des Weckers
    QString alertType;         //! Art des Alarms
    QStringList alertDays;     //! an welchen Tagen? (leer == jeden)
    QString alertLocation;     //! Stationsnummer?
    bool alertIsBusy;          //! ist der alarm gerade in Arbeit?

    public:
    explicit SingleAlertConfig( void );
    //
    // ACCESS GETER/SETTER
    //
    QDate getAlertDate() const;
    void setAlertDate( const QDate &value );
    QTime getAlertTime() const;
    void setAlertTime( const QTime &value );
    bool getAlertRaiseVol() const;
    void setAlertRaiseVol( bool value );
    qint8 getAlertVolume() const;
    void setAlertVolume( const qint8 &value );
    QStringList getAlertDevices() const;
    void setAlertDevices( const QStringList &value );
    QString getSourceAccount() const;
    void setSourceAccount( const QString &value );
    QString getAlertNote() const;
    void setAlertNote( const QString &value );
    bool getAlertEnable() const;
    void setAlertEnable( bool value );
    QString getAlertSource() const;
    void setAlertSource( const QString &value );
    qint16 getAlertDuration() const;
    void setAlertDuration( const qint16 &value );
    QString getAlertType() const;
    void setAlertType( const QString &value );
    QStringList getAlertDays() const;
    void setAlertDays( const QStringList &value );
    QString getAlertLocation() const;
    void setAlertLocation( const QString &value );
    bool getAlertIsBusy() const;
    void setAlertIsBusy( bool value );
    QString getAlertName() const;
    void setAlertName( const QString &value );
    QByteArray serialize( void );
  };
}  // namespace radioalert
#endif  // SINGLEALERTCONFIG_HPP
