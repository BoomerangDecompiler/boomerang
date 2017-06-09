#pragma once

/*
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file MachOBinaryFile.h
 * \brief This file contains the definition of the class MachOBinaryFile.
 */

#include "boom_base/BinaryFile.h"
#include <string>
#include <vector>

/**
 * This file contains the definition of the MachOBinaryFile class, and some
 * other definitions specific to the Mac OS-X version of the BinaryFile object
 * This is my bare bones implementation of a Mac OS-X binary loader.
 */

// Given a little endian value x, load its value assuming big endian order
// Note: must be able to take address of x
// Note: Unlike the LH macro in BinaryFile.h, the parameter is not a pointer
#define _BMMH(x)																							  \
	((unsigned)((Byte *)(&x))[3] + ((unsigned)((Byte *)(&x))[2] << 8) + ((unsigned)((Byte *)(&x))[1] << 16) + \
	 ((unsigned)((Byte *)(&x))[0] << 24))
// With this one, x IS a pounsigneder
#define _BMMH2(x)																						   \
	((unsigned)((Byte *)(x))[3] + ((unsigned)((Byte *)(x))[2] << 8) + ((unsigned)((Byte *)(x))[1] << 16) + \
	 ((unsigned)((Byte *)(x))[0] << 24))

#define _BMMHW(x)    (((unsigned)((Byte *)(&x))[1]) + ((unsigned)((Byte *)(&x))[0] << 8))

struct mach_header;

class MachOBinaryFile : public IFileLoader, public ObjcAccessInterface
{
public:
	MachOBinaryFile();                   // Default constructor
	virtual ~MachOBinaryFile();          // Destructor
	void initialize(IBoomerang *sys) override;
	bool Open(const char *sName);        // Open the file for r/w; ???
	void close() override;               // Close file opened with Open()
	void unload() override;              // Unload the image
	LOAD_FMT getFormat() const override; // Get format (i.e. LOADFMT_MACHO)
	MACHINE getMachine() const override; // Get machine (i.e. MACHINE_PPC)
	bool isLibrary() const;
	ADDRESS getImageBase() override;
	size_t getImageSize() override;

	ADDRESS getMainEntryPoint() override;
	ADDRESS getEntryPoint() override;
	DWord getDelta();

	//
	//        --        --        --        --        --        --        --        --        --
	//
	// Internal information
	// Dump headers, etc
	bool displayDetails(const char *fileName, FILE *f = stdout) override;

protected:
	int machORead2(short *ps) const; // Read 2 bytes from native addr
	int machORead4(int *pi) const;   // Read 4 bytes from native addr

	// void *            BMMH(void *x);
	//    char *              BMMH(char *x);
	//    const char *        BMMH(const char *x);
	// unsigned int        BMMH(long int & x);
	int32_t BMMH(int32_t x);
	uint32_t BMMH(uint32_t x);
	unsigned short BMMHW(unsigned short x);

public:
	std::map<QString, ObjcModule>& getObjcModules() override  { return modules; }

	bool loadFromMemory(QByteArray& data) override;
	int canLoad(QIODevice& dev) const override;

private:
	bool postLoad(void *handle) override; // Called after archive member loaded
	void findJumps(ADDRESS curr);         // Find names for jumps to IATs

	char *base;                           // Beginning of the loaded image
	ADDRESS entrypoint, loaded_addr;
	unsigned loaded_size;
	MACHINE machine;
	bool swap_bytes;
	std::map<QString, ObjcModule> modules;
	std::vector<struct section> sections;
	class IBinaryImage *Image;
	class IBinarySymbolTable *Symbols;
};
