#include <QApplication>

#include "mainwindow.h"
#include "boomerang/util/Log.h"

int main(int argc, char *argv[])
{
    Log::getOrCreateLog();

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/boomerang_icon.png"));
    MainWindow   mainWindow;

    mainWindow.show();
    return app.exec();
}
