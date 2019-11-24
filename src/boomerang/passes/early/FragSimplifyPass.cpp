#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "FragSimplifyPass.h"

#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/util/log/Log.h"


FragSimplifyPass::FragSimplifyPass()
    : IPass("FragSimplify", PassID::FragSimplify)
{
}


bool FragSimplifyPass::execute(UserProc *proc)
{
    for (IRFragment *frag : *proc->getCFG()) {
        frag->simplify();
    }

    return true;
}
