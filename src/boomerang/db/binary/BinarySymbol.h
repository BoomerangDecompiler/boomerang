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


typedef std::shared_ptr<class Type> SharedType;


class BinarySymbol
{
    friend class BinarySymbolTable;

public:
    BinarySymbol(Address location, const QString& name)
        : Location(location)
        , Name(name)
    {}

public:
    const QString& getName() const { return Name; }
    size_t getSize() const { return Size; }
    void setSize(size_t v) { Size = v; }
    Address getLocation() const { return Location; }

    const BinarySymbol& setAttr(const QString& name, const QVariant& v) const
    {
        attributes[name] = v;
        return *this;
    }

    bool isImportedFunction() const;
    bool isStaticFunction() const;
    bool isFunction() const;
    bool isImported() const;
    QString belongsToSourceFile() const;

private:
    Address              Location;
    QString              Name;
    SharedType           type;
    size_t               Size;
    /// it's mutable since no changes in attribute map will influence the layout of symbols in SymTable
    mutable QVariantMap  attributes;
};
