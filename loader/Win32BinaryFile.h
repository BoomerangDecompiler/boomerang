/*
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/* File: Win32BinaryFile.h
 * Desc: This file contains the definition of the class Win32BinaryFile.
*/

#ifndef __WIN32BINARYFILE_H__
#define __WIN32BINARYFILE_H_

#include "BinaryFile.h"
#include <string>

/* $Revision$
 * This file contains the definition of the Win32BinaryFile class, and some
 * other definitions specific to the exe version of the BinaryFile object
*/
/* At present, there is no support for a symbol table. Win32 files do
		not use dynamic linking, but it is possible that some files may
		have debug symbols (in Microsoft Codeview or Borland formats),
		and these may be implemented in the future. The debug info may
		even be exposed as another pseudo section
 * 02 Jun 00 - Mike: Added LMMH for 32 bit endianness conversions
 * 16 Apr 01 - Brian: Removed redefinition of the LH macro. LH is now
 *			   defined in BinaryFile.h.
 */

// Given a little endian value x, load its value assuming little endian order
// Note: must be able to take address of x
// Note: Unlike the LH macro in BinaryFile.h, the paraeter is not a pointer
#define LMMH(x) ((unsigned)((Byte *)(&x))[0] + ((unsigned)((Byte *)(&x))[1] << 8) + \
	((unsigned)((Byte *)(&x))[2] << 16) + ((unsigned)((Byte *)(&x))[3] << 24))
// With this one, x IS a pounsigneder
#define LMMH2(x) ((unsigned)((Byte *)(x))[0] + ((unsigned)((Byte *)(x))[1] << 8) + \
	((unsigned)((Byte *)(x))[2] << 16) + ((unsigned)((Byte *)(x))[3] << 24))

typedef struct {				/* exe file header, just the signature really */
		 Byte	sigLo;			/* .EXE signature: 0x4D 0x5A	 */
		 Byte	sigHi;
} Header;

//#ifdef WIN32
#pragma pack(1)
//#endif

typedef struct {
  Byte sigLo;
  Byte sigHi;
  SWord sigver;
  SWord cputype;
  SWord numObjects;
  DWord TimeDate;
  DWord Reserved1;
  DWord Reserved2;
  SWord NtHdrSize;
  SWord Flags;
  SWord Reserved3;
  Byte LMajor;
  Byte LMinor;
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

typedef struct {
  char ObjectName[8];
  DWord VirtualSize;
  DWord RVA;
  DWord PhysicalSize;
  DWord PhysicalOffset;
  DWord Reserved1;
  DWord Reserved2;
  DWord Reserved3;
  DWord Flags;
} PEObject;

typedef struct {
	DWord originalFirstThunk; // 0 for end of array; also ptr to hintNameArray
	DWord preSnapDate;		// Time and date the import data was pre-snapped
							// or zero if not pre-snapped
	SWord verMajor;			// Major version number of dll being ref'd
	SWord verMinor;			// Minor "		 "
	DWord name;				// RVA of dll name (asciz)
	DWord firstThunk;		// RVA of start of import address table (IAT)
} PEImportDtor;

typedef struct {
	DWord	flags;			// Reserved; 0
	DWord	stamp;			// Time/date stamp export data was created
	SWord	verMajor;		// Version number can be ...
	SWord	verMinor;		//	 ... set by user
	DWord	name;			// RVA of the ascii string containing the name of
							//	 the DLL
	DWord	base;			// Starting ordinal number for exports in this
							//	image. Usually set to 1.
	DWord	numEatEntries;	// Number of entries in EAT (Export ADdress Table)
	DWord	numNptEntries;	// Number of entries in NPT (Name Pointer Table)
							//	(also #entries in the Ordinal Table)
	DWord	eatRVA;			// RVA of the EAT
	DWord	nptRVA;			// RVA of the NPT
	DWord	otRVA;			// RVA of the OT
} PEExportDtor;

//#ifdef WIN32
#pragma pack(4)
//#endif

class Win32BinaryFile : public BinaryFile
{
public:
					Win32BinaryFile();				// Default constructor
virtual				~Win32BinaryFile();				// Destructor
virtual bool		Open(const char* sName);		// Open the file for r/w; ???
virtual void		Close();						// Close file opened with Open()
virtual void		UnLoad();						// Unload the image
virtual LOAD_FMT	GetFormat() const;			// Get format (i.e.
												// LOADFMT_Win32)
virtual MACHINE		GetMachine() const;			// Get machine (i.e.
												// MACHINE_Pentium)
virtual const char*	getFilename() const { return m_pFileName; }
virtual bool		isLibrary() const;
virtual std::list<const char *> getDependencyList();
virtual ADDRESS		getImageBase();
virtual size_t		getImageSize();

virtual std::list<SectionInfo*>& GetEntryPoints(const char* pEntry = "main");
virtual ADDRESS		GetMainEntryPoint();
virtual ADDRESS		GetEntryPoint();
		DWord		getDelta();
virtual const char* SymbolByAddress(ADDRESS dwAddr); // Get sym from addr
virtual ADDRESS		GetAddressByName(const char* name, bool bNoTypeOK = false);		// Find addr given name	 
virtual void		AddSymbol(ADDRESS uNative, const char *pName);
		void		dumpSymbols();					// For debugging

//
//		--		--		--		--		--		--		--		--		--
//
		// Internal information
		// Dump headers, etc
virtual bool		DisplayDetails(const char* fileName, FILE* f = stdout);

protected:

		int			win32Read2(short *ps) const; // Read 2 bytes from native addr
		int			win32Read4(int *pi) const;	 // Read 4 bytes from native addr

public:

virtual int			readNative1(ADDRESS a);         // Read 1 bytes from native addr
virtual int			readNative2(ADDRESS a);			// Read 2 bytes from native addr
virtual int			readNative4(ADDRESS a);			// Read 4 bytes from native addr
virtual QWord		readNative8(ADDRESS a);		// Read 8 bytes from native addr
virtual float		readNativeFloat4(ADDRESS a);	// Read 4 bytes as float
virtual double		readNativeFloat8(ADDRESS a); // Read 8 bytes as float

virtual bool		IsDynamicLinkedProcPointer(ADDRESS uNative);
virtual const char *GetDynamicProcName(ADDRESS uNative);

virtual std::map<ADDRESS, std::string> &getSymbols() { return dlprocptrs; }

protected:
virtual bool  		RealLoad(const char* sName); // Load the file; pure virtual

private:

		bool		PostLoad(void* handle); // Called after archive member loaded
		void		findJumps(ADDRESS curr);// Find names for jumps to IATs

		Header* 	m_pHeader;				// Pointer to header
		PEHeader* 	m_pPEHeader;			// Pointer to pe header
		int			m_cbImage;				// Size of image
		int			m_cReloc;				// Number of relocation entries
		DWord*		m_pRelocTable;			// The relocation table
		char *		base;					// Beginning of the loaded image
		// Map from address of dynamic pointers to library procedure names:
		std::map<ADDRESS, std::string> dlprocptrs;
		const char	*m_pFileName;

};

//#ifdef WIN32
#pragma pack()
//#endif
#endif			// ifndef __WIN32BINARYFILE_H__
