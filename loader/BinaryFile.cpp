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

/** \file BinaryFile.cpp
 * This file contains the implementation of the class BinaryFile
 *
 * This file implements the abstract BinaryFile class.
 * All classes derived from this class must implement the Load()
 * function.
*/

/*
 *    MVE 30/9/97 Created
 * 21 Apr 02 - Mike: mods for boomerang
 * 03 Jun 02 - Trent: if WIN32, no dynamic linking
 * 14 Jun 02 - Mike: Fixed a bug where Windows programs chose the Exe loader
*/

/***************************************************************************//**
 * Dependencies.
 ******************************************************************************/

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstddef>
#include "BinaryFile.h"

BinaryFile::BinaryFile(bool bArch /*= false*/) {
    m_bArchive      = bArch;        // Remember whether an archive member
    m_iNumSections  = 0;            // No sections yet
    m_pSections     = nullptr;            // No section data yet
}

// This struct used to be initialised with a memset, but now that overwrites the virtual table (if compiled under gcc
// and possibly others)
SectionInfo::SectionInfo() :
    pSectionName(nullptr), uNativeAddr(ADDRESS::g(0L)), uHostAddr(ADDRESS::g(0L)), uSectionSize(0), uSectionEntrySize(0),
    uType(0), bCode(false), bData(false), bBss(0), bReadOnly(0) {

}

int BinaryFile::GetNumSections() const {
    return m_iNumSections;
}

SectionInfo *BinaryFile::GetSectionInfo(int idx) const {
    return m_pSections + idx;
}
//! Find section index given name, or -1 if not found
int BinaryFile::GetSectionIndexByName(const char* sName) {
    for (int i=0; i < m_iNumSections; i++) {
        if (strcmp(m_pSections[i].pSectionName, sName) == 0) {
            return i;
        }
    }
    return -1;
}

SectionInfo *BinaryFile::GetSectionInfoByAddr(ADDRESS uEntry) const {
    PSectionInfo pSect;
    for (int i=0; i < m_iNumSections; i++) {
        pSect = &m_pSections[i];
        if ((uEntry >= pSect->uNativeAddr) && (uEntry < pSect->uNativeAddr + pSect->uSectionSize)) {
            // We have the right section
            return pSect;
        }
    }
    // Failed to find the address
    return nullptr;
}

SectionInfo * BinaryFile::GetSectionInfoByName(const char* sName) {
    int i = GetSectionIndexByName(sName);
    if (i == -1)
        return nullptr;
    return &m_pSections[i];
}


///////////////////////
// Trivial functions //
// Overridden if reqd//
///////////////////////
//! Lookup the address, return the name, or 0 if not found
const char* BinaryFile::SymbolByAddress(ADDRESS /*uNative*/) {
    return nullptr;        // Overridden by subclasses that support syms
}
//! Lookup the name, return the address. If not found, return NO_ADDRESS
ADDRESS BinaryFile::GetAddressByName(const char* /*pName*/, bool /*bNoTypeOK*/) {
    return ADDRESS::g(0L);
}
//! Lookup the name, return the size
int BinaryFile::GetSizeByName(const char* /*pName*/, bool /*bNoTypeOK*/) {
    return 0;
}

bool BinaryFile::IsDynamicLinkedProc(ADDRESS /*uNative*/) {
    return false;
}

bool BinaryFile::IsStaticLinkedLibProc(ADDRESS /*uNative*/) {
    return false;
}

bool BinaryFile::IsDynamicLinkedProcPointer(ADDRESS /*uNative*/) {
    return false;
}

ADDRESS BinaryFile::IsJumpToAnotherAddr(ADDRESS /*uNative*/) {
    return NO_ADDRESS;
}

const char *BinaryFile::GetDynamicProcName(ADDRESS /*uNative*/) {
    return "dynamic";
}

bool BinaryFile::DisplayDetails(const char* /*fileName*/, FILE* /*f*/ /* = stdout */) {
    return false;            // Should always be overridden
    // Should display file header, program
    // headers and section headers, as well
    // as contents of each of the sections.
}

/***************************************************************************//**
 *
 * Specific to BinaryFile objects that implement a "global pointer"
 * Gets a pair of unsigned integers representing the address of the
 * abstract global pointer (%agp) (in first) and a constant that will
 * be available in the csrparser as GLOBALOFFSET (second). At present,
 * the latter is only used by the Palm machine, to represent the space
 * allocated below the %a5 register (i.e. the difference between %a5 and
 * %agp). This value could possibly be used for other purposes.
 *
 ******************************************************************************/
std::pair<ADDRESS,unsigned> BinaryFile::GetGlobalPointerInfo() {
    return std::pair<ADDRESS, unsigned>(ADDRESS::g(0L), 0);
}

/***************************************************************************//**
 *
 * \brief Get a map from native addresses to symbolic names of global data items
 * (if any).
 *
 * Those are shared with dynamically linked libraries.
 * Example: __iob (basis for stdout).
 * The ADDRESS is the native address of a pointer to the real dynamic data object.
 * If the derived class doesn't implement this function, return an empty map
 *
 * \note Caller should delete the returned map
 * \returns  map of globals
 ******************************************************************************/

std::map<ADDRESS, const char*>* BinaryFile::GetDynamicGlobalMap() {
    return new std::map<ADDRESS, const char*>;
}

/***************************************************************************//**
 *
 * \brief Get an array of addresses of imported function stubs
 * Set number of these to numImports
 * \param numExports size of returned array
 * \returns  array of stubs
 ******************************************************************************/
ADDRESS* BinaryFile::GetImportStubs(int& numExports) {
    numExports = 0;
    return nullptr;
}
//! Get the lower and upper limits of the text segment
void BinaryFile::getTextLimits() {
    int n = GetNumSections();
    limitTextLow = ADDRESS::g(0xFFFFFFFF);
    limitTextHigh = ADDRESS::g(0L);
    textDelta = 0;
    for (int i=0; i < n; i++) {
        SectionInfo* pSect = GetSectionInfo(i);
        if (!pSect->bCode)
            continue;
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
        ptrdiff_t host_native_diff = (pSect->uHostAddr - pSect->uNativeAddr).m_value;
        if (textDelta == 0)
            textDelta = host_native_diff;
        else {
            if (textDelta != host_native_diff)
                std::cerr << "warning: textDelta different for section " << pSect->pSectionName << " (ignoring).\n";
        }
    }
}
