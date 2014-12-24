#include "mainwindow.h"
#include "filesdialog.h"
#include "ui_filesdialog.h"

FilesDialog::FilesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilesDialog)
{


    ui->setupUi(this);
    MainWindow *obj = (MainWindow *) qApp->activeWindow();

    if(obj->files_dialog_check_ok==1)
    {
        temp=obj->temp;
        mun=obj->mun*1e4;
        mup=obj->mup*1e4;
        vsatp=obj->vsatp*100;
        vsatn=obj->vsatn*100;
        taup=obj->taup;
        taun=obj->taun;
        path_gunn=obj->path_gunn;
        path_efield=obj->path_efield;

        ui->lineEdit_Efield->setText(path_efield);
        ui->lineEdit_Gunn->setText(path_gunn);
        ui->lineEdit_mun->setText(QString::number(mun));
        ui->lineEdit_mup->setText(QString::number(mup));
        ui->lineEdit_taun->setText(QString::number(taun));
        ui->lineEdit_taup->setText(QString::number(taup));
        ui->lineEdit_temp->setText(QString::number(temp));
        ui->lineEdit_vsatn->setText(QString::number(vsatn));
        ui->lineEdit_vsatp->setText(QString::number(vsatp));

        plotEfield();
        plotGunn();


        ui->pushButton_OK->setEnabled(true);

    }

else{
        checkdialog=0;
        ui->pushButton_OK->setEnabled(false);
        L=0;

    }



    connect(ui->lineEdit_mup,SIGNAL(editingFinished()),this,SLOT(update_values()));
    connect(ui->lineEdit_mun,SIGNAL(editingFinished()),this,SLOT(update_values()));
    connect(ui->lineEdit_taup,SIGNAL(editingFinished()),this,SLOT(update_values()));
    connect(ui->lineEdit_taun,SIGNAL(editingFinished()),this,SLOT(update_values()));
    connect(ui->lineEdit_temp,SIGNAL(editingFinished()),this,SLOT(update_values()));
    connect(ui->lineEdit_vsatp,SIGNAL(editingFinished()),this,SLOT(update_values()));
    connect(ui->lineEdit_vsatn,SIGNAL(editingFinished()),this,SLOT(update_values()));




}

FilesDialog::~FilesDialog()
{
    delete ui;
}

void FilesDialog::on_pushButton_efield_clicked()
{
    FileBrowser browser;
    browser.exec();

    path_efield=browser.on_buttonBox_accepted();
    ui->lineEdit_Efield->setText(path_efield);
    plotEfield();
    text_efield();
}



    void FilesDialog::plotEfield()
    {
    efield.resize(0);
    pos_efield.resize(0);

    QFile file(path_efield);
    QTextStream its(&file);
    QString line;

    if (!file.open(QIODevice::ReadOnly))
    {
     qDebug() << "file not found" << endl;
    }


  int i=0;
  space_step_efield=0;
  int array_step_count=0;
  QStringList list;

    while( !its.atEnd() )
    {

        line = its.readLine();
        line.replace(",",".");
        line.replace("\t", "    ");

        list=line.split(" ",QString::SkipEmptyParts);

        if(list.size()>0)
        {
        bool validate1, validate2;

        double converted1 = list.first().toDouble(&validate1);
        double converted2 = list.last().toDouble(&validate2);

        if (validate1 == true && validate2 ==true && list.size()>1)
        {
        pos_efield.push_back(list.first().toDouble()); // udm di input: metri
        efield.push_back(list.last().toDouble());  //udm di input: V/cm

        if(i==1){space_step_efield=(pos_efield[i]-pos_efield[i-1]);}
        if(i>0)
        {if(fabs(pos_efield[i]-pos_efield[i-1] -space_step_efield) >1e-15 ) {array_step_count++;}
        }
        i++;

        }
        }


    }

      int control_deltax_efield=-1;
      int control_deltax=-1;

    if (array_step_count==0)
      {
        deltax=space_step_efield;
        control_deltax_efield=0;
      }

    L=efield.size();

    file.close();


    setupEfield(ui->efield_plot);
    ui->efield_plot->replot();

    update_values();



}


void FilesDialog::on_pushButton_gunn_clicked()
{
  FileBrowser browser;
  browser.exec();

  path_gunn=browser.on_buttonBox_accepted();
  ui->lineEdit_Gunn->setText(path_gunn);
  plotGunn();
    text_gunn();
}

 void FilesDialog::plotGunn()
{
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
space_step_gunn=0;
int array_step_count=0;
QStringList list_gunn;

while( !its_gunn.atEnd() ) {

      line_gunn = its_gunn.readLine();
      line_gunn.replace(",",".");
      line_gunn.replace("\t", "    ");
      list_gunn=line_gunn.split(" ",QString::SkipEmptyParts);

      if(list_gunn.size()>0)
      {
      bool validate1, validate2;

      double converted1 = list_gunn.first().toDouble(&validate1);
      double converted2 = list_gunn.last().toDouble(&validate2);

      if (validate1 == true && validate2 ==true && list_gunn.size()>1)
      {
      pos_gunn.push_back(list_gunn[0].toDouble()); //input in m
      gunn.push_back(list_gunn[1].toDouble()); //input in m



      if(i==1){space_step_gunn=(pos_gunn[i]-pos_gunn[i-1]);}
      if(i>0)
      {
          if(fabs(pos_gunn[i]-pos_gunn[i-1] -space_step_gunn) >1e-15 )
          {
              array_step_count++;
          }
      }

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

  if(L==0)
  {
      L=gunn.size();
  }


  file_gunn.close();


  setupGunn(ui->gunn_plot);
  ui->gunn_plot->replot();

  update_values();

}



void FilesDialog::setupEfield(QCustomPlot *potPlot)
{

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
    x[i] = i*deltax*1e6;
    y0[i] = efield[i]; // V/cm
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
//  potPlot->yAxis->setRange(-0.0,1.0);
  potPlot->xAxis->setRange(-0.0,L*deltax*1e6);
  potPlot->yAxis->setRange(emin*1.2,emax*1.2);
  potPlot->xAxis->setLabel("Position (um)");
  potPlot->yAxis->setLabel("Electric field (V/cm)");
  potPlot->yAxis->setAutoTickCount(5);
  potPlot->xAxis->setAutoTickCount(5);

}



    void FilesDialog::setupGunn(QCustomPlot *potPlot)
    {

        //potPlot->setTitle(title_plot);
        potPlot->addGraph(); // blue line
        potPlot->graph(0)->setPen(QPen(Qt::green));
        potPlot->graph(0)->setBrush(QBrush(QColor(0, 250, 0)));
        potPlot->graph(0)->setAntialiasedFill(false);
        potPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
        // generate some points of data (y0 for first, y1 for second graph):
      QVector<double> x(gunn.size()), y0(gunn.size());
      for (int i=0; i<gunn.size(); ++i)
      {
        x[i] = i*deltax*1e6;
        y0[i] = fabs(gunn[i]);

      }

      // configure right and top axis to show ticks but no labels (could've also just called customPlot->setupFullAxesBox):

      potPlot->xAxis->setVisible(true);
      potPlot->xAxis2->setVisible(false);
      potPlot->xAxis2->setTickLabels(true);
      potPlot->yAxis2->setVisible(true);
      potPlot->yAxis2->setTickLabels(false);
      //potPlot->yAxis->setTickStep(10);
      potPlot->graph(0)->setData(x, y0);
      // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
      potPlot->graph(0)->rescaleAxes(true);
      potPlot->xAxis->setLabel("Position (um)");
      potPlot->yAxis->setLabel("Weighting potential");
      potPlot->yAxis->setRange(-0.0,1.1);
      potPlot->yAxis->setAutoTickCount(5);
      potPlot->xAxis->setRange(-0.0,L*deltax*1e6);
      potPlot->xAxis->setAutoTickCount(5);


}




void FilesDialog::on_pushButton_OK_clicked()
{
    //return ui->lineEdit->text();
        path_efield = ui ->lineEdit_Efield->text();
        path_gunn = ui->lineEdit_Gunn->text();
        temp = ui->lineEdit_temp->text().toDouble();
        mun = ui->lineEdit_mun-> text().toDouble();
        mup = ui->lineEdit_mup-> text().toDouble();
        vsatp = ui->lineEdit_vsatp-> text().toDouble();
        vsatn = ui->lineEdit_vsatn-> text().toDouble();
        taup = ui->lineEdit_taup-> text().toDouble();
        taun = ui->lineEdit_taun-> text().toDouble();
  //  qDebug()<<srim;
    checkdialog=1;



}

void FilesDialog::on_pushButton_cancel_clicked()
{
}






void FilesDialog::update_values()
{

    path_efield = ui ->lineEdit_Efield->text();
    path_gunn = ui->lineEdit_Gunn->text();
    temp = ui->lineEdit_temp->text().toDouble();
    mun = ui->lineEdit_mun-> text().toDouble();
    mup = ui->lineEdit_mup-> text().toDouble();
    vsatp = ui->lineEdit_vsatp-> text().toDouble();
    vsatn = ui->lineEdit_vsatn-> text().toDouble();
    taup = ui->lineEdit_taup-> text().toDouble();
    taun = ui->lineEdit_taun-> text().toDouble();

    if(gunn.size()==efield.size() && L>0 && mup>0 && mun>0 && vsatp>0 && vsatn>0 && taup>0 && taun>0 && temp >=0 )    //DA SPOSTARE SUL BOTTONE OK, o come controllo da qualche parte
    {
    ui->pushButton_OK->setEnabled(true);
    }


}



void FilesDialog::text_efield(){

    ui->textBrowser_efield->clear();
    ui->textBrowser_efield->setText("length:" + QString::number(efield.size()) + " steps" + "\t" + "deltax: " + QString::number(space_step_efield) + " m");
   // ui->textBrowser_efield->append();
}


void FilesDialog::text_gunn(){

    ui->textBrowser_gunn->clear();
    ui->textBrowser_gunn->setText("length:" + QString::number(gunn.size()) + " steps" + "\t" + "deltax: " + QString::number(space_step_gunn) + " m");
 //   ui->textBrowser_gunn->append("deltax: " + QString::number(space_step_gunn) + " m");
}
