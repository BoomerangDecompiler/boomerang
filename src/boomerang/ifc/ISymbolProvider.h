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


#include "boomerang/core/BoomerangAPI.h"

#include <memory>


class QString;
class Signature;
class Prog;
class Project;


/**
 * Provides declarations for symbols and library function signatures.
 */
class BOOMERANG_API ISymbolProvider
{
public:
    ISymbolProvider(Project *) {}
    virtual ~ISymbolProvider() = default;

public:
    /// Read a catalog for library signatures.
    /// \returns true on success.
    virtual bool readLibraryCatalog(const Prog *prog, const QString &fileName) = 0;

    /// Add symbol information from a symbol file to the program.
    /// \returns true on success.
    virtual bool addSymbolsFromSymbolFile(Prog *prog, const QString &fileName) = 0;

    /// \returns a library signature by its name
    virtual std::shared_ptr<Signature> getSignatureByName(const QString &functionName) const = 0;
};
