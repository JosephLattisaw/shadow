#ifndef SHADOW_H
#define SHADOW_H

#include <QObject>
#include <QVector>
#include "process.h"
#include "debug_window.h"
#include "defines.hpp"
#include "core_server.h"

class Shadow : public QObject
{
    Q_OBJECT

public:
    explicit Shadow(QStringList app_names, QStringList app_scripts, QStringList app_ports, QString client_hostname, std::uint16_t client_port, bool client_mode = false, bool debug_window = false, QObject *parent = nullptr);
    ~Shadow();

signals:
    void start_threads();

    //signals to start and stop a process
    void start_process(int id, QString command_line_arguments);
    void stop_process(int id);

    void update_process_status_table(QVector<shadow::APP_STATUS> statuses);

private slots:
    //signals from debug gui functions to start and stop processes
    void start_all(QVector<QString> command_line_arguments);
    void stop_all();

    //signals from process telling us the current state of our processes
    void process_action_handler(int id, shadow::APP_STATUS status);
    void process_crashed(int id);
    void process_failed_to_start(int id);
    void process_started(int id);
    void process_stopped(int id);

private:
    void create_processes();
    void move_object_to_thread(QObject *object, QThread *thread);

    QVector<Process*> processes;
    QVector<int> process_ids;
    QVector<shadow::APP_STATUS> process_statuses;
    QVector<QThread*> threads;

    QStringList application_names;
    QStringList application_scripts;
    QStringList application_ports;


    bool _client_mode;
    std::uint16_t _client_port;
    QString _client_hostname;

    bool debug_window_is_set;
    Debug_Window *_debug_window = nullptr;

    Core_Server *core_server = nullptr;
    QThread *core_server_thread = nullptr;
};

#endif // SHADOW_H
