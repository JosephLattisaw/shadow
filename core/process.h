#ifndef PROCESS_H
#define PROCESS_H

#include <QObject>
#include <QProcess>

class Process : public QObject
{
    Q_OBJECT
public:
    explicit Process(QString name, QString script, int id, QObject *parent = nullptr);

signals:
    void crashed(int id);
    void failed_to_start(int id);

public slots:
    void start_thread();

private slots:
    void start_process(int id);
    void stop_process(int id);

    void error_occurred(QProcess::ProcessError e);
    void process_finished(int exit_code);

private:
    int _id;
    QString _name; //NOTE: name doesn't do anything, just helpful for printouts really
    QString _script; //what script we will actually be running

    QProcess *process = nullptr;
    QProcess *stopper_process = nullptr;

    bool intentional_stop = false;
};

#endif // PROCESS_H
