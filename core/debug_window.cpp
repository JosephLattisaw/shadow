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

void Debug_Window::set_application_data(QStringList app_names, QStringList command_line_args) {
    assert(app_names.size() == command_line_args.size()); //sanity check
    _app_names = app_names;
    _command_line_arguments = command_line_args;

    for(auto i = 0; i < app_names.size(); i++) {
        QTableWidgetItem *app_name_item = new QTableWidgetItem(app_names[i]);
        QTableWidgetItem *status_name_item = new QTableWidgetItem(DEFAULT_STATUS_TEXT);
        QTableWidgetItem *command_line_args_item = new QTableWidgetItem(command_line_args[i]);

        app_name_item->setFlags(Qt::ItemIsEnabled);
        status_name_item->setFlags(Qt::ItemIsEnabled);
        command_line_args_item->setFlags(Qt::ItemIsEnabled);

        ui->table_widget->insertRow(ui->table_widget->rowCount());
        ui->table_widget->setItem(ui->table_widget->rowCount() - 1, APPLICATION_NAME, app_name_item);
        ui->table_widget->setItem(ui->table_widget->rowCount() - 1, STATUS_NAME, status_name_item);
        ui->table_widget->setItem(ui->table_widget->rowCount() - 1, COMMAND_LINE_ARGS, command_line_args_item);
    }
}

void Debug_Window::on_start_button_clicked() {
    toggle_start_and_stop_buttons(true);
    emit start();
}

void Debug_Window::on_stop_button_clicked() {
    toggle_start_and_stop_buttons(false);
    emit stop();
}

void Debug_Window::toggle_start_and_stop_buttons(bool start_clicked) {
    //disabling the button you clicked or re-enabling the one you didn't click
    ui->start_button->setDisabled(start_clicked);
    ui->stop_button->setDisabled(!start_clicked);
}

void Debug_Window::set_process_status_table(QVector<shadow::APP_STATUS> statuses) {
    process_statuses = statuses;
    update_process_status_table(statuses, 0);
}

void Debug_Window::update_process_status_table(QVector<shadow::APP_STATUS> statuses, int starting_index) {
    for(auto i = 0; i < statuses.size(); i++) {
        QTableWidgetItem *status_item = ui->table_widget->item(i + starting_index, STATUS_NAME);
        if(status_item != nullptr) {
            switch (statuses[i]) {
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

void Debug_Window::update_client_status(QVector<QString> client_app_names, QVector<QString> client_app_clas, QVector<shadow::APP_STATUS> client_app_status) {
    if(ui->table_widget->rowCount() != (_app_names.size() + client_app_names.size())) {
        assert(client_app_clas.size() == client_app_names.size()); //sanity check
        _client_app_names = client_app_names;
        _client_command_line_arguments = client_app_clas;

        for(auto i = 0; i < _client_app_names.size(); i++) {
            QTableWidgetItem *app_name_item = new QTableWidgetItem(_client_app_names[i]);
            QTableWidgetItem *status_name_item = new QTableWidgetItem(DEFAULT_STATUS_TEXT);
            QTableWidgetItem *command_line_args_item = new QTableWidgetItem(_client_command_line_arguments[i]);

            app_name_item->setFlags(Qt::ItemIsEnabled);
            status_name_item->setFlags(Qt::ItemIsEnabled);
            command_line_args_item->setFlags(Qt::ItemIsEnabled);

            ui->table_widget->insertRow(ui->table_widget->rowCount());
            ui->table_widget->setItem(ui->table_widget->rowCount() - 1, APPLICATION_NAME, app_name_item);
            ui->table_widget->setItem(ui->table_widget->rowCount() - 1, STATUS_NAME, status_name_item);
            ui->table_widget->setItem(ui->table_widget->rowCount() - 1, COMMAND_LINE_ARGS, command_line_args_item);
        }
    }

    update_process_status_table(client_app_status, process_statuses.size());
}

void Debug_Window::disconnected_from_client() {
    while(ui->table_widget->rowCount() != _app_names.size()) {
        ui->table_widget->removeRow(ui->table_widget->rowCount() - 1);
    }

    _client_app_names.clear();
    _client_command_line_arguments.clear();
    _client_process_statuses.clear();
}
