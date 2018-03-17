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


#include "boomerang/db/binary/IBinarySymbols.h"
#include "boomerang/util/Types.h"

#include <QVariantMap>
#include <memory>
#include <map>
#include <string>


typedef std::shared_ptr<class Type> SharedType;


class BinarySymbol : public IBinarySymbol
{
    friend class SymTab;

public:
    BinarySymbol(Address location, const QString& name)
        : Location(location)
        , Name(name)
    {}

public:
    virtual const QString&       getName() const override { return Name; }
    virtual size_t getSize() const override { return Size; }
    virtual void setSize(size_t v) override { Size = v; }
    virtual Address getLocation() const override { return Location; }

    virtual const IBinarySymbol& setAttr(const QString& name, const QVariant& v) const override
    {
        attributes[name] = v;
        return *this;
    }

    virtual bool isImportedFunction() const override;
    virtual bool isStaticFunction() const override;
    virtual bool isFunction() const override;
    virtual bool isImported() const override;
    virtual QString belongsToSourceFile() const override;

private:
    Address              Location;
    QString              Name;
    SharedType           type;
    size_t               Size;
    /// it's mutable since no changes in attribute map will influence the layout of symbols in SymTable
    mutable QVariantMap  attributes;
};


/**
 * A simple class to implement a symbol table than can be looked up by address or by name.
 *
 * \note Can't readily use operator[] overloaded for address and string parameters. The main problem is
 * that when you do symtab[0x100] = "main", the string map doesn't see the string.
 * If you have one of the maps be a pointer to the other string and use a special comparison operator, then
 * if the strings are ever changed, then the map's internal rb-tree becomes invalid.
 */
class SymTab : public IBinarySymbolTable
{
public:
    SymTab();
    SymTab(const SymTab& other) = delete;
    SymTab(SymTab&& other) = default;

    ~SymTab() override;

    SymTab& operator=(const SymTab& other) = delete;
    SymTab& operator=(SymTab&& other) = default;

public:
    virtual iterator begin()             override { return m_symbolList.begin(); }
    virtual const_iterator begin() const override { return m_symbolList.begin(); }
    virtual iterator end()               override { return m_symbolList.end(); }
    virtual const_iterator end() const   override { return m_symbolList.end(); }

    virtual size_t size()  const { return m_symbolList.size(); }
    virtual bool empty()   const { return m_symbolList.empty(); }
    virtual void clear() override;

    /// \copydoc IBinarySymbolTable::create
    virtual IBinarySymbol& create(Address addr, const QString& name, bool local = false) override;

    /// \copydoc IBinarySymbolTable::find(Address)
    virtual const IBinarySymbol *find(Address addr) const override;

    /// \copydoc IBinarySymbolTable::find(const QString&)
    virtual const IBinarySymbol *find(const QString& name) const override;

    /// \copydoc IBinarySymbolTable::renameSymbol
    virtual bool rename(const QString& oldName, const QString& newName) override;

private:
    /// The map indexed by address.
    std::map<Address, std::shared_ptr<BinarySymbol>> m_addrIndex;

    /// The map indexed by string. Note that the strings are stored twice.
    std::map<QString, std::shared_ptr<BinarySymbol>> m_nameIndex;

    SymbolListType m_symbolList;
};
