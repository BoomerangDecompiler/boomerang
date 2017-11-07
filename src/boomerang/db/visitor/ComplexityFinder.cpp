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


#include "boomerang/db/exp/Location.h"
#include "boomerang/db/proc/UserProc.h"


ComplexityFinder::ComplexityFinder(UserProc* proc)
    : m_proc(proc)
{
}


bool ComplexityFinder::visit(const std::shared_ptr<Location>& exp, bool& dontVisitChildren)
{
    if (m_proc && (m_proc->findFirstSymbol(exp) != nullptr)) {
        // This is mapped to a local. Count it as zero, not about 3 (m[r28+4] -> memof, regof, plus)
        dontVisitChildren = true;
        return true;
    }

    if (exp->isMemOf() || exp->isArrayIndex()) {
        m_count++; // Count the more complex unaries
    }

    dontVisitChildren = false;
    return true;
}


bool ComplexityFinder::visit(const std::shared_ptr<Unary>& /*exp*/, bool& dontVisitChildren)
{
    m_count++;

    dontVisitChildren = false;
    return true;
}


bool ComplexityFinder::visit(const std::shared_ptr<Binary>& /*exp*/, bool& dontVisitChildren)
{
    m_count++;
    dontVisitChildren = false;
    return true;
}


bool ComplexityFinder::visit(const std::shared_ptr<Ternary>& /*exp*/, bool& dontVisitChildren)
{
    m_count++;
    dontVisitChildren = false;
    return true;
}
