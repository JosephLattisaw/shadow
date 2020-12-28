#include "debug_window.h"
#include "ui_debug_window.h"

namespace  {
    const QString DEFAULT_STATUS_TEXT = "Not Running";
}

Debug_Window::Debug_Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Debug_Window)
{
    ui->setupUi(this);
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
