#include "core_client.h"
#include "core_tcp_message_types.hpp"

namespace  {
const int SERVER_CONNECTION_TIMEOUT_MSECS = 10000;
const int SERVER_ATTEMPT_TO_RECONNECT_TIMEOUT_MSECS = 1000;
const int SERVER_KEEP_ALIVE_TIMEOUT_MSECS = 20000;
}

Core_Client::Core_Client(QString hostname, uint16_t port, QObject *parent)
    : _hostname(hostname),
      _port(port),
      QObject(parent)
{}

void Core_Client::start_thread() {
    assert(!thread_started);
    thread_started = true;

    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(ready_read()));

    //creating connection timer to continuously attempt to connect to server every so often
    connection_timer = new QTimer(this);
    connect(connection_timer, SIGNAL(timeout()), this, SLOT(connect_to_server()));
    connection_timer->setSingleShot(true); //gives us a little bit more control when calling the timer vs and endlessly running timer

    keep_alive_timer = new QTimer(this);
    connect(keep_alive_timer, SIGNAL(timeout()), this, SLOT(keep_alive_timeout()));
    keep_alive_timer->setSingleShot(true);

    connect_to_server(); //attempting to connect to server
}

void Core_Client::connected() {
    qDebug() << "connected to server";
    emit connected_alert();
    keep_alive_timer->start(SERVER_KEEP_ALIVE_TIMEOUT_MSECS);
}

void Core_Client::disconnected() {
    qDebug() << "disconnected from server";
    emit disconnected_alert();
    socket->close(); //just in case
    clear_buffers();
    keep_alive_timer->stop();
    connect_to_server(); //start attempting to connect to server
}

void Core_Client::ready_read() {
    while(socket != nullptr && socket->bytesAvailable()) {
        header_buffer.append(socket->read(sizeof(core::header) - header_buffer.size()));

        if(header_buffer.size() == sizeof(core::header)) {
            core::header *hdr = reinterpret_cast<core::header*>(header_buffer.data());
            data_buffer.append(socket->read(hdr->packet_size - data_buffer.size()));

            if(hdr->packet_size == data_buffer.size()) {
                switch (hdr->message_type) {
                case core::header::MESSAGE_TYPE::KEEP_ALIVE: //received keep alive packet
                    keep_alive_timer->stop(); //stopping keep alive from timing out
                    send_keep_alive_packet(); //sending keep alive packet back to server
                    keep_alive_timer->start(SERVER_KEEP_ALIVE_TIMEOUT_MSECS); //restarting keep alive timer
                    break;
                case core::header::MESSAGE_TYPE::START:
                    emit start_all();
                    break;
                case core::header::MESSAGE_TYPE::STOP:
                    emit stop_all();
                    break;
                default:
                    qDebug() << "received unknown packet type, kicking off server";
                    disconnect_from_server();
                }

                clear_buffers(); //clearing buffers now that all data has been read
            }
        }
    }
}

void Core_Client::connect_to_server() {
    qDebug() << "attempting to connect to server, host:" << _hostname << "port:" << _port;
    socket->connectToHost(_hostname, _port);
    if(!socket->waitForConnected(SERVER_CONNECTION_TIMEOUT_MSECS)) {
        qDebug() << "waiting for connection timed out, retrying in" << SERVER_ATTEMPT_TO_RECONNECT_TIMEOUT_MSECS / 1000 << "seconds";
        connection_timer->start(SERVER_ATTEMPT_TO_RECONNECT_TIMEOUT_MSECS);
    }
}

void Core_Client::keep_alive_timeout() {
    qDebug() << "keep alive timed out, disconnecting from server";
    disconnect_from_server(); //disconnecting from server
}

void Core_Client::disconnect_from_server() {
    socket->close(); //closing socket just in case
}

void Core_Client::send_keep_alive_packet() {
    core::header header;
    header.message_type = core::header::KEEP_ALIVE;
    socket->write(reinterpret_cast<char*>(&header), sizeof (header));
}

void Core_Client::clear_buffers() {
    header_buffer.clear();
    data_buffer.clear();
}
