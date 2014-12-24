#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <math.h>
#include <fstream>
#include <stdio.h>
#include <sys/time.h>
#include <iomanip>
#include <string>
#include <sstream>
#include <Qt/qstring.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <qfile.h>
#include <qtextstream.h>
#include <QTextBrowser>
#include "generatori.h"
#include <QDir>
#include <QThread>
#include <QtConcurrentRun>
#include <MyLibraryWrapper.h>

using namespace std;

int resume=0;

MyLibraryWrapper *wrapper = new MyLibraryWrapper();

MyLibraryWrapper::MyLibraryWrapper() : QThread()
{
  // We have to do this to make sure our thread has the
  // correct affinity.
  moveToThread(this);


  // This will do nothing until the user calls start().
}

void MyLibraryWrapper::run()
{
  // This schedules the doTheWork() function
  // to run just after our event loop starts up
     //   MainWindow *w;
  QTimer::singleShot(0, this, SLOT(doTheWork()));

  // This starts the event loop. Note that
  // exec() does not return until the
  // event loop is stopped.
  exec();

}



void MyLibraryWrapper::launch()
{
    run();
}

void MyLibraryWrapper::doTheWork()
{

}

void MainWindow::wrapperDone()
{
wrapper->dataTimer.stop();
ui->pushButton_stop->setDisabled(true);
ui->pushButton_sim->setEnabled(true);
ui->pushButton_simsetup->setEnabled(true);
 ui->textMessageBox->append("Simulation complete.");
disconnect(wrapper, SIGNAL(thread_finished()),this,SLOT(wrapperDone()));
}


double t0, t1;
struct timeval wt2, wt1, wt0;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    title_plot="";
    set_legend=0;
    ui->checkBox->setChecked(false);
    slide_generation_point=-1;

    ui->pushButton_devsetup->setEnabled(true);
    ui->pushButton_simsetup->setEnabled(false);
    ui->pushButton_radsetup->setEnabled(false);

    gnearest=-1;
    gpoint=-1;
    xgpoint.resize(2);
    ygpoint.resize(2);
    xgpoint[0]=gpoint;
    xgpoint[1]=gpoint;
    ygpoint[0]=0;
    ygpoint[1]=2;

    //inizializza variabili per simulation setup
    lat_front=0;
    n_point = 0;
    n_ion = 0;
    n_eh = 0;
    dispersion = 0;    //um
    e_noise= 0;            // %
    e_threshold= 0;

    this->ui->progressBar->setValue(0);
    this->ui->horizontalSlider->setValue(slide_generation_point);
    ui->horizontalSlider->setDisabled(true);

    if(this->ui->horizontalSlider->value()==-1){ this->ui->textBrowser->setText("All data");}
    else{this->ui->textBrowser->setText("x=" + QString::number(slide_generation_point)+" um");}

    last_path="/";
    ui->time_spinBox->setDisabled(true);

    files_dialog_check_ok=0;

    dead_layer=0;
    x_vac_import.resize(100);
    x_vac.resize(100);
    y_vac.resize(100);
    x_ioniz_import.resize(100);
    x_ioniz.resize(100);
    y_ioniz.resize(100);
    L=5001;
    radhard_p=0.2;
    radhard_n=0.2;
    rad_yesno=0;
    fluence=0;
    yvac_inter.resize(L);
    nsigma=1e-19;   //m2
    psigma=1e-19;   //,2
    ptherm=1e5;     //m/s
    ntherm=1e5;     //m/s

    A=3500*100; // V/m
    B=7400*100; //V/m
    F=8.8; //CHECK!!!

    taup_arr.resize(L);
    taun_arr.resize(L);
    ptaup.resize(L);
    ptaun.resize(L);


    for(int k=0; k<L; k++)
    {
        taup_arr[k]=taup;
        taun_arr[k]=taun;
        yvac_inter[k]=0;
    }

    deltax=0;  //m
    deltatp=0;
    deltatn=0;

    setupTau(ui->tau_plot);
    ui->tau_plot->replot();

    init_genrand(time(NULL));

    ui->pushButton_simsetup->setDisabled(true);
    ui->pushButton_sim->setDisabled(true);
    ui->pushButton_stop->setDisabled(true);
    ui->pushButton_resume->setDisabled(true);


exp_import=0;
xshift=0;       //udm?
yscale=1;
lat_front=0;

control_vac=1;
control_ion=1;

lat_front=0;

qDebug() << "numero thread ideale: " << QThread::idealThreadCount();

wrapper->start();


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slideChanged(int value)
{
    slide_generation_point=value;

}


QVector<double> cce_mean, cce_mean_plot, gen_pos;
QVector<int> hit_over_threshold;

void MainWindow::on_pushButton_simsetup_clicked()
{

    Dialog_setup dialog;
    dialog.exec();

ui->pushButton_resume->setDisabled(true);


//SERVE CONTROLLO CHE NON VENGANO DIGITATE CAZZATE COLOSSALI!
if(dialog.simok==0/* && dialog.n_point>0*/)
{

    lat_front=dialog.lat_front;
    n_point = dialog.n_point;
    n_ion = dialog.n_ion;
    n_eh = dialog.n_eh;
    dispersion = dialog.dispersion*1e-6;    //um
    e_noise= dialog.e_noise/100;            // %
    e_threshold= dialog.e_threshold/100;    // %
    dead_layer=dialog.dead_layer;
    ui->textMessageBox->append("Number of generation points for simulation: " + QString::number(n_point) + ".");
    ui->textMessageBox->append("Number of ions per point: " + QString::number(n_ion) + " .");
    ui->textMessageBox->append("Number of e-h pairs per ion: " + QString::number(n_eh) + " .");
    ui->textMessageBox->append("Lateral dispersion: " + QString::number(dispersion) + " m.");
    ui->textMessageBox->append("Electronic noise: " + QString::number(e_noise*100) + " % CCE.");
    ui->textMessageBox->append("Noise threshold: " + QString::number(e_threshold*100) + " % CCE.");

   steps = L / n_point;

   ui->horizontalSlider->setSingleStep((int) steps);
   ui->horizontalSlider->setMaximum(steps*(n_point-1));
   ui->pushButton_sim->setEnabled(true);
   ui->pushButton_radsetup->setEnabled(true);


if(lat_front==1)
{

if(dialog.control_ion==0)
{
last_path_ioniz=dialog.path_ioniz;
last_path=last_path_ioniz;
while(!last_path.endsWith("/"))
{
    last_path.remove(last_path.length()-1,1);
    qDebug() <<last_path;

}


x_ioniz=dialog.x_ioniz;
y_ioniz=dialog.y_ioniz;

yioniz_inter.resize(L);


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

    double sum=0;

    for(int k=0; k<yioniz_inter.size(); k++)
    {

        sum+=yioniz_inter[k];
        CDF_ion[k]=sum;
    }

     for(int k=0; k<CDF_ion.size(); k++)
     {
         CDF_ion[k]/=sum;

     }
}


setupIoniz(ui->ioniz_plot);
ui->ioniz_plot->replot();

ui->CDF_plot->setVisible(true);

setupCDF(ui->CDF_plot);
ui->CDF_plot->replot();

}
}


else
{
    ui->pushButton_radsetup->setEnabled(false);

}

text_simulation();

}



void MainWindow::setupGunn(QCustomPlot *potPlot)
{

potPlot->setTitle(title_plot);
    potPlot->addGraph(); // blue line
    potPlot->graph(0)->setPen(QPen(Qt::green));
    potPlot->graph(0)->setBrush(QBrush(QColor(0, 250, 0)));
    potPlot->graph(0)->setAntialiasedFill(false);
    potPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph

  QVector<double> x(gunn.size()), y0(gunn.size());
  for (int i=0; i<gunn.size(); ++i)
  {
    x[i] = i*deltax/1e-6;
    y0[i] = fabs(gunn[i]);

  }

  // configure right and top axis to show ticks but no labels (could've also just called customPlot->setupFullAxesBox):

  potPlot->xAxis->setVisible(true);
  potPlot->xAxis2->setVisible(false);
  potPlot->xAxis2->setTickLabels(true);
  potPlot->yAxis2->setVisible(true);
  potPlot->yAxis2->setTickLabels(false);
  potPlot->graph(0)->setData(x, y0);
  potPlot->graph(0)->rescaleAxes(true);
  potPlot->xAxis->setLabel("Position (um)");
  potPlot->yAxis->setLabel("Weighting potential");
  potPlot->yAxis->setRange(-0.0,1.1);
  potPlot->yAxis->setAutoTickCount(5);
  potPlot->xAxis->setRange(-0.0,L*deltax*1e6);

}

void MainWindow::setupPdrift(QCustomPlot *potPlot)
{
potPlot->setTitle(title_plot);
    potPlot->addGraph(); // blue line
    potPlot->graph(0)->setPen(QPen(Qt::blue));
    potPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 250)));
    potPlot->graph(0)->setAntialiasedFill(false);
    potPlot->graph(0)->setPen(QPen(Qt::blue));
  QVector<double> x(pdrift.size()), y0(pdrift.size());
  for (int i=0; i<pdrift.size(); ++i)
  {
    x[i] = i*deltax/1e-6;
    y0[i] = pdrift[i];

  }

  potPlot->addGraph();
  potPlot->addGraph();
  potPlot->addGraph();
  potPlot->graph(1)->setPen(QPen(Qt::red));
  potPlot->graph(2)->setPen(QPen(Qt::red));
  potPlot->graph(3)->setPen(QPen(Qt::red));

  QVector<double> x1(2), y1(2), y2(2), y3(2);
  x1[0]=0;
  x1[1]=gunn.size()*deltax/1e-6;
  y1[0]=0.5;
  y1[1]=0.5;
  y2[0]=1.;
  y2[1]=1.;
  y3[0]=0;
  y3[1]=0;

  potPlot->xAxis->setVisible(true);
  potPlot->xAxis2->setVisible(false);
  potPlot->xAxis2->setTickLabels(true);
  potPlot->yAxis2->setVisible(true);
  potPlot->yAxis2->setTickLabels(false);
  potPlot->yAxis->setTickStep(10);
  potPlot->graph(0)->setData(x, y0);
  potPlot->graph(1)->setData(x1, y1);
  potPlot->graph(2)->setData(x1, y2);
  potPlot->graph(3)->setData(x1, y3);
  potPlot->graph(0)->rescaleAxes(true);
  potPlot->xAxis->setLabel("Position (um)");
  potPlot->yAxis->setLabel("Hole motion probab.");
  potPlot->yAxis->setRange(-0.5,1.5);
  potPlot->xAxis->setRange(-0.0,L*deltax*1e6);

}


void MainWindow::setupNdrift(QCustomPlot *potPlot)
{

    potPlot->setTitle(title_plot);
    potPlot->addGraph(); // blue line
    potPlot->graph(0)->setPen(QPen(Qt::blue));
    potPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 250)));
    potPlot->graph(0)->setAntialiasedFill(false);
    potPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
  QVector<double> x(ndrift.size()), y0(ndrift.size());
  for (int i=0; i<ndrift.size(); ++i)
  {
    x[i] = i*deltax/1e-6;
    y0[i] = ndrift[i];

  }

  potPlot->addGraph();
  potPlot->addGraph();
  potPlot->addGraph();
  potPlot->graph(1)->setPen(QPen(Qt::red));
  potPlot->graph(2)->setPen(QPen(Qt::red));
  potPlot->graph(3)->setPen(QPen(Qt::red));

  QVector<double> x1(2), y1(2), y2(2), y3(2);
  x1[0]=0;
  x1[1]=gunn.size()*deltax/1e-6;
  y1[0]=0.5;
  y1[1]=0.5;
  y2[0]=1.;
  y2[1]=1.;
  y3[0]=0;
  y3[1]=0;

  // configure right and top axis to show ticks but no labels (could've also just called customPlot->setupFullAxesBox):

  potPlot->xAxis->setVisible(true);
  potPlot->xAxis2->setVisible(false);
  potPlot->xAxis2->setTickLabels(true);
  potPlot->yAxis2->setVisible(true);
  potPlot->yAxis2->setTickLabels(false);
  potPlot->yAxis->setTickStep(10);
  potPlot->graph(0)->setData(x, y0);
  potPlot->graph(1)->setData(x1, y1);
  potPlot->graph(2)->setData(x1, y2);
  potPlot->graph(3)->setData(x1, y3);
  potPlot->graph(0)->rescaleAxes(true);
  potPlot->xAxis->setLabel("Position (um)");
  potPlot->yAxis->setLabel("Electron motion probab.");
  potPlot->yAxis->setRange(-0.5,1.5);
  potPlot->xAxis->setRange(-0.0,L*deltax*1e6);

}

void MainWindow::setupEfield(QCustomPlot *potPlot)
{
    potPlot->setTitle(title_plot);
    potPlot->addGraph(); // blue line
    potPlot->graph(0)->setPen(QPen(Qt::green));
    potPlot->graph(0)->setBrush(QBrush(QColor(0, 250, 0)));
    potPlot->graph(0)->setAntialiasedFill(false);
    potPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
    // generate some points of data (y0 for first, y1 for second graph):
  QVector<double> x(efield.size()), y0(efield.size());
  double emax=0;
  double emin=0;

  for (int i=0; i<efield.size(); ++i)
  {
    x[i] = i*deltax/1e-6;
    y0[i] = efield[i]/100; // V/cm
    if(y0[i]>emax){emax=y0[i];}
    if(y0[i]<emin){emin=y0[i];}

  }

  // configure right and top axis to show ticks but no labels (could've also just called customPlot->setupFullAxesBox):

  potPlot->xAxis->setVisible(true);
  potPlot->xAxis2->setVisible(false);
  potPlot->xAxis2->setTickLabels(true);
  potPlot->yAxis2->setVisible(true);
  potPlot->yAxis2->setTickLabels(false);
 // potPlot->yAxis->setTickStep(10);
  potPlot->graph(0)->setData(x, y0);
  // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
  potPlot->xAxis->setRange(-0.0,L*deltax*1e6);
  potPlot->yAxis->setRange(emin*1.2,emax*1.2);
  potPlot->xAxis->setLabel("Position (um)");
  potPlot->yAxis->setLabel("Electric field (V/cm)");
}


void MainWindow::setupIoniz(QCustomPlot *potPlot)
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


QVector<double> x_ioniz_um, y_ioniz_um,y_ioniz_input;
x_ioniz_um.resize(x_ioniz.size());
y_ioniz_um.resize(y_ioniz.size());
y_ioniz_input.resize(yioniz_inter.size());
double imax;
imax=0;

for(int i=0; i<x_ioniz.size(); i++)
{
    x_ioniz_um[i]=x_ioniz[i]; //in unità di micrometro
    y_ioniz_um[i]=y_ioniz[i]/100; //in unità eV/cm

}


for(int i=0; i< yioniz_inter.size();i++)
{
    y_ioniz_input[i]=yioniz_inter[i]/100;
}

  // configure right and top axis to show ticks but no labels (could've also just called customPlot->setupFullAxesBox):
QVector<double> x_array_interpol;
x_array_interpol.resize(L);

for(int i=0; i<L; i++)
{
    x_array_interpol[i]=i*deltax*1e6;
}

  potPlot->xAxis->setVisible(true);
  potPlot->xAxis2->setVisible(false);
  potPlot->xAxis2->setTickLabels(true);
  potPlot->yAxis2->setVisible(true);
  potPlot->yAxis2->setTickLabels(false);
 // potPlot->yAxis->setTickStep(10);

  potPlot->graph(0)->setData(x_ioniz_um, y_ioniz_um );
  potPlot->graph(1)->setData(x_array_interpol, y_ioniz_input);

  // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
  potPlot->xAxis->setLabel("Position (um)");
  potPlot->yAxis->setLabel("Ionization (eV/cm)");
  potPlot->graph(0)->rescaleAxes(true);
  potPlot->graph(1)->rescaleAxes(true);
  potPlot->xAxis->setRange(0.0,x_ioniz[x_ioniz.size()-1]);
}


void MainWindow::setupCDF(QCustomPlot *potPlot)
{

    potPlot->addGraph(); // blue line
    potPlot->graph(0)->setPen(QPen(Qt::green));
    potPlot->graph(0)->setBrush(QBrush(QColor(128, 128, 128)));
    potPlot->graph(0)->setAntialiasedFill(false);
    potPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
    // generate some points of data (y0 for first, y1 for second graph):


QVector<double> x_CDF_um;

x_CDF_um.resize(CDF_ion.size());

for(int i=0; i< CDF_ion.size(); i++)
{
    x_CDF_um[i]=i*deltax*1e6;
}

  // configure right and top axis to show ticks but no labels (could've also just called customPlot->setupFullAxesBox):

  potPlot->xAxis->setVisible(true);
  potPlot->xAxis2->setVisible(false);
  potPlot->xAxis2->setTickLabels(true);
  potPlot->yAxis2->setVisible(true);
  potPlot->yAxis2->setTickLabels(false);
  potPlot->graph(0)->setData(x_CDF_um, CDF_ion );
  // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
  potPlot->xAxis->setLabel("Position (um)");
  potPlot->yAxis->setLabel("Ionization (eV/cm)");
  potPlot->graph(0)->rescaleAxes(true);
  potPlot->xAxis->setRange(0.0,x_ioniz[x_ioniz.size()-1]);
  potPlot->yAxis->setRange(0.0,1.00);
}

void MainWindow::setupVac(QCustomPlot *potPlot)
{

    potPlot->addGraph(); // blue line
    potPlot->graph(0)->setPen(QPen(Qt::green));
    potPlot->graph(0)->setBrush(QBrush(QColor(0, 250, 250)));
    potPlot->graph(0)->setAntialiasedFill(false);
    potPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
    potPlot->addGraph(); // blue line
    potPlot->graph(1)->setPen(QPen(Qt::black));
    potPlot->graph(1)->setBrush(QBrush(QColor(250, 0, 0)));
    potPlot->graph(1)->setAntialiasedFill(false);

    // generate some points of data (y0 for first, y1 for second graph):
QVector<double> x_vac_um, y_vac_um;
x_vac_um.resize(x_vac.size());
y_vac_um.resize(y_vac.size());
for(int i=0; i<x_vac.size(); i++)
{
    x_vac_um[i]=x_vac[i]; //in unità di micrometro
    y_vac_um[i]=y_vac[i]/100; //in unità vac/cm
}


QVector<double> x_array_interpol, y_array_interpol;
x_array_interpol.resize(L);
y_array_interpol.resize(L);

for(int i=0; i<L; i++)
{
    x_array_interpol[i]=i*deltax*1e6;
    y_array_interpol[i]=yvac_inter[i]/100;

}

  // configure right and top axis to show ticks but no labels (could've also just called customPlot->setupFullAxesBox):

  potPlot->xAxis->setVisible(true);
  potPlot->xAxis2->setVisible(false);
  potPlot->xAxis2->setTickLabels(true);
  potPlot->yAxis2->setVisible(true);
  potPlot->yAxis2->setTickLabels(false);
  potPlot->graph(0)->setData(x_vac_um, y_vac_um);
  potPlot->graph(1)->setData(x_array_interpol, y_array_interpol);
  // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
  potPlot->xAxis->setLabel("Position (um)");
  potPlot->yAxis->setLabel("Vacancy dens. (1/cm)");
  potPlot->graph(0)->rescaleAxes(true);
  potPlot->xAxis->setRange(0.0,x_vac_import[x_vac_import.size()-1]);

  double max_vac=0;

  for(int i=0; i<y_vac_um.size(); i++)
  {
      if(max_vac<y_vac_um[i]) max_vac=y_vac_um[i];
  }

 potPlot->yAxis->setRange(0.0,max_vac);
}



void MainWindow::setupTau(QCustomPlot *potPlot)
{

    potPlot->addGraph(); // blue line
    potPlot->graph(0)->setPen(QPen(Qt::blue));
    potPlot->graph(0)->setAntialiasedFill(false);
    potPlot->addGraph(); // blue line
    potPlot->graph(1)->setPen(QPen(Qt::red));
    potPlot->graph(1)->setAntialiasedFill(false);

    // generate some points of data (y0 for first, y1 for second graph):

QVector<double> x_array_interpol,taun_interpol, taup_interpol;
x_array_interpol.resize(L);
taun_interpol.resize(L);
taup_interpol.resize(L);

for(int i=0; i<L; i++)
{
    x_array_interpol[i]=i*deltax*1e6;
    taun_interpol[i]=taun_arr[i]*1e9;
    taup_interpol[i]=taup_arr[i]*1e9;
}

  // configure right and top axis to show ticks but no labels (could've also just called customPlot->setupFullAxesBox):

  potPlot->xAxis->setVisible(true);
  potPlot->xAxis2->setVisible(false);
  potPlot->xAxis2->setTickLabels(true);
  potPlot->yAxis2->setVisible(true);
  potPlot->yAxis2->setTickLabels(false);
  potPlot->graph(0)->setData(x_array_interpol, taun_interpol);
  potPlot->graph(1)->setData(x_array_interpol, taup_interpol);
  // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
  potPlot->xAxis->setLabel("Position (um)");
  potPlot->yAxis->setLabel("Lifetime (ns)");
  potPlot->xAxis->setRange(0.0,L*deltax*1e6);

  double max_vac=0;

  for(int i=0; i<taun_interpol.size(); i++)
  {

      if(max_vac<taun_interpol[i]) max_vac=taun_interpol[i];
     if(max_vac<taup_interpol[i]) max_vac=taup_interpol[i];
  }

  potPlot->yAxis->setRange(0.0,max_vac*(1.2));

}


void MainWindow::legendClick()
{
    ui->mc_plot->legend->setVisible(false);
    ui->mc_plot->replot();

}

void MainWindow::setupMC(QCustomPlot *potPlot, QCustomPlot *histo, QCustomPlot *time)
{
    histo->addGraph(); // blue line
    histo->graph(0)->setPen(QPen(Qt::black));
    histo->graph(0)->setLineStyle(QCPGraph::lsStepCenter);
    histo->graph(0)->setBrush(QBrush(QColor(0, 0, 250)));
    histo->graph(0)->setAntialiasedFill(false);
    histo->xAxis->setVisible(true);
    histo->xAxis2->setVisible(false);
    histo->xAxis2->setTickLabels(true);
    histo->yAxis2->setVisible(true);
    histo->yAxis2->setTickLabels(false);
    histo->graph(0)->rescaleAxes(true);
    histo->xAxis->setTickStep(0.25);
    histo->xAxis2->setTickStep(0.25);
    histo->xAxis->setAutoTickCount(5);
    histo->xAxis->setRange(0.0,1.1);
    histo->xAxis->setLabel("CCE");
    histo->yAxis->setLabel("counts");
    histo->addGraph(); // blue line
    QPen graphPen_red;
    graphPen_red.setColor(Qt::red);
    graphPen_red.setWidthF(2);
    histo->graph(1)->setPen(graphPen_red);
 //   histo->graph(1)->setPen(QPen(Qt::red));
    histo->setTitle(title_plot);
    QCPItemText *textLabel = new QCPItemText(histo);
    histo->addItem(textLabel);
    textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel->position->setCoords(0.5,0);
    textLabel->setText("Mean: " + QString::number(/*cce_mean_frontal*/ion));
    textLabel->setPen(QPen(Qt::black));
    textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);

  //  histo->graph(1)->setLineStyle(); //DIMENSIONI LINEA!!!


    time->addGraph(); // blue line
    time->graph(0)->setPen(QPen(Qt::red));

    time->xAxis->setVisible(true);
    time->xAxis2->setVisible(false);
    time->xAxis2->setTickLabels(true);
    time->yAxis2->setVisible(true);
    time->yAxis2->setTickLabels(false);
    time->xAxis->setAutoTickCount(5);
    time->setTitle(title_plot);
    time->xAxis->setLabel("Time (ns)");
    time->yAxis->setLabel("CCE");

    potPlot->addGraph(); // blue line
    potPlot->graph(0)->setPen(QPen(Qt::gray));
    potPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    potPlot->graph(0)->setScatterStyle(QCP::ssCircle);
    potPlot->graph(0)->setScatterSize(2);
    potPlot->graph(0)->setBrush(QBrush(QColor(0, 250, 0)));
    potPlot->graph(0)->setAntialiasedFill(false);
    potPlot->graph(0)->setName("Simulated pulses");
    potPlot->setTitle(title_plot);
    //potPlot->graph(0)->setSelected(QCPPainter::scatterPixmap());

    potPlot->addGraph();
    potPlot->graph(1)->setPen(QPen(Qt::red));
    potPlot->graph(1)->setLineStyle(QCPGraph::lsLine);
    potPlot->graph(1)->setName("Mean profile");

    potPlot->addGraph();
    potPlot->graph(2)->setPen(QPen(Qt::black));
    potPlot->graph(2)->setLineStyle(QCPGraph::lsLine);
    potPlot->graph(2)->setName("Median profile");

  // configure right and top axis to show ticks but no labels (could've also just called customPlot->setupFullAxesBox):

  potPlot->xAxis->setVisible(true);
  potPlot->xAxis2->setVisible(false);
  potPlot->xAxis2->setTickLabels(true);
  potPlot->yAxis2->setVisible(true);
  potPlot->yAxis2->setTickLabels(false);
  // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
  potPlot->graph(0)->rescaleAxes(true);
  potPlot->yAxis->setRange(-0.,1.2);
  potPlot->xAxis->setRange(-0.0,L*deltax*1e6);
  potPlot->xAxis->setLabel("Position (um)");
  potPlot->yAxis->setLabel("CCE");
  potPlot->addGraph();
  QPen graphPen;
  graphPen.setColor(Qt::darkCyan);
  graphPen.setWidthF(1);
  potPlot->graph()->setPen(graphPen);
//  potPlot->graph(3)->setPen(QPen(Qt::cyan));
  potPlot->graph(3)->setLineStyle(QCPGraph::lsLine);
  potPlot->graph(3)->removeFromLegend();


  potPlot->addGraph();
    potPlot->addGraph();

    potPlot->graph(4)->removeFromLegend();
    potPlot->graph(5)->removeFromLegend();

  if(  exp_import==1)
  {

      potPlot->addGraph();
      potPlot->graph(6)->setPen(QPen(Qt::blue));
      potPlot->graph(6)->setLineStyle(QCPGraph::lsLine);
      potPlot->graph(6)->setSelected(false);
      potPlot->graph(6)->setName("Experimental profile");
      potPlot->graph(6)->setData(x_exp_shift, y_exp_scale);

  }


  if(set_legend==0)   { potPlot->legend->setVisible(false);}
  if(set_legend==1)   { potPlot->legend->setVisible(true);}

  potPlot->legend->setSize(50,30);
  QFont legendFont = font();  // start out with MainWindow's font..
  legendFont.setPointSize(9); // and make a bit smaller for legend
  potPlot->legend->setFont(legendFont);
  potPlot->legend->setPositionStyle(QCPLegend::psTopRight);


  if(resume==0)
  {


      qDebug() << "copy to thread relevant values";

  wrapper->lat_front=lat_front;
  wrapper-> temp = temp;
  wrapper-> L =L;
  wrapper-> Tn= Tn;
  wrapper-> Tp=Tp;
  wrapper-> control_arrayL= control_arrayL;
  wrapper-> ndiff = ndiff;
  wrapper-> pdiff = pdiff;
  wrapper-> gunn = gunn;
  wrapper-> n_point = n_point;
  wrapper-> n_ion=n_ion;
  wrapper-> n_eh = n_eh;
  wrapper-> dispersion = dispersion;
  wrapper-> e_noise = e_noise;
  wrapper-> e_threshold = e_threshold;
  wrapper-> ion = ion;
  wrapper-> p = p;
  wrapper-> x0=x0;
  wrapper-> steps=steps;
  wrapper-> taup = taup;
  wrapper-> taun = taun;
  wrapper-> deltax =deltax;
  wrapper-> deltatn = deltatn;
  wrapper-> deltatp = deltatp;
  wrapper-> progress = progress;
  wrapper-> end_sim = end_sim;
  wrapper-> straggle = straggle;
  wrapper-> xstart = xstart;
  wrapper-> cce = cce;
  wrapper-> cce_noise = cce_noise;
  wrapper-> cce_threshold = cce_threshold;
  wrapper-> multipos = multipos;
  wrapper-> cce_median = cce_median;
  wrapper-> tot_spectrum = tot_spectrum;
  wrapper-> maximum = maximum;
  wrapper-> gnearest = gnearest;
  wrapper-> tot_hit_over = tot_hit_over;
  wrapper-> tot_mean_plot = tot_mean_plot;
  wrapper-> histo_step = histo_step;
  wrapper-> x_vac = x_vac;
  wrapper-> y_vac = y_vac;
  wrapper-> x_ioniz = x_ioniz;
  wrapper-> efield = efield;
  wrapper-> pdrift = pdrift;
  wrapper-> ndrift = ndrift;
  wrapper-> taup_arr = taup_arr;
  wrapper-> taun_arr = taun_arr;
  wrapper-> ptaun = ptaun;
  wrapper-> ptaup = ptaup;
  wrapper-> CDF_x = CDF_x;
  wrapper-> xioniz_inter = xioniz_inter;
  wrapper-> yioniz_inter = yioniz_inter;
  wrapper-> CDF_ion = CDF_ion;
  wrapper-> cce_frontal = cce_frontal;
  wrapper-> cce_frontal_ion_n = cce_frontal_ion_n;
  wrapper-> cce_frontal_ion_p = cce_frontal_ion_p;
  wrapper-> cce_frontal_aux = cce_frontal_aux;
  wrapper-> cce_frontal_thresh = cce_frontal_thresh;
  wrapper-> cce_time_frontal_tot = cce_time_frontal_tot;
  wrapper-> cce_tfrontal = cce_tfrontal;
  wrapper-> cce_time_frontal_tot = cce_time_frontal_tot;
  wrapper-> cce_tfrontal_aux = cce_tfrontal_aux;
  wrapper-> hit_frontal = hit_frontal;
  wrapper-> aux_mean_frontal = aux_mean_frontal;
  wrapper-> cce_time_frontal_p = cce_time_frontal_p;
  wrapper-> cce_time_frontal_n = cce_time_frontal_n;
  wrapper-> xgpoint = xgpoint;
  wrapper-> ygpoint = ygpoint;
  wrapper-> slide_generation_point = slide_generation_point;
  wrapper-> gpoint = gpoint;
  wrapper-> cce_frontal_ion_p2 = cce_frontal_ion_p2;
  wrapper-> cce_frontal_ion_n2 = cce_frontal_ion_n2;
  wrapper-> cce_time_frontal_tot2 = cce_time_frontal_tot2;
  wrapper->cce_tfrontal2 = cce_tfrontal2;
  wrapper-> cce_tfrontal_aux2 = cce_tfrontal_aux2;
  wrapper->cce_time_frontal_p2 = cce_time_frontal_p2;
  wrapper-> cce_time_frontal_n2 = cce_time_frontal_n2;
  wrapper-> maxtscale = maxtscale;
  wrapper-> valtscale = valtscale;
  wrapper->prog_show=0;
  }
  if(resume==1)
  {resume=0;}


  connect(wrapper, SIGNAL(updateProgressbar(int)),ui->progressBar, SLOT(setValue(int)));
  connect(wrapper, SIGNAL(passValues()), SLOT(updatePlot() ) );
  connect(wrapper, SIGNAL(thread_finished()),this,SLOT(wrapperDone()));

}


void MainWindow::updatePlot( )
{

            tot_mean_plot=wrapper->tot_mean_plot;
            cce_time_frontal_tot=wrapper->cce_time_frontal_tot;
            cce_tfrontal2=wrapper-> cce_tfrontal2;
            tot_hit_over=wrapper-> tot_hit_over;
            y_exp_scale=wrapper-> y_exp_scale;
            x_exp_shift=wrapper-> x_exp_shift;
            cce_median=wrapper-> cce_median;
          //  valtscale=wrapper-> valtscale;
            cce_threshold=wrapper-> cce_threshold;
            multipos=wrapper-> multipos;
            maximum=wrapper-> maximum;
            histo_step=wrapper-> histo_step;
            tot_spectrum=wrapper-> tot_spectrum;
            multipos=wrapper-> multipos;
            cce_tfrontal=wrapper->cce_tfrontal;
            cce_mean_frontal=wrapper->cce_mean_frontal;
            x_histo_mean=wrapper->x_histo_mean;
            y_histo_mean=wrapper->y_histo_mean;

            if(lat_front==1){

       //         qDebug()<< "sono in frontale";
            ui->time_plot->graph(0)->clearData();
             ui->time_plot->graph(0)->setData(cce_time_frontal_tot,cce_tfrontal);


             ui->time_plot->xAxis->setRange(0,valtscale/*2*max(taun*1e9,taup*1e9)*/);
             ui->time_plot->yAxis->setRange(0,1);
             ui->time_plot->replot();
             ui->histo_plot->clearItems();
             ui->histo_plot->graph(0)->clearData();
             ui->histo_plot->graph(1)->clearData();
             ui->histo_plot->graph(0)->addData(histo_step, tot_spectrum[0]);
             ui->histo_plot->graph(1)->addData(x_histo_mean, y_histo_mean);

             QCPItemText *textLabel = new QCPItemText(ui->histo_plot);
             ui->histo_plot->addItem(textLabel);
             textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
             textLabel->position->setCoords(0.5,0);
             textLabel->setText("Mean: " + QString::number(cce_mean_frontal));
             textLabel->setPen(QPen(Qt::black));
             textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);

             ui->histo_plot->yAxis->setRange(-0.0,maximum[0]*1.2);
             ui->histo_plot->replot();


}


if(lat_front==0){
      ui->mc_plot->graph(3)->clearData();
      ui->mc_plot->graph(4)->clearData();
      ui->mc_plot->graph(3)->setData(xgpoint,ygpoint);

      ui->time_plot->graph(0)->clearData();
      if(gnearest>=0)
      {
      ui->time_plot->graph(0)->setData(cce_time_frontal_tot,cce_tfrontal2[gnearest]);
      ui->time_plot->xAxis->setRange(0,valtscale);
      ui->time_plot->yAxis->setRange(0,1.1);
      ui->time_plot->replot();
      }

    ui->mc_plot->graph(0)->clearData();
    for(int j=0; j<n_point; j++){
     ui->mc_plot->graph(0)->addData(multipos[j], cce_threshold[j]);
    }
     ui->mc_plot->graph(1)->clearData();
     ui->mc_plot->graph(1)->addData(gen_pos, cce_mean_plot);
     ui->mc_plot->graph(2)->clearData();
     ui->mc_plot->graph(2)->addData(gen_pos, cce_median);
     if(exp_import==1)
     {
         ui->mc_plot->graph(3)->clearData();
         ui->mc_plot->graph(3)->setData(x_exp_shift, y_exp_scale);

     }
     ui->mc_plot->replot();
     ui->histo_plot->clearItems();
     ui->histo_plot->graph(0)->clearData();
     ui->histo_plot->graph(1)->clearData();

     QVector<QVector<double> > x_histo_mean_lateral, y_histo_mean_lateral;
     x_histo_mean_lateral.resize(n_point+1);
     y_histo_mean_lateral.resize(n_point+1);

     x_histo_mean_lateral[0].resize(2);
     y_histo_mean_lateral[0].resize(2);
     x_histo_mean_lateral[0][0]=tot_mean_plot/tot_hit_over;
     x_histo_mean_lateral[0][1]=tot_mean_plot/tot_hit_over;
     y_histo_mean_lateral[0][0]=0;
     y_histo_mean_lateral[0][1]=maximum[0];

     for(int q=1; q<n_point; q++)
     {
         x_histo_mean_lateral[q].resize(2);
         y_histo_mean_lateral[q].resize(2);
         x_histo_mean_lateral[q][0]=cce_mean_plot[q-1];
         x_histo_mean_lateral[q][1]=cce_mean_plot[q-1];
         y_histo_mean_lateral[q][0]=0;
         y_histo_mean_lateral[q][1]=maximum[q];

     }


            if(gnearest<0){
                ui->histo_plot->graph(0)->setData(histo_step,tot_spectrum[0]);
                ui->histo_plot->graph(1)->setData(x_histo_mean_lateral[0], y_histo_mean_lateral[0]);

                 ui->histo_plot->yAxis->setRange(-0.0,maximum[0]*1.2);

                 QCPItemText *textLabel = new QCPItemText(ui->histo_plot);
                 ui->histo_plot->addItem(textLabel);
                 textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
                 textLabel->position->setCoords(0.5,0);
                 textLabel->setText("Mean: " + QString::number(tot_mean_plot/tot_hit_over));
                 textLabel->setPen(QPen(Qt::black));
                 textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
            }
            else
            {
                ui->histo_plot->graph(0)->setData(histo_step,tot_spectrum[gnearest+1]);
                ui->histo_plot->graph(1)->setData(x_histo_mean_lateral[gnearest+1], y_histo_mean_lateral[gnearest+1]);

                 ui->histo_plot->yAxis->setRange(-0.0,maximum[gnearest+1]*1.2);
                 QCPItemText *textLabel = new QCPItemText(ui->histo_plot);
                 ui->histo_plot->addItem(textLabel);
                 textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
                 textLabel->position->setCoords(0.5,0);
                 textLabel->setText("Mean: " + QString::number(cce_mean_plot[gnearest]));
                 textLabel->setPen(QPen(Qt::black));
                 textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
            }


            ui->histo_plot->replot();
            }

}


void MainWindow::on_pushButton_sim_clicked()
{


resume=0;
  ui->horizontalSlider->setEnabled(true);
    ui->pushButton_simsetup->setDisabled(true);
  ui->pushButton_sim->setDisabled(true);
    ui->pushButton_stop->setEnabled(true);
    ui->pushButton_resume->setDisabled(true);

    xstart.resize(0);
    cce.resize(0);
    cce_noise.resize(0);

    gen_pos.resize(n_point);
    cce_mean.resize(n_point);
    cce_mean_plot.resize(n_point);
    hit_over_threshold.resize(n_point);
    cce_threshold.resize(n_point);
    cce_median.resize(n_point);
    multipos.resize(n_point);
    for(int k=0; k<n_point; k++)
    {
        gen_pos[k]=k*steps*deltax*1e6;
        cce_mean[k]=0;
        cce_mean_plot[k]=0;
        cce_median[k]=0;
        hit_over_threshold[k]=0;
        cce_threshold[k].resize(0);
        multipos[k].resize(0);

    }

            tot_spectrum.resize(n_point+1);
            maximum.resize(n_point+1);

    histo_step.resize(110);

    for(int i=0; i<n_point+1; i++)
    {
        tot_spectrum[i].resize(110);
        maximum[i]=0;

    for(int k=0; k<110; k++){
       tot_spectrum[i][k]=0;

    }
    }
    for(int k=0; k<histo_step.size(); k++)
    {

        histo_step[k]=k*0.01;
    }

    ion=0;
    p=0;
    progress=0;
    end_sim=n_ion*n_point;

   cce_frontal.resize(0);
   cce_frontal_aux.resize(0);
   cce_frontal_thresh.resize(0);
   aux_mean_frontal=0;
   hit_frontal=0;


   //opzioni temporali

   // Tn, Tp sono l'holding time

    //time_resolved: scelgo array da 0 a 3*taumax, a step di 0.1 ns
    double massimo;
    if(taun>taup)
    {
    massimo=3*taun;
    }
    else
    {
    massimo=3*taup;
    }

    maxtscale=massimo*1e9;
    valtscale=massimo/3*1e9;
    ui->time_spinBox->setValue(valtscale);
    ui->time_spinBox->setMaximum(maxtscale);
    ui->time_spinBox->setSingleStep(maxtscale/300);

    ui->time_spinBox->setEnabled(true);

   cce_frontal_ion_p.resize(1000); //decido arbitrariamente 100 punti x time resoluved: check qui in caso cambi!
   cce_frontal_ion_n.resize(1000);
   cce_tfrontal.resize(1000);
   cce_tfrontal_aux.resize(1000);
   cce_time_frontal_tot.resize(1000);
   cce_time_frontal_n.resize(1000);
   cce_time_frontal_p.resize(1000);

   cce_time_frontal_n2.resize(n_point);
   cce_time_frontal_p2.resize(n_point);
   cce_tfrontal2.resize(n_point);
   cce_tfrontal_aux2.resize(n_point);
   cce_frontal_ion_p2.resize(n_point);
   cce_frontal_ion_n2.resize(n_point);

   for(int i=0; i<n_point; i++)
   {
       cce_time_frontal_n2[i].resize(1000);
       cce_time_frontal_p2[i].resize(1000);
       cce_tfrontal2[i].resize(1000);
       cce_tfrontal_aux2[i].resize(1000);
       cce_frontal_ion_p2[i].resize(1000);
       cce_frontal_ion_n2[i].resize(1000);
   }

   ui->time_spinBox->setEnabled(true);


   for(int i=0; i< 1000; i++)
   {

 cce_time_frontal_p[i]=(i*0.001*massimo/deltatp);
 cce_time_frontal_n[i]=(i*0.001*massimo/deltatn);
 cce_tfrontal[i]=0;
 cce_tfrontal_aux[i]=0;
 cce_time_frontal_tot[i]=cce_time_frontal_p[i]*deltatp*1e9; //in unità di ns
      }

   for(int j=0; j<n_point; j++)
   {
       for(int i=0; i<1000; i++)
       {

      cce_time_frontal_n2[j][i]=floor(i*0.01*massimo/deltatn);
      cce_time_frontal_p2[j][i]=floor(i*0.01*massimo/deltatp);
      cce_tfrontal2[j][i]=0;
      cce_tfrontal_aux2[j][i]=0;

        }
   }

  ui->mc_plot->clearGraphs();
  ui->histo_plot->clearGraphs();
  ui->time_plot->clearGraphs();

wrapper->lat_front=lat_front;
    wrapper->start_thread();
 setupMC(ui->mc_plot, ui->histo_plot,ui->time_plot);
  ui->mc_plot->replot();


  ui->textMessageBox->append("Simulation started.");


}

void MainWindow::on_pushButton_resume_clicked()
{
resume=1;

ui->pushButton_stop->setEnabled(true);
ui->pushButton_sim->setDisabled(true);
ui->pushButton_resume->setDisabled(true);
ui->pushButton_simsetup->setDisabled(true);
  ui->mc_plot->clearGraphs();
  setupMC(ui->mc_plot, ui->histo_plot,ui->time_plot);
  ui->mc_plot->replot();
wrapper->start_thread();

  ui->textMessageBox->append("Simulation resumed.");

}


void MyLibraryWrapper::frontal()
{

    if(thread_status==0)
    {

    if(n_ion>0){

  // calculate two new data points:
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
     double key = 0;
#else
  double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif

  int prog_show = progress/n_ion*100;
  emit updateProgressbar(prog_show);

  static double lastPointKey = 0;
  if (key-lastPointKey >.05) // at most add point every 10 ms
  {

      if(ion<n_ion)
        {
          double ctot=0;
          double counterp=0;
          double countern=0;
          int x;


          for(int i=0; i<1000; i++)
          {
              cce_frontal_ion_p[i]=0;
              cce_frontal_ion_n[i]=0;
          }


          for(int h=0;h<n_eh;h++)
            {
              //DISTRIBUZIONE CUMULATIVA IN PROFONDITA'

                        int index=0;
                        double random2=genrand_real1();
                                while(random2 > CDF_ion[index])
                                {
                                    index++;
                                }
                                if(index!=0){
                                    if(CDF_ion[index]-random2 <= random2 - CDF_ion[index-1]){x0=index;}
                                    if(CDF_ion[index]-random2 > random2 -  CDF_ion[index-1]){x0=index-1;}
                                }
                                else {x0=0;}

              double counter1=0;
              double counter2=0;

              //random walk lacune
              int timecount_p=0;        //QUI CCE_FRONTAL_ION E CONTATORI DA RIDEFINIRE!
              int timecount_n=0;
              int time_arr_p=0;
              int time_arr_n=0;
              x=x0;
              int j1=0;
              int j2=0;
              while(j1==0)
                   {
                   if(x<=0)
                   {
                       while(time_arr_p<1000)
                       {
                       cce_frontal_ion_p[time_arr_p]+= -(gunn[x]-gunn[x0]);
                       time_arr_p++;
                       }
                       break;

                   }
                   if(x>=L)
                   {
                       while(time_arr_p<1000)
                       {
                       cce_frontal_ion_p[time_arr_p]+= -(gunn[x]-gunn[x0]);
                       time_arr_p++;
                       }
                       break;
                   }

                   if(genrand_real1()<ptaup[x])
                   {
                       while(time_arr_p<1000)
                       {
                       cce_frontal_ion_p[time_arr_p]+= -(gunn[x]-gunn[x0]);
                       time_arr_p++;
                       }
                       break;
                   }

                   if(genrand_real1() > pdrift[x]){x--;}  else {x++;}


                   if(timecount_p==cce_time_frontal_p[time_arr_p])
                   {
                   cce_frontal_ion_p[time_arr_p]+= -(gunn[x]-gunn[x0]);
                   time_arr_p++;
                   if(time_arr_p==1000) {break;}
                   }
timecount_p++;

              }
               counter1= -(gunn[x]-gunn[x0]);
                   counterp+=counter1;

               //random walk elettroni

               x=x0;

               while(j2==0)
                   {
                   if(x>=L)
                   {

                       while(time_arr_n<1000)
                       {
                       cce_frontal_ion_n[time_arr_n]+= (gunn[x]-gunn[x0]);
                       time_arr_n++;

                       }

                       break;
                   }

                   if(x<=0)
                   {

                       while(time_arr_n<1000)
                       {
                       cce_frontal_ion_n[time_arr_n]+= (gunn[x]-gunn[x0]);
                       time_arr_n++;
                       }

                       break;
                   }

                   if(genrand_real1()<ptaun[x])
                   {

                       while(time_arr_n<1000)
                       {
                       cce_frontal_ion_n[time_arr_n]+= (gunn[x]-gunn[x0]);
                       time_arr_n++;
                       }
                       break;
                   }

                   if(genrand_real1() > ndrift[x]){x--;}  else {x++;}


                   if(timecount_n==cce_time_frontal_n[time_arr_n])
                   {
                   cce_frontal_ion_n[time_arr_n]+= (gunn[x]-gunn[x0]);
                   time_arr_n++;
                   if(time_arr_n==1000){break;}

                   }
                    timecount_n++;

                   }
                   counter2= (gunn[x]-gunn[x0]);
                   countern+=counter2;
                   ctot+=counter1+counter2;
            }


          for(int i=0; i<1000; i++)
          {
              cce_tfrontal_aux[i]+=(cce_frontal_ion_p[i]+cce_frontal_ion_n[i])/n_eh;
              cce_tfrontal[i]=cce_tfrontal_aux[i]/(ion+1);

          }

          cce_frontal.append(ctot/n_eh);

         //NOISE
         double aux_a,aux_b,r2;
         do
         {
             aux_a = -1 + 2 * genrand_real1();
             aux_b = -1 + 2 * genrand_real1();
             // see if it is in the unit circle
             r2 = aux_a * aux_a + aux_b * aux_b;
         }
         while (r2 > 1.0 || r2 == 0);
         double noise = e_noise/2.355 * aux_b * sqrt (-2.0 * log (r2) / r2);
         double ccenoise=ctot/n_eh+noise;


         //THRESHOLD

         if(ccenoise>e_threshold)
         {

             //conteggia nuovo ione
             hit_frontal++;
             aux_mean_frontal+=ccenoise;
             cce_mean_frontal=aux_mean_frontal/hit_frontal;
            //conteggia la media
            cce_frontal_thresh.append(ccenoise);
            int cce_int=ccenoise*100;

            if(cce_int<tot_spectrum[0].size()+1)
            {
                tot_spectrum[0][cce_int/*+1*/]++;
            }
            else
            {
                tot_spectrum[0][tot_spectrum[0].size()-1]++;
            }
            if(tot_spectrum[0][cce_int/*+1*/]>maximum[0])
            {
                maximum[0]=tot_spectrum[0][cce_int+1];
            }

x_histo_mean.resize(2);
y_histo_mean.resize(2);
x_histo_mean[0]=cce_mean_frontal;
x_histo_mean[1]=cce_mean_frontal;
y_histo_mean[0]=0;
y_histo_mean[1]=maximum[0];

emit passValues();

         }


         progress++;
         ion++;
        }

      else{
          emit thread_finished();
          dataTimer.stop();
          disconnect(&dataTimer, SIGNAL(timeout()), this, SLOT(montecarlo()));
          thread_status=1;

}

    }
    }
}
}

void MainWindow::frontal()
{
}


int x1;

void MyLibraryWrapper::start_thread()
{

    if(lat_front==0)
    {
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(montecarlo()));
    }

    if(lat_front==1)
    {
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(frontal()));
    }

    dataTimer.start(0);
    thread_status=0;
}


void MyLibraryWrapper::stop_thread()
{

    dataTimer.stop();
    if(lat_front==0)
{
    disconnect(&dataTimer, SIGNAL(timeout()), this, SLOT(montecarlo()));
}

    if(lat_front==1)
    {
        disconnect(&dataTimer, SIGNAL(timeout()), this, SLOT(frontal()));
    }
    thread_status=1;
}

void MyLibraryWrapper::montecarlo()
{

    if(thread_status==0)
    {

    if(n_point>0){

  // calculate two new data points:
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
     double key = 0;
#else
  double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif


    prog_show = progress/end_sim*100;
    emit updateProgressbar(prog_show);

  static double lastPointKey = 0;
  if (key-lastPointKey >0.05) // at most add point every 10 ms
  {

      if(ion<n_ion)
        {
      if(p<n_point)
        {

          int xinit = p*steps;

          //BEAM DISPERSION
          double aux_a1,aux_b1,rstraggl;
          do
             {
             aux_a1 = -1 + 2 * genrand_real1();
             aux_b1 = -1 + 2 * genrand_real1();
             // see if it is in the unit circle
             rstraggl = aux_a1 * aux_a1 + aux_b1 * aux_b1;
             }
             while (rstraggl > 1.0 || rstraggl == 0);
             // Box-Muller transform
             x0 = xinit + (int) floor(dispersion/deltax/2.355 * aux_a1 * sqrt (-2.0 * log (rstraggl) / rstraggl));

          double ctot=0;
          double counterp=0;
          double countern=0;
          int x;

          for(int j=0; j<n_point; j++)
          {
          for(int i=0; i<1000; i++)
          {
              cce_frontal_ion_p2[j][i]=0;
              cce_frontal_ion_n2[j][i]=0;
          }
          }

          for(int h=0;h<n_eh;h++)
            {

              x1=x0;
              //inizializzazione contatori ausiliari per il ciclo interno

              double counter1=0;
              double counter2=0;

              //random walk lacune
              int timecount_p=0;
              int timecount_n=0;
              int time_arr_p=0;
              int time_arr_n=0;
              x=x1;
              int j1=0;
              int j2=0;
              while(j1==0)
                   {
                   if(x<=0){
                       while(time_arr_p<1000)
                       {
                       cce_frontal_ion_p2[p][time_arr_p]+= -(gunn[x]-gunn[x1]); //Cce_frontal_ion va ridefinito per il lateral
                       time_arr_p++;
                       }
                       break;
                   }

                   if(x>=L){
                       while(time_arr_p<1000)
                       {
                       cce_frontal_ion_p2[p][time_arr_p]+= -(gunn[x]-gunn[x1]);
                       time_arr_p++;
                       }
                       break;
                   }
                   if(genrand_real1()<ptaup[x]){
                       while(time_arr_p<1000)
                       {
                       cce_frontal_ion_p2[p][time_arr_p]+= -(gunn[x]-gunn[x1]);
                       time_arr_p++;
                       }
                       break;
                   }

                   if(genrand_real1() > pdrift[x]){x--;}  else {x++;}

                   if(timecount_p==cce_time_frontal_p2[p][time_arr_p])
                   {
                   cce_frontal_ion_p2[p][time_arr_p]+= -(gunn[x]-gunn[x1]);
                   time_arr_p++;
                   if(time_arr_p==1000) {break;}
                   }
timecount_p++;
      }

               counter1= -(gunn[x]-gunn[x1]);
                   counterp+=counter1;
                //
                   //random walk elettroni
               x=x1;
               while(j2==0)
                   {
                   if(x>=L){
                       while(time_arr_n<1000)
                       {
                       cce_frontal_ion_n2[p][time_arr_n]+= (gunn[x]-gunn[x1]); //Cce_frontal_ion va ridefinito per il lateral
                       time_arr_n++;
                       }
                       break;}
                   if(x<=0){
                       while(time_arr_n<1000)
                       {
                       cce_frontal_ion_n2[p][time_arr_n]+= (gunn[x]-gunn[x1]); //Cce_frontal_ion va ridefinito per il lateral
                       time_arr_n++;
                       }
                       break;}
                   if(genrand_real1()<ptaun[x]){
                       while(time_arr_n<1000)
                       {
                       cce_frontal_ion_n2[p][time_arr_n]+= (gunn[x]-gunn[x1]); //Cce_frontal_ion va ridefinito per il lateral
                       time_arr_n++;
                       }
                       break;}

                   if(genrand_real1() > ndrift[x]){x--;}  else {x++;}

                   if(timecount_n==cce_time_frontal_n2[p][time_arr_n])
                   {
                   cce_frontal_ion_n2[p][time_arr_n]+= (gunn[x]-gunn[x1]);
                   time_arr_n++;
                   if(time_arr_n==1000) {break;}
                   }
timecount_n++;

                   }

                   counter2= (gunn[x]-gunn[x1]);
                   countern+=counter2;
                   ctot+=counter1+counter2;

            }


          for(int i=0; i<1000; i++)
          {
              cce_tfrontal_aux2[p][i]+=(cce_frontal_ion_p2[p][i]+cce_frontal_ion_n2[p][i])/n_eh;
              cce_tfrontal2[p][i]=cce_tfrontal_aux2[p][i]/(ion+1);

          }

         xstart.append(xinit*deltax*1e6);
         cce.append(ctot/n_eh);

         //NOISE
         double aux_a,aux_b,r2;
         do
         {
             aux_a = -1 + 2 * genrand_real1();
             aux_b = -1 + 2 * genrand_real1();
             // see if it is in the unit circle
             r2 = aux_a * aux_a + aux_b * aux_b;
         }
         while (r2 > 1.0 || r2 == 0);
         double noise = e_noise/2.355 * aux_b * sqrt (-2.0 * log (r2) / r2);
         double ccenoise=ctot/n_eh+noise;
         cce_noise.append(ccenoise);

         //THRESHOLD

         if(ccenoise>e_threshold)
         {

             //conteggia nuovo ione
            hit_over_threshold[p]++;
            cce_mean[p]+=ccenoise;

            //conteggia la media
            cce_mean_plot[p]=cce_mean[p]/hit_over_threshold[p];

             tot_hit_over=0;
             tot_mean_plot=0;

            for(int q=0; q<n_point; q++)
            {
                tot_hit_over+=hit_over_threshold[q];
                tot_mean_plot+=cce_mean[q];
            }

            cce_threshold[p].append(ccenoise);
            multipos[p].append(xinit*deltax*1e6);
            int cce_int=ccenoise*100;

            if(cce_int<tot_spectrum[p+1].size()+1){tot_spectrum[p+1][cce_int/*+1*/]++;} else{tot_spectrum[p+1][tot_spectrum.size()-1]++;}
            if(tot_spectrum[p+1][cce_int/*-1*/]>maximum[p+1]) {maximum[p+1]=tot_spectrum[p+1][cce_int/*-1*/];}

            if(cce_int<tot_spectrum[0].size()+1){tot_spectrum[0][cce_int/*+1*/]++;} else{tot_spectrum[0][tot_spectrum.size()-1]++;}
            if(tot_spectrum[0][cce_int/*-1*/]>maximum[0]) {maximum[0]=tot_spectrum[0][cce_int/*-1*/];}


            //registra nuovo ione su cce_thresh, con posizione su multipos[p] che serve solo per il plot
            //sort di cce_threshold per mediana: DUPLICA CCHE_THRESHOLD per evitare di sovrascriverlo

            double a,b;



             for(int j=0; j< n_point; j++)
             {
                for(int i=0; i< cce_threshold[j].size()-1; i++)
                {



                  if(cce_threshold[j][i]>cce_threshold[j][i+1])
                  {
                    a=cce_threshold[j][i];
                    b=cce_threshold[j][i+1];
                    cce_threshold[j][i]=b;
                    cce_threshold[j][i+1]=a;
                  }


                }
              }

             //ora calcola la mediana di cce_threshold

             int l1;

             for(int j=0; j< n_point; j++)
               {

                l1 = cce_threshold[j].size();

                if(l1 == 0) cce_median[j]=0;
                else{
                   if(l1 % 2 == 1)
                   {cce_median[j]=cce_threshold[j][l1/2];}
                   else {cce_median[j]=(cce_threshold[j][l1/2-1]+cce_threshold[j][l1/2])/2;}
}

              }

         }


         progress++;
          p++;

          emit passValues();



        }
      else{p=0; ion++;}


        }
      else{
      emit thread_finished();


}

      lastPointKey = key;

    }



    }


}
}

void MainWindow::montecarlo()
{
}


void MainWindow::on_pushButton_stop_clicked()
{

    disconnect(wrapper, SIGNAL(updateProgressbar(int)),ui->progressBar, SLOT(setValue(int)));
    disconnect(wrapper, SIGNAL(thread_finished()),this,SLOT(wrapperDone()));
    wrapper->stop_thread();

    ui->pushButton_resume->setEnabled(true);
    ui->pushButton_sim->setEnabled(true);
    ui->pushButton_stop->setDisabled(true);
    ui->pushButton_simsetup->setEnabled(true);

    ui->textMessageBox->append("Simulation stopped.");

    }

void MainWindow::on_pushButton_saveplot_clicked()
{

    FileBrowser browser;
    browser.exec();
    QString dir=browser.on_buttonBox_accepted();
    dir.append("/export");
    dir.append(QDateTime::currentDateTime().toString());
    QDir directory;
    directory.mkpath(dir);

    QString path_plotmc = dir;
    path_plotmc.append("/montecarlo.jpg");
    ui->mc_plot->saveJpg(path_plotmc, 481,311,5,-1);

    QString path_tot_spectrum = dir;
    path_tot_spectrum.append("/spectrum.jpg");
    ui->histo_plot->saveJpg(path_tot_spectrum, 301,161,5,-1);

    QString path_efield_plot = dir;
    path_efield_plot.append("/efield.jpg");
    ui->efield_plot->saveJpg(path_efield_plot,301,161,5,-1);

    QString path_gunn_plot = dir;
    path_gunn_plot.append("/gunn.jpg");
    ui->gunn_plot->saveJpg(path_gunn_plot,301,161,5,-1);

    QString path_pdrift_plot = dir;
    path_pdrift_plot.append("/pdrift.jpg");
    ui->pdrift_plot->saveJpg(path_pdrift_plot,301,161,5,-1);

    QString path_ndrift_plot = dir;
    path_ndrift_plot.append("/ndrift.jpg");
    ui->ndrift_plot->saveJpg(path_ndrift_plot,301,161,5,-1);

    QString path_ioniz_plot = dir;
    path_ioniz_plot.append("/ioniz.jpg");
    ui->ioniz_plot->saveJpg(path_ioniz_plot,301,161,5,-1);

    QString path_vac_plot = dir;
    path_vac_plot.append("/vac.jpg");
    ui->vac_plot->saveJpg(path_vac_plot,301,161,5,-1);

    QString path_time_plot = dir;
    path_time_plot.append("/vac.jpg");
    ui->time_plot->saveJpg(path_time_plot,301,161,5,-1);

    QString path_cceclean = dir;
    path_cceclean.append("/cce_theor_hits.txt");
    QFile file_cce(path_cceclean);
    file_cce.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_cce(&file_cce);

    for(int k=0; k<cce.size(); k++)
    {
        out_cce << xstart[k] << "\t" <<  cce[k]  << "\n";
    }

    file_cce.close();

    QString path_readme = dir;
    path_readme.append("/simulation_contents.txt");
    QFile file_readme(path_readme);
    file_readme.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_readme(&file_readme);

    out_readme << "Transport parameters\n\n";
    out_readme << "deltax: " << deltax << " m\n";
    out_readme << "deltatn: " << deltatn << " s\n";
    out_readme << "deltatp: " << deltatp << " s\n";
    out_readme << "electron mobility: " << mun << " m2/V/s\n";
    out_readme << "hole mobility: " << mup << " m2/V/s\n";
    out_readme << "electron diffusivity: " << ndiff << " m2/s\n";
    out_readme << "hole diffusivity: " << pdiff << " m2/s\n\n\n";
    out_readme << "Simulation parameters\n\n";
    out_readme << "number of generation points: " << n_point << "\n";
    out_readme << "number of ions per point: " << n_ion << "\n";
    out_readme << "number of electron-hole paris per ion: " << n_eh << "\n";
    out_readme << "beam spot size: " << dispersion << "um\n";
    out_readme << "electronic noise: " << e_noise*100 << "%\n";
    out_readme << "electronic threshold: " << e_threshold*100 << "%\n";

    file_readme.close();

    QString path_threshold = dir;
    path_threshold.append("/cce_exp_simulation.txt");

    QFile file_thresh(path_threshold);
    file_thresh.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_thresh(&file_thresh);

    for(int k=0; k<cce_threshold.size(); k++)
    {
        for(int j=0; j<cce_threshold[k].size(); j++)
        {
            out_cce << multipos[k][j] << "\t" <<  cce_threshold[k][j]  << "\n";
        }
    }

    file_cce.close();

    QString path_spectrum = dir;
    path_spectrum.append("/spectrum.txt");
    QFile file_spectrum(path_spectrum);

    file_spectrum.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_spectrum(&file_spectrum);

    for(int k=0; k<n_point+1; k++)
    {
    for(int i=0; i<tot_spectrum.size(); i++)
    {
        out_spectrum << histo_step[i] << "\t" <<  tot_spectrum[k][i]  << "\n";
    }
    }
    file_spectrum.close();

    QString path_ccemean = dir;
    path_ccemean.append("/cce_mean.txt");
    QFile file_ccemean(path_ccemean);
    QString path_cce_median = dir;
    path_cce_median.append("/cce_median.txt");
    QFile file_ccemedian(path_cce_median);

    file_ccemean.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_ccemean(&file_ccemean);
    file_ccemedian.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_ccemedian(&file_ccemedian);


    for(int i=0; i<n_point; i++)
    {
        out_ccemean << i*steps*deltax << "\t" <<  cce_mean_plot[i]  << "\n";
        out_ccemedian << i*steps*deltax << "\t" <<  cce_median[i]  << "\n";
    }

    file_ccemean.close();
    file_ccemedian.close();

    QString path_vac = dir;
    path_vac.append("/vac.txt");
    QFile file_vac(path_vac);
    file_vac.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_vac(&file_vac);

    QString path_ioniz = dir;
    path_ioniz.append("/ioniz.txt");
    QFile file_ioniz(path_ioniz);
    file_ioniz.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_ioniz(&file_ioniz);

    for(int k=0; k<L; k++)
    {
        out_ioniz << k*deltax << "\t" <<  yioniz_inter[k]  << "\n";
        out_vac << k*deltax << "\t" <<    yioniz_inter[k]  << "\n";
    }

    file_ioniz.close();
    file_vac.close();

    QString path_taup = dir;
    path_taup.append("/taup.txt");
    QFile file_taup(path_taup);
    file_taup.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_taup(&file_taup);

    QString path_taun = dir;
    path_taun.append("/taun.txt");
    QFile file_taun(path_taun);
    file_taun.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_taun(&file_taun);

    for(int k=0; k<L; k++)
    {
        out_taup << k*deltax << "\t" <<  taup_arr[k]  << "\n";
        out_taun << k*deltax << "\t" <<  taun_arr[k]  << "\n";

    }

    file_taun.close();
    file_taup.close();

    QString path_ndrift = dir;
    path_ndrift.append("/ndrift.txt");
    QFile file_ndrift(path_ndrift);
    file_ndrift.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_ndrift(&file_ndrift);

    QString path_pdrift = dir;
    path_pdrift.append("/pdrift.txt");
    QFile file_pdrift(path_pdrift);
    file_pdrift.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_pdrift(&file_pdrift);

    for(int k=0; k<L; k++)
    {
        out_pdrift << k*deltax << "\t" <<  pdrift[k]  << "\n";
        out_ndrift << k*deltax << "\t" <<  ndrift[k]  << "\n";
    }

    file_ndrift.close();
    file_pdrift.close();

    QString path_outefield = dir;
    path_outefield.append("/efield.txt");
    QFile file_outefield(path_outefield);
    file_outefield.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_outefield(&file_outefield);

    QString path_outgunn = dir;
    path_outgunn.append("/gunn.txt");
    QFile file_outgunn(path_outgunn);
    file_outgunn.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_outgunn(&file_outgunn);

    for(int k=0; k<L; k++)
    {
        out_outefield << k*deltax << "\t" <<  efield[k]  << "\n";
        out_outgunn << k*deltax << "\t" <<  gunn[k]  << "\n";

    }

    file_outefield.close();
    file_outgunn.close();

    QString path_risetime = dir;
    path_risetime.append("/spectrum.txt");
    QFile file_risetime(path_risetime);
    file_risetime.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_risetime(&file_risetime);



    for(int k=0; k<cce_time_frontal_tot.size(); k++)
    {
        if(lat_front==1)
        {
        out_risetime << cce_time_frontal_tot[k] << "\t" <<  cce_tfrontal[k]  << "\n";
        }

        if(lat_front==0)
        {
        out_risetime << cce_time_frontal_tot[k] << "\t" <<  cce_tfrontal2[gnearest][k]  << "\n";
        }

    }

    file_risetime.close();
}


void MainWindow::electrostatics(){


    gunn.resize(0);
    pos_gunn.resize(0);

    QFile file_gunn(path_gunn);
    QTextStream its_gunn(&file_gunn);
    QString line_gunn;

    if (!file_gunn.open(QIODevice::ReadOnly))
    {
     qDebug() << "file not found" << endl;
    }


int i=0;
double space_step_gunn=0;
int array_step_count=0;

    while( !its_gunn.atEnd() )
    {

        line_gunn = its_gunn.readLine();
        line_gunn.replace(",",".");
        line_gunn.replace("\t", "    ");
        QStringList list_gunn=line_gunn.split(" ",QString::SkipEmptyParts);

        if(list_gunn.size()>1)
        {
        bool validate1, validate2;

        double converted1 = list_gunn.first().toDouble(&validate1);
        double converted2 = list_gunn.last().toDouble(&validate2);

        if (validate1 == true && validate2 ==true && list_gunn.size()>1)
        {
        pos_gunn.push_back(list_gunn.first().toDouble()); // m
        gunn.push_back(list_gunn.last().toDouble()); //m

        if(i==1){space_step_gunn=(pos_gunn[i]-pos_gunn[i-1]);}
        if(i>0)
        {if(fabs(pos_gunn[i]-pos_gunn[i-1] -space_step_gunn) >1e-9 ) {array_step_count++;}}

        i++;

        }
        }
    }

      int control_deltax_gunn=-1;
      int control_deltax_efield=-1;
      int control_deltax=-1;

    if (array_step_count==0)
      {
        deltax=space_step_gunn;
        control_deltax_gunn=0;
      }



    file_gunn.close();


    efield.resize(0);
    pos_efield.resize(0);

    QFile file_efield(path_efield);
    QTextStream its_efield(&file_efield);
    QString line_efield;

    if (!file_efield.open(QIODevice::ReadOnly))
    {
     qDebug() << "file not found" << endl;
    }

    i=0;
    double space_step_efield=0;
    array_step_count=0;

    while( !its_efield.atEnd() )
    {

        line_efield = its_efield.readLine();
        line_efield.replace(",",".");
        line_efield.replace("\t", "    ");
        QStringList list_efield=line_efield.split(" ",QString::SkipEmptyParts);


        if(list_efield.size()>1)
        {


        bool validate1, validate2;

        double converted1 = list_efield.first().toDouble(&validate1);
        double converted2 = list_efield.last().toDouble(&validate2);

        if (validate1 == true && validate2 ==true && list_efield.size()>1)
        {


        pos_efield.push_back(list_efield.first().toDouble()); //espresso in m
        efield.push_back(list_efield.last().toDouble()*100); //convertito in V/m da input in V/cm

        if(i==1){space_step_efield=pos_efield[i]-pos_efield[i-1];}
        if(i>0)
        {if(fabs(pos_efield[i]-pos_efield[i-1] -space_step_efield) >1e-9 )  {array_step_count++;}}


        i++;
        }
        }

    }


    if (array_step_count==0)
      {
        control_deltax_efield=0;
      }

    if(space_step_gunn==space_step_efield)
      {control_deltax=0;}

    file_efield.close();


    int control_array=-1;
    int control_tau=-1;
    int control_mob=-1;
    int control_vsat=-1;
    int control_temp=-1;

    int control=-1;


    //CHECK SU DELTAX!

    if(efield.size()==gunn.size() && gunn.size() > 0) {
        control_array=0;
        L=gunn.size();
        ui->textMessageBox->append("Input files imported successfully. Device lenght is " + QString::number(L) + "space steps.");
      }

    if(taun > 0 && taup > 0)
      {
        control_tau=0;
        ui->textMessageBox->append("Carriers lifetime defined successfully. taun=" + QString::number(taun) +" s, taup=" + QString::number(taup) + " s.");
      }

    taup_arr.resize(L);
    taun_arr.resize(L);
    ptaup.resize(L);
    ptaun.resize(L);


    if(mup > 0 && mun > 0)
      {
        control_mob=0;
        ui->textMessageBox->append("Carriers mobility defined successfully. Dn=" + QString::number(mun) + " m2/V/s, Dp=" +QString::number(mup)+ " m2/V/s.");
      }

    if(vsatn > 0 && vsatp > 0)
      {
        control_vsat=0;
        ui->textMessageBox->append("Saturation velocities  defined successfully. vsatn=" + QString::number(vsatn*100) + " cm/s, vsatp=" +QString::number(vsatp/100)+ " cm/s.");
      }

    if(temp > 0)
      {
        control_temp=0;
        ui->textMessageBox->append("Temperature defined successfully. T=" + QString::number(temp) + " K.");
      }


    if(control_deltax == 0)
      {
        ui->textMessageBox->append("Space step size evaluated successfully. deltax=" + QString::number(deltax) + " m.");
      }


    // QUI INSERISCI CONTROLLO LOGICA

    if(control_array==0 && control_tau==0 && control_vsat==0 && control_temp==0 && control_deltax ==0 && control_deltax_gunn ==0 && control_deltax_efield ==0
) {

        control=0;
        ui->textMessageBox->append("Parameters initialized successfully!");
        ui->pushButton_simsetup->setEnabled(true);
      }
    else{
        if(control_array<0) {ui->textMessageBox->append("ERROR: Array lenght is zero, or arrays don't have the same lenght.");}
        if(control_tau<0){ui->textMessageBox->append("ERROR: Carrier lifetimes are undefined.");}
        if(control_vsat<0){ui->textMessageBox->append("ERROR: Saturation velocities are undefined.");}
        if(control_temp<0){ui->textMessageBox->append("ERROR: Temperature is undefined.");}
        if(control_deltax_gunn<0){ui->textMessageBox->append("ERROR: Irregular space size in weighting potential input file.");}
        if(control_deltax_efield<0){ui->textMessageBox->append("ERROR: Irregular space size in electric field input file.");}
        if(control_deltax<0){ui->textMessageBox->append("ERROR: Space step size for electric field and weighting potential don't match.");}
        ui->textMessageBox->append("Simulation cannot be started.");
      }
    //manca lunghezza in unità di misura standard


    double k_q=8.62e-5; // in K^(-1), se kb/q=8.62e-5 eV/K
    ndiff=k_q*mun*temp; // V/K*K*m2/v/s
    pdiff=k_q*mup*temp;

    ui->textMessageBox->append("Diffusivity evaluated successfully. Dn=" + QString::number(ndiff) + " m2/s, Dp=" + QString::number(pdiff) + " m2/s.");

    pdrift.resize(gunn.size());
    ndrift.resize(gunn.size());

    deltatn=deltax*deltax/2/ndiff;  // m2/m2*s massima consenstita! (warning)
    deltatp=deltax*deltax/2/pdiff;

    ui->textMessageBox->append("Time step sizes evaluated successfully. deltatn=" + QString::number(deltatn) + " s, deltatp=" + QString::number(deltatp) + " s.");



    Tn=floor(3*taun/deltatn);    //holding time
    Tp=floor(3*taup/deltatp);

    ui->textMessageBox->append("Maximum holding time evaluated successfully.");


mn.resize(L);
mp.resize(L);

    for(int i=0; i<gunn.size(); i++){

       mn[i]=mun/(sqrt(1+mun*mun*efield[i]*efield[i]/vsatn/vsatn));
       mp[i]=mup/(sqrt(1+mup*mup*efield[i]*efield[i]/vsatp/vsatp));

        //includes formula for saturation velocity //DA RIVEDERE!!!

        pdrift[i]=0.5+efield[i]*mp[i]/2*deltatp/deltax;
        ndrift[i]=0.5-efield[i]*mn[i]/2*deltatn/deltax;
      }

setupGunn(ui->gunn_plot);
ui->gunn_plot->replot();

setupEfield(ui->efield_plot);
ui->efield_plot->replot();

    setupNdrift(ui->ndrift_plot);
    ui->ndrift_plot->replot();

    setupPdrift(ui->pdrift_plot);
    ui->pdrift_plot->replot();


ui->pushButton_sim->setDisabled(true);

ui->time_spinBox->setDisabled(true);

text_transport();


}






void MainWindow::on_checkBox_clicked()
{

   if(ui->checkBox->isChecked()==false) {set_legend=0; ui->mc_plot->legend->setVisible(false);}
     if(ui->checkBox->isChecked()==true) {set_legend=1; ui->mc_plot->legend->setVisible(true);}
    ui->mc_plot->replot();
}



void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    slide_generation_point=value;
    int nearest;
  if(value!=-1)
    {
     nearest=slide_generation_point/steps;
    ui->horizontalSlider->setValue(nearest*(steps));
}

else {nearest=-100;}
    gnearest=nearest;

    gpoint=(double) (nearest)*(steps*(n_point)-1)*deltax/n_point*1e6;
    if(this->ui->horizontalSlider->value()==-1){ this->ui->textBrowser->setText("All data");}
    else{this->ui->textBrowser->setText("x=" + QString::number(gpoint)+" um");}
    xgpoint[0]=gpoint;
    xgpoint[1]=gpoint;

    ui->mc_plot->graph(3)->clearData();
    ui->mc_plot->graph(5)->clearData();
    ui->mc_plot->graph(5)->setData(xgpoint,ygpoint);
    ui->mc_plot->replot();
    ui->time_plot->replot();
    ui->histo_plot->clearItems();
    ui->histo_plot->graph(0)->clearData();
    ui->histo_plot->graph(1)->clearData();

    QVector<QVector<double> > x_histo_mean_lateral, y_histo_mean_lateral;
    x_histo_mean_lateral.resize(n_point+1);
    y_histo_mean_lateral.resize(n_point+1);

    x_histo_mean_lateral[0].resize(2);
    y_histo_mean_lateral[0].resize(2);
    x_histo_mean_lateral[0][0]=tot_mean_plot/tot_hit_over;
    x_histo_mean_lateral[0][1]=tot_mean_plot/tot_hit_over;
    y_histo_mean_lateral[0][0]=0;
    y_histo_mean_lateral[0][1]=maximum[0];

    for(int q=1; q<n_point; q++)
    {
        x_histo_mean_lateral[q].resize(2);
        y_histo_mean_lateral[q].resize(2);
        x_histo_mean_lateral[q][0]=cce_mean_plot[q-1];
        x_histo_mean_lateral[q][1]=cce_mean_plot[q-1];
        y_histo_mean_lateral[q][0]=0;
        y_histo_mean_lateral[q][1]=maximum[q];

    }


    if(gnearest<0){
    ui->histo_plot->graph(0)->setData(histo_step, tot_spectrum[0]);
    ui->histo_plot->graph(1)->setData(x_histo_mean_lateral[0], y_histo_mean_lateral[0]);
    ui->histo_plot->yAxis->setRange(-0.0,maximum[0]*1.2);

    QCPItemText *textLabel = new QCPItemText(ui->histo_plot);
    ui->histo_plot->addItem(textLabel);
    textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel->position->setCoords(0.5,0);
    textLabel->setText("Mean: " + QString::number(tot_mean_plot/tot_hit_over));
    textLabel->setPen(QPen(Qt::black));
    textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
    }
    else
    {
    ui->histo_plot->graph(0)->setData(histo_step, tot_spectrum[gnearest+1]);
    ui->histo_plot->graph(1)->setData(x_histo_mean_lateral[gnearest+1], y_histo_mean_lateral[gnearest+1]);
    ui->histo_plot->yAxis->setRange(-0.0,maximum[gnearest+1]*1.2);

    ui->histo_plot->yAxis->setRange(-0.0,maximum[gnearest+1]+10);
    QCPItemText *textLabel = new QCPItemText(ui->histo_plot);
    ui->histo_plot->addItem(textLabel);
    textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel->position->setCoords(0.5,0);
    textLabel->setText("Mean: " + QString::number(cce_mean_plot[gnearest]));
    textLabel->setPen(QPen(Qt::black));
    textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
    }



        ui->histo_plot->replot();


        ui->time_plot->graph(0)->clearData();
        if(gnearest>=0)
        {
        ui->time_plot->graph(0)->setData(cce_time_frontal_tot,cce_tfrontal2[gnearest]);
        ui->time_plot->xAxis->setRange(0,valtscale);
        ui->time_plot->yAxis->setRange(0,1.1);
}
ui->time_plot->replot();


}

void MainWindow::on_time_spinBox_editingFinished()
{
valtscale=ui->time_spinBox->value();
ui->time_plot->xAxis->setRange(0,valtscale);
ui->time_plot->replot();

}

void MainWindow::on_time_spinBox_valueChanged(/*int arg1*/)
{
    valtscale=ui->time_spinBox->value();
    ui->time_plot->xAxis->setRange(0,valtscale);
    ui->time_plot->replot();
}


void MainWindow::text_transport(){

    ui->textBrowser_transport->clear();
    ui->textBrowser_transport->setText("deltax: " + QString::number(deltax) + " m");
    ui->textBrowser_transport->append("deltatn: " + QString::number(deltatn)+ " s");
    ui->textBrowser_transport->append("deltatp: " + QString::number(deltatp) + " s");
    ui->textBrowser_transport->append("mun: " + QString::number(mup)+ " m2/V/s");
    ui->textBrowser_transport->append("mun: " + QString::number(mun)+ " m2/V/s");
    ui->textBrowser_transport->append("Dn: " + QString::number(ndiff)+ " m2/s");
    ui->textBrowser_transport->append("Dp: " + QString::number(pdiff)+ " m2/s");
}

void MainWindow::text_simulation(){

    ui->textBrowser_simulation->clear();
    if(lat_front==1)
    {
    ui->textBrowser_simulation->setText("Frontal IBIC simulation");
    }
    if(lat_front==0)
    {
    ui->textBrowser_simulation->setText("Lateral IBIC simulation");
    }

    ui->textBrowser_simulation->append("points: " + QString::number(n_point));
    ui->textBrowser_simulation->append("ions per point: " + QString::number(n_ion));
    ui->textBrowser_simulation->append("pairs per ion: " + QString::number(n_eh) );
    ui->textBrowser_simulation->append("beam size: " + QString::number(dispersion)+ " um");
    ui->textBrowser_simulation->append("noise: " + QString::number(e_noise*100)+ " %");
    ui->textBrowser_simulation->append("threshold: " + QString::number(e_threshold*100)+ " %");
}

void MainWindow::text_radhard(){

    ui->textBrowser_irradiation->clear();
    if(rad_yesno==0)
    {
        ui->textBrowser_irradiation->setText("No irradiation considered.");
    }

    if(rad_yesno==1)
    {
    ui->textBrowser_irradiation->setText("ion fluence: " + QString::number(fluence*1e-4)+ " 1/cm2" );
    ui->textBrowser_irradiation->append("n trap/vacancy: " + QString::number(radhard_n));
    ui->textBrowser_irradiation->append("p trap/vacancy: " + QString::number(radhard_p));
    ui->textBrowser_irradiation->append("n therm velocity: " + QString::number(ntherm)+ " m/s");
    ui->textBrowser_irradiation->append("p therm velocity: " + QString::number(ptherm)+ " m/s");
    ui->textBrowser_irradiation->append("n cross section: " + QString::number(nsigma)+ " m2");
    ui->textBrowser_irradiation->append("p cross section: " + QString::number(psigma)+ " m2");
}

}


void MainWindow::on_pushButton_radsetup_clicked()
{
    Radhard_dialog dialog;

    //    dialog.path_vac=path_vac; COME LO FAI?

   // copia ancora, per lanciare all'avvio, i vettori di vacanze così da plottarli al volo.


    dialog.exec();

    if(dialog.control_window==0)
    {

    fluence=dialog.fluence*1e4; //check udm
    radhard_p=dialog.radhard_p;
    radhard_n=dialog.radhard_n;
    nsigma=dialog.sigman*1e-4;
    psigma=dialog.sigmap*1e-4;
    ptherm=dialog.ptherm/100;
    ntherm=dialog.ntherm/100;
    rad_yesno=dialog.radok;
    control_vac=dialog.control_window;
    }

    if(rad_yesno==1)
    {

    path_vac=dialog.path_vac;


    x_vac_import.resize(100);
    x_vac.resize(100);
    y_vac.resize(100);

    QFile file_vac(path_vac);
    QTextStream its_vac(&file_vac);
    QString line_vac;

    if (!file_vac.open(QIODevice::ReadOnly))
        {
        ui->textMessageBox->append("Vacancy file not found");
        control_vac=1;
    }

    if(control_vac==0)
    {

        int l=0;

        while(!its_vac.atEnd())
        {


        line_vac = its_vac.readLine();
        line_vac.replace(",",".");
        line_vac.replace("\t", "   ");
        QStringList list_vac=line_vac.split(" ",QString::SkipEmptyParts);

        if(list_vac.size()>0)
        {
        bool validate1, validate2;

        double converted1 = list_vac.first().toDouble(&validate1);
        double converted2 = list_vac.last().toDouble(&validate2);

        if (validate1 == true && validate2 ==true && list_vac.size()>1)
        {
        x_vac[l]=list_vac.first().toDouble()*1e-4-dead_layer;  //inserito già in micrometri
        y_vac[l]=list_vac.last().toDouble()*1e10;            //udm: vac/m
        l++;
        }

        }
        }


    file_vac.close();
    }
    }

    else
    {
        for(int l=0; l<x_vac_import.size(); l++)
        {

            x_vac_import[l]=0;  //inserito già in micrometri
            x_vac[l]=x_ioniz_import[l]/*-dead_layer*/;
            y_vac[l]=0;
            control_vac=0;

        }
    }

    yvac_inter.resize(L);

    for(int i=0; i<L; i++)
    {
        int j=0;

        qDebug() << i*deltax*1e6 << "   " << x_vac_import[j] << "   " << x_vac[j];
        while(i*deltax*1e6 > x_vac[j]/*x_vac_import[j]*/) {j++;}

        if(j==0)
        {
            yvac_inter[i]=y_vac[j];
        }
        else if((i*deltax*1e6-x_vac_import[j-1])<(x_vac_import[j]-i*deltax*1e6))
        {
            yvac_inter[i]=y_vac[j-1];
        }
        else
        {
            yvac_inter[i]=y_vac[j];
        }

    }

    setupVac(ui->vac_plot);
    ui->vac_plot->replot();



    for(int k=0; k<L; k++)
    {
        taup_arr[k]=taup/(1+radhard_p*yvac_inter[k]*psigma*ptherm*fluence*taup);
        ptaup[k]=deltatp/taup_arr[k];
        taun_arr[k]=taun/(1+radhard_n*yvac_inter[k]*nsigma*ntherm*fluence*taun);
        ptaun[k]=deltatn/taun_arr[k];

    }


    setupTau(ui->tau_plot);
    ui->tau_plot->replot();

    text_radhard();
}

void MainWindow::on_pushButton_devsetup_clicked()
{
    FilesDialog dialog;
    dialog.exec();


    if(dialog.checkdialog==1)
    {
    path_efield=dialog.path_efield;
    path_gunn=dialog.path_gunn;

    vsatp=dialog.vsatp/100;         //in m/s
    vsatn=dialog.vsatn/100;
    temp=dialog.temp;

    mun=dialog.mun*1e-4;
    mup=dialog.mup*1e-4;
    taun=dialog.taun;
    taup=dialog.taup;

    electrostatics();

    for(int k=0; k<L; k++)
    {
        taup_arr[k]=taup/(1+radhard_p*yvac_inter[k]*psigma*ptherm*fluence*taup);
        ptaup[k]=deltatp/taup_arr[k];
        taun_arr[k]=taun/(1+radhard_n*yvac_inter[k]*nsigma*ntherm*fluence*taun);
        ptaun[k]=deltatn/taun_arr[k];

    }


    setupTau(ui->tau_plot);
    ui->tau_plot->replot();

}
    files_dialog_check_ok=1;
    ui->pushButton_simsetup->setEnabled(true);
}


void MainWindow::on_pushButton_about_clicked()
{
    LicenseDialog dialog;
    dialog.exec();
}
