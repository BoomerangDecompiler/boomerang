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

#include "BinaryFile.h"
#include <string>

/**
 * This file contains the definition of the Win32BinaryFile class, and some
 * other definitions specific to the exe version of the BinaryFile object
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
	DWord name;          // RVA of the ascii string containing the name of
	//     the DLL
	DWord base;          // Starting ordinal number for exports in this
	//    image. Usually set to 1.
	DWord numEatEntries; // Number of entries in EAT (Export ADdress Table)
	DWord numNptEntries; // Number of entries in NPT (Name Pointer Table)
	//    (also #entries in the Ordinal Table)
	DWord eatRVA;        // RVA of the EAT
	DWord nptRVA;        // RVA of the NPT
	DWord otRVA;         // RVA of the OT
} PEExportDtor;
#pragma pack(pop)

class Win32BinaryFile : public QObject, public LoaderInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID LoaderInterface_iid)
	Q_INTERFACES(LoaderInterface)
public:
	Win32BinaryFile();
	virtual ~Win32BinaryFile();
	void Close() override;                 //!< Close file opened with Open()
	void UnLoad() override;                //!< Unload the image
	LOAD_FMT GetFormat() const override;   //!< Get format (i.e.LOADFMT_Win32)
	MACHINE getMachine() const override;   //!< Get machine (i.e. MACHINE_Pentium)
	ADDRESS getImageBase() override;
	size_t getImageSize() override;
	bool isLibrary() const;

	ADDRESS GetMainEntryPoint() override;
	ADDRESS GetEntryPoint() override;
	DWord getDelta();

	//
	//        --        --        --        --        --        --        --        --        --
	//
	// Internal information
	// Dump headers, etc
	bool DisplayDetails(const char *fileName, FILE *f = stdout) override;
	bool LoadFromMemory(QByteArray& arr) override;
	int canLoad(QIODevice& fl) const override;

protected:
	int win32Read2(short *ps) const; // Read 2 bytes from native addr
	int win32Read4(int *pi) const;   // Read 4 bytes from native addr

public:

	bool IsStaticLinkedLibProc(ADDRESS uNative);
	ADDRESS IsJumpToAnotherAddr(ADDRESS uNative) override;

	bool IsMinGWsAllocStack(ADDRESS uNative);
	bool IsMinGWsFrameInit(ADDRESS uNative);
	bool IsMinGWsFrameEnd(ADDRESS uNative);
	bool IsMinGWsCleanupSetup(ADDRESS uNative);
	bool IsMinGWsMalloc(ADDRESS uNative);


	bool hasDebugInfo()  override { return haveDebugInfo; }
	void initialize(IBoomerang *sys) override;

protected:
	void processIAT();
	void readDebugData(QString exename);

private:
	bool PostLoad(void *handle) override; // Called after archive member loaded
	void findJumps(ADDRESS curr);         // Find names for jumps to IATs

	Header *m_pHeader;                    // Pointer to header
	PEHeader *m_pPEHeader;                // Pointer to pe header
	int m_cbImage;                        // Size of image
	int m_cReloc;                         // Number of relocation entries
	DWord *m_pRelocTable;                 // The relocation table
	char *base;                           // Beginning of the loaded image
	// Map from address of dynamic pointers to library procedure names:
	bool haveDebugInfo;
	bool mingw_main;
	class IBinaryImage *Image;
	class IBinarySymbolTable *Symbols;
};
