#include <QCoreApplication>
#include <QStringList>

#include "commandlinedriver.h"

int main(int argc, char* argv[])
{
    QCoreApplication  app(argc, argv);
    CommandlineDriver driver;

    bool decompile = driver.applyCommandline(app.arguments()) == 0;
    if (!decompile) {
        return 0;
    }

    return driver.decompile();
}
