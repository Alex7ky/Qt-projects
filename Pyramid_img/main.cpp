#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString pathName = NULL;
    double factor = 1;

    if (argc > 2 && strcmp(argv[1], "-f") == 0) {
        pathName.append(argv[2]);
        if (argc == 5 && strcmp(argv[3], "-l") == 0) {
            factor = atof(argv[4]);
        }
    }

    MainWindow w(pathName, factor);
    w.show();

    return a.exec();
}
