#include "core_server.h"

namespace  {
    const QHostAddress::SpecialAddress CLIENT_HOSTNAME = QHostAddress::Any;
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
    connect(server, SIGNAL(new_connection()), this, SLOT(new_connection()));

    listen_for_connections(); //start listening for connections
}

void Core_Server::new_connection() {
    if(socket == nullptr) {
        qDebug() << "server is accepting new connection" << _port;
        clear_buffers();
        socket = server->nextPendingConnection(); //take on socket connection
        connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
        connect(socket, SIGNAL(readyRead()), this, SLOT(ready_read()));
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
    qDebug() << "socket was disconnected";
    clear_buffers();
    close_socket(); //calling this just in case and for socket object clean up
}

void Core_Server::disconnected() {
    qDebug() << "socket was disconnected";
    clear_buffers();
    close_socket(); //calling this just in case and for socket object clean up
}

void Core_Server::ready_read() {
    //TODO
}

void Core_Server::close_socket() {
    if(socket != nullptr) {
        qDebug() << "closing previous socket connection";
        disconnect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
        disconnect(socket, SIGNAL(readyRead()), this, SLOT(ready_read()));
        socket = nullptr; //just in case
    }

    clear_buffers();
    listen_for_connections(); //start listening for new connections
}
