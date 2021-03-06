﻿#include "shadow.h"
#include <QThread>
#include <cassert>

Shadow::Shadow(QStringList app_names, QStringList app_scripts, QStringList app_ports, QStringList cl_args, QString client_hostname, uint16_t client_port, bool client_mode, bool debug_window, QObject *parent) :
    QObject (parent),
    application_names(app_names),
    application_scripts(app_scripts),
    application_ports(app_ports),
    command_line_arguments(cl_args),
    debug_window_is_set(debug_window),
    _client_hostname(client_hostname),
    _client_port(client_port),
    _client_mode(client_mode)
{
    //sanity checks
    assert(app_names.length() > 0);
    assert(app_names.length() == app_scripts.length() && app_names.length() == app_ports.length() && app_names.length() == cl_args.length());

    qRegisterMetaType<QVector<QString>>("QVector<QString>");
    qRegisterMetaType<QVector<shadow::APP_STATUS>>("QVector<shadow::APP_STATUS>");

    process_statuses.resize(app_names.size()); //resizes our status table to this size

    create_processes(); //creating all of our processes and threads

    if(debug_window_is_set) {
        _debug_window = new Debug_Window;
        _debug_window->set_application_data(application_names, command_line_arguments);

        _debug_window->show();

        connect(_debug_window, SIGNAL(start()), this, SLOT(start_all())); //starting all processes
        connect(_debug_window, SIGNAL(stop()), this, SLOT(stop_all())); //stopping all processes
        connect(this, SIGNAL(update_process_status_table(QVector<shadow::APP_STATUS>)), _debug_window, SLOT(set_process_status_table(QVector<shadow::APP_STATUS>)));
        connect(this, SIGNAL(update_client_status(QVector<QString>,QVector<QString>,QVector<shadow::APP_STATUS>)), _debug_window, SLOT(update_client_status(QVector<QString>,QVector<QString>,QVector<shadow::APP_STATUS>)));
    }

    if(!client_mode) {
        //created a server if we aren't in client mode
        core_server = new Core_Server(_client_port);
        core_server_thread = new QThread;
        move_object_to_thread(core_server, core_server_thread);
        connect(this, SIGNAL(start_client_processes()), core_server, SLOT(start_client_processes()));
        connect(this, SIGNAL(stop_client_processes()), core_server, SLOT(stop_client_processes()));
        connect(core_server, SIGNAL(client_status_update(QVector<QString>,QVector<QString>,QVector<shadow::APP_STATUS>)), this, SLOT(client_status_update(QVector<QString>,QVector<QString>,QVector<shadow::APP_STATUS>)));
        if(debug_window_is_set) connect(core_server, SIGNAL(disconnected_from_client()), _debug_window, SLOT(disconnected_from_client()));
    }
    else {
        core_client = new Core_Client(_client_hostname, _client_port);
        core_client->set_application_data(application_names, command_line_arguments);

        core_client_thread = new QThread;
        move_object_to_thread(core_client, core_client_thread);
        connect(core_client, SIGNAL(start_all()), this, SLOT(start_all()));
        connect(core_client, SIGNAL(stop_all()), this, SLOT(stop_all()));
        connect(this, SIGNAL(update_process_status_table(QVector<shadow::APP_STATUS>)), core_client, SLOT(set_process_status_table(QVector<shadow::APP_STATUS>)));
    }

    emit start_threads();
}

Shadow::~Shadow() {
    //TODO need to kill threads

    if(_debug_window != nullptr) delete _debug_window; //delete debug window gui if it exist
}

void Shadow::create_processes() {
    processes.clear();

    for(auto i = 0; i < application_names.length(); i++) {
        int process_id = i;
        Process *process = new Process(application_names[i], application_scripts[i], process_id);
        QThread *thread = new QThread;

        move_object_to_thread(process, thread);

        connect(this, SIGNAL(start_process(int,QString)), process, SLOT(start_process(int,QString)));
        connect(this, SIGNAL(stop_process(int)), process, SLOT(stop_process(int)));
        connect(process, SIGNAL(started(int)), this, SLOT(process_started(int)));
        connect(process, SIGNAL(failed_to_start(int)), this, SLOT(process_failed_to_start(int)));
        connect(process, SIGNAL(crashed(int)), this, SLOT(process_crashed(int)));
        connect(process, SIGNAL(stopped(int)), this, SLOT(process_stopped(int)));

        //saving objects in container for clean up or use later
        processes.push_back(process);
        process_ids.push_back(process_id);
        threads.push_back(thread);
    }
}

void Shadow::move_object_to_thread(QObject *object, QThread *thread) {
    object->moveToThread(thread);
    thread->start();
    connect(this, SIGNAL(start_threads()), object, SLOT(start_thread()));
    connect(thread, SIGNAL(finished()), object, SLOT(deleteLater()));
}

void Shadow::start_all() {
    emit start_process(0, command_line_arguments.at(0));
    if(!_client_mode) emit start_client_processes();
}

void Shadow::stop_all() {
    emit stop_process(0);
    for(auto i = 0; i < process_statuses.size(); i++) {
        process_statuses[i] = shadow::APP_STATUS::NOT_RUNNING; //resetting to default status
    }

    update_process_status_table(process_statuses);
    if(!_client_mode) emit stop_client_processes();
}

void Shadow::process_started(int id) {
    process_action_handler(id, shadow::APP_STATUS::RUNNING);
}

void Shadow::process_failed_to_start(int id) {
    process_action_handler(id, shadow::APP_STATUS::FAILED_TO_START);
}

void Shadow::process_crashed(int id) {
    process_action_handler(id, shadow::APP_STATUS::CRASH);
}

void Shadow::process_stopped(int id) {
    process_action_handler(id, shadow::APP_STATUS::NOT_RUNNING);
}

void Shadow::process_action_handler(int id, shadow::APP_STATUS status) {
    assert(id < process_statuses.size()); //sanity check
    process_statuses[id] = status;

    update_process_status_table(process_statuses);
}

void Shadow::client_status_update(QVector<QString> client_app_names, QVector<QString> client_app_clas, QVector<shadow::APP_STATUS> client_p_stats) {
    client_application_names = client_app_names;
    client_command_line_arguments = client_app_clas;
    client_process_statuses = client_p_stats;
    emit update_client_status(client_app_names, client_app_clas, client_p_stats);
}
