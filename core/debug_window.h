#ifndef DEBUG_WINDOW_H
#define DEBUG_WINDOW_H

#include <QMainWindow>

namespace Ui {
class Debug_Window;
}

class Debug_Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Debug_Window(QWidget *parent = nullptr);
    ~Debug_Window();

private:
    Ui::Debug_Window *ui;
};

#endif // DEBUG_WINDOW_H
