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
    QString     sig_path;

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

        sig_path = QFileInfo(filePath).baseName() + sigFilePath;
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


std::shared_ptr<Signature> CSymbolProvider::getSignatureByName(const QString& functionName) const
{
    auto it = m_librarySignatures.find(functionName);
    return it != m_librarySignatures.end()
        ? it.value()
        : nullptr;
}
