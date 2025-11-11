#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileInfo>
#include <QCoreApplication>
#include <QColorDialog>
#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QCloseEvent>
#include <QTimer>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QDebug>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create cbxOpenWith programmatically
    cbxOpenWith = new QComboBox(this);
    cbxOpenWith->setEditable(true);

    // Connect checkbox to combobox enabled state and set initial state
    connect(ui->chkOpenWith, &QCheckBox::toggled, cbxOpenWith, &QComboBox::setEnabled);
    cbxOpenWith->setEnabled(ui->chkOpenWith->isChecked());

    // Create btnBrowseOpenWith programmatically
    // btnBrowseOpenWith = new QPushButton(this);
    // btnBrowseOpenWith->setText(""); // Text will be an icon

    // Create horizontal layout for cbxOpenWith and btnBrowseOpenWith
    QHBoxLayout *horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->addWidget(cbxOpenWith);
    // horizontalLayout_2->addWidget(btnBrowseOpenWith);

    // Get the existing QGridLayout from tabExplorer
    QGridLayout *gridLayout_3 = qobject_cast<QGridLayout*>(ui->tabExplorer->layout());
    if (gridLayout_3) {
        // Get the QVBoxLayout (verticalLayout_2) from the grid layout
        QLayoutItem *item = gridLayout_3->itemAtPosition(0, 0);
        if (item && item->layout()) {
            QVBoxLayout *verticalLayout_2 = qobject_cast<QVBoxLayout*>(item->layout());
            if (verticalLayout_2) {
                // Insert the new horizontal layout after chkOpenWith and before tvwBrowser
                verticalLayout_2->insertLayout(1, horizontalLayout_2);
            } else {
                qWarning() << "Error: verticalLayout_2 is not a QVBoxLayout.";
            }
        } else {
             qWarning() << "Error: Could not find verticalLayout_2 item in gridLayout_3.";
        }
    } else {
        qWarning() << "Error: tabExplorer's layout is not a QGridLayout or not found.";
    }

    // Initialize background color to a default
    m_backgroundColor = Qt::white;

    launcherModel = new QStandardItemModel(this);
    ui->tvwLauncher->setModel(launcherModel);
    ui->tvwLauncher->setEditTriggers(QAbstractItemView::SelectedClicked);

    // Populate icon set combobox
    ui->cmbIconSet->addItem("blue");
    ui->cmbIconSet->addItem("green");
    ui->cmbIconSet->addItem("grey");
    ui->cmbIconSet->addItem("orange");
    ui->cmbIconSet->addItem("pink");
    ui->cmbIconSet->addItem("red");

    // Read settings, potentially overriding the default
    readSettings();

    connect(ui->cmbIconSet, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_cmbIconSet_currentIndexChanged);

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

    restoreWindowGeometry();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::restoreWindowGeometry()
{
    if (!m_geometry.isEmpty())
        restoreGeometry(m_geometry);
    if (!m_state.isEmpty())
        restoreState(m_state);
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
        QStandardItem *newItem = new QStandardItem(fileNameWithoutExtension);
        newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
        newItem->setData(filePath, FilePathRole); // Store the full path in FilePathRole
        newItem->setData(ui->chkSudo->isChecked(), SudoRole);
        newItem->setData(ui->chkTerminal->isChecked(), TerminalRole);

        if (ui->chkOpenWith->isChecked()) {
            QString command = cbxOpenWith->currentText();
            if (!command.isEmpty()) {
                newItem->setData(command, CustomCommandLineRole); // Store the command
                if (cbxOpenWith->findText(command) == -1) {
                    cbxOpenWith->addItem(command);
                }
            }
        }

        QModelIndex currentIndex = ui->tvwLauncher->selectionModel()->currentIndex();
        QStandardItem *parentItem = nullptr;

        if (currentIndex.isValid()) {
            parentItem = launcherModel->itemFromIndex(currentIndex);
            // If the selected item is a shortcut (a child itself), get its parent.
            if (parentItem->parent()) {
                parentItem = parentItem->parent();
            }
        } else {
            // No selection, find the last top-level item.
            if (launcherModel->rowCount() > 0) {
                parentItem = launcherModel->item(launcherModel->rowCount() - 1);
            }
        }

        if (parentItem) {
            parentItem->appendRow(newItem);
        } else {
            // No topics exist yet, add to top level.
            launcherModel->appendRow(newItem);
        }
        ui->statusbar->showMessage("Shortcut added: " + fileNameWithoutExtension, 3000); // Message for 3 seconds
    }
}

void MainWindow::on_tvwLauncher_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    // Retrieve the item from the model
    QStandardItem *item = launcherModel->itemFromIndex(index);
    if (!item) {
        return;
    }

    // Get the stored path from the item's data
    QString filePath = item->data(FilePathRole).toString();
    QString command = item->data(CustomCommandLineRole).toString();
    bool sudo = item->data(SudoRole).toBool();
    bool terminal = item->data(TerminalRole).toBool();

    QProcess *process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::onProcessFinished);

    QString commandToExecute;

    if (!command.isEmpty()) {
        commandToExecute = command;
        commandToExecute.replace("%1", "\"" + filePath + "\"");
    } else if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        // Treat executables, scripts, and jar files as runnable
        if (fileInfo.isExecutable() || fileInfo.suffix() == "sh" || fileInfo.suffix() == "jar") {
            commandToExecute = "\"" + filePath + "\"";
            process->setWorkingDirectory(fileInfo.absolutePath());
        } else {
            // Not an executable, use desktop services
            if (!QDesktopServices::openUrl(QUrl::fromLocalFile(filePath))) {
                qWarning() << "Failed to open file with default application:" << filePath;
                QMessageBox::critical(this, tr("Launch Error"), tr("Failed to open file: %1").arg(filePath));
            }
            process->deleteLater();
            return;
        }
    } else {
        // Nothing to do
        process->deleteLater();
        return;
    }

    if (sudo) {
        commandToExecute.prepend("sudo ");
    }

    QString fullCommandForDisplay = commandToExecute;
    QString program;
    QStringList arguments;

    if (terminal) {
        program = "xterm";
        arguments << "-e" << commandToExecute;
        fullCommandForDisplay = program + " -e '" + commandToExecute + "'";
    } else {
        program = "/bin/sh";
        arguments << "-c" << commandToExecute;
    }

    process->setProperty("commandLine", fullCommandForDisplay);
    process->start(program, arguments);
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
    restoreWindowGeometry();
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
    settings.setValue("state", saveState());
    settings.setValue("backgroundColor", m_backgroundColor.name());
    settings.setValue("currentTab", ui->tabWidget->currentIndex());
    settings.setValue("iconSet", m_iconSet);

    settings.setValue("chkOpenWith", ui->chkOpenWith->isChecked());
    settings.setValue("chkSudo", ui->chkSudo->isChecked());
    settings.setValue("chkTerminal", ui->chkTerminal->isChecked());
    QStringList commands;
    for (int i = 0; i < cbxOpenWith->count(); ++i) {
        commands << cbxOpenWith->itemText(i);
    }
    settings.setValue("openWithCommands", commands);

    writeItemsRecursive(settings, "shortcuts", launcherModel->invisibleRootItem());
}

void MainWindow::readSettings()
{
    QString path = QDir::homePath() + "/.Collector";
    QSettings settings(path + "/settings.ini", QSettings::IniFormat);

    if (settings.contains("geometry"))
        m_geometry = settings.value("geometry").toByteArray();
    if (settings.contains("state"))
        m_state = settings.value("state").toByteArray();
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

    if (settings.contains("chkOpenWith"))
        ui->chkOpenWith->setChecked(settings.value("chkOpenWith").toBool());
    if (settings.contains("chkSudo"))
        ui->chkSudo->setChecked(settings.value("chkSudo").toBool());
    if (settings.contains("chkTerminal"))
        ui->chkTerminal->setChecked(settings.value("chkTerminal").toBool());
    if (settings.contains("openWithCommands"))
        cbxOpenWith->addItems(settings.value("openWithCommands").toStringList());

    launcherModel->clear();
    readItemsRecursive(settings, "shortcuts", launcherModel->invisibleRootItem());
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
    // btnBrowseOpenWith->setIcon(QIcon(QString(":/icons/%1/Folder2.png").arg(m_iconSet)));
    ui->btnAddShortcut->setIcon(QIcon(QString(":/icons/%1/Plus.png").arg(m_iconSet)));
    ui->btnSelectBackgroundColor->setIcon(QIcon(QString(":/icons/%1/Write.png").arg(m_iconSet)));
    ui->btnLoadTemplate->setIcon(QIcon(QString(":/icons/%1/Folder2.png").arg(m_iconSet)));
    ui->btnSave->setIcon(QIcon(QString(":/icons/%1/Save.png").arg(m_iconSet)));
    ui->btnCreateShortcut->setIcon(QIcon(QString(":/icons/%1/Star.png").arg(m_iconSet)));
}

void MainWindow::on_cmbIconSet_currentIndexChanged(int index)
{
    m_iconSet = ui->cmbIconSet->itemText(index);
    applyIconSet();
    restoreWindowGeometry();
}

void MainWindow::on_btnUp_clicked()
{
    QModelIndex index = ui->tvwLauncher->selectionModel()->currentIndex();
    if (!index.isValid()) {
        return;
    }

    QStandardItem *item = launcherModel->itemFromIndex(index);
    if (!item) {
        return;
    }

    QStandardItem *parentItem = item->parent();
    if (!parentItem) {
        parentItem = launcherModel->invisibleRootItem();
    }

    int row = index.row();

    if (row == 0) {
        // If the item is the first child of its parent
        if (item->parent()) {
            // It has a parent, so it's not a top-level item. Move it out of its parent.
            QStandardItem *grandparentItem = item->parent()->parent();
            if (!grandparentItem) {
                grandparentItem = launcherModel->invisibleRootItem();
            }
            int parentRow = item->parent()->row();
            QList<QStandardItem*> items = parentItem->takeRow(row);
            grandparentItem->insertRow(parentRow, items);
            ui->tvwLauncher->selectionModel()->setCurrentIndex(grandparentItem->child(parentRow)->index(), QItemSelectionModel::SelectCurrent);
        } else {
            // Already a top-level item at row 0, cannot move further up.
            return;
        }
    } else {
        // Move up within the current parent
        QList<QStandardItem*> items = parentItem->takeRow(row);
        parentItem->insertRow(row - 1, items);
        ui->tvwLauncher->selectionModel()->setCurrentIndex(parentItem->child(row - 1)->index(), QItemSelectionModel::SelectCurrent);
    }
}

void MainWindow::on_btnDown_clicked()

{

    QModelIndex index = ui->tvwLauncher->selectionModel()->currentIndex();

    if (!index.isValid()) {

        return;

    }



    QStandardItem *item = launcherModel->itemFromIndex(index);

    if (!item) {

        return;

    }



    QStandardItem *parentItem = item->parent();

    if (!parentItem) {

        parentItem = launcherModel->invisibleRootItem();

    }



    int row = index.row();



    // Check if the item can be moved into a topic below it (only for top-level items)

    if (!item->parent() && (row + 1 < parentItem->rowCount())) {

        QStandardItem *nextSibling = parentItem->child(row + 1);

        // If the next sibling is a topic (i.e., it doesn't have a path, or it has children)

        if (nextSibling && nextSibling->data(Qt::UserRole).toString().isEmpty()) {

            QList<QStandardItem*> items = parentItem->takeRow(row);

            nextSibling->appendRow(items);

            ui->tvwLauncher->selectionModel()->setCurrentIndex(nextSibling->child(nextSibling->rowCount() - 1)->index(), QItemSelectionModel::SelectCurrent);

            return;

        }

    }



    // Existing logic for moving down within the current parent or moving out of a topic

    if (row == parentItem->rowCount() - 1) {

        // If the item is the last child of its parent (or last top-level item)

        if (item->parent()) {

            // Item is a child, and it's the last one. Try to move it out of its parent.

            QStandardItem *grandparentItem = item->parent()->parent();

            if (!grandparentItem) {

                grandparentItem = launcherModel->invisibleRootItem();

            }

            int parentRow = item->parent()->row();



            if (parentRow < grandparentItem->rowCount() - 1) {

                // There is a sibling topic below the current parent, move the item into it

                QStandardItem *siblingTopic = grandparentItem->child(parentRow + 1);

                QList<QStandardItem*> items = parentItem->takeRow(row);

                siblingTopic->appendRow(items);

                ui->tvwLauncher->selectionModel()->setCurrentIndex(siblingTopic->child(siblingTopic->rowCount() - 1)->index(), QItemSelectionModel::SelectCurrent);

            } else {

                // No sibling topic below, move it out of its parent and make it a sibling of its former parent

                QList<QStandardItem*> items = parentItem->takeRow(row);

                grandparentItem->insertRow(parentRow + 1, items);

                ui->tvwLauncher->selectionModel()->setCurrentIndex(grandparentItem->child(parentRow + 1)->index(), QItemSelectionModel::SelectCurrent);

            }

        } else {

            // Item is a top-level item and it's the last one, cannot move further down.

            return;

        }

    } else {

        // Move down within the current parent

        QList<QStandardItem*> items = parentItem->takeRow(row);

        parentItem->insertRow(row + 1, items);

        ui->tvwLauncher->selectionModel()->setCurrentIndex(parentItem->child(row + 1)->index(), QItemSelectionModel::SelectCurrent);

    }

}

void MainWindow::on_btnDelete_clicked()
{
    QModelIndex index = ui->tvwLauncher->selectionModel()->currentIndex();
    if (!index.isValid()) {
        return;
    }

    QStandardItem *item = launcherModel->itemFromIndex(index);
    if (!item) {
        return;
    }

    QString itemName = item->text(); // Get text before removing the item

    QStandardItem *parentItem = item->parent();
    if (parentItem) {
        // Item has a parent, remove it from the parent
        parentItem->removeRow(index.row());
    } else {
        // Item is a top-level item, remove it from the launcherModel
        launcherModel->removeRow(index.row());
    }
    ui->statusbar->showMessage("Item deleted: " + itemName, 3000);
}

// Recursive function to write items to settings
void writeItemsRecursive(QSettings &settings, const QString &groupName, QStandardItem *parentItem) {
    settings.beginWriteArray(groupName);
    for (int i = 0; i < parentItem->rowCount(); ++i) {
        settings.setArrayIndex(i);
        QStandardItem *child = parentItem->child(i);
        settings.setValue("text", child->text());
        settings.setValue("path", child->data(FilePathRole).toString());
        settings.setValue("command", child->data(CustomCommandLineRole).toString());
        settings.setValue("sudo", child->data(SudoRole).toBool());
        settings.setValue("terminal", child->data(TerminalRole).toBool());
        if (child->hasChildren()) {
            writeItemsRecursive(settings, "children", child);
        }
    }
    settings.endArray();
}

// Recursive function to read items from settings
void readItemsRecursive(QSettings &settings, const QString &groupName, QStandardItem *parentItem) {
    int size = settings.beginReadArray(groupName);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString text = settings.value("text").toString();
        QString path = settings.value("path").toString();
        QString command = settings.value("command").toString();
        bool sudo = settings.value("sudo", false).toBool();
        bool terminal = settings.value("terminal", false).toBool();

        QStandardItem *newItem = new QStandardItem(text);
        newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
        newItem->setData(path, FilePathRole);
        if (!command.isEmpty()) {
            newItem->setData(command, CustomCommandLineRole);
        }
        newItem->setData(sudo, SudoRole);
        newItem->setData(terminal, TerminalRole);
        parentItem->appendRow(newItem);

        if (settings.childGroups().contains("children", Qt::CaseInsensitive)) {
             readItemsRecursive(settings, "children", newItem);
        }
    }
    settings.endArray();
}


void MainWindow::on_btnAddTopic_clicked()
{
    QString topicName = ui->txtTopicName->text().trimmed();
    if (!topicName.isEmpty()) {
        QStandardItem *item = new QStandardItem(topicName);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        launcherModel->appendRow(item);
        ui->txtTopicName->clear();
    }
}

void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    if (!process) {
        return;
    }

    if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        QString errorOutput = process->readAllStandardError();
        QString commandLine = process->property("commandLine").toString();
        QString message;

        if (!commandLine.isEmpty()) {
            message += tr("Command: %1\n").arg(commandLine);
        }
        message += tr("Exit Code: %1\n").arg(exitCode);

        if (!errorOutput.isEmpty()) {
            message += tr("Error Output:\n%1").arg(errorOutput);
        } else if (exitStatus == QProcess::CrashExit) {
            message += tr("The process crashed.");
        } else {
            message += tr("The process finished with a non-zero exit code.");
        }
        QMessageBox::critical(this, tr("Process Error"), message);
    }

    process->deleteLater();
}

