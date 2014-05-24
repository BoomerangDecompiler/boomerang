/*
 * Copyright (C) 2005, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************//**
 * \file        SymTab.cpp
 * \brief    This file contains the implementation of the class SymTab, a simple class to maintain a pair of maps
 *                so that symbols can be accessed by symbol or by name
 ******************************************************************************/
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
        return nullptr;
    return ff->second.c_str();
}

ADDRESS SymTab::find(const char* s) {
    std::map<std::string, ADDRESS>::iterator ff;
    ff = smap.find(s);
    if (ff == smap.end())
        return NO_ADDRESS;
    return ff->second;
}

