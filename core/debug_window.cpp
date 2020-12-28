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

        ui->table_widget->insertRow(ui->table_widget->rowCount());
        ui->table_widget->setItem(ui->table_widget->rowCount() - 1, APPLICATION_NAME, app_name_item);
        ui->table_widget->setItem(ui->table_widget->rowCount() - 1, STATUS_NAME, status_name_item);
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
