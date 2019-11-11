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


class BinaryImage;
class BinarySymbolTable;
class IFileLoader;

class QByteArray;


/// This enum allows a sort of run time type identification, without using
/// compiler specific features
enum class LoadFmt : uint8_t
{
    INVALID = 0xFF,
    ELF     = 0,
    PE,
    EXE,
    MACHO,
    LX,
    ST20
};

/// determines which instruction set to use
enum class Machine : uint8_t
{
    INVALID = 0xFF,
    UNKNOWN = 0,
    X86,
    SPARC,
    PPC,
    ST20
};


/**
 * This class provides file-format independent access to loaded binary files.
 */
class BOOMERANG_API BinaryFile
{
public:
    BinaryFile(const QByteArray &rawData, IFileLoader *loader);
    BinaryFile(const BinaryFile &) = delete;
    BinaryFile(BinaryFile &&)      = delete;

    ~BinaryFile();

    BinaryFile &operator=(const BinaryFile &) = delete;
    BinaryFile &operator=(BinaryFile &&) = delete;

public:
    BinaryImage *getImage();
    const BinaryImage *getImage() const;

    BinarySymbolTable *getSymbols();
    const BinarySymbolTable *getSymbols() const;

public:
    /// \returns the file format of the binary file.
    LoadFmt getFormat() const;

    /// \returns the primary instruction set used in the binary file.
    Machine getMachine() const;

    /// \returns the address of the entry point. If the address is not known (yet),
    /// returns Address::INVALID (and not Address::ZERO because Address::ZERO is a
    /// potentially valid start address)
    Address getEntryPoint() const;

    /// \returns the address of main()/WinMain(), if found, else Address::INVALID
    Address getMainEntryPoint() const;

    /// \returns true if \p addr is the destination of a relocated symbol.
    bool isRelocationAt(Address addr) const;

    /// \returns the destination of a jump at address \p addr, taking relocation into account
    Address getJumpTarget(Address addr) const;

    /// \note not yet implemented.
    bool hasDebugInfo() const;

    /// \returns the default instruction size of the instruction set in the binary,
    /// or 0 if not known.
    int getBitness() const;

    void setBitness(int bitness);

private:
    std::unique_ptr<BinaryImage> m_image;
    std::unique_ptr<BinarySymbolTable> m_symbols;

    IFileLoader *m_loader = nullptr;
    int m_bitness         = 0;
};
