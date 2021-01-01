#include "shadow.h"
#include "defines.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv); //creating application
    QApplication::setApplicationName(shadow::APPLICATION_NAME); //setting application name
    QApplication::setApplicationVersion(shadow::VERSION_NUMBER); //setting application version

    //create command line parser with basic options
    QCommandLineParser parser;
    parser.setApplicationDescription(shadow::APPLICATION_DESC); //setting application description
    parser.addHelpOption(); //adding default help option
    parser.addVersionOption(); //adding default version option

    QCommandLineOption application_name(QStringList() << "a" << "application_name",
                                  "application name for a program to be automatically executed", "app_name");

    QCommandLineOption application_port(QStringList() << "p" << "application_port",
                                  "TCP port to communicate with external application", "app_port");

    QCommandLineOption application_script(QStringList() << "s" << "application_script",
                                  "script needed to execute application", "app_script");

    QCommandLineOption command_line_argument(QStringList() << "c" << "command_line_arguments",
                                            "Command Line Arguments for Applications to be automatically executed", "args");

    QCommandLineOption debug_window(QStringList() << "debug_window",
                                    "If this is set we display a debug window gui when running shadow");

    QCommandLineOption client_mode(QStringList() << "client_mode",
                                   "run core in client mode (core running inside of VM)");

    QCommandLineOption client_host(QStringList() << "client_host",
                                   "TCP/IP Server IP Address (Needed for Client Mode)", "127.0.0.1", "127.0.0.1");

    QCommandLineOption client_port(QStringList() << "client_port",
                                   "TCP/IP Port (Needed for Client and non - Client Mode)", "2000", "2000");

    const QList<QCommandLineOption> command_line_options = {
        application_name,
        application_port,
        application_script,
        command_line_argument,
        debug_window,
        client_mode,
        client_host,
        client_port
    };

    //parser
    parser.addOptions(command_line_options);
    parser.process(app); //process command line arguments

    //getting all values of options
    QStringList application_names = parser.values(application_name);
    QStringList application_ports = parser.values(application_port);
    QStringList application_scripts = parser.values(application_script);
    QStringList command_line_arguments = parser.values(command_line_argument);
    QString client_hostname = parser.isSet(client_host) ? parser.value(client_host) : client_host.defaultValues().first();
    std::uint16_t client_port_number = parser.isSet(client_port) ? parser.value(client_port).toUShort() : client_port.defaultValues().first().toUShort();

    //Need to make sure there are an equal amount of options for each. Basically a positional argument.
    //We are creating a table with these options and expect an equal number for each
    QList<QStringList> equal_length_options = {
        application_names,
        application_ports,
        application_scripts,
        command_line_arguments
    };

    int length_of_legal_option = equal_length_options.first().length(); //get length of first option
    for(auto i:equal_length_options) {
        if(length_of_legal_option != i.length()) {
            qDebug() << "usage: application_name, application_ports, and application_scripts must all have an equal number of options";
            std::exit(EXIT_FAILURE);
        }
    }

    //Need at least one legal option for the application even to be doing anything
    if(length_of_legal_option < 1) {
        qDebug() << "usage: There must be at least one option set, application_name, application_ports or application_scripts";
        std::exit(EXIT_FAILURE);
    }

    bool debug_window_set = parser.isSet(debug_window);
    bool client_mode_set = parser.isSet(client_mode);

    qDebug() << application_names << application_ports << application_scripts << client_hostname << client_port_number << client_mode_set << debug_window_set;

    Shadow w(application_names, application_scripts, application_ports, client_hostname, client_port_number, client_mode_set, debug_window_set);

    return app.exec();
}
