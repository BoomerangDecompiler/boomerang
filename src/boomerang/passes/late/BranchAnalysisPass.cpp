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
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/ssl/RTL.h"


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

        // recalculate phi assignments of referencing BBs.
        for (BasicBlock *bb : *proc->getCFG()) {
            BasicBlock::RTLIterator rtlIt;
            StatementList::iterator stmtIt;

            for (Statement *stmt = bb->getFirstStmt(rtlIt, stmtIt); stmt;
                 stmt            = bb->getNextStmt(rtlIt, stmtIt)) {
                if (!stmt->isPhi()) {
                    continue;
                }

                PhiAssign *phiStmt       = static_cast<PhiAssign *>(stmt);
                PhiAssign::PhiDefs &defs = phiStmt->getDefs();

                for (PhiAssign::PhiDefs::iterator defIt = defs.begin(); defIt != defs.end();) {
                    if (!proc->getCFG()->hasBB(defIt->first)) {
                        // remove phi reference to deleted bb
                        defIt = defs.erase(defIt);
                    }
                    else {
                        ++defIt;
                    }
                }
            }
        }
    }

    return removedBBs;
}


bool BranchAnalysisPass::doBranchAnalysis(UserProc *proc)
{
    StatementList stmts;
    proc->getStatements(stmts);

    std::set<BasicBlock *> bbsToRemove;

    for (Statement *stmt : stmts) {
        if (!stmt->isBranch()) {
            continue;
        }

        BranchStatement *firstBranch = static_cast<BranchStatement *>(stmt);

        if (!firstBranch->getFallBB() || !firstBranch->getTakenBB()) {
            continue;
        }

        StatementList fallstmts;
        firstBranch->getFallBB()->appendStatementsTo(fallstmts);
        Statement *nextAfterBranch = !fallstmts.empty() ? fallstmts.front() : nullptr;

        if (nextAfterBranch && nextAfterBranch->isBranch()) {
            BranchStatement *secondBranch = static_cast<BranchStatement *>(nextAfterBranch);

            //   branch to A if cond1
            //   branch to B if cond2
            // A: something
            // B:
            // ->
            //   branch to B if !cond1 && cond2
            // A: something
            // B:
            //
            if ((secondBranch->getFallBB() == firstBranch->getTakenBB()) &&
                (secondBranch->getBB()->getNumPredecessors() == 1) &&
                isOnlyBranch(secondBranch->getBB())) {
                SharedExp cond = Binary::get(opAnd, Unary::get(opLNot, firstBranch->getCondExpr()),
                                             secondBranch->getCondExpr());
                firstBranch->setCondExpr(cond->clone()->simplify());

                firstBranch->setDest(secondBranch->getFixedDest());
                firstBranch->setTakenBB(secondBranch->getTakenBB());
                firstBranch->setFallBB(secondBranch->getFallBB());

                // remove second branch BB
                BasicBlock *secondBranchBB = secondBranch->getBB();
                assert(secondBranchBB->getNumPredecessors() == 0);
                assert(secondBranchBB->getNumSuccessors() == 2);
                BasicBlock *succ1 = secondBranch->getBB()->getSuccessor(BTHEN);
                BasicBlock *succ2 = secondBranch->getBB()->getSuccessor(BELSE);

                secondBranchBB->removeSuccessor(succ1);
                secondBranchBB->removeSuccessor(succ2);
                succ1->removePredecessor(secondBranchBB);
                succ2->removePredecessor(secondBranchBB);

                bbsToRemove.insert(secondBranchBB);
            }

            //   branch to B if cond1
            //   branch to B if cond2
            // A: something
            // B:
            // ->
            //   branch to B if cond1 || cond2
            // A: something
            // B:
            if ((secondBranch->getTakenBB() == firstBranch->getTakenBB()) &&
                (secondBranch->getBB()->getNumPredecessors() == 1) &&
                isOnlyBranch(secondBranch->getBB())) {
                const SharedExp cond = Binary::get(opOr, firstBranch->getCondExpr(),
                                                   secondBranch->getCondExpr());

                firstBranch->setCondExpr(cond->clone()->simplify());
                firstBranch->setFallBB(secondBranch->getFallBB());

                BasicBlock *secondBranchBB = secondBranch->getBB();
                assert(secondBranchBB->getNumPredecessors() == 0);
                assert(secondBranchBB->getNumSuccessors() == 2);
                BasicBlock *succ1 = secondBranchBB->getSuccessor(BTHEN);
                BasicBlock *succ2 = secondBranchBB->getSuccessor(BELSE);

                secondBranchBB->removeSuccessor(succ1);
                secondBranchBB->removeSuccessor(succ2);
                succ1->removePredecessor(secondBranchBB);
                succ2->removePredecessor(secondBranchBB);

                bbsToRemove.insert(secondBranchBB);
            }
        }
    }

    const bool removedBBs = !bbsToRemove.empty();
    for (BasicBlock *bb : bbsToRemove) {
        proc->getCFG()->removeBB(bb);
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

        SharedExp hl = static_cast<BranchStatement *>(stmt)->getCondExpr();

        // of the form: x{n} - 1 >= 0
        if (hl && (hl->getOper() == opGtrEq) && hl->getSubExp2()->isIntConst() &&
            (hl->access<Const, 2>()->getInt() == 0) && (hl->getSubExp1()->getOper() == opMinus) &&
            hl->getSubExp1()->getSubExp2()->isIntConst() &&
            (hl->access<Const, 1, 2>()->getInt() == 1) &&
            hl->getSubExp1()->getSubExp1()->isSubscript()) {
            Statement *n = hl->access<RefExp, 1, 1>()->getDef();

            if (n && n->isPhi()) {
                PhiAssign *p = static_cast<PhiAssign *>(n);

                for (const auto &phi : *p) {
                    if (!phi->getDef()->isAssign()) {
                        continue;
                    }

                    Assign *a = static_cast<Assign *>(phi->getDef());

                    if (*a->getRight() == *hl->getSubExp1()) {
                        hl->setSubExp1(RefExp::get(a->getLeft(), a));
                        break;
                    }
                }
            }
        }
    }
}


bool BranchAnalysisPass::isOnlyBranch(BasicBlock* bb) const
{
    const RTLList *rtls = bb->getRTLs();
    if (!rtls || rtls->empty()) {
        return false;
    }

    StatementList::reverse_iterator sIt;
    BasicBlock::RTLRIterator rIt;
    bool last = true;

    for (Statement *s = bb->getLastStmt(rIt, sIt); s != nullptr; s = bb->getPrevStmt(rIt, sIt)) {
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
