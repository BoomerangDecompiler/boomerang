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
#include "boomerang.h"

#include <QDebug>
#include <cassert>
SymTab::SymTab() {}

SymTab::~SymTab() {
    clear();
}
void SymTab::clear() {
    for(IBinarySymbol *s : SymbolList)
        delete s;
    for(std::pair<const ADDRESS, BinarySymbol *> &s : amap)
        delete s.second;
    SymbolList.clear();
    amap.clear();
    smap.clear();
}
IBinarySymbol &SymTab::create(ADDRESS a, const QString &s, bool local) {
    assert(amap.find(a)==amap.end());
    assert(smap.find(s)==smap.end());
    BinarySymbol * sym = new BinarySymbol;
    sym->Location = a;
    sym->Name = s;
    amap[a] = sym;
    if(!local)
        smap[s] = sym;
    return *sym;
}

const IBinarySymbol *SymTab::find(ADDRESS a) const {
    auto ff = amap.find(a);
    if (ff == amap.end())
        return nullptr;
    return ff->second;
}

const IBinarySymbol *SymTab::find(const QString &s) const {
    auto ff = smap.find(s);
    if (ff == smap.end())
        return nullptr;
    return ff->second;
}


bool BinarySymbol::rename(const QString &s)
{
    //TODO: this code assumes only one BinarySymbolTable instance exists
    SymTab *sym_tab = (SymTab *)Boomerang::get()->getSymbols();
    if(sym_tab->smap.find(s)!=sym_tab->smap.end()) {
        qDebug()<<"Renaming symbol " << Name << " to " << s << " failed - new name clashes with another symbol";
        return false; // symbol name clash
    }
    sym_tab->smap.erase(Name);
    Name = s;
    sym_tab->smap[Name] = this;
    return true;
}
bool BinarySymbol::isImported() const {
    return attributes.contains("Imported") && attributes["Imported"].toBool();
}

QString BinarySymbol::belongsToSourceFile() const
{
    if(!attributes.contains("SourceFile"))
        return "";
    return attributes["SourceFile"].toString();
}
bool BinarySymbol::isFunction() const {
    return attributes.contains("Function") && attributes["Function"].toBool();
}
bool BinarySymbol::isImportedFunction() const
{
    return isImported() && isFunction();
}

bool BinarySymbol::isStaticFunction() const
{
    return attributes.contains("StaticFunction") && attributes["StaticFunction"].toBool();
}
