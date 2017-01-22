#include <QApplication>
#include <signal.h>
#include "mainwindow.hpp"


#include "common/CMainFunctions.hpp"

int main(int argc, char *argv[])
{
    signal(SIGSEGV, CMainFunctions::printTrace);
    signal(SIGABRT, CMainFunctions::printTrace);
    QApplication a(argc, argv);
    CMainFunctions::mpApplication = &a;
    CMainFunctions::parseParams(0, argv, __INSTALL_BIN_DIR__);
    MainWindow w;
    w.show();

    return a.exec();
}
