#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QDir>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QColor>

QT_BEGIN_NAMESPACE
namespace Ui {

class MainWindow;

}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_tvwBrowser_doubleClicked(const QModelIndex &index);
    void on_btnSelectBackgroundColor_clicked();

private:
    Ui::MainWindow *ui;
    QFileSystemModel *fileSystemModel;
    QStandardItemModel *launcherModel;
    QColor m_backgroundColor;

    void applyBackgroundColor();
    void updateBackgroundColorPreview();
};
#endif // MAINWINDOW_H
