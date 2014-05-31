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

/***************************************************************************/ /**
  * Dependencies.
  ******************************************************************************/

#include "BinaryFile.h"

#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstddef>

// This struct used to be initialised with a memset, but now that overwrites the virtual table (if compiled under gcc
// and possibly others)
SectionInfo::SectionInfo()
    : pSectionName(nullptr), uNativeAddr(ADDRESS::g(0L)), uHostAddr(ADDRESS::g(0L)), uSectionSize(0),
      uSectionEntrySize(0), uType(0), bCode(false), bData(false), bBss(0), bReadOnly(0) {}

///////////////////////
// Trivial functions //
// Overridden if reqd//
///////////////////////

/***************************************************************************/ /**
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
std::pair<ADDRESS, unsigned> BinaryFile::GetGlobalPointerInfo() {
    return std::pair<ADDRESS, unsigned>(ADDRESS::g(0L), 0);
}

/***************************************************************************/ /**
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

std::map<ADDRESS, const char *> *BinaryFile::GetDynamicGlobalMap() { return new std::map<ADDRESS, const char *>; }

/***************************************************************************/ /**
  *
  * \brief Get an array of addresses of imported function stubs
  * Set number of these to numImports
  * \param numExports size of returned array
  * \returns  array of stubs
  ******************************************************************************/
ADDRESS *BinaryFile::GetImportStubs(int &numExports) {
    numExports = 0;
    return nullptr;
}
//! Get the lower and upper limits of the text segment
void LoaderCommon::getTextLimits() {
    int n = GetNumSections();
    limitTextLow = ADDRESS::g(0xFFFFFFFF);
    limitTextHigh = ADDRESS::g(0L);
    textDelta = 0;
    for (int i = 0; i < n; i++) {
        SectionInfo *pSect = GetSectionInfo(i);
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
