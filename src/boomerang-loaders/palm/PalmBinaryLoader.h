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


#include "boomerang/loader/IFileLoader.h"

#include <QtCore/QObject>


/**
 * Loader for Palm Pilot .prc files.
 */
class PalmBinaryLoader : public IFileLoader
{
public:
    PalmBinaryLoader();
    virtual ~PalmBinaryLoader();

public:
    /// \copydoc IFileLoader::initialize
    void initialize(IBinaryImage *image, IBinarySymbolTable *table) override;

    /// \copydoc IFileLoader::canLoad
    int canLoad(QIODevice& dev) const override;

    /// \copydoc IFileLoader::loadFromMemory
    bool loadFromMemory(QByteArray& data) override;

    /// \copydoc IFileLoader::unload
    void unload() override;

    /// \copydoc IFileLoader::close
    void close() override;

    /// \copydoc IFileLoader::getFormat
    LoadFmt getFormat() const override;

    /// \copydoc IFileLoader::getMachine
    Machine getMachine() const override;

    /// \copydoc IFileLoader::getMainEntryPoint
    virtual Address getMainEntryPoint() override;

    /// \copydoc IFileLoader::getEntryPoint
    virtual Address getEntryPoint() override;

    // Analysis functions
    bool isLibrary() const;

private:
    // Specific to BinaryFile objects that implement a "global pointer"
    // Gets a pair of unsigned integers representing the address of %agp (first) and the value for GLOBALOFFSET (second)
    std::pair<Address, unsigned> getGlobalPointerInfo();

    // Palm specific calls

    // Get the ID number for this application. It's possible that the app uses
    // this number internally, so this needs to be used in the final make
    int getAppID() const;

    // Generate binary files for non code and data sections
    void generateBinFiles(const QString& path) const;


    void addTrapSymbols();

private:
    unsigned char *m_image; ///< Points to loaded image
    unsigned char *m_data;  ///< Points to data

    /// Offset from start of data to where register a5 should be initialised to
    unsigned int m_sizeBelowA5;

    IBinaryImage *m_binaryImage;
    IBinarySymbolTable *m_symbols;
};
