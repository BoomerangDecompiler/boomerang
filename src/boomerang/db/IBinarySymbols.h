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
    virtual bool rename(const QString& s) = 0; ///< Rename an existing symbol
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

    virtual const IBinarySymbol *find(Address a) const        = 0; ///< Find an entry by address; nullptr if none
    virtual const IBinarySymbol *find(const QString& s) const = 0; ///< Find an entry by name; Address::INVALID if none

    /// Add a new symbol to table, if \a local is set than the symbol is local, thus it won't be
    /// added to global name->symbol mapping
    virtual IBinarySymbol& create(Address a, const QString& s, bool local = false) = 0;

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
