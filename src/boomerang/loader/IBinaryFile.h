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


#include "boomerang/util/Address.h"
#include "boomerang/util/Types.h"

#include <QByteArray>

typedef QByteArray IFileData;

class IBoomerang;
class QIODevice;

/// This enum allows a sort of run time type identification, without using
/// compiler specific features
enum class LoadFmt : uint8_t
{
    ELF,
    PE,
    PALM,
    PAR,
    EXE,
    MACHO,
    LX,
    COFF
};

/// determines which instruction set to use
enum class Machine : uint8_t
{
    UNKNOWN = 0,
    PENTIUM,
    SPARC,
    HPRISC,
    PALM,
    PPC,
    ST20,
    MIPS,
    M68K
};
