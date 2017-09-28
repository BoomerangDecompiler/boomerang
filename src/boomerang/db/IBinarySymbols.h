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

#include <QString>

class IBinarySymbol
{
public:
    virtual ~IBinarySymbol() {}
    virtual const QString& getName() const = 0;
    virtual size_t getSize() const         = 0;

    virtual void setSize(size_t) = 0;
    virtual Address getLocation() const         = 0;
    virtual bool isImportedFunction() const     = 0;
    virtual bool isStaticFunction() const       = 0;
    virtual bool isFunction() const             = 0;
    virtual bool isImported() const             = 0;
    virtual QString belongsToSourceFile() const = 0;
    virtual const IBinarySymbol& setAttr(const QString& name, const QVariant&) const = 0;

    //    virtual IBinarySymbol &setName(const QString &name) = 0;
    //    virtual IBinarySymbol &setSize(size_t sz) = 0;
    virtual bool rename(const QString& newName) = 0; ///< Rename an existing symbol
};

class QString;
typedef std::shared_ptr<class Type> SharedType;


class IBinarySymbolTable
{
public:
    typedef std::vector<IBinarySymbol *>   SymbolListType;
    typedef SymbolListType::iterator       iterator;
    typedef SymbolListType::const_iterator const_iterator;

public:
    virtual ~IBinarySymbolTable() {}

    /// \returns the binary symbol at address \p addr, or nullptr if no such symbol exists.
    virtual const IBinarySymbol *find(Address addr) const        = 0;

    /// \returns the binary symbol with name \p name, or nullptr if no such symbol exists.
    virtual const IBinarySymbol *find(const QString& name) const = 0;

    /**
     * Add a new symbol to table, if \p local is set than the symbol is local,
     * thus it won't be added to global name->symbol mapping.
     * If the symbol already exists in the global name->symbol table, the the symbol address \p addr
     * is redirected to the already exsting symbol (the old symbol is NOT overwritten).
     *
     * \param addr address of the new symbol
     * \param name (unique) name of the new symbol
     */
    virtual IBinarySymbol& create(Address addr, const QString& name, bool local = false) = 0;

    virtual iterator begin()             = 0;
    virtual const_iterator begin() const = 0;
    virtual iterator end()             = 0;
    virtual const_iterator end() const = 0;
    virtual void clear() = 0;

    //    virtual IBinarySymbol &addSymbol(ADDRESS a) = 0;
    //    virtual bool hasSymbolAt(ADDRESS a) = 0;
    //    virtual bool hasSymbol(const QString &name) = 0;
    //    virtual void addEntryPointSymbol(const QString &) = 0;
    //    virtual void addEntryPoint(ADDRESS) = 0;
    //    virtual void removeEntryPoint(ADDRESS) = 0;
    //    virtual void addImport(ADDRESS) = 0; /// mark address as containing pointer to imported function
    //    virtual void addExport(ADDRESS) = 0; /// mark address as being exported
};
