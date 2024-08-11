#include "irkenArbitrageScaner.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    IrkenArbitrageScaner scaner;
    return a.exec();
}
