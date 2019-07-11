#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "LocalTypeAnalysisPass.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ifc/ITypeRecovery.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/util/log/Log.h"


LocalTypeAnalysisPass::LocalTypeAnalysisPass()
    : IPass("LocalTypeAnalysis", PassID::LocalTypeAnalysis)
{
}


bool LocalTypeAnalysisPass::execute(UserProc *proc)
{
    // Now we need to add the implicit assignments. Doing this earlier
    // is extremely problematic, because of all the m[...] that change
    // their sorting order as their arguments get subscripted or propagated into.
    // Do this regardless of whether doing dfa-based TA, so things
    // like finding parameters can rely on implicit assigns.
    PassManager::get()->executePass(PassID::ImplicitPlacement, proc);

    Project *project   = proc->getProg()->getProject();
    ITypeRecovery *rec = project->getTypeRecoveryEngine();

    // Data flow based type analysis
    // Want to be after all propagation, but before converting expressions to locals etc
    if (rec && project->getSettings()->useTypeAnalysis) {
        rec->recoverFunctionTypes(proc);
        return true;
    }
    else {
        return false;
    }
}
