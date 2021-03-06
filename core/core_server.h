#ifndef Core_Server_H
#define Core_Server_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include "defines.hpp"

class Core_Server : public QObject
{
    Q_OBJECT
public:
    explicit Core_Server(std::uint16_t port, QObject *parent = nullptr);

signals:
    void client_status_update(QVector<QString> client_app_names, QVector<QString> client_app_clas, QVector<shadow::APP_STATUS> statuses);
    void disconnected_from_client();

public slots:
    void start_client_processes();
    void stop_client_processes();

    void start_thread();

private slots:
    void disconnected();
    void new_connection();
    void ready_read();

    void keep_alive_send_timeout();
    void keep_alive_receive_timeout();

private:
    void clear_buffers();
    void close_socket();
    void listen_for_connections();

    QTcpServer *server = nullptr;
    QTcpSocket *socket = nullptr;

    bool thread_started = false;
    std::uint16_t _port;

    QTimer *keep_alive_send_timer;
    QTimer *keep_alive_receive_timer;

    QByteArray header_buffer;
    QByteArray data_buffer;
};

#endif // Core_Server_H
