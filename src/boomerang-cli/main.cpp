#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License


#include "CommandlineDriver.h"
#include "boomerang/core/Boomerang.h"

#include <QCoreApplication>
#include <QStringList>


int main(int argc, char *argv[])
{
    Boomerang::get();

    QCoreApplication app(argc, argv);
    CommandlineDriver driver;

    const bool decompile = driver.applyCommandline(app.arguments()) == 0;
    if (!decompile) {
        Boomerang::destroy();
        return 0;
    }

    int status = driver.decompile();
    Boomerang::destroy();
    return status;
}
