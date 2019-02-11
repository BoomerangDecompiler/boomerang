#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BadMemofFinder.h"

#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"


BadMemofFinder::BadMemofFinder()
    : m_found(false)
{
}


bool BadMemofFinder::preVisit(const std::shared_ptr<Location> &exp, bool &visitChildren)
{
    if (exp->isMemOf()) {
        m_found = true; // A bare memof
        return false;
    }

    visitChildren = true;
    return true; // Continue searching
}


bool BadMemofFinder::preVisit(const std::shared_ptr<RefExp> &exp, bool &visitChildren)
{
    SharedExp base = exp->getSubExp1();

    if (base->isMemOf()) {
        // Beware: it may be possible to have a bad memof inside a subscripted one
        SharedExp addr = base->getSubExp1();
        addr->acceptVisitor(this);

        if (m_found) {
            return false; // Don't continue searching
        }
    }

    visitChildren = false; // Don't look inside the refexp
    return true;           // It has a symbol; nothing bad found yet but continue searching
}
