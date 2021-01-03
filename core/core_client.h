#ifndef CORE_CLIENT_H
#define CORE_CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include "defines.hpp"

class Core_Client : public QObject
{
    Q_OBJECT
public:
    explicit Core_Client(QString hostname, std::uint16_t port, QObject *parent = nullptr);

    void set_application_data(QStringList app_names, QStringList command_line_args);

signals:
    void connected_alert();
    void disconnected_alert();

    void start_all();
    void stop_all();

public slots:
    void start_thread();

    void set_process_status_table(QVector<shadow::APP_STATUS> statuses);

private slots:
    void connected();
    void disconnected();
    void ready_read();

    void connect_to_server();
    void disconnect_from_server();

    void keep_alive_timeout();
    void status_update_timeout();

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

    QVector<QByteArray> _app_names;
    QVector<QByteArray> _command_line_arguments;
    QVector<shadow::APP_STATUS> process_statuses;

    QTimer *status_update_timer;
};

#endif // CORE_CLIENT_H
