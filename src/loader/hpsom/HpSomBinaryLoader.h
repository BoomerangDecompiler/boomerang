#pragma once

/* * Copyright (C) 2000-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file HpSomBinaryLoader.h
 * \brief This file contains the definition of the class HpSomBinaryLoader.
 */

/***************************************************************************/ /**
 * Dependencies.
 ******************************************************************************/

#include "core/BinaryFile.h"
#include "db/IBinarySymbols.h"
#include <set>

struct import_entry
{
	int   name;
	short reserved2;
	Byte  type;
	Byte  reserved1;
};

struct export_entry
{
	int   next;
	int   name;
	int   value;
	int   size; // Also misc_info
	Byte  type;
	char  reserved1;
	short module_index;
};

struct space_dictionary_record
{
	unsigned name;
	unsigned flags;
	int      space_number;
	int      subspace_index;
	unsigned subspace_quantity;
	int      loader_fix_index;
	unsigned loader_fix_quantity;
	int      init_pointer_index;
	unsigned init_pointer_quantity;
};

struct subspace_dictionary_record
{
	int      space_index;
	unsigned flags;
	int      file_loc_init_value;
	unsigned initialization_length;
	unsigned subspace_start;
	unsigned subspace_length;
	unsigned alignment;
	unsigned name;
	int      fixup_request_index;
	int      fixup_request_quantity;
};

struct plt_record
{
	uint32_t value;    // Address in the library
	uint32_t r19value; // r19 value needed
};

struct symElem
{
	const char *name; // Simple symbol table entry
	ADDRESS    value;
};


class HpSomBinaryLoader : public IFileLoader
{
public:
	HpSomBinaryLoader();    // Constructor
	virtual ~HpSomBinaryLoader();

	/// @copydoc IFileLoader::initialize
	void initialize(IBinaryImage *image, IBinarySymbolTable *symbols) override;

	/// @copydoc IFileLoader::canLoad
	int canLoad(QIODevice& dev) const override;

	/// @copydoc IFileLoader::loadFromMemory
	bool loadFromMemory(QByteArray& data) override;

	/// @copydoc IFileLoader::unload
	void unload() override;

	/// @copydoc IFileLoader::close
	void close() override;

	/// @copydoc IFileLoader::getMainEntryPoint
	ADDRESS getMainEntryPoint() override;

	/// @copydoc IFileLoader::getEntryPoint
	ADDRESS getEntryPoint() override;


	/// @copydoc IFileLoader::getFormat
	LoadFmt getFormat() const override;

	/// @copydoc IFileLoader::getMachine
	Machine getMachine() const override;

	/// @copydoc IFileLoader::getImageBase
	ADDRESS getImageBase() override;

	/// @copydoc IFileLoader::getImageSize
	size_t getImageSize() override;

	bool isLibrary() const;

protected:
	// Analysis functions
	//        bool        IsDynamicLinkedProc(ADDRESS wNative);
	//        ADDRESS     NativeToHostAddress(ADDRESS uNative);
	bool postLoad(void *handle) override;  // For archive files only

private:
	/// Specific to BinaryFile objects that implement a "global pointer"
	/// Gets a pair of unsigned integers representing the address of %agp (first)
	/// and the value for GLOBALOFFSET (unused for pa-risc)
	std::pair<ADDRESS, unsigned> getGlobalPointerInfo();

	// Get a map from ADDRESS to const char*. This map contains the native
	// addresses and symbolic names of global data items (if any) which are
	// shared with dynamically linked libraries. Example: __iob (basis for
	// stdout).The ADDRESS is the native address of a pointer to the real dynamic data object.
	std::map<ADDRESS, const char *> *getDynamicGlobalMap();

	// Private method to get the start and length of a given subspace
	std::pair<ADDRESS, int> getSubspaceInfo(const char *ssname);

	Byte *m_loadedImage;            ///< Points to loaded image
	IBinarySymbolTable *m_symbols;  ///< Symbol table object
	std::set<ADDRESS> m_imports;    ///< Set of imported proc addr's
	IBinaryImage *m_image;

public:
	void processSymbols();
};
