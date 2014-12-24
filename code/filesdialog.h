#ifndef FILESDIALOG_H
#define FILESDIALOG_H

#include <QDialog>
#include "filebrowser.h"
#include "./qcustomplot.h"


namespace Ui {
    class FilesDialog;
}

class FilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilesDialog(QWidget *parent = 0);
    ~FilesDialog();
QString path_gunn, path_efield, path_vac, path_ioniz;

double temp, mun, mup, vsatp, vsatn, taup, taun, deltax, space_step_gunn, space_step_efield;
int srim, L;
int checkdialog; //dà la possibilità di fare ok per chiudere il dialogo

QVector<double> gunn, pos_gunn, efield, pos_efield;

private:
    Ui::FilesDialog *ui;

private slots:

    void on_pushButton_gunn_clicked();
    void on_pushButton_efield_clicked();
    void plotEfield();
    void plotGunn();
    void setupGunn(QCustomPlot *potPlot);
    void setupEfield(QCustomPlot *potPlot);
    void on_pushButton_OK_clicked();
    void on_pushButton_cancel_clicked();
    void update_values();
    void text_efield();
    void text_gunn();

public slots:


};

#endif // FILESDIALOG_H
