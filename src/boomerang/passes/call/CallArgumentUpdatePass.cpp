#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CallArgumentUpdatePass.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/util/log/Log.h"


CallArgumentUpdatePass::CallArgumentUpdatePass()
    : IPass("CallArgumentUpdate", PassID::CallArgumentUpdate)
{
}


bool CallArgumentUpdatePass::execute(UserProc *proc)
{
    proc->getProg()->getProject()->alertDecompiling(proc);

    for (IRFragment *bb : *proc->getCFG()) {
        IRFragment::RTLRIterator rrit;
        StatementList::reverse_iterator srit;
        SharedStmt s = bb->getLastStmt(rrit, srit);

        // Note: we may have removed some statements, so there may no longer be a last statement!
        if (!s || !s->isCall()) {
            continue;
        }

        s->as<CallStatement>()->updateArguments();
        LOG_VERBOSE2("Updated call statement to %1", s->as<CallStatement>());
    }

    return true;
}
