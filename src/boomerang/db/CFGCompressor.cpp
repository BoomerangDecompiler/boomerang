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


#include "boomerang/db/CFG.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/statements/Statement.h"

#include <deque>

bool CFGCompressor::compressCFG(Cfg* cfg)
{
    bool changed = false;

    // FIXME: The below was working while we still had reaching definitions. It seems to me that it would be easy to
    // search the BB for definitions between the two branches (so we don't need reaching defs, just the SSA property of
    //  unique definition).
    //
    // Look in CVS for old code.

    // Find A -> J -> B where J is a BB that is only a jump and replace it by A -> B
    for (BasicBlock *a : *cfg) {
        for (int i = 0; i < a->getNumSuccessors(); i++) {
            BasicBlock *jmpBB = a->getSuccessor(i);

            if (jmpBB->getNumSuccessors() != 1) { // only consider oneway jumps
                continue;
            }

            if (jmpBB->getRTLs()->size() != 1 ||
                jmpBB->getRTLs()->front()->size() != 1 ||
                jmpBB->getRTLs()->front()->front()->isGoto()) {
                continue;
            }

            // Found an out-edge to an only-jump BB.
            // Replace edge A -> J -> B by A -> B
            BasicBlock *b = jmpBB->getSuccessor(0);
            a->setSuccessor(i, b);

            for (int j = 0; j < b->getNumPredecessors(); j++) {
                if (b->getPredecessor(j) == jmpBB) {
                    b->setPredecessor(j, a);
                    break;
                }
            }

            // remove predecessor from j. Cannot remove successor now since there might be several predecessors
            // which need the successor information.
            jmpBB->removePredecessor(a);

            if (jmpBB->getNumPredecessors() == 0) {
                jmpBB->removeAllSuccessors(); // now we can remove the successors
                cfg->removeBB(jmpBB);
            }
        }
    }

    changed |= removeOrphanBBs(cfg);
    return changed;
}

bool CFGCompressor::removeOrphanBBs(Cfg *cfg)
{
    std::deque<BasicBlock *> orphans;

    for (BasicBlock *potentialOrphan : *cfg) {
        if (potentialOrphan == cfg->getEntryBB()) { // don't remove entry BasicBlock
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

            if (child->getNumPredecessors() == 0) {
                orphans.push_back(child);
            }
        }

        cfg->removeBB(b);
    }

    return bbsRemoved;
}
