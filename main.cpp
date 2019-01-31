#include "mainwindow.h"
#include <QApplication>
#include "udisk.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    auto pDisk = new UDisk;
    a.installNativeEventFilter(pDisk);
    MainWindow w;
    w.setDisk(pDisk);
    w.show();

    return a.exec();
}
