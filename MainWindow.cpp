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
    QColor textColor = getContrastingTextColor(m_backgroundColor);
    // Apply the background color to all widgets in the application
    if (QApplication::instance()) {
        qobject_cast<QApplication*>(QApplication::instance())->setStyleSheet(QString("QWidget { background-color: %1; color: %2; }").arg(m_backgroundColor.name()).arg(textColor.name()));
    }
}

void MainWindow::updateBackgroundColorPreview()
{
    QString colorName = m_backgroundColor.name();
    QColor textColor = getContrastingTextColor(m_backgroundColor);

    // Set the background of the QLabel to the selected color and the contrasting text color
    ui->lblBackgroundColorPreview->setStyleSheet(QString("background-color: %1; color: %2;").arg(colorName).arg(textColor.name()));

    // Set the text of the QLabel to the hex color value and center it
    ui->lblBackgroundColorPreview->setText(colorName);
    ui->lblBackgroundColorPreview->setAlignment(Qt::AlignCenter);
}

QColor MainWindow::getContrastingTextColor(const QColor &backgroundColor)
{
    int r = backgroundColor.red();
    int g = backgroundColor.green();
    int b = backgroundColor.blue();

    double y = (r * 299 + g * 587 + b * 114) / 1000.0;

    if (y >= 128) {
        return Qt::black;
    } else {
        return Qt::white;
    }
}
