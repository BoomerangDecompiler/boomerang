#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpPropagator.h"


#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/statements/Assign.h"


ExpPropagator::ExpPropagator()
    : m_changed(false)
{
}


SharedExp ExpPropagator::postVisit(const std::shared_ptr<RefExp>& exp)
{
    // No need to call e->canRename() here, because if e's base expression is not suitable for renaming, it will never
    // have been renamed, and we never would get here
    if (!Statement::canPropagateToExp(*exp)) { // Check of the definition statement is suitable for propagating
        return exp;
    }

    Statement *def = exp->getDef();
    SharedExp res  = exp;

    if (def && def->isAssign()) {
        SharedExp lhs = ((Assign *)def)->getLeft();
        SharedExp rhs = ((Assign *)def)->getRight();
        bool      ch;
        res = exp->searchReplaceAll(RefExp(lhs, def), rhs->clone(), ch);

        if (ch) {
            m_changed       = true;    // Record this change
            m_unchanged &= ~m_mask; // Been changed now (so simplify parent)

            if (res->isSubscript()) {
                res = postVisit(std::static_pointer_cast<RefExp>(res)); // Recursively propagate more if possible
            }
        }
    }

    return res;
}
