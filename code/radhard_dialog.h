#ifndef RADHARD_DIALOG_H
#define RADHARD_DIALOG_H

#include <QDialog>
#include <QObject>
#include "./qcustomplot.h"
#include "mainwindow.h"

namespace Ui {
class Radhard_dialog;
}

class Radhard_dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit Radhard_dialog(QWidget *parent = 0);
    ~Radhard_dialog();
    
    double radhard_p, radhard_n, fluence, sigmap, sigman, ptherm, ntherm, dead_layer;
    QString path_vac;
    QVector<double> x_vac, y_vac;
    int control_vac, radok;
    int control_window;


private slots:
    void on_pushButton_vacBrowse_clicked();
    void setupVac(QCustomPlot *potPlot);
    void update_values();

    void on_radioButton_yes_clicked();

    void on_radioButton_no_clicked();

    void on_pushButton_ok_clicked();

    void on_pushButton_cancel_clicked();

    void setup_vac_plot_first();

private:
    Ui::Radhard_dialog *ui;
};

#endif // RADHARD_DIALOG_H
