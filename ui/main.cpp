#include <QApplication>
#include <QCoreApplication>
#include "mainwindow.h"
#include "commandlinedriver.h"

void init_sslparser();  // various initialisation functions
void init_basicblock(); // for garbage collection safety

int main(int argc, char *argv[]) {
    init_sslparser();
    init_basicblock();

    if(argc>1) {
        QCoreApplication app(argc,argv);
        CommandlineDriver driver;
        driver.applyCommandline(app.arguments());
        return driver.decompile();
    }
    else {
        QApplication app(argc, argv);
        MainWindow mainWindow;
        mainWindow.show();
        return app.exec();
    }
}
