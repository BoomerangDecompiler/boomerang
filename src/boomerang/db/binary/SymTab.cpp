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


#include "boomerang/util/Log.h"
#include "boomerang/core/Boomerang.h"

#include <cassert>


SymTab::SymTab()
{
}


SymTab::~SymTab()
{
}


void SymTab::clear()
{
    m_symbolList.clear();
    m_addrIndex.clear();
    m_nameIndex.clear();
}


IBinarySymbol& SymTab::create(Address addr, const QString& name, bool local)
{
    assert(m_addrIndex.find(addr) == m_addrIndex.end());

    // If the symbol already exists, redirect the new symbol to the old one.
    std::map<QString, std::shared_ptr<BinarySymbol>>::iterator it = m_nameIndex.find(name);

    if (it != m_nameIndex.end()) {
        LOG_WARN("Symbol '%1' already exists in the global symbol table!", name);
        std::shared_ptr<BinarySymbol> existingSymbol = it->second;
        m_addrIndex[addr] = existingSymbol;
        return *existingSymbol;
    }

    std::shared_ptr<BinarySymbol> sym = std::make_shared<BinarySymbol>(addr, name);
    m_addrIndex[addr]    = sym;

    if (!local) {
        m_nameIndex[name] = sym;
    }

    return *sym;
}


const IBinarySymbol *SymTab::find(Address addr) const
{
    auto ff = m_addrIndex.find(addr);

    if (ff == m_addrIndex.end()) {
        return nullptr;
    }

    return ff->second.get();
}


const IBinarySymbol *SymTab::find(const QString& s) const
{
    auto ff = m_nameIndex.find(s);

    if (ff == m_nameIndex.end()) {
        return nullptr;
    }

    return ff->second.get();
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


bool SymTab::rename(const QString& oldName, const QString& newName)
{
    auto oldIt = m_nameIndex.find(oldName);
    auto newIt = m_nameIndex.find(newName);

    if (oldIt == m_nameIndex.end()) {
        // symbol not found
        LOG_ERROR("Could not rename symbol '%1' to '%2': Symbol not found.", oldName, newName);
        return false;
    }
    else if (newIt != m_nameIndex.end()) {
        // symbol name clash
        LOG_ERROR("Could not rename symbol '%1' to '%2': A symbol with name '%2' already exists",
                  oldName, newName);
        return false;
    }

    std::shared_ptr<BinarySymbol> oldSymbol = oldIt->second;
    m_nameIndex.erase(oldIt);
    oldSymbol->Name = newName;
    m_nameIndex[newName] = oldSymbol;

    return true;
}
