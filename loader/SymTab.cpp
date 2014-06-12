/*
 * Copyright (C) 2005, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
  * \file        SymTab.cpp
  * \brief    This file contains the implementation of the class SymTab, a simple class to maintain a pair of maps
  *                so that symbols can be accessed by symbol or by name
  ******************************************************************************/
#include "SymTab.h"

SymTab::SymTab() {}

SymTab::~SymTab() {}

void SymTab::Add(ADDRESS a, const QString &s) {
    amap[a] = s;
    smap[s] = a;
}

const QString &SymTab::find(ADDRESS a) {
    static QString null_res;
    auto ff = amap.find(a);
    if (ff == amap.end())
        return null_res;
    return ff->second;
}

ADDRESS SymTab::find(const QString &s) {
    auto ff = smap.find(s);
    if (ff == smap.end())
        return NO_ADDRESS;
    return ff->second;
}
