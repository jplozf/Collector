#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QDir>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QColor>
#include <QCloseEvent>
#include <QSettings>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui {

class MainWindow;

}
QT_END_NAMESPACE

enum CustomRoles {
    FilePathRole = Qt::UserRole,
    CustomCommandLineRole = Qt::UserRole + 1,
    SudoRole = Qt::UserRole + 2,
    TerminalRole = Qt::UserRole + 3
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_tvwLauncher_doubleClicked(const QModelIndex &index);
    void on_tvwBrowser_doubleClicked(const QModelIndex &index);
    void on_btnSelectBackgroundColor_clicked();
    void restoreWindowGeometry();
    void on_cmbIconSet_currentIndexChanged(int index);
    void on_btnUp_clicked();
    void on_btnDown_clicked();
    void on_btnDelete_clicked();
    void on_btnAddTopic_clicked();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    Ui::MainWindow *ui;
    QFileSystemModel *fileSystemModel;
    QStandardItemModel *launcherModel;
    QColor m_backgroundColor;
    QString m_iconSet;
    QByteArray m_geometry;
    QByteArray m_state;
    QTimer *m_saveGeometryTimer;
    QComboBox *cbxOpenWith;
    // QPushButton *btnBrowseOpenWith;

    void applyBackgroundColor();
    void updateBackgroundColorPreview();
    QColor getContrastingTextColor(const QColor &backgroundColor);
    void writeSettings();
    void readSettings();
    void applyIconSet();

protected:
    void closeEvent(QCloseEvent *event) override;
};

void writeItemsRecursive(QSettings &settings, const QString &groupName, QStandardItem *parentItem);
void readItemsRecursive(QSettings &settings, const QString &groupName, QStandardItem *parentItem);

#endif // MAINWINDOW_H
