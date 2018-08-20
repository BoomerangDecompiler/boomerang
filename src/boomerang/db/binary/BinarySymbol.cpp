#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BinarySymbol.h"


BinarySymbol::BinarySymbol(Address location, const QString &name)
    : m_name(name)
    , m_address(location)
    , m_size(0)
{}


void BinarySymbol::setAttribute(const QString &key, const QVariant &value) const
{
    m_attributes[key] = value;
}


bool BinarySymbol::isImported() const
{
    return m_attributes.contains("Imported") && m_attributes["Imported"].toBool();
}


QString BinarySymbol::belongsToSourceFile() const
{
    if (!m_attributes.contains("SourceFile")) {
        return "";
    }

    return m_attributes["SourceFile"].toString();
}


bool BinarySymbol::isFunction() const
{
    return m_attributes.contains("Function") && m_attributes["Function"].toBool();
}


bool BinarySymbol::isImportedFunction() const
{
    return isImported() && isFunction();
}


bool BinarySymbol::isStaticFunction() const
{
    return m_attributes.contains("StaticFunction") && m_attributes["StaticFunction"].toBool();
}
