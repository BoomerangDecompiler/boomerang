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
#include "boomerang/db/visitor/expmodifier/ImplicitConverter.h"
#include "boomerang/db/exp/Location.h"


ParameterSymbolMapPass::ParameterSymbolMapPass()
    : IPass("ParameterSymbolMap", PassID::ParameterSymbolMap)
{
}


bool ParameterSymbolMapPass::execute(UserProc* proc)
{
    ImplicitConverter ic(proc->getCFG());
    int i = 0;

    for (auto it = proc->getParameters().begin(); it != proc->getParameters().end(); ++it, ++i) {
        SharedExp lhs = static_cast<Assignment *>(*it)->getLeft();
        lhs = lhs->expSubscriptAllNull();
        lhs = lhs->accept(&ic);
        SharedExp to = Location::param(proc->getSignature()->getParamName(i), proc);
        proc->mapSymbolTo(lhs, to);
    }

    return true;
}
