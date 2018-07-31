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


#include "boomerang/frontend/SigEnum.h"
#include "boomerang/ifc/ISymbolProvider.h"

#include <QMap>


class Prog;


class CSymbolProvider final : public ISymbolProvider
{
public:
    CSymbolProvider(Prog *prog);
    virtual ~CSymbolProvider() = default;

public:
    /// \copydoc ISymbolProvider::readLibraryCatalog
    bool readLibraryCatalog(const QString& fileName) override;

    /// \copydoc ISymbolProvider::addSymbolsFromSymbolFile
    bool addSymbolsFromSymbolFile(const QString& fileName) override;

    /// \copydoc ISymbolProvider::getSignatureByName
    std::shared_ptr<Signature> getSignatureByName(const QString& functionName) const override;

private:
    bool readLibrarySignatures(const QString& signatureFile, CallConv cc);

private:
    Prog *m_prog;
    QMap<QString, std::shared_ptr<Signature>> m_librarySignatures;
};
