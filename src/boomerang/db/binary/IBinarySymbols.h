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

typedef std::shared_ptr<class Type> SharedType;


class IBinarySymbol
{
public:
    virtual ~IBinarySymbol() = default;

public:
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
};
