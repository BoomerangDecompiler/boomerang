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


class QString;


class ISymbolProvider
{
public:
    ISymbolProvider() = default;
    virtual ~ISymbolProvider() = default;

public:
    /// Read a catalog for library signatures.
    /// \returns true on success.
    bool readLibraryCatalog(const QString& fileName);
};
