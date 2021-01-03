#include "core_server.h"
#include "core_tcp_message_types.hpp"
#include "defines.hpp"

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
                case core::header::MESSAGE_TYPE::STATUS:
                {
                    int size = 0;
                    core::status_message sm;

                    //getting the status message
                    memcpy(&sm, data_buffer.mid(size, sizeof(core::status_message)).data(), sizeof (sm));
                    size += sizeof(core::status_message);

                    //getting application name sizes
                    std::uint32_t app_name_sizes[sm.app_name_size];
                    memcpy(&app_name_sizes, data_buffer.mid(size, sm.app_name_size).data(), sm.app_name_size);
                    size += sm.app_name_size;


                    //getting the total number of elements
                    int nbr_elements = (sm.app_name_size / sizeof (app_name_sizes[0])) / sizeof (app_name_sizes[0]);

                    //Getting the client application names
                    QVector<QString> client_app_names;
                    for (auto i = 0; i < nbr_elements; i++) {
                        client_app_names.push_back(QString::fromUtf8(data_buffer.mid(size, app_name_sizes[i])));
                        size += app_name_sizes[i];
                    }

                    //gettings cla name sizes
                    std::uint32_t cla_sizes[sm.cla_size];
                    memcpy(&cla_sizes, data_buffer.mid(size, sm.cla_size).data(), sm.cla_size);
                    size += sm.cla_size;

                    //getting the total number of elements
                    nbr_elements = (sm.cla_size / sizeof (cla_sizes[0])) / sizeof (cla_sizes[0]);

                    //Getting the client application names
                    QVector<QString> client_app_clas;
                    for (auto i = 0; i < nbr_elements; i++) {
                        client_app_clas.push_back(QString::fromUtf8(data_buffer.mid(size, cla_sizes[i])));
                        size += cla_sizes[i];
                    }

                    //getting process sizes
                    std::uint32_t p_sizes[sm.status_size];
                    memcpy(&p_sizes, data_buffer.mid(size, sm.status_size).data(), sm.status_size);
                    size += sm.status_size;

                    //getting the total number of elements
                    nbr_elements = (sm.status_size / sizeof (p_sizes[0])) / sizeof (p_sizes[0]);

                    QVector<shadow::APP_STATUS> statuses;
                    for(auto i = 0; i < nbr_elements; i++) {
                        statuses.push_back(static_cast<shadow::APP_STATUS>(p_sizes[i]));
                    }

                    emit client_status_update(client_app_names, client_app_clas, statuses);
                }
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
    emit disconnected_from_client();
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

void Core_Server::start_client_processes() {
    if(socket != nullptr && socket->isOpen()) {
        core::header header;
        header.message_type = core::header::START;
        socket->write(reinterpret_cast<char*>(&header), sizeof (header));
    }
}

void Core_Server::stop_client_processes() {
    if(socket != nullptr && socket->isOpen()) {
        core::header header;
        header.message_type = core::header::STOP;
        socket->write(reinterpret_cast<char*>(&header), sizeof (header));
    }
}
