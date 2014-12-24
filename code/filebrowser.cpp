#include "filebrowser.h"
#include "ui_filebrowser.h"

FileBrowser::FileBrowser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileBrowser)
{
    ui->setupUi(this);
    QString sPath="/";
                               dirmodel = new QFileSystemModel(this);
                               dirmodel -> setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
                               dirmodel->setRootPath(sPath);
                               ui->treeView->setModel(dirmodel);
                               ui->treeView->sortByColumn(0,Qt::AscendingOrder);






}

FileBrowser::~FileBrowser()
{
    delete ui;
}


/*
QString FileBrowser::on_pushButton_clicked()
{

}

*/

void FileBrowser::on_treeView_clicked(QModelIndex index)
{
    QString sPath = dirmodel->fileInfo(index).absoluteFilePath();
        ui->lineEdit->setText(sPath);
}

/*
QString FileBrowser::on_pushButton_2_clicked()
{
   return "  ";
}
*/

QString FileBrowser::on_buttonBox_accepted()
{
    return ui->lineEdit->text();
}
