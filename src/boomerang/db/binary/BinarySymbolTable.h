#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/util/Address.h"

#include <memory>
#include <vector>


class BinarySymbol;


/**
 * A simple class to implement a symbol table than can be looked up by address or by name.
 *
 * \note Can't readily use operator[] overloaded for address and string parameters.
 * The main problem is that when you do symtab[0x100] = "main", the string map
 * doesn't see the string. If you have one of the maps be a pointer to the other string
 * and use a special comparison operator, then if the strings are ever changed,
 * then the map's internal rb-tree becomes invalid.
 */
class BinarySymbolTable
{
    typedef std::vector<BinarySymbol *>        SymbolList;

    typedef SymbolList::iterator               iterator;
    typedef SymbolList::const_iterator         const_iterator;
    typedef SymbolList::reverse_iterator       reverse_iterator;
    typedef SymbolList::const_reverse_iterator const_reverse_iterator;

public:
    BinarySymbolTable();
    BinarySymbolTable(const BinarySymbolTable& other) = delete;
    BinarySymbolTable(BinarySymbolTable&& other) = default;

    ~BinarySymbolTable();

    BinarySymbolTable& operator=(const BinarySymbolTable& other) = delete;
    BinarySymbolTable& operator=(BinarySymbolTable&& other) = default;

public:
    iterator begin()             { return m_symbolList.begin(); }
    iterator end()               { return m_symbolList.end(); }
    const_iterator begin() const { return m_symbolList.begin(); }
    const_iterator end()   const { return m_symbolList.end(); }

    reverse_iterator rbegin() { return m_symbolList.rbegin(); }
    reverse_iterator rend()   { return m_symbolList.rend(); }
    const_reverse_iterator rbegin() const { return m_symbolList.rbegin(); }
    const_reverse_iterator rend()   const { return m_symbolList.rend(); }

public:
    int size() const { return m_symbolList.size(); }
    bool empty()  const { return m_symbolList.empty(); }
    void clear();

    /// Creates a symbol if it does not exist.
    BinarySymbol *createSymbol(Address addr, const QString& name, bool local = false);

    BinarySymbol *findSymbolByAddress(Address addr);
    const BinarySymbol *findSymbolByAddress(Address addr) const;

    BinarySymbol *findSymbolByName(const QString& name);
    const BinarySymbol *findSymbolByName(const QString& name) const;

    /// \returns true iff the rename was successful
    bool renameSymbol(const QString& oldName, const QString& newName);

private:
    /// The map indexed by address.
    std::map<Address, std::shared_ptr<BinarySymbol>> m_addrIndex;

    /// The map indexed by string. Note that the strings are stored twice.
    std::map<QString, std::shared_ptr<BinarySymbol>> m_nameIndex;

    SymbolList m_symbolList;
};
