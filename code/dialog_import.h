#ifndef DIALOG_IMPORT_H
#define DIALOG_IMPORT_H

#include <QDialog>

namespace Ui {
class Dialog_import;
}

class Dialog_import : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog_import(QWidget *parent = 0);
    ~Dialog_import();
    
    int check_import;
    QString path_experiment;


private:
    Ui::Dialog_import *ui;

public slots:



    void on_pushButton_browse_clicked();
    void on_buttonBox_accepted();

    private slots:


};

#endif // DIALOG_IMPORT_H
