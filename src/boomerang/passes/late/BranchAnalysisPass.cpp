#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BranchAnalysisPass.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/PhiAssign.h"


BranchAnalysisPass::BranchAnalysisPass()
    : IPass("BranchAnalysis", PassID::BranchAnalysis)
{
}


bool BranchAnalysisPass::execute(UserProc *proc)
{
    bool removedFragments = doBranchAnalysis(proc);
    fixUglyBranches(proc);

    if (removedFragments) {
        // redo the data flow
        PassManager::get()->executePass(PassID::Dominators, proc);
        PassManager::get()->executePass(PassID::PhiPlacement, proc);
        PassManager::get()->executePass(PassID::BlockVarRename, proc);
    }

    return removedFragments;
}


bool BranchAnalysisPass::doBranchAnalysis(UserProc *proc)
{
    std::set<IRFragment *, Util::ptrCompare<IRFragment>> fragsToRemove;

    for (IRFragment *a : *proc->getCFG()) {
        if (!a->isType(FragType::Twoway)) {
            continue;
        }
        else if (fragsToRemove.find(a) != fragsToRemove.end()) {
            continue;
        }

        if (a->isEmpty() || isOnlyBranch(a)) {
            if (a->getSuccessor(BTHEN) == a) {
                for (IRFragment *pred : a->getPredecessors()) {
                    proc->getCFG()->replaceEdge(pred, a, a->getSuccessor(BELSE));
                }
                fragsToRemove.insert(a);
                continue;
            }
            else if (a->getSuccessor(BELSE) == a) {
                for (IRFragment *pred : a->getPredecessors()) {
                    proc->getCFG()->replaceEdge(pred, a, a->getSuccessor(BTHEN));
                }
                fragsToRemove.insert(a);
                continue;
            }
        }

        IRFragment *b = a->getSuccessor(BELSE);
        if (!b || !b->isType(FragType::Twoway)) {
            continue;
        }
        else if (!isOnlyBranch(b)) {
            continue;
        }

        assert(a->getLastStmt()->isBranch());
        assert(b->getLastStmt()->isBranch());

        std::shared_ptr<BranchStatement> aBranch = a->getLastStmt()->as<BranchStatement>();
        std::shared_ptr<BranchStatement> bBranch = b->getLastStmt()->as<BranchStatement>();

        // A: branch to D if cond1
        // B: branch to D if cond2
        // C: something
        // D:
        // ->
        // A  branch to D if cond1 || cond2
        // C: something
        // D:
        if (b->getSuccessor(BTHEN) == a->getSuccessor(BTHEN) && b->getNumPredecessors() == 1) {
            const SharedExp newCond = Binary::get(opOr, aBranch->getCondExpr(),
                                                  bBranch->getCondExpr());

            aBranch->setCondExpr(newCond->clone()->simplify());
            aBranch->setFallFragment(bBranch->getFallFragment());

            assert(b->getNumPredecessors() == 0);
            assert(b->getNumSuccessors() == 2);

            IRFragment *succ1 = b->getSuccessor(BTHEN);
            IRFragment *succ2 = b->getSuccessor(BELSE);

            b->removeSuccessor(succ1);
            b->removeSuccessor(succ2);

            succ1->removePredecessor(b);
            succ2->removePredecessor(b);

            fragsToRemove.insert(b);
        }

        // A: branch to C if cond1
        // B: branch to D if cond2
        // C: something
        // D:
        // ->
        // A: branch to D if !cond1 && cond2
        // C: something
        // D:
        else if (a->getSuccessor(BTHEN) == b->getSuccessor(BELSE) && b->getNumPredecessors() == 1) {
            const SharedExp newCond = Binary::get(opAnd, Unary::get(opLNot, aBranch->getCondExpr()),
                                                  bBranch->getCondExpr());

            aBranch->setCondExpr(newCond->clone()->simplify());
            aBranch->setDest(bBranch->getFixedDest());
            aBranch->setTakenFragment(bBranch->getTakenFragment());
            aBranch->setFallFragment(bBranch->getFallFragment());

            assert(b->getNumPredecessors() == 0);
            assert(b->getNumSuccessors() == 2);

            IRFragment *succ1 = b->getSuccessor(BTHEN);
            IRFragment *succ2 = b->getSuccessor(BELSE);

            b->removeSuccessor(succ1);
            b->removeSuccessor(succ2);

            succ1->removePredecessor(b);
            succ2->removePredecessor(b);

            fragsToRemove.insert(b);
        }
    }

    const bool removedFragments = !fragsToRemove.empty();
    for (IRFragment *bb : fragsToRemove) {
        proc->getCFG()->removeFragment(bb);
    }

    return removedFragments;
}


void BranchAnalysisPass::fixUglyBranches(UserProc *proc)
{
    StatementList stmts;
    proc->getStatements(stmts);

    for (auto stmt : stmts) {
        if (!stmt->isBranch()) {
            continue;
        }

        SharedExp hl = stmt->as<BranchStatement>()->getCondExpr();

        // of the form: x{n} - 1 >= 0
        if (hl && (hl->getOper() == opGtrEq) && hl->getSubExp2()->isIntConst() &&
            (hl->access<Const, 2>()->getInt() == 0) && (hl->getSubExp1()->getOper() == opMinus) &&
            hl->getSubExp1()->getSubExp2()->isIntConst() &&
            (hl->access<Const, 1, 2>()->getInt() == 1) &&
            hl->getSubExp1()->getSubExp1()->isSubscript()) {
            SharedStmt n = hl->access<RefExp, 1, 1>()->getDef();

            if (n && n->isPhi()) {
                std::shared_ptr<PhiAssign> p = n->as<PhiAssign>();

                for (const auto &phi : *p) {
                    if (!phi->getDef()->isAssign()) {
                        continue;
                    }

                    std::shared_ptr<Assign> a = phi->getDef()->as<Assign>();

                    if (*a->getRight() == *hl->getSubExp1()) {
                        hl->setSubExp1(RefExp::get(a->getLeft(), a));
                        break;
                    }
                }
            }
        }
    }
}


bool BranchAnalysisPass::isOnlyBranch(IRFragment *frag) const
{
    const RTLList *rtls = frag->getRTLs();
    if (!rtls || rtls->empty()) {
        return false;
    }

    StatementList::reverse_iterator sIt;
    IRFragment::RTLRIterator rIt;
    bool last = true;

    for (SharedStmt s = frag->getLastStmt(rIt, sIt); s != nullptr;
         s            = frag->getPrevStmt(rIt, sIt)) {
        if (!last) {
            return false; // there are other statements beside the last branch
        }
        else if (!s->isBranch()) {
            return false; // last stmt is not a branch, can't handle this
        }
        else {
            last = false;
        }
    }

    return true;
}
