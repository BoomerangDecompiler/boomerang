/*
 * Copyright (C) 1997-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/* File: BinaryFile.cpp
 * Desc: This file contains the implementation of the class BinaryFile
 * 
 * This file implements the abstract BinaryFile class.
 * All classes derived from this class must implement the Load()
 *function.
*/

/*
 *	MVE 30/9/97 Created
 * 21 Apr 02 - Mike: mods for boomerang
 * 03 Jun 02 - Trent: if WIN32, no dynamic linking
 * 14 Jun 02 - Mike: Fixed a bug where Windows programs chose the Exe loader
*/

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "BinaryFile.h"
#include "ElfBinaryFile.h"
#include "Win32BinaryFile.h"
#include "PalmBinaryFile.h"
#include "HpSomBinaryFile.h"
#include "ExeBinaryFile.h"
#include <iostream>
#include <stdio.h>
#include <assert.h>
#ifndef WIN32
#include <dlfcn.h>
#endif


BinaryFile::BinaryFile(bool bArch /*= false*/)
{
	m_bArchive = bArch;			// Remember whether an archive member
	m_iNumSections = 0;			// No sections yet
	m_pSections = 0;			// No section data yet
}

BinaryFile *BinaryFile::Load( const char *sName )
{
	BinaryFile *pBF = BinaryFile::getInstanceFor( sName );
	if( pBF == NULL ) {
		std::cerr << "unrecognised binary file format.\n";
		return NULL;
	}
	if( pBF->RealLoad( sName ) == 0 ) {
		fprintf( stderr, "Loading '%s' failed\n", sName );
		delete pBF;
		return NULL;
	}
	pBF->getTextLimits();
	return pBF;
}

#define TESTMAGIC2(buf,off,a,b)		(buf[off] == a && buf[off+1] == b)
#define TESTMAGIC4(buf,off,a,b,c,d) (buf[off] == a && buf[off+1] == b && \
									 buf[off+2] == c && buf[off+3] == d)

// Declare a pointer to a constructor function; returns a BinaryFile*
typedef BinaryFile* (*constructFcn)();

BinaryFile* BinaryFile::getInstanceFor( const char *sName )
{
	FILE *f;
	char buf[64];
	std::string libName;
	BinaryFile *res = NULL;

	f = fopen (sName, "ro");
	if( f == NULL ) {
		fprintf(stderr, "Unable to open binary file: %s\n", sName );
		return NULL;
	}
	fread (buf, sizeof(buf), 1, f);
	if( TESTMAGIC4(buf,0, '\177','E','L','F') ) {
		/* ELF Binary */
		libName = "libElfBinaryFile.so";
	} else if( TESTMAGIC2( buf,0, 'M','Z' ) ) { /* DOS-based file */
		int peoff = LMMH(buf[0x3C]);
		if( peoff != 0 && fseek(f, peoff, SEEK_SET) != -1 ) {
			fread( buf, 4, 1, f );
			if( TESTMAGIC4( buf,0, 'P','E',0,0 ) ) {
				/* Win32 Binary */
				libName = "libWin32BinaryFile.so";
#ifdef WIN32
				res = new Win32BinaryFile;
				fclose(f);
				return res;
#endif
			} else if( TESTMAGIC2( buf,0, 'N','E' ) ) {
				/* Win16 / Old OS/2 Binary */
			} else if( TESTMAGIC2( buf,0, 'L','E' ) ) {
				/* Win32 VxD (Linear Executable) */
			} else if( TESTMAGIC2( buf,0, 'L','X' ) ) {
				/* New OS/2 Binary */
			}
		}
		/* Assume MS-DOS Real-mode binary. */
		if( libName.size() == 0 )
			libName = "libExeBinaryFile.so";
#ifdef WIN32
				res = new ExeBinaryFile;
				fclose(f);
				return res;
#endif
	} else if( TESTMAGIC4( buf,0x3C, 'a','p','p','l' ) ||
			   TESTMAGIC4( buf,0x3C, 'p','a','n','l' ) ) {
		/* PRC Palm-pilot binary */
		libName = "libPalmBinaryFile.so";
#ifdef WIN32
				res = new PalmBinaryFile;
				fclose(f);
				return res;
#endif
	} else if( buf[0] == 0x02 && buf[2] == 0x01 &&
			   (buf[1] == 0x10 || buf[1] == 0x0B) &&
			   (buf[3] == 0x07 || buf[3] == 0x08 || buf[4] == 0x0B) ) {
		/* HP Som binary (last as it's not really particularly good magic) */
		libName = "libHpSomBinaryFile.so";
#ifdef WIN32
				res = new HpSomBinaryFile;
				fclose(f);
				return res;
#endif
	} else {
		fprintf( stderr, "Unrecognised binary file\n" );
		fclose(f);
		return NULL;
	}
	
#ifndef WIN32			// Note: For now, Win32 statically links to all loaders
// Load the specific loader library
	libName = std::string("lib/") + libName;
	void* dlHandle = dlopen(libName.c_str(), RTLD_LAZY);
	if (dlHandle == NULL) {
		fprintf( stderr, "Could not open dynamic loader library %s\n",
			libName.c_str());
		fprintf( stderr, "%s\n", dlerror());
		fclose(f);
		return NULL;
	}
	// Use the handle to find the "construct" function
	constructFcn pFcn = (constructFcn) dlsym(dlHandle, "construct");
	if (pFcn == NULL) {
		fprintf( stderr, "Loader library %s does not have a construct "
			"function\n", libName.c_str());
		fclose(f);
		return NULL;
	}
	// Call the construct function
	res = (*pFcn)();
#else
	res = new ElfBinaryFile;
#endif

	fclose(f);
	return res;
}

int BinaryFile::GetNumSections() const
{
	return m_iNumSections;
}

PSectionInfo BinaryFile::GetSectionInfo(int idx) const
{
	return m_pSections + idx;
}

int BinaryFile::GetSectionIndexByName(const char* sName)
{
	for (int i=0; i < m_iNumSections; i++)
	{
		if (strcmp(m_pSections[i].pSectionName, sName) == 0)
		{
			return i;
		}
	}
	return -1;
}

PSectionInfo BinaryFile::GetSectionInfoByAddr(ADDRESS uEntry) const
{
	PSectionInfo pSect;
	for (int i=0; i < m_iNumSections; i++)
	{
		pSect = &m_pSections[i];
		if ((uEntry >= pSect->uNativeAddr) &&
			(uEntry < pSect->uNativeAddr + pSect->uSectionSize))
		{
			// We have the right section
			return pSect;
		}
	}
	// Failed to find the address
	return NULL;
}

PSectionInfo BinaryFile::GetSectionInfoByName(const char* sName)
{
	int i = GetSectionIndexByName(sName);
	if (i == -1) return 0;
	return &m_pSections[i];
}


	///////////////////////
	// Trivial functions //
	// Overridden if reqd//
	///////////////////////

char* BinaryFile::SymbolByAddress(ADDRESS uNative)
{
	return 0;		// Overridden by subclasses that support syms
}

ADDRESS BinaryFile::GetAddressByName(const char* pName, bool bNoTypeOK)
{
	return 0;
}

int BinaryFile::GetSizeByName(const char* pName, bool bNoTypeOK)
{
	return 0;
}


bool BinaryFile::IsAddressRelocatable(ADDRESS uNative)
{
	return false;
}

ADDRESS BinaryFile::GetRelocatedAddress(ADDRESS uNative)
{
	return NO_ADDRESS;
}

#if 0
WORD  BinaryFile::ApplyRelocation(ADDRESS uNative, WORD wWord)
{
	return 0;
}
#endif
				// Get symbol associated with relocation at address, if any
const char* BinaryFile::GetRelocSym(ADDRESS uNative)
{
	return 0;
}

bool BinaryFile::IsDynamicLinkedProc(ADDRESS uNative)
{
	return false;
}

bool BinaryFile::IsDynamicLinkedProcPointer(ADDRESS uNative)
{
	return false;
}

const char *BinaryFile::GetDynamicProcName(ADDRESS uNative)
{
	return "dynamic";
}

bool BinaryFile::DisplayDetails(const char* fileName, FILE* f /* = stdout */)
{
	return false;			// Should always be overridden
							// Should display file header, program 
							// headers and section headers, as well 
							// as contents of each of the sections. 
}

// Specific to BinaryFile objects that implement a "global pointer"
// Gets a pair of unsigned integers representing the address of %agp, and
// a machine specific value (GLOBALOFFSET)
// This is a stub routine that should be overridden if required
std::pair<unsigned,unsigned> BinaryFile::GetGlobalPointerInfo()
{
	return std::pair<unsigned, unsigned>(0, 0);
}

// Get a pointer to a new map of dynamic global data items.
// If the derived class doesn't implement this function, return an empty map
// Caller should delete the returned map
std::map<ADDRESS, const char*>* BinaryFile::GetDynamicGlobalMap()
{
	return new std::map<ADDRESS, const char*>;
}

// Get an array of exported function stub addresses. Normally overridden.
ADDRESS* BinaryFile::GetImportStubs(int& numExports)
{
	numExports = 0;
	return NULL;
}

void BinaryFile::getTextLimits()
{
	int n = GetNumSections();
	limitTextLow = 0xFFFFFFFF;
	limitTextHigh = 0;
	textDelta = 0;
	for (int i=0; i < n; i++) {
		SectionInfo* pSect = GetSectionInfo(i);
		if (pSect->bCode) {
			// The .plt section is an anomaly. It's code, but we never want to
			// decode it, and in Sparc ELF files, it's actually in the data
			// segment (so it can be modified). For now, we make this ugly
			// exception
			if (strcmp(".plt", pSect->pSectionName) == 0)
				continue;
			if (pSect->uNativeAddr < limitTextLow)
				limitTextLow = pSect->uNativeAddr;
			ADDRESS hiAddress = pSect->uNativeAddr + pSect->uSectionSize;
			if (hiAddress > limitTextHigh)
				limitTextHigh = hiAddress;
			if (textDelta == 0)
				textDelta = pSect->uHostAddr - pSect->uNativeAddr;
			else {
				if (textDelta != (int) (pSect->uHostAddr - pSect->uNativeAddr))
					std::cerr << "warning: textDelta different for section " << pSect->pSectionName <<
						" (ignoring).\n";
			}
		}
	}
}
