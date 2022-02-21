#include "mainwindow.h"

#include <QApplication>
#include "UI/UIController.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UIController w;
    w.show();
    return a.exec();
}
