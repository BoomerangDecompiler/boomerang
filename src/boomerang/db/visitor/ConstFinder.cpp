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


ConstFinder::ConstFinder(std::list<std::shared_ptr<Const> >& _lc)
    : m_constList(_lc)
{
}


bool ConstFinder::visit(const std::shared_ptr<Const>& e)
{
    m_constList.push_back(e);
    return true;
}


bool ConstFinder::visit(const std::shared_ptr<Location>& e, bool& override)
{
    if (e->isMemOf()) {
        override = false; // We DO want to see constants in memofs
    }
    else {
        override = true; // Don't consider register numbers, global names, etc
    }

    return true;
}
