#ifndef SHADOW_H
#define SHADOW_H

#include <QObject>

class Shadow : public QObject
{
    Q_OBJECT

public:
    explicit Shadow(QStringList app_names, QStringList app_scripts, QStringList app_ports, QObject *parent = nullptr);

private:
    QStringList application_names;
    QStringList application_scripts;
    QStringList application_ports;
};

#endif // SHADOW_H
