#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CFGCompressor.h"

#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/util/log/Log.h"

#include <deque>


bool CFGCompressor::compressCFG(ProcCFG *cfg)
{
    // FIXME: The below was working while we still had reaching definitions. It seems to me that it
    // would be easy to search the BB for definitions between the two branches (so we don't need
    // reaching defs, just the SSA property of
    //  unique definition).
    //
    // Look in CVS for old code.

    bool changed = false;

    changed |= removeEmptyJumps(cfg);
    changed |= removeOrphanBBs(cfg);
    return changed;
}


bool CFGCompressor::removeEmptyJumps(ProcCFG *cfg)
{
    std::deque<BasicBlock *> bbsToRemove;

    for (BasicBlock *bb : *cfg) {
        // Check if the BB can be removed
        if (bb->getNumSuccessors() == 1 && bb != cfg->getEntryBB() &&
            (bb->isEmpty() || bb->isEmptyJump())) {
            bbsToRemove.push_back(bb);
        }
    }

    const bool changed = !bbsToRemove.empty();

    while (!bbsToRemove.empty()) {
        BasicBlock *bb = bbsToRemove.front();
        bbsToRemove.pop_front();

        assert(bb->getNumSuccessors() == 1);
        BasicBlock *succ = bb->getSuccessor(0); // the one and only successor

        if (succ == bb) {
            continue;
        }

        succ->removePredecessor(bb);
        bb->removeSuccessor(succ);

        for (BasicBlock *pred : bb->getPredecessors()) {
            for (int i = 0; i < pred->getNumSuccessors(); i++) {
                if (pred->getSuccessor(i) == bb) {
                    pred->setSuccessor(i, succ);
                    succ->addPredecessor(pred);
                }
            }
        }

        bb->removeAllPredecessors();
        cfg->removeBB(bb);
    }

    return changed;
}


bool CFGCompressor::removeOrphanBBs(ProcCFG *cfg)
{
    std::deque<BasicBlock *> orphans;

    for (BasicBlock *potentialOrphan : *cfg) {
        if (potentialOrphan == cfg->getEntryBB()) {
            // don't remove entry BasicBlock
            continue;
        }
        else if (potentialOrphan->isType(BBType::Ret)) {
            // Don't remove the ReturnStatement for noreturn functions
            continue;
        }

        if (potentialOrphan->getNumPredecessors() == 0) {
            orphans.push_back(potentialOrphan);
        }
    }

    const bool bbsRemoved = !orphans.empty();

    while (!orphans.empty()) {
        BasicBlock *b = orphans.front();
        orphans.pop_front();

        for (BasicBlock *child : b->getSuccessors()) {
            child->removePredecessor(b);
            b->removeSuccessor(child);
        }

        cfg->removeBB(b);
    }

    return bbsRemoved;
}
