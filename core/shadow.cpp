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

    create_processes(); //creating all of our processes and threads

    if(debug_window_is_set) {
        _debug_window = new Debug_Window;
        _debug_window->set_application_names(application_names);

        _debug_window->show();
    }

    emit start_threads();

    emit start_process(0);
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
