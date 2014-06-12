/*
 * Copyright (C) 2005, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
  * \file        SymTab.h
  * \brief    This file contains the definition of the class SymTab
  * A simple class to implement a symbol table
  * than can be looked up by address or my name.
  * \note Can't readily use operator[] overloaded for address and string parameters. The main problem is
  * that when you do symtab[0x100] = "main", the string map doesn't see the string.
  * If you have one of the maps be a pointer to the other string and use a special comparison operator, then
  * if the strings are ever changed, then the map's internal rb-tree becomes invalid.
  ******************************************************************************/

#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include "types.h"
#include <map>
#include <string>

class SymTab {
    // The map indexed by address.
    std::map<ADDRESS, QString> amap;
    // The map indexed by string. Note that the strings are stored twice.
    std::map<QString, ADDRESS> smap;

  public:
    SymTab();                     // Constructor
    ~SymTab();                    // Destructor
    void Add(ADDRESS a, const QString &s); // Add a new entry
    const QString &find(ADDRESS a);  // Find an entry by address; nullptr if none
    ADDRESS find(const QString &s);  // Find an entry by name; NO_ADDRESS if none
#if 0
        char*       FindAfter(ADDRESS& dwAddr);     // Find entry with >= given value
        char*       FindNext(ADDRESS& dwAddr);      // Find next entry (after a Find())
        int         FindIndex(ADDRESS dwAddr);      // Find index for entry
        ADDRESS     FindSym(char* pName);           // Linear search for addr from name
#endif
    std::map<ADDRESS, QString> &getAll() { return amap; }
};
#endif // __SYMTAB_H__
