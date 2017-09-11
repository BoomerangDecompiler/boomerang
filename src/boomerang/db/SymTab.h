#pragma once

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

#include "boomerang/db/IBinarySymbols.h"
#include "boomerang/util/Types.h"

#include <QVariantMap>
#include <memory>
#include <map>
#include <string>

typedef std::shared_ptr<class Type> SharedType;

struct BinarySymbol : public IBinarySymbol
{
    const QString&       getName() const override { return Name; }
    size_t getSize() const override { return Size; }
    void setSize(size_t v) override { Size = v; }
    Address getLocation() const override { return Location; }

    const IBinarySymbol& setAttr(const QString& name, const QVariant& v) const override
    {
        attributes[name] = v;
        return *this;
    }

    bool rename(const QString& newName) override;

    bool isImportedFunction() const override;
    bool isStaticFunction() const override;
    bool isFunction() const override;
    bool isImported() const override;
    QString belongsToSourceFile() const override;

public:
    QString              Name;
    Address              Location;
    SharedType           type;
    size_t               Size;
    /// it's mutable since no changes in attribute map will influence the layout of symbols in SymTable
    mutable QVariantMap  attributes;
};


class SymTab : public IBinarySymbolTable
{
    friend struct BinarySymbol;

public:
    SymTab();                     // Constructor
    ~SymTab();                    // Destructor

    /// \copydoc IBinarySymbolTable::find(Address)
    const IBinarySymbol *find(Address addr) const override;

    /// \copydoc IBinarySymbolTable::find(const QString&)
    const IBinarySymbol *find(const QString& name) const override;

    /// \copydoc IBinarySymbolTable::create
    IBinarySymbol& create(Address addr, const QString& name, bool local = false) override;


    SymbolListType& getSymbolList() { return SymbolList; }
    iterator begin()       override { return SymbolList.begin(); }
    const_iterator begin() const override { return SymbolList.begin(); }
    iterator end()       override { return SymbolList.end(); }
    const_iterator end() const override { return SymbolList.end(); }
    size_t size()  const { return SymbolList.size(); }
    bool empty() const { return SymbolList.empty(); }
    void clear() override;

private:
    // The map indexed by address.
    std::map<Address, BinarySymbol *> amap;
    // The map indexed by string. Note that the strings are stored twice.
    std::map<QString, BinarySymbol *> smap;
    std::vector<IBinarySymbol *> SymbolList;
};
