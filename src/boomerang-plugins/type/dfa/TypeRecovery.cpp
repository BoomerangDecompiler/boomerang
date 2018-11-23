#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "TypeRecovery.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/module/Module.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/util/log/Log.h"


TypeRecoveryCommon::TypeRecoveryCommon(Project *project, const QString &name)
    : ITypeRecovery(project)
    , m_name(name)
{
}


const QString &TypeRecoveryCommon::getName()
{
    return m_name;
}


void TypeRecoveryCommon::recoverProgramTypes(Prog *prog)
{
    if (prog->getProject()->getSettings()->debugTA) {
        LOG_VERBOSE("=== start %1 type analysis ===", getName());
    }

    // FIXME: This needs to be done in bottom-up order of the call-tree first,
    // repeating until no changes for cycles in the call graph
    for (const auto &module : prog->getModuleList()) {
        for (Function *pp : *module) {
            UserProc *proc = dynamic_cast<UserProc *>(pp);

            if (!proc || !proc->isDecoded()) {
                continue;
            }

            // FIXME: this just does local TA again. Need to resolve types for all
            // parameter/arguments, and return/results! This will require a "repeat until no change"
            // loop
            LOG_VERBOSE("Global type analysis for %1", proc->getName());
            recoverFunctionTypes(pp);
        }
    }

    if (prog->getProject()->getSettings()->debugTA) {
        LOG_VERBOSE("=== end type analysis ===");
    }
}
