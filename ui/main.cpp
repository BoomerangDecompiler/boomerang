#include <QApplication>
#include "mainwindow.h"
#include "commandlinedriver.h"

void init_dfa();        // Prototypes for
void init_sslparser();  // various initialisation functions
void init_basicblock(); // for garbage collection safety

int main(int argc, char *argv[]) {
    init_dfa();
    init_sslparser();
    init_basicblock();

    QApplication app(argc, argv);
    if(app.arguments().size()>1) {
        CommandlineDriver driver;
        init_dfa();
        init_sslparser();
        init_basicblock();
        driver.applyCommandline(app.arguments());
        return driver.decompile();
    }
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
