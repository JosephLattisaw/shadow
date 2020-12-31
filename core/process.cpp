#include "process.h"
#include <QDebug>

Process::Process(QString name, QString script, int id, QObject *parent) : _name(name), _script(script), _id(id), QObject(parent)
{}

void Process::start_thread() {
    assert(!thread_started);
    thread_started = true;
    log("thread started");
    process = new QProcess(this);
    stopper_process = new QProcess(this);

    connect(process, SIGNAL(finished(int)), this, SLOT(process_finished(int)));
    connect(process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(error_occurred(QProcess::ProcessError)));
    connect(process, SIGNAL(started()), this, SLOT(process_started()));
}

void Process::stop_process(int id) {
    if(_id == id) {
        log("stopping process with script (" + _script + ")");
        intentional_stop = true;

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

void Process::start_process(int id, QString command_line_arguments) {
    if(id == _id) {
        stop_process(_id);

        log("starting process with script (" + _script + "), and command line args: [" + command_line_arguments + "]");
        process->start(_script, QStringList() << command_line_arguments); //starting the process
    }
}

void Process::process_finished(int exit_code) {
    log("process finished with exit code " + QString::number(exit_code));
    if(!intentional_stop) emit crashed(_id); //assuming we crashed if it was closed not by us
    else emit stopped(_id);
}

void Process::error_occurred(QProcess::ProcessError e) {
    switch(e) {
    case QProcess::FailedToStart:
        log("ERROR OCCURRED => failed to start");
        emit failed_to_start(_id); //sending failed to start signal
        break;
    case QProcess::Crashed:
        log("ERROR OCCURRED => crashed");
        emit crashed(_id); //sending crashed signal
        break;
    case QProcess::Timedout:
        log("ERROR OCCURRED => process timed out");
        break;
    case QProcess::WriteError:
        log("ERROR OCCURRED => error occurred when attempting to write to process");
        break;
    case QProcess::ReadError:
        log("ERROR OCCURRED => error occurred when attempting to read from process");
        break;
    case QProcess::UnknownError:
    default:
        log("ERROR OCCURRED => unknow error occurred");
        break;
    }
}

void Process::process_started() {
    emit started(_id);
}

void Process::log(QString string) {
    qDebug() << QString("Process <" + _name + ">:").toStdString().c_str() << string.toStdString().c_str();
}
