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

#include <QMetaType>
#include <QVariantMap>

#include <memory>


typedef std::shared_ptr<class Type> SharedType;


class BOOMERANG_API BinarySymbol
{
    friend class BinarySymbolTable;

public:
    BinarySymbol(Address location, const QString &name);

public:
    const QString &getName() const { return m_name; }
    int getSize() const { return m_size; }
    void setSize(int v) { m_size = v; }
    Address getLocation() const { return m_address; }

    void setAttribute(const QString &key, const QVariant &value) const;

    bool isImportedFunction() const;
    bool isStaticFunction() const;
    bool isFunction() const;
    bool isImported() const;
    QString belongsToSourceFile() const;

private:
    QString m_name;
    Address m_address = Address::INVALID;
    int m_size        = 0;
    SharedType m_type;

    /// it's mutable since no changes in attribute map will influence
    /// the layout of symbols in SymTable
    mutable QVariantMap m_attributes;
};
