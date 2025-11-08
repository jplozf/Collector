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

    // Populate icon set combobox
    ui->cmbIconSet->addItem("blue");
    ui->cmbIconSet->addItem("green");
    ui->cmbIconSet->addItem("grey");
    ui->cmbIconSet->addItem("orange");
    ui->cmbIconSet->addItem("pink");
    ui->cmbIconSet->addItem("red");

    // Read settings, potentially overriding the default
    readSettings();

    connect(ui->cmbIconSet, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_cmbIconSet_currentIndexChanged);

    // Apply the final background color and update the UI
    applyBackgroundColor();
    updateBackgroundColorPreview();
    applyIconSet(); // Apply icon set after reading settings and populating combobox

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
    settings.setValue("iconSet", m_iconSet);

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
    if (settings.contains("iconSet")) {
        m_iconSet = settings.value("iconSet").toString();
        int index = ui->cmbIconSet->findText(m_iconSet);
        if (index != -1) {
            ui->cmbIconSet->setCurrentIndex(index);
        }
    } else {
        m_iconSet = "blue"; // Default if not found
    }

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

void MainWindow::applyIconSet()
{
    // Update tab icons
    ui->tabWidget->setTabIcon(0, QIcon(QString(":/icons/%1/Start.png").arg(m_iconSet)));
    ui->tabWidget->setTabIcon(1, QIcon(QString(":/icons/%1/Folder2.png").arg(m_iconSet)));
    ui->tabWidget->setTabIcon(2, QIcon(QString(":/icons/%1/Gear.png").arg(m_iconSet)));

    // Update button icons
    ui->btnAddTopic->setIcon(QIcon(QString(":/icons/%1/Plus.png").arg(m_iconSet)));
    ui->btnUp->setIcon(QIcon(QString(":/icons/%1/Arrow1 Up.png").arg(m_iconSet)));
    ui->btnDown->setIcon(QIcon(QString(":/icons/%1/Arrow1 Down.png").arg(m_iconSet)));
    ui->btnDelete->setIcon(QIcon(QString(":/icons/%1/Trash.png").arg(m_iconSet)));
    ui->btnBrowseOpenWith->setIcon(QIcon(QString(":/icons/%1/Folder2.png").arg(m_iconSet)));
    ui->btnAddShortcut->setIcon(QIcon(QString(":/icons/%1/Plus.png").arg(m_iconSet)));
    ui->btnSelectBackgroundColor->setIcon(QIcon(QString(":/icons/%1/Write.png").arg(m_iconSet)));
    ui->btnLoadTemplate->setIcon(QIcon(QString(":/icons/%1/Folder2.png").arg(m_iconSet)));
    ui->btnSave->setIcon(QIcon(QString(":/icons/%1/Save.png").arg(m_iconSet)));
    ui->btnCreateShortcut->setIcon(QIcon(QString(":/icons/%1/Star.png").arg(m_iconSet)));
}

void MainWindow::on_cmbIconSet_currentIndexChanged(const QString &arg1)
{
    m_iconSet = arg1;
    applyIconSet();
}
