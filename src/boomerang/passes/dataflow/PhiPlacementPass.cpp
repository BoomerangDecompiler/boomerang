#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PhiPlacementPass.h"

#include "boomerang/db/proc/UserProc.h"


PhiPlacementPass::PhiPlacementPass()
    : IPass("PhiPlacement", PassID::PhiPlacement)
{}


bool PhiPlacementPass::execute(UserProc *proc)
{
    return proc->getDataFlow()->placePhiFunctions();
}
