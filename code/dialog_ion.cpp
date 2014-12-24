#include "mainwindow.h"

#include "dialog_ion.h"
#include "ui_dialog_ion.h"

Dialog_ion::Dialog_ion(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_ion)
{
    ui->setupUi(this);
   // lat_front=0;
   // qDebug() << lat_front;
    //if(lat_front==0){ui->lin
}

Dialog_ion::~Dialog_ion()
{
    delete ui;
}

void Dialog_ion::on_radioButton_lateral_clicked()
{
    ui->radioButton_frontal->setChecked(false);
    ui->lineEdit_ioniz->setDisabled(true);
    ui->pushButton_ionfile->setDisabled(true);
    lat_front=0;
}

void Dialog_ion::on_radioButton_frontal_clicked()
{
    ui->radioButton_lateral->setChecked(false);
    ui->lineEdit_ioniz->setEnabled(true);
    ui->pushButton_ionfile->setEnabled(true);
    lat_front=1;
}


void Dialog_ion::on_pushButton_ionfile_clicked()
{
    FileBrowser browser;
    browser.exec();
    ui->lineEdit_ioniz->setText(browser.on_buttonBox_accepted());
}

void Dialog_ion::on_pushButton_vacfile_clicked()
{
    FileBrowser browser;
    browser.exec();
    ui->lineEdit_vac->setText(browser.on_buttonBox_accepted());
}

void Dialog_ion::on_buttonBox_accepted()
{

    vac_path=ui->lineEdit_vac->text();
    ion_path=ui->lineEdit_ioniz->text();

    ntherm=ui->lineEdit_etherm->text().toDouble();
    ptherm=ui->lineEdit_htherm->text().toDouble();
    nsigma=ui->lineEdit_esigma->text().toDouble();
    psigma=ui->lineEdit_hsigma->text().toDouble();
    fluence=ui->lineEdit_fluence->text().toDouble();
    radhard=ui->lineEdit_prob->text().toDouble();
}

