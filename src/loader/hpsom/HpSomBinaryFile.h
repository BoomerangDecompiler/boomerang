#pragma once

/* * Copyright (C) 2000-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file HpSomBinaryFile.h
 * \brief This file contains the definition of the class HpSomBinaryFile.
 */

/***************************************************************************/ /**
 * Dependencies.
 ******************************************************************************/

#include "boom_base/BinaryFile.h"
#include "include/IBinarySymbols.h"
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

class HpSomBinaryFile : public QObject, public LoaderInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID LoaderInterface_iid)
	Q_INTERFACES(LoaderInterface)

public:
	HpSomBinaryFile(); // Constructor
	virtual ~HpSomBinaryFile();
	void initialize(IBoomerang *sys) override;

	void UnLoad() override;                // Unload the image
	void Close() override;                 // Close file opened with Open()
	bool PostLoad(void *handle) override;  // For archive files only
	LOAD_FMT GetFormat() const override;   // Get format i.e. LOADFMT_PALM
	MACHINE getMachine() const override;   // Get format i.e. MACHINE_HPRISC

	bool isLibrary() const;
	ADDRESS getImageBase() override;
	size_t getImageSize() override;

	// Specific to BinaryFile objects that implement a "global pointer"
	// Gets a pair of unsigned integers representing the address of %agp (first)
	// and the value for GLOBALOFFSET (unused for pa-risc)
	std::pair<ADDRESS, unsigned> GetGlobalPointerInfo();

	// Get a map from ADDRESS to const char*. This map contains the native
	// addresses and symbolic names of global data items (if any) which are
	// shared with dynamically linked libraries. Example: __iob (basis for
	// stdout).The ADDRESS is the native address of a pointer to the real dynamic data object.
	std::map<ADDRESS, const char *> *GetDynamicGlobalMap();

	//
	//  --  --  --  --  --  --  --  --  --  --  --
	//
	// Internal information
	// Dump headers, etc
	// virtual bool    DisplayDetails(const char* fileName, FILE* f = stdout);

	// Analysis functions
	ADDRESS GetMainEntryPoint() override;
	ADDRESS GetEntryPoint() override;

	//        bool        IsDynamicLinkedProc(ADDRESS wNative);
	//        ADDRESS     NativeToHostAddress(ADDRESS uNative);

	bool LoadFromMemory(QByteArray& data) override;
	int canLoad(QIODevice& dev) const override;

private:
	// Private method to get the start and length of a given subspace
	std::pair<ADDRESS, int> getSubspaceInfo(const char *ssname);

	unsigned char *m_pImage;     // Points to loaded image
	IBinarySymbolTable *Symbols; // Symbol table object
	std::set<ADDRESS> imports;   // Set of imported proc addr's
	class IBinaryImage *Image;
	// LoaderInterface interface

public:
	void processSymbols();
};
