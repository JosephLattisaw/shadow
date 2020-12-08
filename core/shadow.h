#ifndef SHADOW_H
#define SHADOW_H

#include <QObject>
#include <QVector>
#include "process.h"

class Shadow : public QObject
{
    Q_OBJECT

public:
    explicit Shadow(QStringList app_names, QStringList app_scripts, QStringList app_ports, QObject *parent = nullptr);

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
};

#endif // SHADOW_H
