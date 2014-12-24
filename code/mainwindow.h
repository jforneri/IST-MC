#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "filesdialog.h"
#include "dialog_setup.h"
#include "dialog_import.h"
#include "wizard.h"
#include "./qcustomplot.h"
#include <QTimer>
#include "dialog_import.h"
#include "radhard_dialog.h"
#include "licensedialog.h"


namespace Ui {
    class MainWindow;

}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


QString path_gunn, path_efield, path_vac, path_ioniz, path_experiment;
double temp;
int L, Tn, Tp;
int control_arrayL;
double ndiff, pdiff, vsatp, vsatn, taup, taun, mun, mup;
double deltax, deltatn, deltatp;
double dead_layer;
int import_srim;
QVector<double> x_histo_mean, y_histo_mean;
int n_point, n_ion, n_eh;
double dispersion, e_noise, e_threshold;
int ion;
int p,x0, steps;
int exp_import;
double progress, end_sim;
int lat_front;
double straggle, ntherm, ptherm, radhard_p, radhard_n, nsigma, psigma, fluence;
int rad_yesno;
double radhard;

QVector<double> xstart;
QVector<double> cce;
QVector<double> cce_noise;
QVector<QVector<double> > cce_threshold;
QVector<QVector<double> > multipos;
QVector<double> cce_median;
QVector<QVector<double > > tot_spectrum; //HO MODIFICATO QUA
QVector<int> maximum;
int gnearest;

double tot_hit_over;
double tot_mean_plot;
QVector<double> histo_step;
QVector<double> x_vac, x_vac_import, y_vac, x_ioniz, x_ioniz_import, y_ioniz;
QVector<double> x_exp, y_exp, x_exp_shift, y_exp_scale;
QVector<double> pos_gunn;
QVector<double> pos_efield;
QVector<double> gunn;
QVector<double> efield;
QVector<double> pdrift, mp;
QVector<double> ndrift, mn;
QVector<double> taup_arr, taun_arr, ptaup, ptaun;
QVector<double> yvac_inter, xvac_inter, yioniz_inter, xioniz_inter, CDF_ion;
QVector<double> CDF_test, CDF_x;
QVector<double> cce_frontal_ion_p, cce_frontal_ion_n, cce_frontal, cce_frontal_aux, cce_frontal_thresh;
QVector<double>  cce_time_frontal_tot, cce_tfrontal, cce_tfrontal_aux;
int hit_frontal;
double aux_mean_frontal, cce_mean_frontal;
QVector<int> cce_time_frontal_p, cce_time_frontal_n;
double A,F, B;
int control_vac, control_ion;
QString title_plot;
QVector<double> xgpoint, ygpoint;

int example_control;

double xshift, yscale;

double slide_generation_point, gpoint;
int set_legend;

QVector<QVector<double> > cce_frontal_ion_p2, cce_frontal_ion_n2;
QVector<QVector<double> >  cce_time_frontal_tot2, cce_tfrontal2, cce_tfrontal_aux2;
QVector<QVector<int> > cce_time_frontal_p2, cce_time_frontal_n2;

int files_dialog_check_ok;

double maxtscale, valtscale;


QString last_path, last_path_ioniz;

private:
    Ui::MainWindow *ui;

void slideChanged(int value);
void legendClick();
void setupGunn(QCustomPlot *potPlot);
void setupEfield(QCustomPlot *potPlot);
void setupIoniz(QCustomPlot *potPlot);
void setupCDF(QCustomPlot *potPlot);
void setupVac(QCustomPlot *potPlot);
void setupTau(QCustomPlot *potPlot);
void setupPdrift(QCustomPlot *potPlot);
void setupNdrift(QCustomPlot *potPlot);
void setupMC(QCustomPlot *potPlot, QCustomPlot *histo, QCustomPlot *time);


QTimer dataTimer;

private slots:


void wrapperDone();


    void on_pushButton_simsetup_clicked();
    void on_pushButton_sim_clicked();
    void montecarlo();
    void frontal();
    void electrostatics();
    void updatePlot();
    void on_pushButton_resume_clicked();
    void on_pushButton_stop_clicked();
    void on_pushButton_saveplot_clicked();
    void on_checkBox_clicked();
    void on_horizontalSlider_valueChanged(int value);
    void on_time_spinBox_editingFinished();
    void on_time_spinBox_valueChanged();
    void text_transport();
    void text_simulation();
    void text_radhard();
    void on_pushButton_radsetup_clicked();
    void on_pushButton_devsetup_clicked();
    void on_pushButton_about_clicked();
};




#endif // MAINWINDOW_H

