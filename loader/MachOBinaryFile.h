/*
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/* File: MachOBinaryFile.h
 * Desc: This file contains the definition of the class MachOBinaryFile.
*/

#ifndef __MACHOBINARYFILE_H__
#define __MACHOBINARYFILE_H_

#include "BinaryFile.h"
#include <string>
#include <vector>

/* $Revision$
 * This file contains the definition of the MachOBinaryFile class, and some
 * other definitions specific to the Mac OS-X version of the BinaryFile object
*/
/* This is my bare bones implementation of a Mac OS-X binary loader.
 * 13 Jan 05 - Trent: Created
 */

// Given a little endian value x, load its value assuming big endian order
// Note: must be able to take address of x
// Note: Unlike the LH macro in BinaryFile.h, the paraeter is not a pointer
#define BMMH(x) ((unsigned)((Byte *)(&x))[3] + ((unsigned)((Byte *)(&x))[2] << 8) + \
	((unsigned)((Byte *)(&x))[1] << 16) + ((unsigned)((Byte *)(&x))[0] << 24))
// With this one, x IS a pounsigneder
#define BMMH2(x) ((unsigned)((Byte *)(x))[3] + ((unsigned)((Byte *)(x))[2] << 8) + \
	((unsigned)((Byte *)(x))[1] << 16) + ((unsigned)((Byte *)(x))[0] << 24))

//#ifdef WIN32
#pragma pack(1)
//#endif

typedef unsigned long cpu_type_t;     // I guessed
typedef unsigned long cpu_subtype_t;  // I guessed
typedef unsigned long vm_prot_t;   // I guessed

#include "nlist.h"
#include "macho-apple.h"

//#ifdef WIN32
#pragma pack(4)
//#endif

class MachOBinaryFile : public BinaryFile
{
public:
				MachOBinaryFile();				// Default constructor
  virtual		~MachOBinaryFile();				// Destructor
  virtual bool	Open(const char* sName);		// Open the file for r/w; ???
  virtual void	Close();						// Close file opened with Open()
  virtual void	UnLoad();						// Unload the image
  virtual LOAD_FMT GetFormat() const;			// Get format (i.e.
												// LOADFMT_MACHO)
  virtual MACHINE GetMachine() const;			// Get machine (i.e.
												// MACHINE_PPC)
  virtual const char *getFilename() const { return m_pFileName; }
  virtual bool isLibrary() const;
  virtual std::list<const char *> getDependencyList();
  virtual ADDRESS getImageBase();
  virtual size_t getImageSize();

  virtual std::list<SectionInfo*>& GetEntryPoints(const char* pEntry = "main");
  virtual ADDRESS GetMainEntryPoint();
  virtual ADDRESS GetEntryPoint();
  DWord getDelta();
  virtual char* SymbolByAddress(ADDRESS dwAddr); // Get sym from addr
  virtual ADDRESS GetAddressByName(const char* name,
	bool bNoTypeOK = false);					// Find addr given name	 
  virtual void AddSymbol(ADDRESS uNative, const char *pName);

//
//		--		--		--		--		--		--		--		--		--
//
		// Internal information
		// Dump headers, etc
virtual bool	DisplayDetails(const char* fileName, FILE* f = stdout);

protected:

		int machORead2(short *ps) const; // Read 2 bytes from native addr
		int machORead4(int *pi) const;	 // Read 4 bytes from native addr

public:

virtual int readNative1(ADDRESS a);         // Read 1 bytes from native addr
virtual int readNative2(ADDRESS a);			// Read 2 bytes from native addr
virtual int readNative4(ADDRESS a);			// Read 4 bytes from native addr
virtual QWord readNative8(ADDRESS a);	// Read 8 bytes from native addr
virtual float readNativeFloat4(ADDRESS a);	// Read 4 bytes as float
virtual double readNativeFloat8(ADDRESS a); // Read 8 bytes as float

virtual const char *GetDynamicProcName(ADDRESS uNative);

	virtual std::map<ADDRESS, std::string> &getSymbols() { return m_SymA; }

  protected:
	virtual bool  RealLoad(const char* sName); // Load the file; pure virtual

  private:

		bool	PostLoad(void* handle); // Called after archive member loaded
		void	findJumps(ADDRESS curr);// Find names for jumps to IATs

		struct mach_header header;      // The Mach-O header
		char *	base;					// Beginning of the loaded image
		const char *m_pFileName;
        ADDRESS entrypoint, loaded_addr;
        unsigned loaded_size;
        std::map<ADDRESS, std::string> m_SymA, dlprocs;
};

//#ifdef WIN32
#pragma pack()
//#endif
#endif			// ifndef __WIN32BINARYFILE_H__
