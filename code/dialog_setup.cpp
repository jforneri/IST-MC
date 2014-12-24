
#include "dialog_setup.h"
#include "ui_dialog_setup.h"
#include <QDebug>
#include <fstream>
#include <stdio.h>
#include <sys/time.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <Qt/qstring.h>
#include <iostream>
#include <stdlib.h>
#include <qfile.h>
#include <qtextstream.h>
#include <QTextBrowser>
#include <QDir>


Dialog_setup::Dialog_setup(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::Dialog_setup)
{
  ui->setupUi(this);





//  qDebug() << lat_front;

  MainWindow *obj = (MainWindow *) qApp->activeWindow();
//  lat_front=obj->lat_front;
 // qDebug() << lat_front;
  length=obj->L;
  deltax=obj->deltax;
  if(!obj->last_path_ioniz.isEmpty())
  {
      path_ioniz=obj->last_path_ioniz;
  }
  control_ion=-1;


  lat_front=obj->lat_front;
  n_point = obj->n_point;
  n_ion = obj->n_ion;
  n_eh = obj->n_eh;
  dispersion = obj->dispersion*1e6;    //um
  e_noise= obj->e_noise*100;            // %
//   straggle=dialog.straggl*1e-6;           // um
  e_threshold= obj->e_threshold*100;
  dead_layer=obj->dead_layer;


  if(lat_front==0)
  {
  ui->radioButton->setChecked(false);
  ui->radioButton_2->setChecked(true);
  ui->lineEdit_ionpath->setDisabled(true);
  ui->pushButton_browseion->setDisabled(true);
  ui->lineEdit_deadlayer->setDisabled(true);
  }
  else
  {
      ui->radioButton->setChecked(true);
      ui->radioButton_2->setChecked(false);
      ui->lineEdit_npoints->setDisabled(true);
      ui->lineEdit_dispersion->setDisabled(true);

      setup_frontal_plot_first();
  }

  //SET ANCHE DA LAT/FRONT
  ui->lineEdit_npoints->setText(QString::number(n_point));
  ui->lineEdit_nions->setText(QString::number(n_ion));
  ui->lineEdit_npairs->setText(QString::number(n_eh));
  ui->lineEdit_dispersion->setText(QString::number(dispersion));
  ui->lineEdit_noise->setText(QString::number(e_noise));
  ui->lineEdit_threshold->setText(QString::number(e_threshold));
  ui->lineEdit_deadlayer->setText(QString::number(dead_layer));
 // ui->lineEdit_ionpath->set       IL PATH E' LA COSA PIU' FASTIDIOSA




  connect(ui->lineEdit_nions,SIGNAL(editingFinished()),this,SLOT(update_values()));
  connect(ui->lineEdit_npairs,SIGNAL(editingFinished()),this,SLOT(update_values()));
  connect(ui->lineEdit_npoints,SIGNAL(editingFinished()),this,SLOT(update_values()));
  connect(ui->lineEdit_dispersion,SIGNAL(editingFinished()),this,SLOT(update_values()));
  connect(ui->lineEdit_threshold,SIGNAL(editingFinished()),this,SLOT(update_values()));
  connect(ui->lineEdit_noise,SIGNAL(editingFinished()),this,SLOT(update_values()));
  connect(ui->lineEdit_deadlayer,SIGNAL(editingFinished()),this,SLOT(update_values()));




}

Dialog_setup::~Dialog_setup()
{
  delete ui;
    simok=0;

}


void Dialog_setup::setData(int lateral)
{
    lat_front=lateral;
 //   qDebug() << lat_front;

}



void Dialog_setup::on_pushButton_2_clicked()
{

}

/*void Dialog_setup::on_buttonBox_accepted()
{

  }
*/



void Dialog_setup::on_pushButton_cancel_clicked()
{
simok=1;
}


void Dialog_setup::on_radioButton_2_clicked()
{
    ui->lineEdit_ionpath->setDisabled(true);
    ui->pushButton_browseion->setDisabled(true);
    ui->lineEdit_deadlayer->setDisabled(true);
    ui->ioniz_plot->setVisible(false);
    ui->lineEdit_ionpath->setText("");
    lat_front=0;
    ui->lineEdit_npoints->setEnabled(true);
    ui->lineEdit_dispersion->setEnabled(true);
    control_ion=-1;
}

void Dialog_setup::on_radioButton_clicked()
{
    ui->lineEdit_ionpath->setEnabled(true);
    ui->pushButton_browseion->setEnabled(true);
    ui->lineEdit_deadlayer->setEnabled(true);
        ui->ioniz_plot->setVisible(true);
        ui->lineEdit_ionpath->setText(path_ioniz);
    lat_front=1;
    ui->lineEdit_npoints->setText("1");
    ui->lineEdit_npoints->setDisabled(true);
    ui->lineEdit_dispersion->setText("0");
    ui->lineEdit_dispersion->setDisabled(true);
    setup_frontal_plot_first();

}


void Dialog_setup::on_pushButton_browseion_clicked()
{
    FileBrowser browser;
    browser.exec();
     // qDebug()<<"iniziato files dialog";
    path_ioniz=browser.on_buttonBox_accepted();
    //qDebug() <<path_ioniz;
    control_ion=0;

ui->lineEdit_ionpath->setText(path_ioniz);
    QFile file_ioniz(path_ioniz);
    QTextStream its_ioniz(&file_ioniz);
    QString line_ioniz;
    if (!file_ioniz.open(QIODevice::ReadOnly))
        {       //  ui->textMessageBox->append("Ionization file not found");
         control_ion=1;
        }
    else
    {
        control_ion=0;
    }


    x_ioniz.resize(100);
    y_ioniz.resize(100);

    if(control_ion==0)
    {


        int l=0;

        while(!its_ioniz.atEnd())
        {


        line_ioniz = its_ioniz.readLine();
        line_ioniz.replace(",",".");
        line_ioniz.replace("\t", "   ");


        QStringList list_ioniz=line_ioniz.split(" ",QString::SkipEmptyParts);

        if(list_ioniz.size()>1)
        {
        bool validate1, validate2;

        double converted1 = list_ioniz.first().toDouble(&validate1);
        double converted2 = list_ioniz[1].toDouble(&validate2);


        if (validate1 == true && validate2 ==true && list_ioniz.size()>1)
        {

    //    x_ioniz_import[l]=list_ioniz[0].toDouble()*1e-4; QUI IL DEAD LAYER E'SPARITO!! //inserito già in micrometri
        x_ioniz[l]=list_ioniz.first().toDouble()*1e-4;//x_ioniz_import[l]-dead_layer; //QUI SIAMO IN micron
        y_ioniz[l]=list_ioniz[1].toDouble()*1e10; //udm: eV/m
        l++;
        }
        }


    }


    file_ioniz.close();

}

    else
    {for(int l=0; l<100/*_import.size()*/; l++){

          //  x_ioniz_import[l]=0;  //inserito già in micrometri
            x_ioniz[l]=0;//x_ioniz_import[l]-dead_layer;
            y_ioniz[l]=0;
            control_ion=0;

        }}


// QUESTO E' SPOSTABILE NEL MAIN WINDOW
/*
    yioniz_inter.resize(length);

    for(int i=0; i<L; i++)
    {
        int j=0;
        while(i*deltax*1e6 > x_ioniz[j]) {j++;}

        if(j==0)
        {
            yioniz_inter[i]=y_ioniz[j];
        }
        else if((i*deltax*1e6-x_ioniz[j-1])<(x_ioniz[j]-i*deltax*1e6))
        {
            yioniz_inter[i]=y_ioniz[j-1];

        }
        else
        {
            yioniz_inter[i]=y_ioniz[j];

        }

    }

    CDF_ion.resize(yioniz_inter.size());

  //  qDebug() <<control_ion;
    if(control_ion==0)
    {
        double sum=0;

        for(int k=0; k<yioniz_inter.size(); k++)
        {

            sum+=yioniz_inter[k];
            CDF_ion[k]=sum;
         //   qDebug()<<yioniz_inter[k]<<sum<<CDF_ion[k];

        }

         for(int k=0; k<CDF_ion.size(); k++)
         {
             CDF_ion[k]/=sum;

         }
        // qDebug()<< yioniz_inter[0]<<sum<<CDF_ion[0]<< CDF_ion[L-1];
    }
    */

    setupIoniz(ui->ioniz_plot);
    ui->ioniz_plot->replot();



/*
if(control_ion==0 || control_vac==0)
{
    ui->pushButton_deadlayer->setEnabled(true);
    ui->doubleSpinBox->setEnabled(true);


}*/
}

void Dialog_setup::setupIoniz(QCustomPlot *potPlot)
{

    potPlot->addGraph(); // blue line
    potPlot->graph(0)->setPen(QPen(Qt::green));
    potPlot->graph(0)->setBrush(QBrush(QColor(128, 128, 128)));
    potPlot->graph(0)->setAntialiasedFill(false);
    potPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
    // generate some points of data (y0 for first, y1 for second graph):
    potPlot->addGraph();
    potPlot->graph(1)->setPen(QPen(Qt::red));
    potPlot->graph(1)->setBrush(QBrush(QColor(250, 0, 0)));
    potPlot->graph(1)->setAntialiasedFill(false);
    potPlot->yAxis->setAutoTickCount(5);
    potPlot->xAxis->setAutoTickCount(5);

QVector<double> x_ioniz_um, y_ioniz_um;
x_ioniz_um.resize(100);
y_ioniz_um.resize(100);


for(int i=0; i<100; i++)
{
    x_ioniz_um[i]=x_ioniz[i]; //in unità di micrometro
    y_ioniz_um[i]=y_ioniz[i]/100; //in unità eV/cm

}


  // configure right and top axis to show ticks but no labels (could've also just called customPlot->setupFullAxesBox):
/*QVector<double> x_array_interpol;
x_array_interpol.resize(length);



for(int i=0; i<length; i++)
{
    x_array_interpol[i]=i*deltax*1e6;
}

*/

  potPlot->xAxis->setVisible(true);
  potPlot->xAxis2->setVisible(false);
  potPlot->xAxis2->setTickLabels(true);
  potPlot->yAxis2->setVisible(true);
  potPlot->yAxis2->setTickLabels(false);
 // potPlot->yAxis->setTickStep(10);

  potPlot->graph(0)->setData(x_ioniz/*_um*/, y_ioniz/*_um*/);
 // potPlot->graph(1)->setData(x_array_interpol, y_ioniz_input);

  // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
  potPlot->xAxis->setLabel("Position (um)");
  potPlot->yAxis->setLabel("Ionization (eV/cm)");
  potPlot->graph(0)->rescaleAxes(true);
  potPlot->graph(1)->rescaleAxes(true);
  potPlot->xAxis->setRange(0.0,x_ioniz.last());

//  potPlot->yAxis->setRange(0.0,100);
//qDebug() <<imax;
}

void Dialog_setup::update_values()
{
    n_point = ui->lineEdit_npoints->text().toInt();
  //  qDebug() << n_point;
  //  straggl = ui->lineEdit_straggling->text().toDouble();
    n_ion = ui->lineEdit_nions-> text().toInt();
    n_eh = ui->lineEdit_npairs-> text().toInt();
    dispersion = ui->lineEdit_dispersion-> text().toDouble();
    e_noise = ui->lineEdit_noise-> text().toDouble();
    e_threshold = ui->lineEdit_threshold-> text().toDouble();
    dead_layer=ui->lineEdit_deadlayer->text().toDouble();
    if(lat_front==1 && control_ion==0)
    {

        setup_frontal_plot_first();
        setupIoniz(ui->ioniz_plot);
        ui->ioniz_plot->replot();
    }

    if(n_point>0 && n_ion >0 && n_eh >0 && control_ion<1)
    {
    simok=0;
    ui->pushButton_2->setEnabled(true);
    }
    else
    {
    ui->pushButton_2->setDisabled(true);
    simok=1;
    }

}


void Dialog_setup::setup_frontal_plot_first(){


    control_ion=0;

    ui->lineEdit_ionpath->setText(path_ioniz);
    QFile file_ioniz(path_ioniz);
    QTextStream its_ioniz(&file_ioniz);
    QString line_ioniz;
    if (!file_ioniz.open(QIODevice::ReadOnly))
        {       //  ui->textMessageBox->append("Ionization file not found");
         control_ion=1;
        }
    else
    {
        control_ion=0;
    }

    QVector<double> x_ion_import;
    x_ion_import.resize(0);
    x_ioniz.resize(0);
    y_ioniz.resize(0);

    if(control_ion==0)
    {

        int l=0;

        while(!its_ioniz.atEnd())
        {


        line_ioniz = its_ioniz.readLine();
        line_ioniz.replace(",",".");
        line_ioniz.replace("\t", "   ");


        QStringList list_ioniz=line_ioniz.split(" ",QString::SkipEmptyParts);

        if(list_ioniz.size()>1)
        {
        bool validate1, validate2;

        double converted1 = list_ioniz.first().toDouble(&validate1);
        double converted2 = list_ioniz[1].toDouble(&validate2);


        if (validate1 == true && validate2 ==true && list_ioniz.size()>1)
        {

        x_ion_import.push_back(list_ioniz.first().toDouble()*1e-4);  //inserito già in micrometri
        x_ioniz.push_back(x_ion_import[l]-dead_layer);
        y_ioniz.push_back(list_ioniz[1].toDouble()*1e10);            //udm: vac/m
        l++;


        }
        }
    }
    file_ioniz.close();

/*
    for(int l=0; l<23; l++)
    {
        line_ioniz = its_ioniz.readLine();

    }


  //  qDebug()<< "fine lettura header";


    for(int l=0; l<100; l++)
    {
        line_ioniz = its_ioniz.readLine();
        line_ioniz.replace(",",".");
        QStringList list_ioniz=line_ioniz.split(" ",QString::SkipEmptyParts);
        x_ioniz[l]=list_ioniz[0].toDouble()*1e-4;//x_ioniz_import[l]-dead_layer; //QUI SIAMO IN micron
        y_ioniz[l]=list_ioniz[1].toDouble()*1e10; //udm: eV/m

    }


    file_ioniz.close();
    */
}

    else
    {
        for(int l=0; l<100/*_import.size()*/; l++)
        {

          //  x_ioniz_import[l]=0;  //inserito già in micrometri
            x_ioniz.push_back(l);//x_ioniz_import[l]-dead_layer;
            y_ioniz.push_back(0);
            control_ion=0;

        }
    }

    setupIoniz(ui->ioniz_plot);
    ui->ioniz_plot->replot();

}

