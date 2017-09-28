#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SymTab.h"


/***************************************************************************/ /**
 * \file        SymTab.cpp
 * \brief    This file contains the implementation of the class SymTab, a simple class to maintain a pair of maps
 *                so that symbols can be accessed by symbol or by name
 ******************************************************************************/

#include "boomerang/util/Log.h"
#include "boomerang/core/Boomerang.h"

#include <cassert>


SymTab::SymTab()
{
}


SymTab::~SymTab()
{
    clear();
}


void SymTab::clear()
{
    for (IBinarySymbol *s : SymbolList) {
        delete s;
    }

    for (std::pair<const Address, BinarySymbol *>& s : amap) {
        delete s.second;
    }

    SymbolList.clear();
    amap.clear();
    smap.clear();
}


IBinarySymbol& SymTab::create(Address addr, const QString& name, bool local)
{
    assert(amap.find(addr) == amap.end());

    // If the symbol already exists, redirect the new symbol to the old one.
    std::map<QString, BinarySymbol*>::iterator it = smap.find(name);
    if (it != smap.end()) {
        LOG_WARN("Symbol '%1' already exists in the global symbol table!", name);
        BinarySymbol *existingSymbol = it->second;
        amap[addr] = existingSymbol;
        return *existingSymbol;
    }

    BinarySymbol *sym = new BinarySymbol;
    sym->Location = addr;
    sym->Name     = name;
    amap[addr]    = sym;

    if (!local) {
        smap[name] = sym;
    }

    return *sym;
}


const IBinarySymbol *SymTab::find(Address a) const
{
    auto ff = amap.find(a);

    if (ff == amap.end()) {
        return nullptr;
    }

    return ff->second;
}


const IBinarySymbol *SymTab::find(const QString& s) const
{
    auto ff = smap.find(s);

    if (ff == smap.end()) {
        return nullptr;
    }

    return ff->second;
}


bool BinarySymbol::rename(const QString& newName)
{
    // TODO: this code assumes only one BinarySymbolTable instance exists
    SymTab *sym_tab = (SymTab *)Boomerang::get()->getSymbols();

    if (sym_tab->smap.find(newName) != sym_tab->smap.end()) {
        LOG_ERROR("Renaming symbol %1 to %2 failed - new name clashes with another symbol", Name, newName);
        return false; // symbol name clash
    }

    sym_tab->smap.erase(Name);
    Name = newName;
    sym_tab->smap[Name] = this;
    return true;
}


bool BinarySymbol::isImported() const
{
    return attributes.contains("Imported") && attributes["Imported"].toBool();
}


QString BinarySymbol::belongsToSourceFile() const
{
    if (!attributes.contains("SourceFile")) {
        return "";
    }

    return attributes["SourceFile"].toString();
}


bool BinarySymbol::isFunction() const
{
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
