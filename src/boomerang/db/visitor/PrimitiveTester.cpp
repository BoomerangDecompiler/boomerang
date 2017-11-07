#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PrimitiveTester.h"


#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"


bool PrimitiveTester::visit(const std::shared_ptr<Location>& /*exp*/, bool& dontVisitChildren)
{
    // We reached a bare (unsubscripted) location. This is certainly not primitive
    dontVisitChildren = true;
    m_result   = false;
    return false; // No need to continue searching
}


bool PrimitiveTester::visit(const std::shared_ptr<RefExp>& exp, bool& dontVisitChildren)
{
    Statement *def = exp->getDef();

    // If defined by a call, e had better not be a memory location (crude approximation for now)
    if ((def == nullptr) || (def->getNumber() == 0) || (def->isCall() && !exp->getSubExp1()->isMemOf())) {
        // Implicit definitions are always primitive
        // The results of calls are always primitive
        dontVisitChildren = true; // Don't recurse into the reference
        return true;     // Result remains true
    }

    // For now, all references to other definitions will be considered non primitive. I think I'll have to extend this!
    m_result   = false;
    dontVisitChildren = true; // Regardless of outcome, don't recurse into the reference
    return true;
}
