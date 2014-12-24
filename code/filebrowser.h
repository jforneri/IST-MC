#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <QDialog>
#include <QFileSystemModel>
#include <QLineEdit>
#include <mainwindow.h>


namespace Ui {
    class FileBrowser;
}

class FileBrowser : public QDialog
{
    Q_OBJECT

public:
    explicit FileBrowser(QWidget *parent = 0);
    ~FileBrowser();
    QString last_path;

private:
    Ui::FileBrowser *ui;
    QFileSystemModel *dirmodel;
private slots:

  void on_treeView_clicked(QModelIndex index);

public slots:
  QString on_buttonBox_accepted();

};

#endif // FILEBROWSER_H
