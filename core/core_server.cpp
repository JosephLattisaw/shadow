#include "core_server.h"
#include "core_tcp_message_types.hpp"

namespace  {
    const QHostAddress::SpecialAddress CLIENT_HOSTNAME = QHostAddress::Any;
    const int KEEP_ALIVE_SEND_PACKET_TIMEOUT_MSECS = 1000;
    const int KEEP_ALIVE_RECEIVE_PACKET_TIMEOUT_MSECS = 20000;
}

Core_Server::Core_Server(uint16_t port, QObject *parent)
    : _port(port),
      QObject(parent)
{

}

void Core_Server::start_thread() {
    assert(!thread_started);
    thread_started = true;

    //creating tcp/ip server
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(new_connection()));

    //create keep alive timers
    keep_alive_send_timer = new QTimer(this);
    keep_alive_receive_timer = new QTimer(this);
    connect(keep_alive_send_timer, SIGNAL(timeout()), this, SLOT(keep_alive_send_timeout()));
    connect(keep_alive_receive_timer, SIGNAL(timeout()), this, SLOT(keep_alive_receive_timeout()));
    keep_alive_send_timer->setSingleShot(true); //not a continuous timer, we want control
    keep_alive_receive_timer->setSingleShot(true);

    listen_for_connections(); //start listening for connections
    keep_alive_send_timeout(); //start sending keep alive packets
}

void Core_Server::new_connection() {
    if(socket == nullptr) {
        qDebug() << "server is accepting new connection" << _port;
        clear_buffers();
        socket = server->nextPendingConnection(); //take on socket connection
        connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
        connect(socket, SIGNAL(readyRead()), this, SLOT(ready_read()));
        keep_alive_receive_timer->start(KEEP_ALIVE_RECEIVE_PACKET_TIMEOUT_MSECS);
    }
    server->close(); //now that we've made connection and we're only doing one we can stop listening
}

void Core_Server::listen_for_connections() {
    if(!server->isListening() && socket == nullptr) {
        bool res = server->listen(CLIENT_HOSTNAME, _port);
        assert(res && server->isListening());
        qDebug() << "server is listening for new connections on port" << _port << server->isListening();
    }
}

void Core_Server::clear_buffers() {
    header_buffer.clear();
    data_buffer.clear();
}

void Core_Server::disconnected() {
    qDebug() << "socket was disconnected";
    close_socket(); //calling this just in case and for socket object clean up
}

void Core_Server::ready_read() {
    while(socket != nullptr && socket->bytesAvailable()) {
        header_buffer.append(socket->read(sizeof(core::header) - header_buffer.size()));

        if(header_buffer.size() == sizeof(core::header)) {
            core::header *hdr = reinterpret_cast<core::header*>(header_buffer.data());
            data_buffer.append(socket->read(hdr->packet_size - data_buffer.size()));

            if(hdr->packet_size == data_buffer.size()) {
                switch (hdr->message_type) {
                    case core::header::MESSAGE_TYPE::KEEP_ALIVE: //received keep alive packet
                    keep_alive_receive_timer->stop(); //stopping the receive timer
                    keep_alive_receive_timer->start(KEEP_ALIVE_RECEIVE_PACKET_TIMEOUT_MSECS); //starting the receive timer
                    break;
                default:
                    qDebug() << "received unknown packet type, kicking off server";
                    close_socket();
                }

                clear_buffers(); //clearing buffers now that all data has been read
            }
        }
    }
}

void Core_Server::close_socket() {
    if(socket != nullptr) {
        qDebug() << "closing previous socket connection";
        disconnect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
        disconnect(socket, SIGNAL(readyRead()), this, SLOT(ready_read()));

        socket->close();
        socket = nullptr; //just in case
    }

    clear_buffers();
    listen_for_connections(); //start listening for new connections
}

void Core_Server::keep_alive_send_timeout() {
    if(socket != nullptr && socket->isOpen()) {
        core::header header;
        header.message_type = core::header::KEEP_ALIVE;
        socket->write(reinterpret_cast<char*>(&header), sizeof (header));
    }
    keep_alive_send_timer->start(KEEP_ALIVE_SEND_PACKET_TIMEOUT_MSECS); //send another keep alive packet after this many seconds
}

void Core_Server::keep_alive_receive_timeout() {
    qDebug() << "keep alive timeout, closing socket connection";
    close_socket();
}
