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


TempToLocalMapper::TempToLocalMapper(UserProc* p)
    : proc(p)
{
}


bool TempToLocalMapper::visit(const std::shared_ptr<Location>& e, bool& override)
{
    if (e->isTemp()) {
        // We have a temp subexpression; get its name
        QString    tempName = e->access<Const, 1>()->getStr();
        SharedType ty       = Type::getTempType(tempName); // Types for temps strictly depend on the name
        // This call will do the mapping from the temp to a new local:
        proc->getSymbolExp(e, ty, true);
    }

    override = true; // No need to examine the string
    return true;
}
