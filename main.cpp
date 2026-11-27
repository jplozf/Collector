#include "MainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QCommandLineParser>
#include "version.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/green/Database.png"));
    QCoreApplication::setApplicationVersion(APP_VERSION_STRING);

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Collector - launcher for files and programs"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("list",
                                 QObject::tr("List to open: a file path, or a name from ~/.Collector/lists (e.g. \"games\")."),
                                 "[list]");
    QCommandLineOption settingsOption(QStringList() << "s" << "settings",
                                      QObject::tr("Use <file> as the application settings file."), "file");
    parser.addOption(settingsOption);
    parser.process(a);

    const QStringList positional = parser.positionalArguments();
    const QString listPath = positional.isEmpty() ? QString() : positional.first();
    const QString settingsPath = parser.value(settingsOption);

    MainWindow w(nullptr, settingsPath, listPath);
    w.show();
    return a.exec();
}
