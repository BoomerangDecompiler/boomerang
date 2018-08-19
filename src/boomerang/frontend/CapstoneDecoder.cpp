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


CapstoneDecoder::CapstoneDecoder(Prog *prog, cs::cs_arch arch, cs::cs_mode mode,
                                 const QString &sslFileName)
    : m_prog(prog)
    , m_dict(prog->getProject()->getSettings()->debugDecoder)
    , m_debugMode(prog->getProject()->getSettings()->debugDecoder)
{
    cs::cs_open(arch, mode, &m_handle);
    cs::cs_option(m_handle, cs::CS_OPT_DETAIL, cs::CS_OPT_ON);

    m_dict.readSSLFile(
        prog->getProject()->getSettings()->getDataDirectory().absoluteFilePath(sslFileName));
}


CapstoneDecoder::~CapstoneDecoder()
{
    cs::cs_close(&m_handle);
}


bool CapstoneDecoder::isInstructionInGroup(const cs::cs_insn *instruction, uint8_t group)
{
    for (int i = 0; i < instruction->detail->groups_count; i++) {
        if (instruction->detail->groups[i] == group) {
            return true;
        }
    }

    return false;
}
