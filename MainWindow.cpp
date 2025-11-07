#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileInfo>
#include <QCoreApplication>
#include <QColorDialog>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lblVersion->setText(QCoreApplication::applicationVersion());

    // Initialize background color
    m_backgroundColor = Qt::white;
    applyBackgroundColor();
    updateBackgroundColorPreview();

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

void MainWindow::on_btnSelectBackgroundColor_clicked()
{
    QColor newColor = QColorDialog::getColor(m_backgroundColor, this, "Select Background Color");
    if (newColor.isValid()) {
        m_backgroundColor = newColor;
        applyBackgroundColor();
        updateBackgroundColorPreview();
    }
}

void MainWindow::applyBackgroundColor()
{
    // Apply the background color to all widgets in the application
    if (QApplication::instance()) {
        qobject_cast<QApplication*>(QApplication::instance())->setStyleSheet(QString("QWidget { background-color: %1; }").arg(m_backgroundColor.name()));
    }
}

void MainWindow::updateBackgroundColorPreview()
{
    // Set the background of the QLabel to the selected color
    ui->lblBackgroundColorPreview->setStyleSheet(QString("background-color: %1;").arg(m_backgroundColor.name()));
}
