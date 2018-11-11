#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DominatorPass.h"

#include "boomerang/db/proc/UserProc.h"


DominatorPass::DominatorPass()
    : IPass("Dominator", PassID::Dominators)
{
}


bool DominatorPass::execute(UserProc *proc)
{
    return proc->getDataFlow()->calculateDominators();
}
