#include "core_client.h"
#include "core_tcp_message_types.hpp"

namespace  {
const int SERVER_CONNECTION_TIMEOUT_MSECS = 10000;
const int SERVER_ATTEMPT_TO_RECONNECT_TIMEOUT_MSECS = 1000;
const int SERVER_KEEP_ALIVE_TIMEOUT_MSECS = 20000;
const int STATUS_UPDATE_TIMEOUT_MSECS = 1000;
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

    //creating keep alive timer
    keep_alive_timer = new QTimer(this);
    connect(keep_alive_timer, SIGNAL(timeout()), this, SLOT(keep_alive_timeout()));
    keep_alive_timer->setSingleShot(true);

    //creating status update timer
    status_update_timer = new QTimer(this);
    connect(status_update_timer, SIGNAL(timeout()), this, SLOT(status_update_timeout()));
    status_update_timer->start(STATUS_UPDATE_TIMEOUT_MSECS);


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

void Core_Client::status_update_timeout() {
    if(socket->isOpen() && _app_names.size() > 0) {
        assert(_app_names.size() == _command_line_arguments.size());

        //creating header
        core::header header;
        header.message_type = core::header::STATUS;

        //creating status message
        core::status_message sm;

        //recording app names sizes
        std::uint32_t a_sizes[_app_names.size()]; //creating an array of sizes
        sm.app_name_sizes = &a_sizes[0]; //putting that array in our status message
        for(auto i = 0; i < _app_names.size(); i++) a_sizes[i] = _app_names[i].size(); //recording all of our statuses in the array

        //recording total app name size
        sm.app_name_size = sizeof(a_sizes) * sizeof(a_sizes[0]);

        //updating the total packet size
        header.packet_size = sizeof (sm) + sm.app_name_size; //status message plus the app name size
        for(auto i = 0; i < _app_names.size(); i++) header.packet_size += _app_names[i].size(); // adding the byte sizes

        //------------------------------------------------------

        //recording cla names sizes
        std::uint32_t c_sizes[_command_line_arguments.size()]; //creating an array of sizes
        sm.cla_sizes = &c_sizes[0];
        for(auto i = 0; i < _command_line_arguments.size(); i++) c_sizes[i] = _command_line_arguments[i].size();

        //recording total cla name size
        sm.cla_size = sizeof (c_sizes) * sizeof (c_sizes[0]);

        //updating the total packet size
        header.packet_size += sm.cla_size;
        for(auto i = 0; i < _command_line_arguments.size(); i++) header.packet_size += _command_line_arguments[i].size();

        //-------------------------------------------------------
        std::uint32_t p_sizes[process_statuses.size()];
        sm.statuses = &p_sizes[0];
        for(auto i = 0; i < process_statuses.size(); i++) p_sizes[i] = process_statuses[i];

        sm.status_size = sizeof (p_sizes) * sizeof (p_sizes[0]);
        header.packet_size += sm.status_size;

        //writing data to socket
        socket->write(reinterpret_cast<char*>(&header), sizeof (header)); //writing header
        socket->write(reinterpret_cast<char*>(&sm), sizeof (sm)); //writing status message

        socket->write(reinterpret_cast<char*>(sm.app_name_sizes), sm.app_name_size); //writing app names sizes
        for(auto i = 0; i < _app_names.size(); i++) socket->write(_app_names[i]); //writing app names data

        socket->write(reinterpret_cast<char*>(sm.cla_sizes), sm.cla_size);
        for(auto i = 0; i < _command_line_arguments.size(); i++) socket->write(_command_line_arguments[i]);

        socket->write(reinterpret_cast<char*>(sm.statuses), sm.status_size);
    }
    else qDebug() << "error, not sending status update" << _app_names.size() << _command_line_arguments.size();
}

void Core_Client::clear_buffers() {
    header_buffer.clear();
    data_buffer.clear();
}

void Core_Client::set_application_data(QStringList app_names, QStringList command_line_args) {
    qDebug() << "setting application data" << app_names.length() << command_line_args.length();
    assert(app_names.size() == command_line_args.size());

    _app_names.clear();
    _command_line_arguments.clear();
    process_statuses.clear();

    for(auto i = 0; i < app_names.size(); i++) {
        _app_names.push_back(app_names[i].toUtf8());
        _command_line_arguments.push_back(command_line_args[i].toUtf8());
        process_statuses.push_back(shadow::APP_STATUS::NOT_RUNNING);
    }
}

void Core_Client::set_process_status_table(QVector<shadow::APP_STATUS> statuses) {
    qDebug() << "does this ever get called?";
    process_statuses = statuses;
    assert(statuses.size() == _app_names.size());
}
