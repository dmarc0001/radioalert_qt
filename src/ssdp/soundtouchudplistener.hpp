#ifndef SOUNDTOUCHUDPLISTENER_H
#define SOUNDTOUCHUDPLISTENER_H

#include <QObject>
#include <QUdpSocket>

#include "iputilitys.hpp"

namespace radio
{
  class SoundTouchUDPListener : public QUdpSocket
  {
    Q_OBJECT
    public:
    /*!
     * Constructor.
     *
     * \param address ip address that will be bind to the socket.
     * \param port port that will be bind to the socket.
     * \param objectName name of the object or class that will be using this
     * one. Used on warning messages.
     * \param parent parent.
     */
    SoundTouchUDPListener( QString address, quint32 port, QString objectName, QObject *parent = 0 );

    /*!
     * Destructor
     */
    virtual ~SoundTouchUDPListener();

    /*!
     * Starts listening to the address and port passed on constructor.
     */
    void start();

    private:
    /*!
     * \property objectName
     *
     * Object or class name that will be used on debug messages.
     */
    QString objectName;

    /*!
     * \property address
     *
     * IP address that will be bind to the socket.
     */
    QString address;

    /*!
     * \property port
     *
     * Port that will be bind to the socket.
     */
    quint32 port;
  };

}  // namespace radio

#endif  // SOUNDTOUCHUDPLISTENER_H
