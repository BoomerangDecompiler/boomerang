#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "AnsiCParserDriver.h"

#include "AnsiCParser.hpp"

#include "boomerang/util/log/Log.h"


AnsiCParserDriver::AnsiCParserDriver()
    : trace_parsing(false)
    , trace_scanning(false)
{
}


int AnsiCParserDriver::parse(const QString &fileName, Machine machine, CallConv _cc)
{
    this->plat = machine;
    this->cc   = _cc;

    file = fileName.toStdString();
    location.initialize(&file);

    if (!scanBegin()) {
        return false;
    }

    AnsiC::parser parser(*this);
    parser.set_debug_level(trace_parsing);

    const int res = parser.parse();
    scanEnd();
    return res;
}
