/*
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file Win32BinaryFile.h
 * \brief This file contains the definition of the class Win32BinaryFile.
 */

#pragma once

#include "boomerang/core/BinaryFileFactory.h"
#include <string>

/**
 * This file contains the definition of the Win32BinaryLoader class.
 *
 * At present, there is no support for a symbol table. Win32 files do
 * not use dynamic linking, but it is possible that some files may
 * have debug symbols (in Microsoft Codeview or Borland formats),
 * and these may be implemented in the future. The debug info may
 * even be exposed as another pseudo section
 */

// Given a little endian value x, load its value assuming little endian order
// Note: must be able to take address of x
// Note: Unlike the LH macro in BinaryFile.h, the parameter is not a pointer
#define LMMH(x)																								  \
	((unsigned)((Byte *)(&x))[0] + ((unsigned)((Byte *)(&x))[1] << 8) + ((unsigned)((Byte *)(&x))[2] << 16) + \
	 ((unsigned)((Byte *)(&x))[3] << 24))
// With this one, x is a pointer to unsigned
#define LMMH2(x)																						   \
	((unsigned)((Byte *)(x))[0] + ((unsigned)((Byte *)(x))[1] << 8) + ((unsigned)((Byte *)(x))[2] << 16) + \
	 ((unsigned)((Byte *)(x))[3] << 24))

typedef struct  /* exe file header, just the signature really */
{
	Byte sigLo; /* .EXE signature: 0x4D 0x5A     */
	Byte sigHi;
} Header;

#pragma pack(push,1)

typedef struct
{
	Byte  sigLo;
	Byte  sigHi;
	SWord sigver;
	SWord cputype;
	SWord numObjects;
	DWord TimeDate;
	DWord Reserved1;
	DWord Reserved2;
	SWord NtHdrSize;
	SWord Flags;
	SWord Reserved3;
	Byte  LMajor;
	Byte  LMinor;
	DWord Reserved4;
	DWord Reserved5;
	DWord Reserved6;
	DWord EntrypointRVA;
	DWord Reserved7;
	DWord Reserved8;
	DWord Imagebase;
	DWord ObjectAlign;
	DWord FileAlign;
	SWord OSMajor;
	SWord OSMinor;
	SWord UserMajor;
	SWord UserMinor;
	SWord SubsysMajor;
	SWord SubsysMinor;
	DWord Reserved9;
	DWord ImageSize;
	DWord HeaderSize;
	DWord FileChecksum;
	SWord Subsystem;
	SWord DLLFlags;
	DWord StackReserveSize;
	DWord StackCommitSize;
	DWord HeapReserveSize;
	DWord HeapCommitSize;
	DWord Reserved10;
	DWord nInterestingRVASizes;
	DWord ExportTableRVA;
	DWord TotalExportDataSize;
	DWord ImportTableRVA;
	DWord TotalImportDataSize;
	DWord ResourceTableRVA;
	DWord TotalResourceDataSize;
	DWord ExceptionTableRVA;
	DWord TotalExceptionDataSize;
	DWord SecurityTableRVA;
	DWord TotalSecurityDataSize;
	DWord FixupTableRVA;
	DWord TotalFixupDataSize;
	DWord DebugTableRVA;
	DWord TotalDebugDirectories;
	DWord ImageDescriptionRVA;
	DWord TotalDescriptionSize;
	DWord MachineSpecificRVA;
	DWord MachineSpecificSize;
	DWord ThreadLocalStorageRVA;
	DWord TotalTLSSize;
} PEHeader;

typedef struct            // The real Win32 name of this struct is IMAGE_SECTION_HEADER
{
	char  ObjectName[8];  // Name
	DWord VirtualSize;
	DWord RVA;            // VirtualAddress
	DWord PhysicalSize;   // SizeOfRawData
	DWord PhysicalOffset; // PointerToRawData
	DWord Reserved1;      // PointerToRelocations
	DWord Reserved2;      // PointerToLinenumbers
	DWord Reserved3;      // WORD NumberOfRelocations; WORD NumberOfLinenumbers;
	DWord Flags;          // Characteristics
} PEObject;

typedef struct
{
	DWord originalFirstThunk; // 0 for end of array; also ptr to hintNameArray
	DWord preSnapDate;        // Time and date the import data was pre-snapped
	// or zero if not pre-snapped
	SWord verMajor;           // Major version number of dll being ref'd
	SWord verMinor;           // Minor "         "
	DWord name;               // RVA of dll name (asciz)
	DWord firstThunk;         // RVA of start of import address table (IAT)
} PEImportDtor;

typedef struct
{
	DWord flags;         // Reserved; 0
	DWord stamp;         // Time/date stamp export data was created
	SWord verMajor;      // Version number can be ...
	SWord verMinor;      //     ... set by user
	DWord name;          // RVA of the ascii string containing the name of the DLL
	DWord base;          // Starting ordinal number for exports in this image. Usually set to 1.
	DWord numEatEntries; // Number of entries in EAT (Export ADdress Table)
	DWord numNptEntries; // Number of entries in NPT (Name Pointer Table) (also #entries in the Ordinal Table)
	DWord eatRVA;        // RVA of the EAT
	DWord nptRVA;        // RVA of the NPT
	DWord otRVA;         // RVA of the OT
} PEExportDtor;
#pragma pack(pop)

/**
 * Class for loading Win32 binary ".exe" files
 */
class Win32BinaryLoader : public IFileLoader
{
public:
	Win32BinaryLoader();
	virtual ~Win32BinaryLoader();

	/// @copydoc IFileLoader::initialize
	void initialize(IBinaryImage *image, IBinarySymbolTable *symbols) override;

	/// @copydoc IFileLoader::canLoad
	int canLoad(QIODevice& fl) const override;

	/// @copydoc IFileLoader::loadFromMemory
	bool loadFromMemory(QByteArray& arr) override;

	/// @copydoc IFileLoader::unload
	void unload() override;

	/// @copydoc IFileLoader::close
	void close() override;

	/// @copydoc IFileLoader::getFormat
	LoadFmt getFormat() const override;

	/// @copydoc IFileLoader::getMachine
	Machine getMachine() const override;

	/// @copydoc IFileLoader::getMainEntryPoint
	   Address getMainEntryPoint() override;

	/// @copydoc IFileLoader::getEntryPoint
	   Address getEntryPoint() override;

	/// @copydoc IFileLoader::getImageBase
	   Address getImageBase() override;

	/// @copydoc IFileLoader::getImageSize
	size_t getImageSize() override;

public:
	/// @copydoc IFileLoader::isJumpToAnotherAddr
	   Address isJumpToAnotherAddr(Address uNative) override;

	/// @copydoc IFileLoader::displayDetails
	bool displayDetails(const char *fileName, FILE *f = stdout) override;

	/// @copydoc IFileLoader::hasDebugInfo
	bool hasDebugInfo() override { return m_hasDebugInfo; }

	bool isLibrary() const;

	DWord getDelta();

protected:
	/// @copydoc IFileLoader::postLoad
	bool postLoad(void *handle) override;

	int win32Read2(short *ps) const; // Read 2 bytes from native addr
	int win32Read4(int *pi) const;   // Read 4 bytes from native addr

public:
	bool isStaticLinkedLibProc(Address uNative);

	bool isMinGWsAllocStack(Address uNative);
	bool isMinGWsFrameInit(Address uNative);
	bool isMinGWsFrameEnd(Address uNative);
	bool isMinGWsCleanupSetup(Address uNative);
	bool isMinGWsMalloc(Address uNative);

protected:
	void processIAT();
	void readDebugData(QString exename);

private:
	/// Find names for jumps to IATs
	void findJumps(Address curr);

	Header *m_pHeader;                    ///< Pointer to header
	PEHeader *m_pPEHeader;                ///< Pointer to pe header
	int m_cbImage;                        ///< Size of image
	int m_cReloc;                         ///< Number of relocation entries
	char *m_base;                         ///< Beginning of the loaded image
	bool m_hasDebugInfo;
	bool m_mingw_main;
	IBinaryImage *m_image;
	IBinarySymbolTable *m_symbols;
};
