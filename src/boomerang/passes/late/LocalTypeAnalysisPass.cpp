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


#include "boomerang/db/proc/UserProc.h"
#include "boomerang/util/Log.h"
#include "boomerang/type/TypeRecovery.h"
#include "boomerang/core/Boomerang.h"


LocalTypeAnalysisPass::LocalTypeAnalysisPass()
    : IPass("LocalTypeAnalysis", PassID::LocalTypeAnalysis)
{
}


bool LocalTypeAnalysisPass::execute(UserProc *proc)
{
    // Now we need to add the implicit assignments. Doing this earlier is extremely problematic, because
    // of all the m[...] that change their sorting order as their arguments get subscripted or propagated into
    // Do this regardless of whether doing dfa-based TA, so things like finding parameters can rely on implicit assigns
    proc->addImplicitAssigns();

    ITypeRecovery *rec = Boomerang::get()->getOrCreateProject()->getTypeRecoveryEngine();
    // Data flow based type analysis
    // Want to be after all propagation, but before converting expressions to locals etc
    if (DFA_TYPE_ANALYSIS) {
        rec->recoverFunctionTypes(proc);
    }

    return true;
}
