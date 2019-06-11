#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StrengthReductionReversalPass.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/util/log/Log.h"


StrengthReductionReversalPass::StrengthReductionReversalPass()
    : IPass("StrengthReductionReversal", PassID::StrengthReductionReversal)
{
}


bool StrengthReductionReversalPass::execute(UserProc *proc)
{
    StatementList stmts;
    proc->getStatements(stmts);

    for (Statement *s : stmts) {
        if (!s->isAssign()) {
            continue;
        }

        Assign *as = static_cast<Assign *>(s);

        // of the form x = x{p} + c
        if ((as->getRight()->getOper() == opPlus) && as->getRight()->getSubExp1()->isSubscript() &&
            (*as->getLeft() == *as->getRight()->getSubExp1()->getSubExp1()) &&
            as->getRight()->getSubExp2()->isIntConst()) {
            int c  = as->getRight()->access<Const, 2>()->getInt();
            auto r = as->getRight()->access<RefExp, 1>();

            if (r->getDef() && r->getDef()->isPhi()) {
                PhiAssign *p = static_cast<PhiAssign *>(r->getDef());

                if (p->getNumDefs() == 2) {
                    Statement *first  = (*p->begin())->getDef();
                    Statement *second = (*p->rbegin())->getDef();

                    if (first == as) {
                        // want the increment in second
                        std::swap(first, second);
                    }

                    // first must be of form x := 0
                    if (first && first->isAssign() &&
                        static_cast<Assign *>(first)->getRight()->isIntConst() &&
                        static_cast<Assign *>(first)->getRight()->access<Const>()->getInt() == 0) {
                        // ok, fun, now we need to find every reference to p and
                        // replace with x{p} * c
                        StatementList stmts2;
                        proc->getStatements(stmts2);

                        for (Statement *stmt2 : stmts2) {
                            if (stmt2 != as) {
                                stmt2->searchAndReplace(
                                    *r, Binary::get(opMult, r->clone(), Const::get(c)));
                            }
                        }

                        // that done we can replace c with 1 in as
                        as->getRight()->access<Const, 2>()->setInt(1);
                    }
                }
            }
        }
    }

    return true;
}
