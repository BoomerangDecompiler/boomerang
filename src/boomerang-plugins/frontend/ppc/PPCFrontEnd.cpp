#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PPCFrontEnd.h"

#include "boomerang/core/Project.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/Register.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/util/log/Log.h"

#include <cassert>
#include <iomanip>
#include <sstream>


PPCFrontEnd::PPCFrontEnd(Project *project)
    : DefaultFrontEnd(project)
{
    Plugin *plugin = project->getPluginManager()->getPluginByName("Capstone PPC decoder plugin");
    if (plugin) {
        m_decoder = plugin->getIfc<IDecoder>();
        m_decoder->initialize(project);
    }
}


Address PPCFrontEnd::findMainEntryPoint(bool &gotMain)
{
    const Address mainAddr = m_binaryFile->getMainEntryPoint();
    if (mainAddr != Address::INVALID) {
        gotMain = true;
        return mainAddr;
    }

    const Address entryPoint = m_binaryFile->getEntryPoint();
    if (entryPoint != Address::INVALID) {
        gotMain = true;
        return entryPoint;
    }

    gotMain = false;
    return Address::INVALID;
}


bool PPCFrontEnd::processProc(UserProc *proc, Address entryAddr)
{
    // Call the base class to do most of the work
    if (!DefaultFrontEnd::processProc(proc, entryAddr)) {
        return false;
    }

    // This will get done twice; no harm
    proc->setEntryBB();

    return true;
}


BOOMERANG_DEFINE_PLUGIN(PluginType::FrontEnd, PPCFrontEnd, "PPC FrontEnd plugin", BOOMERANG_VERSION,
                        "Boomerang developers")
