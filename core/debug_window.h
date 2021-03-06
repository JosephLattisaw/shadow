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

    void set_application_data(QStringList app_names, QStringList command_line_args);

public slots:
    void disconnected_from_client();
    void set_process_status_table(QVector<shadow::APP_STATUS> statuses);
    void update_client_status(QVector<QString> client_app_names, QVector<QString> client_app_clas, QVector<shadow::APP_STATUS> client_app_status);

private slots:
    //start and stop buttons
    void on_start_button_clicked();
    void on_stop_button_clicked();

private:
    void toggle_start_and_stop_buttons(bool start_clicked);
    void update_process_status_table(QVector<shadow::APP_STATUS> statuses, int starting_index);

    Ui::Debug_Window *ui;

    QStringList _app_names;
    QStringList _command_line_arguments;
    QVector<shadow::APP_STATUS> process_statuses;

    QVector<QString> _client_app_names;
    QVector<QString> _client_command_line_arguments;
    QVector<shadow::APP_STATUS> _client_process_statuses;

    enum table {
        APPLICATION_NAME = 0,
        STATUS_NAME = 1,
        COMMAND_LINE_ARGS = 2,
    };
};

#endif // DEBUG_WINDOW_H
