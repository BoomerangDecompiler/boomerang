#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ComplexityFinder.h"


#include "boomerang/ssl/exp/Location.h"
#include "boomerang/db/proc/UserProc.h"


ComplexityFinder::ComplexityFinder(UserProc* proc)
    : m_proc(proc)
{
}


bool ComplexityFinder::preVisit(const std::shared_ptr<Location>& exp, bool& visitChildren)
{
    if (m_proc && (m_proc->findFirstSymbol(exp) != nullptr)) {
        // This is mapped to a local. Count it as zero, not about 3 (m[r28+4] -> memof, regof, plus)
        visitChildren = false;
        return true;
    }

    if (exp->isMemOf() || exp->isArrayIndex()) {
        m_count++; // Count the more complex unaries
    }

    visitChildren = true;
    return true;
}


bool ComplexityFinder::preVisit(const std::shared_ptr<Unary>& /*exp*/, bool& visitChildren)
{
    m_count++;

    visitChildren = true;
    return true;
}


bool ComplexityFinder::preVisit(const std::shared_ptr<Binary>& /*exp*/, bool& visitChildren)
{
    m_count++;
    visitChildren = true;
    return true;
}


bool ComplexityFinder::preVisit(const std::shared_ptr<Ternary>& /*exp*/, bool& visitChildren)
{
    m_count++;
    visitChildren = true;
    return true;
}
