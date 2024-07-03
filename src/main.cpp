#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.showFullScreen();
    a.connect(&w, &MainWindow::closeButtonPressed, &a, &QApplication::quit);
    return a.exec();
}
