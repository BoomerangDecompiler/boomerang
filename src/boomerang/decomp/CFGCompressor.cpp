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
#include "boomerang/db/IRFragment.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/util/log/Log.h"

#include <deque>


bool CFGCompressor::compressCFG(ProcCFG *cfg)
{
    // FIXME: The below was working while we still had reaching definitions. It seems to me that it
    // would be easy to search the BB for definitions between the two branches
    // (so we don't need reaching defs, just the SSA property of unique definition).
    //
    // Look in CVS for old code.

    bool changed = false;

    changed |= removeEmptyJumps(cfg);
    changed |= removeOrphanBBs(cfg);
    return changed;
}


bool CFGCompressor::removeEmptyJumps(ProcCFG *cfg)
{
    std::deque<IRFragment *> fragsToRemove;

    for (IRFragment *frag : *cfg) {
        // Check if the BB can be removed
        if (frag->getNumSuccessors() == 1 && frag != cfg->getEntryFragment() &&
            (frag->isEmpty() || frag->isEmptyJump())) {
            fragsToRemove.push_back(frag);
        }
    }

    bool bbsRemoved = false;

    while (!fragsToRemove.empty()) {
        IRFragment *frag = fragsToRemove.front();
        fragsToRemove.pop_front();

        assert(frag->getNumSuccessors() == 1);
        IRFragment *succ = frag->getSuccessor(0); // the one and only successor

        if (succ == frag) {
            continue;
        }

        succ->removePredecessor(frag);
        frag->removeSuccessor(succ);

        for (IRFragment *pred : frag->getPredecessors()) {
            for (int i = 0; i < pred->getNumSuccessors(); i++) {
                if (pred->getSuccessor(i) == frag) {
                    pred->setSuccessor(i, succ);
                    succ->addPredecessor(pred);
                }
            }
        }

        frag->removeAllPredecessors();
        cfg->removeFragment(frag);
        bbsRemoved = true;
    }

    return bbsRemoved;
}


bool CFGCompressor::removeOrphanBBs(ProcCFG *cfg)
{
    std::deque<IRFragment *> orphans;

    for (IRFragment *potentialOrphan : *cfg) {
        if (potentialOrphan == cfg->getEntryBB()) {
            // don't remove entry fragment
            continue;
        }
        else if (potentialOrphan->isType(FragType::Ret)) {
            // Don't remove the ReturnStatement for noreturn functions
            continue;
        }

        if (potentialOrphan->getNumPredecessors() == 0) {
            orphans.push_back(potentialOrphan);
        }
    }

    const bool bbsRemoved = !orphans.empty();

    while (!orphans.empty()) {
        IRFragment *b = orphans.front();
        orphans.pop_front();

        for (IRFragment *child : b->getSuccessors()) {
            child->removePredecessor(b);
            b->removeSuccessor(child);
        }

        cfg->removeFragment(b);
    }

    return bbsRemoved;
}
