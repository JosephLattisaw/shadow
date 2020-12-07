#ifndef SHADOW_H
#define SHADOW_H

#include <QObject>

class Shadow : public QObject
{
    Q_OBJECT

public:
    explicit Shadow(QObject *parent = nullptr);
    ~Shadow();
};

#endif // SHADOW_H
