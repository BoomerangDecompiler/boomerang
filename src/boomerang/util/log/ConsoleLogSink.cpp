#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ConsoleLogSink.h"

#include <QString>

#include <iostream>


void ConsoleLogSink::write(const QString &s)
{
    std::cout << qPrintable(s);
}


void ConsoleLogSink::flush()
{
    std::cout.flush();
}
