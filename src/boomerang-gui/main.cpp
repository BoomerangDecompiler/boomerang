#include <QApplication>

#include "mainwindow.h"
#include "boomerang/core/Boomerang.h"

int main(int argc, char *argv[])
{
    Boomerang::get()->getOrCreateLog();
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/boomerang_icon.png"));
    MainWindow   mainWindow;

    mainWindow.show();
    return app.exec();
}
