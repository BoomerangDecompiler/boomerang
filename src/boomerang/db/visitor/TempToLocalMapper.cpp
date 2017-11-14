#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "TempToLocalMapper.h"


#include "boomerang/db/exp/Location.h"
#include "boomerang/db/proc/UserProc.h"


TempToLocalMapper::TempToLocalMapper(UserProc* proc)
    : m_proc(proc)
{
}


bool TempToLocalMapper::visit(const std::shared_ptr<Location>& exp, bool& visitChildren)
{
    if (exp->isTemp()) {
        // We have a temp subexpression; get its name
        QString    tempName = exp->access<Const, 1>()->getStr();
        SharedType ty       = Type::getTempType(tempName); // Types for temps strictly depend on the name
        // This call will do the mapping from the temp to a new local:
        m_proc->getSymbolExp(exp, ty, true);
    }

    visitChildren = false; // No need to examine the string
    return true;
}
