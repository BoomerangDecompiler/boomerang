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


#include "boomerang/db/Managed.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Terminal.h"


UsedLocsFinder::UsedLocsFinder(LocationSet& _used, bool _memOnly)
    : m_used(&_used)
    , m_memOnly(_memOnly)
{}


bool UsedLocsFinder::visit(const std::shared_ptr<Location>& e, bool& override)
{
    if (!m_memOnly) {
        m_used->insert(e->shared_from_this());       // All locations visited are used
    }

    if (e->isMemOf()) {
        // Example: m[r28{10} - 4]    we use r28{10}
        SharedExp child = e->access<Exp, 1>();
        // Care! Need to turn off the memOnly flag for work inside the m[...], otherwise everything will get ignored
        bool wasMemOnly = m_memOnly;
        m_memOnly = false;
        child->accept(this);
        m_memOnly = wasMemOnly;
        override  = true; // Already looked inside child
    }
    else {
        override = false;
    }

    return true; // Continue looking for other locations
}


bool UsedLocsFinder::visit(const std::shared_ptr<Terminal>& e)
{
    if (m_memOnly) {
        return true; // Only interested in m[...]
    }

    switch (e->getOper())
    {
    case opPC:
    case opFlags:
    case opFflags:
    case opDefineAll:
    // Fall through
    // The carry flag can be used in some SPARC idioms, etc
    case opDF:
    case opCF:
    case opZF:
    case opNF:
    case opOF: // also these
        m_used->insert(e);
        break;

    default:
        break;
    }

    return true; // Always continue recursion
}


bool UsedLocsFinder::visit(const std::shared_ptr<RefExp>& arg, bool& override)
{
    if (m_memOnly) {
        override = false; // Look inside the ref for m[...]
        return true;      // Don't count this reference
    }

    std::shared_ptr<RefExp> e = arg;

    if (m_used->find(e) == m_used->end()) {
        // e = (RefExp *)arg.clone();
        m_used->insert(e);       // This location is used
    }

    // However, e's subexpression is NOT used ...
    override = true;
    // ... unless that is a m[x], array[x] or .x, in which case x (not m[x]/array[x]/refd.x) is used
    SharedExp refd = e->getSubExp1();

    if (refd->isMemOf()) {
        refd->getSubExp1()->accept(this);
    }
    else if (refd->isArrayIndex()) {
        refd->getSubExp1()->accept(this);
        refd->getSubExp2()->accept(this);
    }
    else if (refd->isMemberOf()) {
        refd->getSubExp1()->accept(this);
    }

    return true;
}

