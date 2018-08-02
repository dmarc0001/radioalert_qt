#ifndef ASYNCALERTCOMMAND_HPP
#define ASYNCALERTCOMMAND_HPP

#include <QtCore/QObject>
#include <QtCore/qglobal.h>

class AsyncAlertCommand : public QObject
{
    Q_OBJECT
  public:
    explicit AsyncAlertCommand(QObject *parent = nullptr);

  signals:

  public slots:
};

#endif // ASYNCALERTCOMMAND_HPP