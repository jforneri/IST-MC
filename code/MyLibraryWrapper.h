#include <QThread>
#include "mainwindow.h"
#include <QVector>

#ifndef MYLIBRARYWRAPPER_H
#define MYLIBRARYWRAPPER_H

class MyLibraryWrapper : public QThread
{
Q_OBJECT



public:

    MyLibraryWrapper();

QVector<double> x_histo_mean, y_histo_mean;

    int lat_front;
    QString path_gunn, path_efield, path_vac, path_ioniz, path_experiment;
    double temp;
    int L, Tn, Tp;
    int control_arrayL;
    double ndiff, pdiff, vsatp, vsatn, taup, taun, mun, mup;
    double deltax, deltatn, deltatp;
    double dead_layer;
    int import_srim;

    int n_point, n_ion, n_eh;
    double dispersion, e_noise, e_threshold;
    int ion;
    int p,x0, steps;
    int exp_import;
    double progress, end_sim;
    double straggle, ntherm, ptherm, radhard, nsigma, psigma, fluence;

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

    //int h, stragg_control, p_control, n_control, x, xinit;
    //double counter1, counter2, countern, counterp, ctot;

    double xshift, yscale;

    double slide_generation_point, gpoint;
    int set_legend;


    //DA RIADATTARE PER POSITION RISETIME E SPECTRUM
    QVector<QVector<double> > cce_frontal_ion_p2, cce_frontal_ion_n2;
    QVector<QVector<double> >  cce_time_frontal_tot2, cce_tfrontal2, cce_tfrontal_aux2;
    QVector<QVector<int> > cce_time_frontal_p2, cce_time_frontal_n2;


    double maxtscale, valtscale;

    QTimer dataTimer;

int thread_status;
    int prog_show;

private:



protected:

   void run();
signals:
   void done(const QString &results);
   int updateProgressbar(int value);
   void passValues(/*QVector<double> value1, QVector<QVector<double> > value2, double value3, QVector<QVector<double> > value4, QVector<QVector<double > > value5,
                   QVector<double> value6,QVector<double> value7,QVector<double> value8,double value9,double value10,QVector<int> value11,QVector<double> value12,QVector<QVector<double> > value13*/);
   void thread_finished();

public slots:

      void stop_thread();
      void launch();

      void start_thread();
      //void execute();

private slots:
   void doTheWork();
   void montecarlo();
   void frontal();

};

#endif // MYLIBRARYWRAPPER_H
