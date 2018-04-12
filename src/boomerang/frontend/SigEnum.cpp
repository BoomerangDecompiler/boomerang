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



QString Util::getPlatformName(Platform plat)
{
    switch (plat)
    {
    case Platform::PENTIUM: return "pentium";
    case Platform::SPARC:   return "sparc";
    case Platform::M68K:    return "m68k";
    case Platform::PARISC:  return "parisc";
    case Platform::PPC:     return "ppc";
    case Platform::MIPS:    return "mips";
    case Platform::ST20:    return "st20";
    default:                return "???";
    }
}


QString Util::getCallConvName(CallConv cc)
{
    switch (cc)
    {
    case CallConv::C:        return "stdc";
    case CallConv::Pascal:   return "pascal";
    case CallConv::ThisCall: return "thiscall";
    case CallConv::FastCall: return "fastcall";
    default:                 return "??";
    }
}
