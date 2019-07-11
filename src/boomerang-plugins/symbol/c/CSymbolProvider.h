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
#include "boomerang/frontend/SigEnum.h"
#include "boomerang/ifc/ISymbolProvider.h"

#include <QMap>


class Prog;


/// Symbol provider for reading signatures and symbols from C-like headers.
/// (cf. also the files in data/signature/)
class BOOMERANG_PLUGIN_API CSymbolProvider : public ISymbolProvider
{
public:
    CSymbolProvider(Project *project);
    virtual ~CSymbolProvider() = default;

public:
    /// \copydoc ISymbolProvider::readLibraryCatalog
    bool readLibraryCatalog(const Prog *prog, const QString &fileName) override;

    /// \copydoc ISymbolProvider::addSymbolsFromSymbolFile
    bool addSymbolsFromSymbolFile(Prog *prog, const QString &fileName) override;

    /// \copydoc ISymbolProvider::getSignatureByName
    std::shared_ptr<Signature> getSignatureByName(const QString &functionName) const override;

private:
    bool readLibrarySignatures(const QString &signatureFile, const Prog *prog, CallConv cc);

private:
    QMap<QString, std::shared_ptr<Signature>> m_librarySignatures;
};
