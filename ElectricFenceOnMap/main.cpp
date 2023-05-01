#include "ElectricFenceOnMap.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ElectricFenceOnMap w;
    w.show();
    return a.exec();
}
