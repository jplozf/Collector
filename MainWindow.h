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
#include <QMenu>
#include <QSystemTrayIcon>
#include <QLockFile>
#include <memory>

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
    explicit MainWindow(QWidget *parent = nullptr,
                        const QString &settingsFilePath = QString(),
                        const QString &listFilePath = QString());
    ~MainWindow();

private slots:
    void on_tvwLauncher_doubleClicked(const QModelIndex &index);
    void on_tvwBrowser_doubleClicked(const QModelIndex &index);
    void on_tvwBrowser_clicked(const QModelIndex &index);
    void on_btnSelectBackgroundColor_clicked();
    void restoreWindowGeometry();
    void on_cmbIconSet_currentIndexChanged(int index);
    void on_btnUp_clicked();
    void on_btnDown_clicked();
    void on_btnDelete_clicked();
    void on_btnAddTopic_clicked();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_btnLockUnlock_clicked();
    void on_actionNew_List_triggered();
    void on_actionOpen_List_triggered();
    void on_actionSave_List_triggered();
    void on_actionSave_List_As_triggered();

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
    QString m_defaultTerminalEmulator;
    int m_browserSortOrder; // 0: Name Asc, 1: Name Desc, 2: Date Newest, 3: Date Oldest
    QStringList m_documentSuffixes;
    QString m_currentSettingsFilePath;   // réglages de l'application (settings.ini)
    QString m_currentListFilePath;       // liste de raccourcis actuellement ouverte
    QStringList m_recentLists;
    QMenu *m_menuRecent;
    QString m_cliListPath;                       // liste demandée sur la ligne de commande
    std::unique_ptr<QLockFile> m_listLock;       // verrou de la liste courante
    bool m_listReadOnly = false;
    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_trayMenu = nullptr;
    QTimer *m_trayRebuildTimer = nullptr;
    bool m_locked = true;                        // interface verrouillée par défaut                 // liste déjà ouverte dans une autre fenêtre
    // QPushButton *btnBrowseOpenWith;

    void applyBackgroundColor();
    void updateBackgroundColorPreview();
    QColor getContrastingTextColor(const QColor &backgroundColor);
    void writeSettings(const QString &filePath);
    void readSettings(const QString &filePath);
    void applyIconSet();
    void applyBrowserSortOrder();
    void updateWindowTitle();

    // --- Gestion des listes (un fichier par liste) ---
    QString listsDirectory() const;
    QString defaultListPath() const;
    void loadList(const QString &filePath);
    bool saveList(const QString &filePath);
    bool openList(const QString &filePath);
    void setCurrentList(const QString &filePath);
    void updateRecentMenu();
    void applyListAppearance();
    void setLocked(bool locked);
    void updateLockButton();

    // --- Lancement et zone de notification (system tray) ---
    void launchItem(QStandardItem *item);
    bool trayUsable() const;
    void setupTrayIcon();
    void scheduleTrayRebuild();
    void rebuildTrayMenu();
    void addItemsToTrayMenu(QMenu *menu, QStandardItem *parentItem);
    void toggleWindow();
    void showFromTray();

    // --- Ajout de raccourcis (explorateur, glisser-déposer) ---
    void appendShortcut(QStandardItem *item, QStandardItem *targetTopic = nullptr);
    QStandardItem *createShortcutItem(const QString &filePath);
    QStandardItem *createItemFromDesktopFile(const QString &desktopFilePath);
    void addDroppedFiles(const QList<QUrl> &urls, const QPoint &viewportPos);
    QString resolveListArgument(const QString &argument) const;
    std::unique_ptr<QLockFile> tryLockList(const QString &filePath);
    bool switchListLock(const QString &filePath);
    void on_cmbBrowserSortOrder_currentIndexChanged(int index);

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void changeEvent(QEvent *event) override;
};

void writeItemsRecursive(QSettings &settings, const QString &groupName, QStandardItem *parentItem);
void readItemsRecursive(QSettings &settings, const QString &groupName, QStandardItem *parentItem);

#endif // MAINWINDOW_H
