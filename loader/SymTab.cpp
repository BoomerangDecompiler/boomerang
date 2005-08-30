/*
 * Copyright (C) 2005, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:        SymTab.cpp
 * OVERVIEW:    This file contains the implementation of the class SymTab, a simple class to maintain a pair of maps
 *				so that symbols can be accessed by symbol or by name
 *============================================================================*/
/*
 * $Revision$
 *
 * 12 Jul 05 - Mike: threw out the bsearch() code and implemented dual maps instead
*/

#include "SymTab.h"

SymTab::SymTab() {
}

SymTab::~SymTab() {
}

void SymTab::Add(ADDRESS a, char* s) {
	amap[a] = s;
	smap[s] = a;
}

const char* SymTab::find(ADDRESS a) {
	std::map<ADDRESS, std::string>::iterator ff;
	ff = amap.find(a);
	if (ff == amap.end())
		return NULL;
	return ff->second.c_str();
}

ADDRESS SymTab::find(const char* s) {
	std::map<std::string, ADDRESS>::iterator ff;
	ff = smap.find(s);
	if (ff == smap.end())
		return NO_ADDRESS;
	return ff->second;
}

#if 0
// Find the next symbol (after calling Find() or FindAfter()).
// Returns the symbol, and sets the reference parameter to the value
char* SymTab::FindNext(ADDRESS& dwAddr)
{
    if (++m_iFindEnt > m_iCurEnt)
        return NULL;
    dwAddr = m_pEnt[m_iFindEnt].dwValue;
    return m_pEnt[m_iFindEnt].pName;
}

// Find the next symbol after the given address
char* SymTab::FindAfter(ADDRESS& dwAddr)
{
    int bot = 0;
    int top = m_iCurEnt-1;
    int curr;
    do {
        curr = (bot + top) >> 1;
        PSYMTABENT pEnt = m_pEnt + curr;    // Point to current entry
        if (pEnt->dwValue > dwAddr)
            top = curr-1;
        else if (pEnt->dwValue < dwAddr)
            bot = curr+1;
        else {      // Found the address
            m_iFindEnt = curr;
            return pEnt->pName;
        }
    } while (bot <= top);
    // If we get here, we don't have an exact match. So just use the entry at bot, which is the next highest value
    m_iFindEnt = bot;
    dwAddr = m_pEnt[bot].dwValue;
    return m_pEnt[bot].pName;
}
#endif

