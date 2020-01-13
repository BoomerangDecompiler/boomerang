#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UsedLocsFinder.h"

#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/util/LocationSet.h"


UsedLocsFinder::UsedLocsFinder(LocationSet &used, bool memOnly)
    : m_used(&used)
    , m_memOnly(memOnly)
{
}


bool UsedLocsFinder::preVisit(const std::shared_ptr<Location> &exp, bool &visitChildren)
{
    if (!m_memOnly) {
        m_used->insert(exp->shared_from_this()); // All locations visited are used
    }

    if (exp->isMemOf()) {
        // Example: m[r28{10} - 4]    we use r28{10}
        SharedExp child = exp->access<Exp, 1>();
        // Care! Need to turn off the memOnly flag for work inside the m[...], otherwise everything
        // will get ignored
        bool wasMemOnly = m_memOnly;
        m_memOnly       = false;
        child->acceptVisitor(this);
        m_memOnly     = wasMemOnly;
        visitChildren = false; // Already looked inside child
    }
    else {
        visitChildren = true;
    }

    return true; // Continue looking for other locations
}


bool UsedLocsFinder::visit(const std::shared_ptr<Terminal> &exp)
{
    if (m_memOnly) {
        return true; // Only interested in m[...]
    }

    switch (exp->getOper()) {
    case opPC:
    case opFlags:
    case opFflags:
    case opDefineAll:
    // Fall through
    case opDF:
    case opCF:
    case opZF:
    case opNF:
    case opOF: // also these
        m_used->insert(exp);
        break;

    default: break;
    }

    return true; // Always continue recursion
}


bool UsedLocsFinder::preVisit(const std::shared_ptr<RefExp> &exp, bool &visitChildren)
{
    if (m_memOnly) {
        visitChildren = true; // Look inside the ref for m[...]
        return true;          // Don't count this reference
    }

    std::shared_ptr<RefExp> e = exp;
    m_used->insert(e); // This location is used

    // However, e's subexpression is NOT used ...
    visitChildren = false;
    // ... unless that is a m[x], array[x] or .x, in which case x (not m[x]/array[x]/refd.x) is used
    SharedExp refd = e->getSubExp1();

    if (refd->isMemOf()) {
        refd->getSubExp1()->acceptVisitor(this);
    }
    else if (refd->isArrayIndex()) {
        refd->getSubExp1()->acceptVisitor(this);
        refd->getSubExp2()->acceptVisitor(this);
    }
    else if (refd->isMemberOf()) {
        refd->getSubExp1()->acceptVisitor(this);
    }

    return true;
}
