#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "NJMCDecoder.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/Proc.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/Register.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/util/Util.h"
#include "boomerang/util/log/Log.h"

#include <cassert>
#include <cstdarg>
#include <cstring>


NJMCDecoder::NJMCDecoder(Project *project, const QString &sslFileName)
    : IDecoder(project)
    , m_rtlDict(project->getSettings()->debugDecoder)
{
    const Settings *settings = project->getSettings();
    QString realSSLFileName;

    if (!settings->sslFileName.isEmpty()) {
        realSSLFileName = settings->getWorkingDirectory().absoluteFilePath(settings->sslFileName);
    }
    else {
        realSSLFileName = settings->getDataDirectory().absoluteFilePath(sslFileName);
    }

    if (!m_rtlDict.readSSLFile(realSSLFileName)) {
        LOG_ERROR("Cannot read SSL file '%1'", realSSLFileName);
        throw std::runtime_error("Cannot read SSL file");
    }
}


bool NJMCDecoder::initialize(Project *project)
{
    m_prog = project->getProg();
    return true;
}


std::unique_ptr<RTL> NJMCDecoder::instantiate(Address pc, const char *name,
                                              const std::initializer_list<SharedExp> &args)
{
    // Take the argument, convert it to upper case and remove any .'s
    const QString sanitizedName = QString(name).remove(".").toUpper();

    // Put the operands into a vector
    std::vector<SharedExp> actuals(args);

    if (m_prog && m_prog->getProject()->getSettings()->debugDecoder) {
        OStream q_cout(stdout);
        // Display a disassembly of this instruction if requested
        q_cout << pc << ": " << name << " ";

        for (const SharedExp &itd : actuals) {
            if (itd->isIntConst()) {
                int val = itd->access<Const>()->getInt();

                if ((val > 100) || (val < -100)) {
                    q_cout << "0x" << QString::number(val, 16);
                }
                else {
                    q_cout << val;
                }
            }
            else {
                itd->print(q_cout);
            }

            q_cout << " ";
        }

        q_cout << '\n';
    }

    std::unique_ptr<RTL> rtl = m_rtlDict.instantiateRTL(sanitizedName, pc, actuals);
    if (!rtl) {
        LOG_ERROR("Could not find semantics for instruction '%1', treating instruction as NOP",
                  name);
        return m_rtlDict.instantiateRTL("NOP", pc, {});
    }

    return rtl;
}


QString NJMCDecoder::getRegNameByNum(RegNum regNum) const
{
    return m_rtlDict.getRegDB()->getRegNameByNum(regNum);
}


int NJMCDecoder::getRegSizeByNum(RegNum regNum) const
{
    return m_rtlDict.getRegDB()->getRegSizeByNum(regNum);
}


RegNum NJMCDecoder::getRegNumByName(const QString &name) const
{
    return m_rtlDict.getRegDB()->getRegNumByName(name);
}


bool NJMCDecoder::isRestore(HostAddress)
{
    return false;
}
