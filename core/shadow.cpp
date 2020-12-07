#include "shadow.h"
#include "ui_shadow.h"

Shadow::Shadow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Shadow)
{
    ui->setupUi(this);
}

Shadow::~Shadow()
{
    delete ui;
}
