#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CallLivenessRemovalPass.h"

#include "boomerang/db/IRFragment.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/statements/CallStatement.h"


CallLivenessRemovalPass::CallLivenessRemovalPass()
    : IPass("CallLivenessRemoval", PassID::CallLivenessRemoval)
{
}


bool CallLivenessRemovalPass::execute(UserProc *proc)
{
    for (IRFragment *frag : *proc->getCFG()) {
        const SharedStmt last = frag->getLastStmt();
        if (!last || !last->isCall()) {
            continue;
        }

        last->as<CallStatement>()->removeAllLive();
    }

    return true;
}
