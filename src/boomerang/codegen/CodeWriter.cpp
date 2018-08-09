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


CodeWriter::WriteDest::WriteDest(const QString& outFileName)
    : m_outFile(outFileName)
    , m_os(&m_outFile)
{
    m_outFile.open(QFile::WriteOnly | QFile::Text);
}


CodeWriter::CodeWriter::WriteDest::~WriteDest()
{
    m_os.flush();
    m_outFile.close();
}


bool CodeWriter::writeCode(const Module *module, const QStringList& lines)
{
    WriteDestMap::iterator it = m_dests.find(module);

    if (it == m_dests.end()) {
        const QString outPath = module->getOutPath("c");
        if (!QFile(outPath).exists()) {
            module->makeDirs();
        }

        bool inserted = false;
        std::tie(it, inserted) = m_dests.insert(std::make_pair(module, outPath));
        assert(inserted);
    }

    assert(it != m_dests.end());
    it->second.m_os << lines.join('\n') << '\n';
    return true;
}

