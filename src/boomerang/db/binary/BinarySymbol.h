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


typedef std::shared_ptr<class Type> SharedType;


class BinarySymbol : public IBinarySymbol
{
    friend class BinarySymbolTable;

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
