#include "mainhub.h"
#include "ui_mainhub.h"
#include "mainwindow.h"

mainhub::mainhub(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainhub)
{
    ui->setupUi(this);
}

mainhub::~mainhub()
{
    delete ui;
}

void mainhub::on_pushButton_clicked()
{
    MainWindow f;
    f.ConnectMe();
}
