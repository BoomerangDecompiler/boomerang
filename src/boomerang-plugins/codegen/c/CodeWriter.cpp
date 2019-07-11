#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CodeWriter.h"

#include "boomerang/db/module/Module.h"

#include <cassert>


CodeWriter::WriteDest::WriteDest(const QString &outFileName)
    : m_outFile(outFileName)
    , m_os(&m_outFile)
{
    if (!m_outFile.open(QFile::WriteOnly | QFile::Text)) {
        throw std::runtime_error("Could not open file!");
    }
}


CodeWriter::CodeWriter::WriteDest::~WriteDest()
{
    m_os.flush();
    m_outFile.close();
}


CodeWriter::CodeWriter()
{
}


bool CodeWriter::writeCode(const Module *module, const QStringList &lines)
{
    WriteDestMap::iterator it = m_dests.find(module);

    if (it == m_dests.end()) {
        const QString outPath = module->getOutPath("c");
        if (!QFile(outPath).exists()) {
            module->makeDirs();
        }

        bool inserted = false;
        try {
            std::tie(it, inserted) = m_dests.insert(std::make_pair(module, outPath));
            assert(inserted);
        }
        catch (const std::runtime_error &) {
            return false;
        }
    }

    assert(it != m_dests.end());
    it->second << lines.join('\n') << '\n';
    return true;
}
