#ifndef WIZARD_H
#define WIZARD_H

#include <QWizard>
#include <QDebug>
#include "filebrowser.h"

namespace Ui {
    class Wizard;
}

class Wizard : public QWizard
{
    Q_OBJECT

public:
    explicit Wizard(QWidget *parent = 0);
    ~Wizard();
    QString wpath1, wpath2, wpath3;
    double diffusion;


private:
    Ui::Wizard *ui;


private slots:

    void on_pushButton_3_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
    void on_radioButton_2_clicked();
    void on_radioButton_clicked();

};

#endif // WIZARD_H
