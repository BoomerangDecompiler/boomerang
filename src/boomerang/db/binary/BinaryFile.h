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


#include <memory>


class QByteArray;
class BinaryImage;
class BinarySymbolTable;
class IFileLoader;


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
    INVALID = 0xFF,
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


class BinaryFile
{
public:
    BinaryFile(const QByteArray& rawData, IFileLoader *loader);

public:
    BinaryImage *getImage();
    const BinaryImage *getImage() const;

    BinarySymbolTable *getSymbols();
    const BinarySymbolTable *getSymbols() const;

public:
    LoadFmt getFormat() const;
    Machine getMachine() const;

    Address getEntryPoint() const;
    Address getMainEntryPoint() const;

    bool isRelocationAt(Address addr) const;
    Address getJumpTarget(Address addr) const;

    bool hasDebugInfo() const;

private:
    std::unique_ptr<BinaryImage> m_image;
    std::unique_ptr<BinarySymbolTable> m_symbols;

    IFileLoader *m_loader = nullptr;
};
