#include "debug_window.h"
#include "ui_debug_window.h"

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
