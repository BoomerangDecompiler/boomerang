#include <QApplication>

#include "mainwindow.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/boomerang_icon.png"));
    MainWindow   mainWindow;

    mainWindow.show();
    return app.exec();
}
