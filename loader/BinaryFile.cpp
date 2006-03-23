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
#include <iostream>
#include <stdio.h>


BinaryFile::BinaryFile(bool bArch /*= false*/)
{
	m_bArchive = bArch;			// Remember whether an archive member
	m_iNumSections = 0;			// No sections yet
	m_pSections = 0;			// No section data yet
}

// This struct used to be initialised with a memset, but now that overwrites the virtual table (if compiled under gcc
// and possibly others)
SectionInfo::SectionInfo() :
	pSectionName(NULL), uNativeAddr(0), uHostAddr(0), uSectionSize(0), uSectionEntrySize(0), uType(0),
	bCode(false), bData(false), bBss(0), bReadOnly(0)
{}

SectionInfo::~SectionInfo()
{}

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

const char* BinaryFile::SymbolByAddress(ADDRESS uNative) {
	return 0;		// Overridden by subclasses that support syms
}

ADDRESS BinaryFile::GetAddressByName(const char* pName, bool bNoTypeOK) {
	return 0;
}

int BinaryFile::GetSizeByName(const char* pName, bool bNoTypeOK) {
	return 0;
}


#if 0
bool BinaryFile::IsAddressRelocatable(ADDRESS uNative) {
	return false;
}

ADDRESS BinaryFile::GetRelocatedAddress(ADDRESS uNative) {
	return NO_ADDRESS;
}

WORD  BinaryFile::ApplyRelocation(ADDRESS uNative, WORD wWord) {
	return 0;
}
				// Get symbol associated with relocation at address, if any
const char* BinaryFile::GetRelocSym(ADDRESS uNative) {
	return 0;
}
#endif

bool BinaryFile::IsDynamicLinkedProc(ADDRESS uNative) {
	return false;
}

bool BinaryFile::IsStaticLinkedLibProc(ADDRESS uNative) {
	return false;
}

bool BinaryFile::IsDynamicLinkedProcPointer(ADDRESS uNative)
{
	return false;
}

ADDRESS BinaryFile::IsJumpToAnotherAddr(ADDRESS uNative)
{
	return NO_ADDRESS;
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
