#include "ImgAdjust.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImgAdjust w;
    w.show();

    return a.exec();
}
