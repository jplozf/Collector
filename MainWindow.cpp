#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileInfo>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lblVersion->setText(QCoreApplication::applicationVersion());

    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setRootPath(QDir::homePath());

    ui->tvwBrowser->setModel(fileSystemModel);
    ui->tvwBrowser->setRootIndex(fileSystemModel->index(QDir::homePath()));
    ui->tvwBrowser->hideColumn(1);
    ui->tvwBrowser->hideColumn(2);
    ui->tvwBrowser->hideColumn(3);

    launcherModel = new QStandardItemModel(this);
    ui->tvwLauncher->setModel(launcherModel);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_tvwBrowser_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    QString filePath = fileSystemModel->filePath(index);
    QFileInfo fileInfo(filePath);

    if (fileInfo.isFile()) {
        QString fileNameWithoutExtension = fileInfo.completeBaseName();
        QStandardItem *item = new QStandardItem(fileNameWithoutExtension);
        launcherModel->appendRow(item);
    }
}

