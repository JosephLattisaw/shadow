#include "core_client.h"

namespace  {
    const int SERVER_CONNECTION_TIMEOUT_MSECS = 10000;
    const int SERVER_ATTEMPT_TO_RECONNECT_TIMEOUT_MSECS = 1000;
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

    connect(connection_timer, SIGNAL(timeout()), this, SLOT(connect_to_server()));
    connection_timer->setSingleShot(true); //gives us a little bit more control when calling the timer vs and endlessly running timer
    connect_to_server();
}

void Core_Client::connected() {
    emit connected_alert();
}

void Core_Client::disconnected() {
    emit disconnected_alert();
    connect_to_server(); //start attempting to connect to server
}

void Core_Client::ready_read() {
    //TODO
}

void Core_Client::connect_to_server() {
    socket->connectToHost(_hostname, _port);
    if(!socket->waitForConnected(SERVER_CONNECTION_TIMEOUT_MSECS)) connection_timer->start(SERVER_ATTEMPT_TO_RECONNECT_TIMEOUT_MSECS);
}
