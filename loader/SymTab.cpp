/*
 * Copyright (C) 1997-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:		SymTab.cc
 * OVERVIEW:	This file contains the implementation of the class SymTab,
 *				a simple class to look up symbols using bsearch()
 *============================================================================*/
/*
 * $Revision$
 *
 *		 97 - Mike: Created
 * 3 Mar 98 - Cristina
 *	changed ADDR for ADDRESS for consistency with other tools.
 * 4 Sep 98 - Mike
 *	Sort() works on number of symbols added, not the number initialised for.
 *	Overcomes problem whereby last real symbol can get lost due to an
 *	uninitialised entry sorting before it
 * 22 Jun 00 - Mike: Fixed up some const issues
 * 07 Aug 00 - Mike: Added FindAfter(), FindNext()
 * 31 Mar 00 - Mike: use NO_ADDRESS for invalid ADDRESS
 * 10 Apr 01 - Mike: Use m_iCurEnt to prevent accessing past end with FindNext()
*/

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "SymTab.h"

SymTab::SymTab()
{
	m_iNumEnt = 0;				// No entries allocated yet
	m_pEnt = NULL;				// Null pointer
	m_iCurEnt = 0;				// No entries added yet
}

SymTab::~SymTab()
{
	// Free the entries themselves
	if (m_pEnt)
		delete [] m_pEnt;

	m_pEnt = NULL;
}

int SymTab::Init(int iNumEnt)
{
	m_pEnt = new SymTabEnt[iNumEnt];
	if (m_pEnt)
	{
		m_iNumEnt = iNumEnt;
		return 1;
	}
	else
		return 0;
}

// The comparison function required by bsearch() and qsort()
// Sorts by address
int SymComp(const void* e1, const void* e2) 
{
	PSYMTABENT p1 = (PSYMTABENT) e1;
	PSYMTABENT p2 = (PSYMTABENT) e2;
		 if (p1->dwValue < p2->dwValue) return -1;
	else if (p1->dwValue > p2->dwValue) return 1;
	else return 0;
}

int SymTab::FindIndex(ADDRESS dwAddr)
{
	// Find entry in table, and return an index
	// Note: only consider the first m_iCurEnt entries
	if (m_iCurEnt == 0) return -1;		// Else may fault
	SymTabEnt ent;
	ent.dwValue = dwAddr;
	SymTabEnt* p = (PSYMTABENT) bsearch(&ent, m_pEnt, m_iCurEnt, 
		sizeof(SymTabEnt), SymComp);
	if (p) return p - m_pEnt;
	else return -1;
}

char* SymTab::Find(ADDRESS dwAddr)
{
	m_iFindEnt = FindIndex(dwAddr);
	if (m_iFindEnt < 0) return 0;
	return m_pEnt[m_iFindEnt].pName;
}

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
		PSYMTABENT pEnt = m_pEnt + curr;	// Point to current entry
		if (pEnt->dwValue > dwAddr)
			top = curr-1;
		else if (pEnt->dwValue < dwAddr)
			bot = curr+1;
		else {		// Found the address
			m_iFindEnt = curr;
			return pEnt->pName;
		}
	} while (bot <= top);
	// If we get here, we don't have an exact match. So just use the entry
	// at bot, which is the next highest value
	m_iFindEnt = bot;
	dwAddr = m_pEnt[bot].dwValue;
	return m_pEnt[bot].pName;
}

// Add a symbol to the table
void SymTab::Add(ADDRESS dwAddr, char* pName)
{
	// Ignore any excess entries
	if (m_iCurEnt >= m_iNumEnt) return;

	m_pEnt[m_iCurEnt].pName = pName;
	m_pEnt[m_iCurEnt].dwValue = dwAddr;
	m_iCurEnt++;			// Number currently added
}

// Sort by address
void SymTab::Sort()
{
	// Note: only consider the first m_iCurEnt entries
	qsort(m_pEnt, m_iCurEnt, sizeof(SymTabEnt), SymComp);
}

// Linear search for given name (for debugging symbols)
ADDRESS SymTab::FindSym(char* pName)
{
	for (int i=0; i < m_iCurEnt; i++) {
		if (strcmp(pName, m_pEnt[i].pName) == 0)
			return m_pEnt[i].dwValue;
	}
	return NO_ADDRESS;
}
