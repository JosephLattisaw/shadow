#ifndef DEBUG_WINDOW_H
#define DEBUG_WINDOW_H

#include <QMainWindow>
#include "defines.hpp"

namespace Ui {
class Debug_Window;
}

class Debug_Window : public QMainWindow
{
    Q_OBJECT

signals:
    void start();
    void stop();

public:
    explicit Debug_Window(QWidget *parent = nullptr);
    ~Debug_Window();

    void set_application_names(QStringList app_names);

public slots:
    void set_process_status_table(QVector<shadow::APP_STATUS> statuses);

private slots:
    //start and stop buttons
    void on_start_button_clicked();
    void on_stop_button_clicked();

private:
    void toggle_start_and_stop_buttons(bool start_clicked);
    void toggle_cmd_line_arg_editable(bool editable);

    Ui::Debug_Window *ui;

    QStringList _app_names;
    QVector<shadow::APP_STATUS> process_statuses;

    Qt::ItemFlags default_command_line_arg_flags = Qt::NoItemFlags;

    enum table {
        APPLICATION_NAME = 0,
        STATUS_NAME = 1,
        COMMAND_LINE_ARGS = 2,
    };
};

#endif // DEBUG_WINDOW_H
