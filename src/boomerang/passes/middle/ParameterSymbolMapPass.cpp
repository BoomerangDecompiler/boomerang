#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ParameterSymbolMapPass.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/visitor/expmodifier/ImplicitConverter.h"


ParameterSymbolMapPass::ParameterSymbolMapPass()
    : IPass("ParameterSymbolMap", PassID::ParameterSymbolMap)
{
}


bool ParameterSymbolMapPass::execute(UserProc *proc)
{
    ImplicitConverter ic(proc->getCFG());
    int i = 0;

    for (auto it = proc->getParameters().begin(); it != proc->getParameters().end(); ++it, ++i) {
        SharedExp lhs = (*it)->as<Assignment>()->getLeft();
        lhs           = lhs->expSubscriptAllNull();
        lhs           = lhs->acceptModifier(&ic);
        SharedExp to  = Location::param(proc->getSignature()->getParamName(i), proc);
        proc->mapSymbolTo(lhs, to);
    }

    return true;
}
