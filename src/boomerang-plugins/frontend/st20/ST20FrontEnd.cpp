#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ST20FrontEnd.h"

#include "boomerang/core/Project.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/Register.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/util/log/Log.h"


ST20FrontEnd::ST20FrontEnd(Project *project)
    : DefaultFrontEnd(project)
{
    Plugin *plugin = project->getPluginManager()->getPluginByName("ST20 decoder plugin");
    if (plugin) {
        m_decoder = plugin->getIfc<IDecoder>();
        m_decoder->initialize(project);
    }
}


Address ST20FrontEnd::findMainEntryPoint(bool &gotMain)
{
    gotMain       = true;
    Address start = m_binaryFile->getMainEntryPoint();

    if (start != Address::INVALID) {
        return start;
    }

    start   = m_binaryFile->getEntryPoint();
    gotMain = false;

    if (start == Address::INVALID) {
        return Address::INVALID;
    }

    gotMain = true;
    return start;
}


bool ST20FrontEnd::processProc(UserProc *proc, Address entryAddr)
{
    // Call the base class to do most of the work
    if (!DefaultFrontEnd::processProc(proc, entryAddr)) {
        return false;
    }

    // This will get done twice; no harm
    proc->setEntryFragment();

    return true;
}

BOOMERANG_DEFINE_PLUGIN(PluginType::FrontEnd, ST20FrontEnd, "ST20 FrontEnd plugin",
                        BOOMERANG_VERSION, "Boomerang developers")
