#include "mainwindow.h"
#include "dialog_import.h"
#include "ui_dialog_import.h"


Dialog_import::Dialog_import(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_import)
{
    ui->setupUi(this);
    check_import=0;
}

Dialog_import::~Dialog_import()
{
    delete ui;
}

void Dialog_import::on_pushButton_browse_clicked()
{
    FileBrowser browser;
    browser.exec();
    ui->lineEdit->setText(browser.on_buttonBox_accepted());
}

void Dialog_import::on_buttonBox_accepted()
{
        path_experiment = ui ->lineEdit->text();


}
