#include "shadow.h"
#include <QThread>
#include <cassert>

Shadow::Shadow(QStringList app_names, QStringList app_scripts, QStringList app_ports, bool debug_window, QObject *parent) :
    QObject (parent),
    application_names(app_names),
    application_scripts(app_scripts),
    application_ports(app_ports),
    debug_window_is_set(debug_window)
{
    //sanity checks
    assert(app_names.length() > 0);
    assert(app_names.length() == app_scripts.length() && app_names.length() == app_ports.length());

    process_statuses.resize(app_names.size()); //resizes our status table to this size

    create_processes(); //creating all of our processes and threads

    if(debug_window_is_set) {
        _debug_window = new Debug_Window;
        _debug_window->set_application_names(application_names);

        _debug_window->show();

        connect(_debug_window, SIGNAL(start()), this, SLOT(start_all())); //starting all processes
        connect(_debug_window, SIGNAL(stop()), this, SLOT(stop_all())); //stopping all processes
        connect(this, SIGNAL(update_process_status_table(QVector<shadow::APP_STATUS>)), _debug_window, SLOT(set_process_status_table(QVector<shadow::APP_STATUS>)));
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

        connect(this, SIGNAL(start_process(int)), process, SLOT(start_process(int)));
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
    emit start_process(0);
}

void Shadow::stop_all() {
    emit stop_process(0);
    for(auto i = 0; i < process_statuses.size(); i++) {
        process_statuses[i] = shadow::APP_STATUS::NOT_RUNNING; //resetting to default status
    }

    update_process_status_table(process_statuses);
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
