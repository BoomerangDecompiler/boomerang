#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UsedLocalFinder.h"


#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/TypedExp.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Managed.h"


UsedLocalFinder::UsedLocalFinder(LocationSet& _used, UserProc* _proc)
    : used(&_used)
    , proc(_proc)
    , all(false)
{
}

bool UsedLocalFinder::visit(const std::shared_ptr<Location>& e, bool& override)
{
    override = false;

    if (e->isLocal()) {
        used->insert(e); // Found a local
    }

    return true;         // Continue looking for other locations
}


bool UsedLocalFinder::visit(const std::shared_ptr<TypedExp>& e, bool& override)
{
    override = false;
    SharedType ty = e->getType();

    // Assumption: (cast)exp where cast is of pointer type means that exp is the address of a local
    if (ty->resolvesToPointer()) {
        SharedExp sub = e->getSubExp1();
        SharedExp mof = Location::memOf(sub);

        if (!proc->findLocal(mof, ty).isNull()) {
            used->insert(mof);
            override = true;
        }
    }

    return true;
}


bool UsedLocalFinder::visit(const std::shared_ptr<Terminal>& e)
{
    if (e->getOper() == opDefineAll) {
        all = true;
    }

    QString sym = proc->findFirstSymbol(e);

    if (!sym.isNull()) {
        used->insert(e);
    }

    return true; // Always continue recursion
}
