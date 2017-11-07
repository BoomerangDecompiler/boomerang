#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ConstFinder.h"


#include "boomerang/db/exp/Location.h"


ConstFinder::ConstFinder(std::list<std::shared_ptr<Const> >& list)
    : m_constList(list)
{
}


bool ConstFinder::visit(const std::shared_ptr<Const>& exp)
{
    m_constList.push_back(exp);
    return true;
}


bool ConstFinder::visit(const std::shared_ptr<Location>& exp, bool& visitChildren)
{
    if (exp->isMemOf()) {
        visitChildren = true; // We DO want to see constants in memofs
    }
    else {
        visitChildren = false; // Don't consider register numbers, global names, etc
    }

    return true;
}
