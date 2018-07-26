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


#include <memory>


class QString;
class Signature;


/**
 * Provides declarations for symbols and library function signatures.
 */
class ISymbolProvider
{
public:
    ISymbolProvider() = default;
    virtual ~ISymbolProvider() = default;

public:
    /// Read a catalog for library signatures.
    /// \returns true on success.
    virtual bool readLibraryCatalog(const QString& fileName) = 0;

    /// \returns a library signature by its name
    virtual std::shared_ptr<Signature> getSignatureByName(const QString& functionName) const = 0;
};
