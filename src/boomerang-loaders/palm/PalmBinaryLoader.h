#pragma once

/*
 * Copyright (C) 2000-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file PalmBinaryLoader.h
 * \brief This file contains the definition of the class PalmBinaryLoader.
 */

/***************************************************************************/ /**
 * Dependencies.
 ******************************************************************************/

#include "boomerang/core/BinaryFileFactory.h"
#include <QtCore/QObject>

/**
 * This class loads a Palm Pilot .prc file.
 */
class PalmBinaryLoader : public IFileLoader
{
public:
	PalmBinaryLoader(); // Constructor
	virtual ~PalmBinaryLoader();

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

	/// \copydoc IFileLoader::getImageBase
	   Address getImageBase() override;

	/// \copydoc IFileLoader::getImageSize
	size_t getImageSize() override;

	// Analysis functions
	//    bool        IsDynamicLinkedProc(ADDRESS wNative);
	//    ADDRESS     NativeToHostAddress(ADDRESS uNative);
	bool isLibrary() const;

	/// \copydoc IFileLoader::postLoad
	bool postLoad(void *handle) override;  // For archive files only

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

	unsigned char *m_pImage; ///< Points to loaded image
	unsigned char *m_pData;  ///< Points to data

	/// Offset from start of data to where register a5 should be initialised to
	unsigned int m_sizeBelowA5;

	IBinaryImage *m_image;
	IBinarySymbolTable *m_symbols;
};
