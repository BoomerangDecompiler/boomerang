
#include <QApplication>

#include "mainwindow.h"

#include "gc.h"

void init_dfa();			// Prototypes for
void init_sslparser();		// various initialisation functions
void init_basicblock();		// for garbage collection safety

int main(int argc, char *argv[])
{
    init_dfa();
    init_sslparser();
    init_basicblock();
    
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}

