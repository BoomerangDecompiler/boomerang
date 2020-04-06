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


#include "boomerang/db/binary/BinaryFile.h"
#include "boomerang/db/binary/BinaryImage.h"


class Project;

class QIODevice;


/**
 * Abstract class for loading binary and text files for analysis and decompilation.
 * The derived classes define the actual functionality of loading files and are
 * implemented as plugins.
 */
class BOOMERANG_API IFileLoader
{
public:
    IFileLoader(Project *) {}
    virtual ~IFileLoader() = default;

public:
    /**
     * Initialize this loader.
     * \param image   Binary file to load this executable file into.
     * \param symbols Symbol table to fill
     */
    virtual void initialize(BinaryFile *file, BinarySymbolTable *symbols) = 0;

    /// Checks if the file can be loaded by this loader.
    /// If the file can be loaded, the function returns a score ( > 0)
    /// corresponding to the number of bytes checked.
    /// If the file cannot be loaded, this function returns 0.
    virtual int canLoad(QIODevice &data) const = 0;

    /// Load the file with path \p path into memory.
    virtual bool loadFromFile(BinaryFile *file)
    {
        initialize(file, file->getSymbols());
        return loadFromMemory(file->getImage()->getRawData());
    };

    /// Load the file from an already existing buffer.
    /// \note \p data cannot be const
    /// \returns true for a good load
    virtual bool loadFromMemory(QByteArray &data) = 0;

    /// Unload the file.
    /// Cleans up and unloads the binary image.
    virtual void unload() = 0;

    /// Close file opened with Open()
    virtual void close() = 0;

    /// Get the format (e.g. LoadFmt::ELF)
    virtual LoadFmt getFormat() const = 0;

    /// Get the expected machine (e.g. MACHINE::X86)
    virtual Machine getMachine() const = 0;

    /// \returns the address of main()/WinMain() etc.
    virtual Address getMainEntryPoint() = 0;

    /// \returns the "real" entry point, ie where execution of the program begins
    virtual Address getEntryPoint() = 0;

public:
    /// Relocation functions
    virtual bool isRelocationAt(Address addr)
    {
        Q_UNUSED(addr);
        return false;
    }

    /// \returns the target of the jmp/jXX instruction at address \p addr.
    /// If there is no jump at address \p addr, returns Address::INVALID.
    virtual Address getJumpTarget(Address addr) const
    {
        Q_UNUSED(addr);
        return Address::INVALID;
    }
    virtual bool hasDebugInfo() const { return false; }
};
