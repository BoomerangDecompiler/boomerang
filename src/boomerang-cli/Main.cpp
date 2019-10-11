#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License


#include "boomerang-cli/CommandlineDriver.h"

#include <QCoreApplication>
#include <QStringList>


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    CommandlineDriver driver;

    const int applyResult = driver.applyCommandline(app.arguments());
    if (applyResult != 0) {
        return applyResult;
    }

    return driver.decompile();
}
