#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BlockVarRenamePass.h"


#include "boomerang/db/proc/UserProc.h"


BlockVarRenamePass::BlockVarRenamePass()
    : IPass("BlockVarRename", PassID::BlockVarRename)
{
}


bool BlockVarRenamePass::execute(UserProc *proc)
{
    return proc->getDataFlow()->renameBlockVars();
}
