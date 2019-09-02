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

    for (SharedStmt s : stmts) {
        if (!s->isAssign()) {
            continue;
        }

        std::shared_ptr<Assign> as = s->as<Assign>();

        // of the form x = x{p} + c
        if ((as->getRight()->getOper() == opPlus) && as->getRight()->getSubExp1()->isSubscript() &&
            (*as->getLeft() == *as->getRight()->getSubExp1()->getSubExp1()) &&
            as->getRight()->getSubExp2()->isIntConst()) {
            int c  = as->getRight()->access<Const, 2>()->getInt();
            auto r = as->getRight()->access<RefExp, 1>();

            if (r->getDef() && r->getDef()->isPhi()) {
                std::shared_ptr<PhiAssign> p = r->getDef()->as<PhiAssign>();

                if (p->getNumDefs() == 2) {
                    SharedStmt first  = (*p->begin())->getDef();
                    SharedStmt second = (*p->rbegin())->getDef();

                    if (first == as) {
                        // want the increment in second
                        std::swap(first, second);
                    }

                    // first must be of form x := 0
                    if (first && first->isAssign() &&
                        first->as<Assign>()->getRight()->isIntConst() &&
                        first->as<Assign>()->getRight()->access<Const>()->getInt() == 0) {
                        // ok, fun, now we need to find every reference to p and
                        // replace with x{p} * c
                        StatementList stmts2;
                        proc->getStatements(stmts2);

                        for (SharedStmt stmt2 : stmts2) {
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
