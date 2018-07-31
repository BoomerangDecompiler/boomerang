#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


class QString;


/// Needed by both signature.h and frontend.h
enum class Platform
{
    GENERIC = 0,
    PENTIUM,
    SPARC,
    M68K,
    PARISC,
    PPC,
    MIPS,
    ST20,
    PlatformCount
};


enum class CallConv
{
    INVALID = 0,
    C,        ///< Standard C, no callee pop
    Pascal,   ///< callee pop
    ThisCall, ///< MSVC "thiscall": one parameter in register ecx
    FastCall, ///< MSVC fastcall convention ECX,EDX,stack, callee pop
    CallConvCount
};


namespace Util
{
    // ascii versions of platform, calling convention name
    const QString& getPlatformName(Platform plat);
    const QString& getCallConvName(CallConv cc);
}
