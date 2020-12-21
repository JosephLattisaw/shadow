#ifndef SHADOW_H
#define SHADOW_H

#include <QObject>
#include <QVector>
#include "process.h"
#include "debug_window.h"

class Shadow : public QObject
{
    Q_OBJECT

public:
    explicit Shadow(QStringList app_names, QStringList app_scripts, QStringList app_ports, bool debug_window = false, QObject *parent = nullptr);
    ~Shadow();

signals:
    void start_threads();

    void start_process(int id);
    void stop_process(int id);

private:
    void create_processes();
    void move_object_to_thread(QObject *object, QThread *thread);

    QVector<Process*> processes;
    QVector<int> process_ids;
    QVector<QThread*> threads;

    QStringList application_names;
    QStringList application_scripts;
    QStringList application_ports;

    bool debug_window_is_set;
    Debug_Window *_debug_window = nullptr;
};

#endif // SHADOW_H
