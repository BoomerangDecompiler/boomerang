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
    bool removedBBs = doBranchAnalysis(proc);
    fixUglyBranches(proc);

    if (removedBBs) {
        // redo the data flow
        PassManager::get()->executePass(PassID::Dominators, proc);
        PassManager::get()->executePass(PassID::PhiPlacement, proc);
        PassManager::get()->executePass(PassID::BlockVarRename, proc);
    }

    return removedBBs;
}


bool BranchAnalysisPass::doBranchAnalysis(UserProc *proc)
{
    std::set<IRFragment *, Util::ptrCompare<IRFragment>> bbsToRemove;

    for (IRFragment *a : *proc->getCFG()) {
        if (!a->isType(FragType::Twoway)) {
            continue;
        }
        else if (bbsToRemove.find(a) != bbsToRemove.end()) {
            continue;
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
            aBranch->setFallBB(bBranch->getFallBB());

            assert(b->getNumPredecessors() == 0);
            assert(b->getNumSuccessors() == 2);

            IRFragment *succ1 = b->getSuccessor(BTHEN);
            IRFragment *succ2 = b->getSuccessor(BELSE);

            b->removeSuccessor(succ1);
            b->removeSuccessor(succ2);

            succ1->removePredecessor(b);
            succ2->removePredecessor(b);

            bbsToRemove.insert(b);
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
            aBranch->setTakenBB(bBranch->getTakenBB());
            aBranch->setFallBB(bBranch->getFallBB());

            assert(b->getNumPredecessors() == 0);
            assert(b->getNumSuccessors() == 2);

            IRFragment *succ1 = b->getSuccessor(BTHEN);
            IRFragment *succ2 = b->getSuccessor(BELSE);

            b->removeSuccessor(succ1);
            b->removeSuccessor(succ2);

            succ1->removePredecessor(b);
            succ2->removePredecessor(b);

            bbsToRemove.insert(b);
        }
    }

    const bool removedBBs = !bbsToRemove.empty();
    for (IRFragment *bb : bbsToRemove) {
        proc->getCFG()->removeFragment(bb);
    }

    return removedBBs;
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


bool BranchAnalysisPass::isOnlyBranch(IRFragment *bb) const
{
    const RTLList *rtls = bb->getRTLs();
    if (!rtls || rtls->empty()) {
        return false;
    }

    StatementList::reverse_iterator sIt;
    IRFragment::RTLRIterator rIt;
    bool last = true;

    for (SharedStmt s = bb->getLastStmt(rIt, sIt); s != nullptr; s = bb->getPrevStmt(rIt, sIt)) {
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
