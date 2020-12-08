#include "process.h"
#include <QDebug>

Process::Process(QString name, QString script, int id, QObject *parent) : _name(name), _script(script), _id(id), QObject(parent)
{}

void Process::start_thread() {
    qDebug() << "Process: <" << _name.toStdString().c_str() << "> thread started";
    process = new QProcess(this);
    stopper_process = new QProcess(this);

    connect(process, SIGNAL(finished(int)), this, SLOT(process_finished(int)));
    connect(process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(error_occurred(QProcess::ProcessError)));
}

void Process::stop_process(int id) {
    if(_id == id) {
        intentional_stop = false;

        //to avoid crash signal since programs don't always cleanly handle terminate/kill
        disconnect(process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(error_occurred(QProcess::ProcessError)));

        process->terminate(); //terminating process
        process->waitForFinished(3000); //waiting for it to finish cleanly
        process->kill(); //just in case process gets hung, kill should always work
        process->waitForFinished(); //just in case because we'd like to wait for it to finish
        intentional_stop = false;
        connect(process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(error_occurred(QProcess::ProcessError))); //reconnecting disconnected signal
    }
}

void Process::start_process(int id) {
    if(id == _id) {
        stop_process(_id);

        qDebug() << "starting process:" << _name << "path:" << _script;
        process->start(_script, QStringList()); //starting the process
    }
}

void Process::process_finished(int exit_code) {
    qDebug() << "process finished with exit code" << exit_code;
}

void Process::error_occurred(QProcess::ProcessError e) {
    switch(e) {
    case QProcess::FailedToStart:
        qDebug() << _name << "failed to start";
        emit failed_to_start(_id); //sending failed to start signal
        break;
    case QProcess::Crashed:
        qDebug() << _name << "crashed";
        emit crashed(_id); //sending crashed signal
        break;
    case QProcess::Timedout:
        qDebug() << _name << "last waitFor function timed out";
        break;
    case QProcess::WriteError:
        qDebug() << _name << "error occurred when attempting to write to process";
        break;
    case QProcess::ReadError:
        qDebug() << _name << "error occurred when attempting to read from process";
        break;
    case QProcess::UnknownError:
    default:
        qDebug() << _name << "unknown error occurred";
        break;
    }
}


