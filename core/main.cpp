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

    const QList<QCommandLineOption> command_line_options = {
        application_name,
        application_port,
        application_script
    };

    //parser
    parser.addOptions(command_line_options);
    parser.process(app); //process command line arguments

    //getting all values of options
    QStringList application_names = parser.values(application_name);
    QStringList application_ports = parser.values(application_port);
    QStringList application_scripts = parser.values(application_script);

    //Need to make sure there are an equal amount of options for each. Basically a positional argument.
    //We are creating a table with these options and expect an equal number for each
    QList<QStringList> equal_length_options = {
        application_names,
        application_ports,
        application_scripts
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

    qDebug() << application_names << application_ports << application_scripts;

    Shadow w(application_names, application_scripts, application_ports);

    return app.exec();
}
