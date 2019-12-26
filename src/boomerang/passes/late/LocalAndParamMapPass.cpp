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
#include "boomerang/visitor/expmodifier/DFALocalMapper.h"
#include "boomerang/visitor/stmtmodifier/StmtModifier.h"


LocalAndParamMapPass::LocalAndParamMapPass()
    : IPass("LocalAndParamMap", PassID::LocalAndParamMap)
{
}


bool LocalAndParamMapPass::execute(UserProc *proc)
{
    LOG_VERBOSE("### Mapping expressions to local variables for %1 ###", proc->getName());

    StatementList stmts;
    proc->getStatements(stmts);

    for (SharedStmt s : stmts) {
        DfaLocalMapper dlm(proc);
        StmtModifier sm(&dlm, true); // True to ignore def collector in return statement

        s->accept(&sm);

        if (dlm.change) {
            LOG_VERBOSE2("Statement '%1' mapped with new local(s)", s);
        }
    }

    LOG_VERBOSE("### End mapping expressions to local variables for %1 ###", proc->getName());
    return true;
}
