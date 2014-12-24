#ifndef DIALOG_SETUP_H
#define DIALOG_SETUP_H

#include <QDialog>
#include "mainwindow.h"
#include "./qcustomplot.h"

namespace Ui {
  class Dialog_setup;
}

class Dialog_setup : public QDialog
{
  Q_OBJECT
  
public:
  explicit Dialog_setup(QWidget *parent = 0);
  ~Dialog_setup();
  void setData(int lateral);


QString path_ioniz;

  int n_point,n_ion, n_eh, srim, simok, lat_front, length, control_ion;
  double dispersion, e_noise, e_threshold, deltax, dead_layer;
//  double straggl;
  QVector<double> x_ioniz, y_ioniz;



private slots:
//  void on_buttonBox_accepted();

  //void on_buttonBox_rejected();

  void on_radioButton_2_clicked();

  void on_radioButton_clicked();

  void on_pushButton_browseion_clicked();
  void setupIoniz(QCustomPlot *potPlot);

  void update_values();


  void on_pushButton_2_clicked();

  void on_pushButton_cancel_clicked();


  void setup_frontal_plot_first();

private:
  Ui::Dialog_setup *ui;
};

#endif // DIALOG_SETUP_H










