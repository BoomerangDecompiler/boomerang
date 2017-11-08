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


#include "boomerang/loader/IBinaryFile.h"
#include "boomerang/core/Plugin.h"

class IBinaryImage;
class IBinarySymbolTable;


/**
 * Abstract class for loading binary and text files for analysis and decompilation.
 * The derived classes define the actual functionality of loading files and are
 * implemented as plugins.
 */
class IFileLoader
{
public:
    IFileLoader()          = default;
    virtual ~IFileLoader() = default;

    /**
     * Initialize this loader.
     * \param image   Binary image to load this file into.
     * \param symbols Symbol table to fill
     */
    virtual void initialize(IBinaryImage *image, IBinarySymbolTable *symbols) = 0;

    /// Test if this file can be loaded by this loader.
    /// This method does not necessarily need to read the whole file,
    /// only the part needed to understand the file format
//    virtual bool canLoadFile(const QString& path) const = 0;

    /// Checks if the file can be loaded by this loader.
    /// If the file can be loaded, the function returns a score ( > 0)
    /// corresponding to the number of bytes checked.
    /// If the file cannot be loaded, this function returns 0.
    virtual int canLoad(QIODevice& data) const = 0;

    /// Load the file with path \p path into memory.
//    virtual IBinaryFile* loadFromFile(const QString& path) = 0;

    /// Load the file from an already existing buffer.
    /// \note \p data cannot be const
    /// \returns true for a good load
    virtual bool loadFromMemory(QByteArray& data) = 0;

    /// Unload the file.
    /// Cleans up and unloads the binary image.
    virtual void unload() = 0;

    /// Close file opened with Open()
    virtual void close() = 0;

    /// Get the format (e.g. LOADFMT_ELF)
    virtual LoadFmt getFormat() const = 0;

    /// Get the expected machine (e.g. MACHINE_PENTIUM)
    virtual Machine getMachine() const = 0;

    /// \returns the address of main()/WinMain() etc.
    virtual Address getMainEntryPoint() = 0;

    /// \returns the "real" entry point, ie where execution of the program begins
    virtual Address getEntryPoint() = 0;

public:
    /// Relocation functions
    virtual bool isRelocationAt(Address /*uNative*/) { return false; }

    /// \returns the target of the jmp/jXX instruction at address \p addr
    virtual Address getJumpTarget(Address addr) const { Q_UNUSED(addr); return Address::INVALID; }
    virtual bool hasDebugInfo() const { return false; }
};

typedef Plugin<IFileLoader> LoaderPlugin;
