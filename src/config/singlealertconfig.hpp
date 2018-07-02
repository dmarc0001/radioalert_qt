#ifndef SRC_CONFIG_SINGLEALERTCONFIG_HPP
#define SRC_CONFIG_SINGLEALERTCONFIG_HPP

#include <qglobal.h>
#include <QObject>

namespace radioalert
{
  class SingleAlertConfig : public QObject
  {
    Q_OBJECT
    public:
    explicit SingleAlertConfig( QObject *parent = nullptr );

    signals:

    public slots:
  };
}  // namespace radioalert
#endif  // SINGLEALERTCONFIG_HPP
