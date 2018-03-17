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



bool BinarySymbol::isImported() const
{
    return attributes.contains("Imported") && attributes["Imported"].toBool();
}


QString BinarySymbol::belongsToSourceFile() const
{
    if (!attributes.contains("SourceFile")) {
        return "";
    }

    return attributes["SourceFile"].toString();
}


bool BinarySymbol::isFunction() const
{
    return attributes.contains("Function") && attributes["Function"].toBool();
}


bool BinarySymbol::isImportedFunction() const
{
    return isImported() && isFunction();
}


bool BinarySymbol::isStaticFunction() const
{
    return attributes.contains("StaticFunction") && attributes["StaticFunction"].toBool();
}
