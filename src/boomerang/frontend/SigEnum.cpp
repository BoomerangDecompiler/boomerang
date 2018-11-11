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


// clang-format off
static const QString g_callConvNames[(int)CallConv::CallConvCount] =
{
    "??",
    "stdc",
    "pascal",
    "thiscall",
    "fastcall"
};
// clang-format on


const QString &Util::getCallConvName(CallConv cc)
{
    return g_callConvNames[(int)cc];
}
