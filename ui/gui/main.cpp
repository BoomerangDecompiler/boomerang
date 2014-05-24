
#include <QApplication>

#include "mainwindow.h"

#ifdef HAVE_LIBGC
//#define GC_DEBUG 1        // Uncomment to debug the garbage collector
#include "gc.h"
#else
#define NO_NEW_OR_DELETE_OPERATORS
#define NO_GARBAGE_COLLECTOR
#endif

void init_dfa();            // Prototypes for
void init_sslparser();        // various initialisation functions
void init_basicblock();        // for garbage collection safety

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

