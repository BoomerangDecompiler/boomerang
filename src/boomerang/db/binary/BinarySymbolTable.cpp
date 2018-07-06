#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BinarySymbolTable.h"


#include "boomerang/util/Log.h"

#include <cassert>


BinarySymbolTable::BinarySymbolTable()
{
}


BinarySymbolTable::~BinarySymbolTable()
{
    clear();
}


void BinarySymbolTable::clear()
{
    m_addrIndex.clear();
    m_symbolList.clear();
    m_nameIndex.clear();
}


BinarySymbol *BinarySymbolTable::createSymbol(Address addr, const QString& name, bool local)
{
    if (m_addrIndex.find(addr) != m_addrIndex.end()) {
        return nullptr; // symbol already exists
    }

    // If the symbol already exists, redirect the new symbol to the old one.
    std::map<QString, std::shared_ptr<BinarySymbol>>::iterator it = m_nameIndex.find(name);

    if (it != m_nameIndex.end()) {
        LOG_WARN("Symbol '%1' already exists in the global symbol table!", name);
        std::shared_ptr<BinarySymbol> existingSymbol = it->second;
        m_addrIndex[addr] = existingSymbol;
        return existingSymbol.get();
    }

    std::shared_ptr<BinarySymbol> sym = std::make_shared<BinarySymbol>(addr, name);
    m_addrIndex[addr]    = sym;

    if (!local) {
        m_nameIndex[name] = sym;
    }

    m_symbolList.push_back(sym.get());
    return sym.get();
}


BinarySymbol *BinarySymbolTable::findSymbolByAddress(Address addr)
{
    auto ff = m_addrIndex.find(addr);
    return (ff != m_addrIndex.end()) ? ff->second.get() : nullptr;
}


const BinarySymbol *BinarySymbolTable::findSymbolByAddress(Address addr) const
{
    auto ff = m_addrIndex.find(addr);
    return (ff != m_addrIndex.end()) ? ff->second.get() : nullptr;
}


BinarySymbol *BinarySymbolTable::findSymbolByName(const QString& name)
{
    auto ff = m_nameIndex.find(name);
    return (ff != m_nameIndex.end()) ? ff->second.get() : nullptr;
}


const BinarySymbol *BinarySymbolTable::findSymbolByName(const QString& name) const
{
    auto ff = m_nameIndex.find(name);
    return (ff != m_nameIndex.end()) ? ff->second.get() : nullptr;
}


bool BinarySymbolTable::renameSymbol(const QString& oldName, const QString& newName)
{
    if (oldName == newName) {
        return true;
    }

    auto oldIt = m_nameIndex.find(oldName);
    auto newIt = m_nameIndex.find(newName);

    if (oldIt == m_nameIndex.end()) { // symbol not found
        LOG_ERROR("Could not rename symbol '%1' to '%2': A symbol with name '%1' was not found.",
                  oldName, newName);
        return false;
    }
    else if (newIt != m_nameIndex.end()) { // symbol name clash
        LOG_ERROR("Could not rename symbol '%1' to '%2': A symbol with name '%2' already exists",
                  oldName, newName);
        return false;
    }

    std::shared_ptr<BinarySymbol> oldSymbol = oldIt->second;
    m_nameIndex.erase(oldIt);
    oldSymbol->m_name = newName;
    m_nameIndex[newName] = oldSymbol;

    return true;
}
