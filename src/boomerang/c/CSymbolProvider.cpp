#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CSymbolProvider.h"


#include "boomerang/c/ansi-c-parser.h"
#include "boomerang/db/Prog.h"
#include "boomerang/util/Log.h"

#include <QFileInfo>
#include <QDir>


CSymbolProvider::CSymbolProvider(Prog* prog)
    : m_prog(prog)
{
}


bool CSymbolProvider::readLibraryCatalog(const QString& filePath)
{
    // TODO: this is a work for generic semantics provider plugin : HeaderReader
    QFile file(filePath);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        LOG_ERROR("Cannot open library signature catalog `%1'", filePath);
        return false;
    }

    QTextStream inf(&file);

    while (!inf.atEnd()) {
        QString sigFilePath;
        inf >> sigFilePath;
        sigFilePath = sigFilePath.mid(0, sigFilePath.indexOf('#')); // cut the line to first '#'

        if ((sigFilePath.size() > 0) && sigFilePath.endsWith('\n')) {
            sigFilePath = sigFilePath.mid(0, sigFilePath.size() - 1);
        }

        if (sigFilePath.isEmpty()) {
            continue;
        }

        CallConv cc = CallConv::C; // Most APIs are C calling convention

        if (sigFilePath == "windows.h") {
            cc = CallConv::Pascal; // One exception
        }

        if (sigFilePath == "mfc.h") {
            cc = CallConv::ThisCall; // Another exception
        }

        const QString sig_path = QFileInfo(filePath).absoluteDir()
            .absoluteFilePath(sigFilePath);
        if (!readLibrarySignatures(qPrintable(sig_path), cc)) {
            return false;
        }
    }

    return true;
}


bool CSymbolProvider::readLibrarySignatures(const QString& signatureFile, CallConv cc)
{
    std::unique_ptr<AnsiCParser> p;

    try {
        p.reset(new AnsiCParser(qPrintable(signatureFile), false));
    }
    catch (const char *err) {
        LOG_ERROR("Cannot read library signature file '%1': %2", signatureFile, err);
        return false;
    }

    p->yyparse(m_prog->getFrontEndId(), cc);

    for (auto& signature : p->signatures) {
        m_librarySignatures[signature->getName()] = signature;
        signature->setSigFilePath(signatureFile);
    }

    return true;
}


bool CSymbolProvider::addSymbolsFromSymbolFile(const QString& fname)
{
    std::unique_ptr<AnsiCParser> parser = nullptr;

    try {
        parser.reset(new AnsiCParser(qPrintable(fname), false));
    }
    catch (const char *msg) {
        LOG_ERROR("Cannot read symbol file '%1': %2", fname, msg);
        return false;
    }

    Platform plat = m_prog->getFrontEndId();
    CallConv cc   = m_prog->isWin32() ? CallConv::Pascal : CallConv::C;

    parser->yyparse(plat, cc);
    Module *targetModule = m_prog->getRootModule();

    for (Symbol *sym : parser->symbols) {
        if (sym->sig) {
            QString name = sym->sig->getName();
            targetModule = m_prog->getOrInsertModuleForSymbol(name);
            auto bin_sym     = m_prog->getBinaryFile()->getSymbols()->findSymbolByAddress(sym->addr);
            const bool isLib = (bin_sym && bin_sym->isImportedFunction()) ||
                // NODECODE isn't really the right modifier; perhaps we should have a LIB modifier,
                // to specifically specify that this function obeys library calling conventions
                sym->mods->noDecode;
            Function *p = targetModule->createFunction(name, sym->addr, isLib);

            if (!sym->mods->incomplete) {
                p->setSignature(sym->sig->clone());
                p->getSignature()->setForced(true);
            }
        }
        else {
            QString name = sym->name;
            SharedType ty = sym->ty;

            m_prog->createGlobal(sym->addr, sym->ty, sym->name);
        }
    }

    for (SymbolRef *ref : parser->refs) {
        m_prog->getFrontEnd()->addRefHint(ref->m_addr, ref->m_name);
    }

    return true;
}


std::shared_ptr<Signature> CSymbolProvider::getSignatureByName(const QString& functionName) const
{
    auto it = m_librarySignatures.find(functionName);
    return it != m_librarySignatures.end()
        ? it.value()
        : nullptr;
}
