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
