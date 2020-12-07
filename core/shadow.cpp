#include "shadow.h"

Shadow::Shadow(QStringList app_names, QStringList app_scripts, QStringList app_ports, QObject *parent) :
    QObject (parent),
    application_names(app_names),
    application_scripts(app_scripts),
    application_ports(app_ports)
{}
