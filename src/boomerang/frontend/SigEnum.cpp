#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SigEnum.h"


#include <QString>

static const QString g_platformNames[(int)Platform::PlatformCount] = {
    "???", // GENERIC
    "pentium",
    "sparc",
    "m68k",
    "parisc",
    "ppc",
    "mips",
    "st20"
};

static const QString g_callConvNames[(int)CallConv::CallConvCount] = {
    "??",
    "stdc",
    "pascal",
    "thiscall",
    "fastcall"
};


const QString& Util::getPlatformName(Platform plat)
{
    return g_platformNames[(int)plat];
}


const QString& Util::getCallConvName(CallConv cc)
{
    return g_callConvNames[(int)cc];
}
