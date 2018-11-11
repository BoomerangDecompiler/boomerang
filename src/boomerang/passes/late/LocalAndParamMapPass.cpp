#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "LocalAndParamMapPass.h"

#include "boomerang/core/Project.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/util/log/Log.h"


LocalAndParamMapPass::LocalAndParamMapPass()
    : IPass("LocalAndParamMap", PassID::LocalAndParamMap)
{
}


bool LocalAndParamMapPass::execute(UserProc *proc)
{
    proc->getProg()->getProject()->alertDecompileDebugPoint(
        proc, "Before mapping locals from dfa type analysis");

    LOG_VERBOSE("### Mapping expressions to local variables for %1 ###", proc->getName());

    StatementList stmts;
    proc->getStatements(stmts);

    for (Statement *s : stmts) {
        s->dfaMapLocals();
    }

    LOG_VERBOSE("### End mapping expressions to local variables for %1 ###", proc->getName());
    return true;
}
