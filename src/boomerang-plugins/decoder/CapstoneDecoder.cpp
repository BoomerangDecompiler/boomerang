#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CapstoneDecoder.h"

#include "inttypes.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/util/log/Log.h"


CapstoneDecoder::CapstoneDecoder(Project *project, cs::cs_arch arch, cs::cs_mode mode,
                                 const QString &sslFileName)
    : IDecoder(project)
    , m_dict(project->getSettings()->debugDecoder)
    , m_debugMode(project->getSettings()->debugDecoder)
{
    cs::cs_open(arch, mode, &m_handle);
    cs::cs_option(m_handle, cs::CS_OPT_DETAIL, cs::CS_OPT_ON);

    const Settings *settings = project->getSettings();
    QString realSSLFileName;

    if (!settings->sslFileName.isEmpty()) {
        realSSLFileName = settings->getWorkingDirectory().absoluteFilePath(settings->sslFileName);
    }
    else {
        realSSLFileName = settings->getDataDirectory().absoluteFilePath(sslFileName);
    }

    if (!m_dict.readSSLFile(realSSLFileName)) {
        LOG_ERROR("Cannot read SSL file '%1'", realSSLFileName);
        throw std::runtime_error("Cannot read SSL file");
    }
}


CapstoneDecoder::~CapstoneDecoder()
{
    cs::cs_close(&m_handle);
}


bool CapstoneDecoder::initialize(Project *project)
{
    m_prog = project->getProg();
    return true;
}


bool CapstoneDecoder::isInstructionInGroup(const cs::cs_insn *instruction, uint8_t group) const
{
    for (int i = 0; i < instruction->detail->groups_count; i++) {
        if (instruction->detail->groups[i] == group) {
            return true;
        }
    }

    return false;
}
