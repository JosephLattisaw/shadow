#ifndef CORE_CLIENT_H
#define CORE_CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>

class Core_Client : public QObject
{
    Q_OBJECT
public:
    explicit Core_Client(QString hostname, std::uint16_t port, QObject *parent = nullptr);

signals:
    void connected_alert();
    void disconnected_alert();

public slots:
    void start_thread();

private slots:
    void connected();
    void disconnected();
    void ready_read();

    void connect_to_server();
    void disconnect_from_server();

    void keep_alive_timeout();

private:
    void clear_buffers();
    void send_keep_alive_packet();

    bool thread_started = false;
    QTcpSocket *socket = nullptr;

    QTimer *connection_timer;

    QString _hostname;
    std::uint16_t _port;

    QTimer *keep_alive_timer;

    QByteArray header_buffer;
    QByteArray data_buffer;
};

#endif // CORE_CLIENT_H
