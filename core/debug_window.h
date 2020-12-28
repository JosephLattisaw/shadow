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

    void set_application_names(QStringList app_names);

private:
    Ui::Debug_Window *ui;

    QStringList _app_names;

    enum table {
        APPLICATION_NAME = 0,
        STATUS_NAME = 1,
    };
};

#endif // DEBUG_WINDOW_H
