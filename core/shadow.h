#ifndef SHADOW_H
#define SHADOW_H

#include <QMainWindow>

namespace Ui {
class Shadow;
}

class Shadow : public QMainWindow
{
    Q_OBJECT

public:
    explicit Shadow(QWidget *parent = nullptr);
    ~Shadow();

private:
    Ui::Shadow *ui;
};

#endif // SHADOW_H
