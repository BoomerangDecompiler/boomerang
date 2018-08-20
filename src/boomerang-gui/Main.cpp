#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License


#include "boomerang-gui/MainWindow.h"

#include "boomerang/util/log/Log.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/boomerang_icon.png"));
    MainWindow mainWindow;

    mainWindow.show();
    return app.exec();
}
