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


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/ifc/IFileLoader.h"

#include <string>


/// Class for loading ST20 ".bin" files.
class BOOMERANG_PLUGIN_API ST20BinaryLoader : public IFileLoader
{
public:
    ST20BinaryLoader(Project *project);
    virtual ~ST20BinaryLoader();

public:
    /// \copydoc IFileLoader::initialize
    void initialize(BinaryFile *file, BinarySymbolTable *symbols) override;

    /// \copydoc IFileLoader::canLoad
    int canLoad(QIODevice &fl) const override;

    /// \copydoc IFileLoader::loadFromMemory
    bool loadFromMemory(QByteArray &arr) override;

    /// \copydoc IFileLoader::unload
    void unload() override;

    /// \copydoc IFileLoader::close
    void close() override;

    /// \copydoc IFileLoader::getFormat
    LoadFmt getFormat() const override;

    /// \copydoc IFileLoader::getMachine
    Machine getMachine() const override;

    /// \copydoc IFileLoader::getMainEntryPoint
    Address getMainEntryPoint() override;

    /// \copydoc IFileLoader::getEntryPoint
    Address getEntryPoint() override;

public:
    /// \copydoc IFileLoader::getJumpTarget
    Address getJumpTarget(Address addr) const override;

    /// \copydoc IFileLoader::hasDebugInfo
    bool hasDebugInfo() const override { return false; }

private:
    char *m_image;

    BinaryImage *m_binaryImage;
    BinarySymbolTable *m_symbols;
};
