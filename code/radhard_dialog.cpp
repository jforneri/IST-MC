#include "radhard_dialog.h"
#include "ui_radhard_dialog.h"

Radhard_dialog::Radhard_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Radhard_dialog)
{
    ui->setupUi(this);

    MainWindow *obj = (MainWindow *) qApp->activeWindow();

    radok=obj->rad_yesno;

    dead_layer=obj->dead_layer;

    if(radok==0)
    {
        ui->lineEdit_fluence->setEnabled(false);
        ui->lineEdit_sigma_n->setEnabled(false);
        ui->lineEdit_sigma_p->setEnabled(false);
        ui->lineEdit_vac->setEnabled(false);
        ui->lineEdit_vth_n->setEnabled(false);
        ui->lineEdit_vth_p->setEnabled(false);
        ui->pushButton_vacBrowse->setEnabled(false);
        ui->doubleSpinBox_n->setEnabled(false);
        ui->doubleSpinBox_p->setEnabled(false);
        ui->radioButton_no->setChecked(true);
        ui->radioButton_yes->setChecked(false);
        path_vac=obj->path_vac;
        ui->lineEdit_vac->setText(path_vac);
        setup_vac_plot_first();
        ui->vac_plot->setVisible(false);
    }
    else if(radok==1)
    {

        ui->lineEdit_fluence->setEnabled(true);
        ui->lineEdit_sigma_n->setEnabled(true);
        ui->lineEdit_sigma_p->setEnabled(true);
        ui->lineEdit_vac->setEnabled(true);
        ui->lineEdit_vth_n->setEnabled(true);
        ui->lineEdit_vth_p->setEnabled(true);
        ui->pushButton_vacBrowse->setEnabled(true);
        ui->doubleSpinBox_n->setEnabled(true);
        ui->doubleSpinBox_p->setEnabled(true);
        ui->radioButton_no->setChecked(false);
        ui->radioButton_yes->setChecked(true);

        path_vac=obj->path_vac;
        ui->lineEdit_vac->setText(path_vac);
        setup_vac_plot_first();
    }


    fluence=obj->fluence*1e-4;       //check da apposita commandline
    sigmap=obj->psigma*1e4;     //qui in cm-2, in mainwindow è in m-2
    sigman=obj->nsigma*1e4;
    ptherm=obj->ptherm*100;     //qui in cm/s, in mainwindow è in m/s
    ntherm=obj->ntherm*100;
    radhard_p=obj->radhard_p;
    radhard_n=obj->radhard_n;



    //QUI SOPRA DEVI DEFINIRE I VALORI COME TI ARRIVANO DA MAINWINDOW!
    ui->lineEdit_fluence->setText(QString::number(fluence));
    ui->doubleSpinBox_n->setValue(radhard_n);
    ui->doubleSpinBox_p->setValue(radhard_p);
    ui->lineEdit_sigma_n->setText(QString::number(sigman));
    ui->lineEdit_sigma_p->setText(QString::number(sigmap));
    ui->lineEdit_vth_n->setText(QString::number(ntherm));
    ui->lineEdit_vth_p->setText(QString::number(ptherm));
    // ui->lineEdit_ionpath->set       IL PATH E' LA COSA PIU' FASTIDIOSA




    connect(ui->lineEdit_fluence,SIGNAL(editingFinished()),this,SLOT(update_values()));
    connect(ui->lineEdit_sigma_n,SIGNAL(editingFinished()),this,SLOT(update_values()));
    connect(ui->lineEdit_sigma_p,SIGNAL(editingFinished()),this,SLOT(update_values()));
    connect(ui->lineEdit_vth_n,SIGNAL(editingFinished()),this,SLOT(update_values()));
    connect(ui->lineEdit_vth_p,SIGNAL(editingFinished()),this,SLOT(update_values()));
    connect(ui->doubleSpinBox_n,SIGNAL(valueChanged(double)),this,SLOT(update_values()));
    connect(ui->doubleSpinBox_p,SIGNAL(valueChanged(double)),this,SLOT(update_values()));


}

Radhard_dialog::~Radhard_dialog()
{
    delete ui;
}


//sigma è da moltipilcare per 1e-4: nel programma ho tutto in metri

void Radhard_dialog::on_pushButton_vacBrowse_clicked()
{
    FileBrowser browser;
    browser.exec();
    path_vac=browser.on_buttonBox_accepted();
    ui->lineEdit_vac->setText(path_vac);


x_vac.resize(100);
y_vac.resize(100);

control_vac=0;

QFile file_vac(path_vac);
QTextStream its_vac(&file_vac);
QString line_vac;
if (!file_vac.open(QIODevice::ReadOnly))
    {
    //ui->textMessageBox->append("Vacancy file not found");
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

        if(list_vac.first().toDouble()*1e-4-dead_layer>=0)
        {
    x_vac[l]=list_vac.first().toDouble()*1e-4-dead_layer;  //inserito già in micrometri
   // x_vac[l]=x_vac_import[l]-dead_layer;
    y_vac[l]=list_vac.last().toDouble()*1e10;            //udm: vac/m
//QUI DEAD LAYER TEMPORANEAMENTE SOPPRESSO
    l++;

}
    }
    }
}
file_vac.close();



setupVac(ui->vac_plot);
ui->vac_plot->replot();
}

}

void Radhard_dialog::setupVac(QCustomPlot *potPlot)
{
    potPlot->addGraph(); // blue line
    potPlot->graph(0)->setPen(QPen(Qt::green));
    potPlot->graph(0)->setBrush(QBrush(QColor(0, 250, 250)));
    potPlot->graph(0)->setAntialiasedFill(false);
    potPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph

    // generate some points of data (y0 for first, y1 for second graph):
QVector<double> x_vac_um, y_vac_um;
x_vac_um.resize(x_vac.size());
y_vac_um.resize(y_vac.size());

for(int i=0; i<x_vac.size(); i++)
{
    x_vac_um[i]=x_vac[i]; //in unità di micrometro
    y_vac_um[i]=y_vac[i]/100; //in unità vac/cm
}


  // configure right and top axis to show ticks but no labels (could've also just called customPlot->setupFullAxesBox):

  potPlot->xAxis->setVisible(true);
  potPlot->xAxis2->setVisible(false);
  potPlot->xAxis2->setTickLabels(true);
  potPlot->yAxis2->setVisible(true);
  potPlot->yAxis2->setTickLabels(false);
 // potPlot->yAxis->setTickStep(10);
 potPlot->graph(0)->setData(x_vac_um, y_vac_um);
  // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
  potPlot->xAxis->setLabel("Position (um)");
  potPlot->yAxis->setLabel("Vacancy dens. (1/cm)");
  potPlot->graph(0)->rescaleAxes(true);
  potPlot->xAxis->setRange(0.0,x_vac[x_vac.size()-1]);

  double max_vac=0;

  for(int i=0; i<y_vac_um.size(); i++)
  {

      if(max_vac<y_vac_um[i]) max_vac=y_vac_um[i];

  }

 potPlot->yAxis->setRange(0.0,max_vac);

}

void Radhard_dialog::update_values()
{



    fluence = ui->lineEdit_fluence->text().toDouble();
    sigmap = ui->lineEdit_sigma_p-> text().toDouble();
    sigman = ui->lineEdit_sigma_n-> text().toDouble();
    ptherm = ui->lineEdit_vth_p-> text().toDouble();
    ntherm = ui->lineEdit_vth_n-> text().toDouble();
    radhard_p = ui->doubleSpinBox_p->value();
    radhard_n = ui->doubleSpinBox_n->value();

    radok=1;
}

void Radhard_dialog::on_radioButton_yes_clicked()
{
    ui->lineEdit_fluence->setEnabled(true);
    ui->lineEdit_sigma_n->setEnabled(true);
    ui->lineEdit_sigma_p->setEnabled(true);
    ui->lineEdit_vac->setEnabled(true);
    ui->lineEdit_vth_n->setEnabled(true);
    ui->lineEdit_vth_p->setEnabled(true);
    ui->pushButton_vacBrowse->setEnabled(true);
    ui->doubleSpinBox_n->setEnabled(true);
    ui->doubleSpinBox_p->setEnabled(true);
    radok=1;
    ui->vac_plot->setVisible(true);
}

void Radhard_dialog::on_radioButton_no_clicked()
{
    ui->lineEdit_fluence->setEnabled(false);
    ui->lineEdit_sigma_n->setEnabled(false);
    ui->lineEdit_sigma_p->setEnabled(false);
    ui->lineEdit_vac->setEnabled(false);
    ui->lineEdit_vth_n->setEnabled(false);
    ui->lineEdit_vth_p->setEnabled(false);
    ui->pushButton_vacBrowse->setEnabled(false);
    ui->doubleSpinBox_n->setEnabled(false);
    ui->doubleSpinBox_p->setEnabled(false);
    control_vac=0;
    ui->vac_plot->setVisible(false);
}

void Radhard_dialog::on_pushButton_ok_clicked()
{


    if(control_vac==0 && ui->radioButton_yes->isChecked())
    {
        control_window=0;
        radok=1;

    }

    else if(control_vac==0 && ui->radioButton_no->isChecked())
    {
        control_window=0;
        radok=0;

    }
    else
    {
        control_window=1;
    }

}

void Radhard_dialog::on_pushButton_cancel_clicked()
{
    control_window=1;
}

void Radhard_dialog::setup_vac_plot_first()
{


    x_vac.resize(100);
    y_vac.resize(100);

    control_vac=0;

    QFile file_vac(path_vac);
    QTextStream its_vac(&file_vac);
    QString line_vac;
    if (!file_vac.open(QIODevice::ReadOnly))
        {
        //ui->textMessageBox->append("Vacancy file not found");
    control_vac=1;
    }

    if(control_vac==0)
    {

    for(int l=0; l<28; l++)
    {
        line_vac = its_vac.readLine();

    }

    for(int l=0; l<100; l++)
    {
        line_vac = its_vac.readLine();

        line_vac.replace(",",".");

        //sostituisci virgola con punto

        QStringList list_vac=line_vac.split(" ",QString::SkipEmptyParts);
        x_vac/*_import*/[l]=list_vac[0].toDouble()*1e-4;  //inserito già in micrometri
       // x_vac[l]=x_vac_import[l]-dead_layer;
        y_vac[l]=list_vac[2].toDouble()*1e10;            //udm: vac/m
    //QUI DEAD LAYER TEMPORANEAMENTE SOPPRESSO
    }

    file_vac.close();



    setupVac(ui->vac_plot);
    ui->vac_plot->replot();

    }


}
