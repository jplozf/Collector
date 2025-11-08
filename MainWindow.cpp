#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileInfo>
#include <QCoreApplication>
#include <QColorDialog>
#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialize background color to a default
    m_backgroundColor = Qt::white;

    launcherModel = new QStandardItemModel(this);
    ui->tvwLauncher->setModel(launcherModel);

    // Read settings, potentially overriding the default
    readSettings();

    // Apply the final background color and update the UI
    applyBackgroundColor();
    updateBackgroundColorPreview();

    ui->lblVersion->setText(QCoreApplication::applicationVersion());

    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setRootPath(QDir::homePath());

    ui->tvwBrowser->setModel(fileSystemModel);
    ui->tvwBrowser->setRootIndex(fileSystemModel->index(QDir::homePath()));
    ui->tvwBrowser->hideColumn(1);
    ui->tvwBrowser->hideColumn(2);
    ui->tvwBrowser->hideColumn(3);
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
        item->setData(filePath, Qt::UserRole); // Store the full path in UserRole
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
    ui->lblBackgroundColorPreview->setText(colorName.toUpper());
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::writeSettings()
{
    QString path = QDir::homePath() + "/.Collector";
    QDir dir(path);
    if (!dir.exists())
        dir.mkpath(".");

    QSettings settings(path + "/settings.ini", QSettings::IniFormat);

    settings.setValue("geometry", saveGeometry());
    settings.setValue("backgroundColor", m_backgroundColor.name());
    settings.setValue("currentTab", ui->tabWidget->currentIndex());

    settings.beginWriteArray("shortcuts");
    for (int i = 0; i < launcherModel->rowCount(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("path", launcherModel->item(i)->data(Qt::UserRole).toString());
    }
    settings.endArray();
}

void MainWindow::readSettings()
{
    QString path = QDir::homePath() + "/.Collector";
    QSettings settings(path + "/settings.ini", QSettings::IniFormat);

    if (settings.contains("geometry"))
        restoreGeometry(settings.value("geometry").toByteArray());
    if (settings.contains("backgroundColor"))
        m_backgroundColor.setNamedColor(settings.value("backgroundColor").toString());
    if (settings.contains("currentTab"))
        ui->tabWidget->setCurrentIndex(settings.value("currentTab").toInt());

    int size = settings.beginReadArray("shortcuts");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString fullPath = settings.value("path").toString();
        QFileInfo fileInfo(fullPath);
        if (fileInfo.isFile()) {
            QStandardItem *item = new QStandardItem(fileInfo.completeBaseName());
            item->setData(fullPath, Qt::UserRole);
            launcherModel->appendRow(item);
        }
    }
    settings.endArray();
}
