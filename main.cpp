#include "MainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include "version.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationVersion(APP_VERSION_STRING);
    MainWindow w;
    w.show();
    return a.exec();
}
