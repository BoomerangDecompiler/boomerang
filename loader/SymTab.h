/*
 * Copyright (C) 1998-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:		SymTab.h
 * OVERVIEW:	This file contains the definition of the class SymTab,
 *				a simple class to look up symbols using bsearch()
 *============================================================================*/

/*
 * $Revision$
 * To use:
	- Instantiate an object.
	- Call Init() with the number of entries in the table. If
		this fails, it will return 0.
	- For each entry to add (in any order), call Add().
	- Call Sort().
	- After Sort() is called, you can call Find() to get the string
		associated with an address, or FindIndex() to find an index
		into the table.
	- After calling Find(), you can call FindNext() to get the next symbol
		(in address order)
	- Free the object.
 * Changes:
 * 03 Mar 98 - Cristina: replaced ADDR for ADDRESS for consistency with other
 *				tools.
 * 22 Jun 00 - Mike: Fixed some const issues
 * 04 Aug 00 - Mike: Added the FindNext() method
 * 10 Apr 01 - Mike: Documented correct use of m_iNumEnt and m_iCurEnt
*/

#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include "types.h"
#include <stdlib.h>						// bsearch(), qsort()
#include <string.h>						// strcmp etc

typedef struct tSymTabEnt {
	ADDRESS dwValue;					// Value of the symbol
	char* pName;						// Name of the symbol	
} SymTabEnt;
typedef SymTabEnt* PSYMTABENT;
 
class SymTab
{
public:
			SymTab();						// Constructor
			~SymTab();						// Destructor
	int		Init(int iNumEnt);				// Allocate space; true if success
	void	Add(ADDRESS dwAddr, char* pName); // Add a new entry
	void	Sort();							// Sort the entries
	char*	Find(ADDRESS dwAddr);			// Find an entry
	char*	FindAfter(ADDRESS& dwAddr);		// Find entry with >= given value
	char*	FindNext(ADDRESS& dwAddr);		// Find next entry (after a Find())
	int		FindIndex(ADDRESS dwAddr);		// Find index for entry
	ADDRESS FindSym(char* pName);			// Linear search for addr from name
private:
	PSYMTABENT	m_pEnt;					// Points to array of entries
	int		m_iNumEnt;					// Max number of entries (num alloc'd)
	int		m_iCurEnt;					// Numner added by the Add() method,
										// i.e. the num of actual entries
	int		m_iFindEnt;					// Used by Find() method
};

#ifndef NULL
#define NULL 0			// Normally in stdio.h, it seems!
#endif

#endif	// __SYMTAB_H__
