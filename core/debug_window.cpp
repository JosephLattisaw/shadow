#include "debug_window.h"
#include "ui_debug_window.h"
#include <QDebug>

namespace  {
const QString DEFAULT_STATUS_TEXT = "Not Running";
const QString RUNNING_STATUS_TEXT = "Running";
const QString CRASHED_STATUS_TEXT = "CRASHED";
const QString FAILED_TO_START_TEXT = "Failed to Start";
}

Debug_Window::Debug_Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Debug_Window)
{
    ui->setupUi(this);
    toggle_start_and_stop_buttons(false); //by default the start button is enabled and stop is disabled
}

Debug_Window::~Debug_Window()
{
    delete ui;
}

void Debug_Window::set_application_names(QStringList app_names) {
    _app_names = app_names;

    for(auto i:app_names) {
        QTableWidgetItem *app_name_item = new QTableWidgetItem(i);
        QTableWidgetItem *status_name_item = new QTableWidgetItem(DEFAULT_STATUS_TEXT);
        QTableWidgetItem *command_line_args_item = new QTableWidgetItem();

        app_name_item->setFlags(Qt::ItemIsEnabled);
        status_name_item->setFlags(Qt::ItemIsEnabled);
        default_command_line_arg_flags = command_line_args_item->flags(); //they should all be the same

        ui->table_widget->insertRow(ui->table_widget->rowCount());
        ui->table_widget->setItem(ui->table_widget->rowCount() - 1, APPLICATION_NAME, app_name_item);
        ui->table_widget->setItem(ui->table_widget->rowCount() - 1, STATUS_NAME, status_name_item);
        ui->table_widget->setItem(ui->table_widget->rowCount() - 1, COMMAND_LINE_ARGS, command_line_args_item);
    }
}

void Debug_Window::on_start_button_clicked() {
    toggle_start_and_stop_buttons(true);
    toggle_cmd_line_arg_editable(false);
    emit start(get_command_line_arguments());
}

void Debug_Window::on_stop_button_clicked() {
    toggle_start_and_stop_buttons(false);
    toggle_cmd_line_arg_editable(true);
    emit stop();
}

void Debug_Window::toggle_start_and_stop_buttons(bool start_clicked) {
    //disabling the button you clicked or re-enabling the one you didn't click
    ui->start_button->setDisabled(start_clicked);
    ui->stop_button->setDisabled(!start_clicked);
}

void Debug_Window::toggle_cmd_line_arg_editable(bool editable) {
    for(auto i = 0; i < ui->table_widget->rowCount(); i++) {
        QTableWidgetItem *item = ui->table_widget->item(i, COMMAND_LINE_ARGS);

        if(editable) item->setFlags(default_command_line_arg_flags);
        else item->setFlags(Qt::ItemIsEnabled);
    }
}

QVector<QString> Debug_Window::get_command_line_arguments() {
    QVector<QString> result;

    for(auto i = 0; i < ui->table_widget->rowCount(); i++) {
        QTableWidgetItem *item = ui->table_widget->item(i, COMMAND_LINE_ARGS);

        result.push_back(item->text());
    }

    return result;
}

void Debug_Window::set_process_status_table(QVector<shadow::APP_STATUS> statuses) {
    process_statuses = statuses;
    assert(statuses.size() == ui->table_widget->rowCount());

    for(auto i = 0; i < process_statuses.size(); i++) {
        QTableWidgetItem *status_item = ui->table_widget->item(i, STATUS_NAME);
        if(status_item != nullptr) {
            switch (process_statuses[i]) {
            case shadow::APP_STATUS::NOT_RUNNING:
                status_item->setText(DEFAULT_STATUS_TEXT);
                status_item->setBackgroundColor(Qt::white);
                break;
            case shadow::APP_STATUS::CRASH:
                status_item->setText(CRASHED_STATUS_TEXT);
                status_item->setBackgroundColor(Qt::red);
                break;
            case shadow::APP_STATUS::RUNNING:
                status_item->setText(RUNNING_STATUS_TEXT);
                status_item->setBackgroundColor(Qt::green);
                break;
            case shadow::APP_STATUS::FAILED_TO_START:
                status_item->setText(FAILED_TO_START_TEXT);
                status_item->setBackgroundColor(QColor(255,131,0)); //color is orange
                break;
            default:
                assert(false);
            }
        }
    }
}
