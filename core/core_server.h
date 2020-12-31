#ifndef Core_Server_H
#define Core_Server_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class Core_Server : public QObject
{
    Q_OBJECT
public:
    explicit Core_Server(std::uint16_t port, QObject *parent = nullptr);

signals:

public slots:
    void start_thread();

private slots:
    void disconnected();
    void new_connection();
    void ready_read();

private:
    void clear_buffers();
    void close_socket();
    void listen_for_connections();

    QTcpServer *server = nullptr;
    QTcpSocket *socket = nullptr;

    bool thread_started = false;
    std::uint16_t _port;
};

#endif // Core_Server_H
