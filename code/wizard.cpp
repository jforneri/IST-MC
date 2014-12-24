#include "wizard.h"
#include "ui_wizard.h"
#include <QDebug>

Wizard::Wizard(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::Wizard)
{
    ui->setupUi(this);
}

Wizard::~Wizard()
{
    delete ui;
}

void Wizard::on_radioButton_clicked()
{

}

void Wizard::on_radioButton_2_clicked()
{

}

void Wizard::on_pushButton_clicked()
{
    FileBrowser browser;
    browser.exec();
    ui->lineEdit_vn->setText(browser.on_buttonBox_accepted());
    wpath1 = ui ->lineEdit_vn->text();
}



void Wizard::on_pushButton_2_clicked()
{
    FileBrowser browser;
    browser.exec();
    ui->lineEdit_vp->setText(browser.on_buttonBox_accepted());
    wpath2 = ui->lineEdit_vp-> text();
}

void Wizard::on_pushButton_3_clicked()
{
    FileBrowser browser;
    browser.exec();
    ui->lineEdit_gunn->setText(browser.on_buttonBox_accepted());
    wpath3 = ui->lineEdit_gunn->text();
}





