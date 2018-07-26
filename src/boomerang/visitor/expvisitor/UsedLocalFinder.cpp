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


#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/exp/TypedExp.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/type/Type.h"


UsedLocalFinder::UsedLocalFinder(LocationSet& used, UserProc* proc)
    : m_used(&used)
    , m_proc(proc)
    , all(false)
{
}

bool UsedLocalFinder::preVisit(const std::shared_ptr<Location>& exp, bool& visitChildren)
{
    visitChildren = true;

    if (exp->isLocal()) {
        m_used->insert(exp); // Found a local
    }

    return true;         // Continue looking for other locations
}


bool UsedLocalFinder::preVisit(const std::shared_ptr<TypedExp>& exp, bool& visitChildren)
{
    visitChildren = true;
    SharedType ty = exp->getType();

    // Assumption: (cast)exp where cast is of pointer type means that exp is the address of a local
    if (ty->resolvesToPointer()) {
        SharedExp sub = exp->getSubExp1();
        SharedExp mof = Location::memOf(sub);

        if (!m_proc->findLocal(mof, ty).isNull()) {
            m_used->insert(mof);
            visitChildren = false;
        }
    }

    return true;
}


bool UsedLocalFinder::visit(const std::shared_ptr<Terminal>& exp)
{
    if (exp->getOper() == opDefineAll) {
        all = true;
    }

    QString sym = m_proc->findFirstSymbol(exp);

    if (!sym.isNull()) {
        m_used->insert(exp);
    }

    return true; // Always continue recursion
}
