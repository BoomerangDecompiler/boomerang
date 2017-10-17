#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License


#include <QApplication>

#include "mainwindow.h"
#include "boomerang/util/Log.h"

int main(int argc, char *argv[])
{
    Log::getOrCreateLog();

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/boomerang_icon.png"));
    MainWindow mainWindow;

    mainWindow.show();
    return app.exec();
}
