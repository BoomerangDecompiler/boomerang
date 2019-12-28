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
#include <stack>


/**
 * Stack where each element is present at most once
 */
template<typename T>
class UniqueStack
{
public:
    bool empty() const { return m_stack.empty(); }

    T pop()
    {
        T val = m_stack.back();
        m_stack.pop_back();
        return val;
    }

    // Push a new item onto the stack. If the item already exists, nothing happens
    void push(T val)
    {
        if (std::find(m_stack.begin(), m_stack.end(), val) == m_stack.end()) {
            m_stack.push_back(val);
        }
    }

    void erase(T val)
    {
        auto it = std::find(m_stack.begin(), m_stack.end(), val);
        if (it != m_stack.end()) {
            m_stack.erase(it);
        }
    }

private:
    std::deque<T> m_stack;
};


bool CFGCompressor::compressCFG(ProcCFG *cfg)
{
    // FIXME: The below was working while we still had reaching definitions. It seems to me that it
    // would be easy to search the fragments for definitions between the two branches
    // (so we don't need reaching defs, just the SSA property of unique definition).
    //
    // Look in CVS for old code.

    bool changed = false;

    changed |= removeEmptyJumps(cfg);
    changed |= removeOrphanFragments(cfg);
    changed |= compressFallthroughs(cfg);

    return changed;
}


bool CFGCompressor::removeEmptyJumps(ProcCFG *cfg)
{
    std::deque<IRFragment *> fragsToRemove;

    for (IRFragment *frag : *cfg) {
        // Check if the fragment can be removed
        if (frag->getNumSuccessors() == 1 && frag != cfg->getEntryFragment() &&
            (frag->isEmpty() || frag->isEmptyJump())) {
            fragsToRemove.push_back(frag);
        }
    }

    bool fragsRemoved = false;

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
        fragsRemoved = true;
    }

    return fragsRemoved;
}


bool CFGCompressor::removeOrphanFragments(ProcCFG *cfg)
{
    std::deque<IRFragment *> orphans;

    for (IRFragment *potentialOrphan : *cfg) {
        if (potentialOrphan == cfg->getEntryFragment()) {
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

    const bool fragsRemoved = !orphans.empty();

    while (!orphans.empty()) {
        IRFragment *b = orphans.front();
        orphans.pop_front();

        for (IRFragment *child : b->getSuccessors()) {
            child->removePredecessor(b);
            b->removeSuccessor(child);
        }

        cfg->removeFragment(b);
    }

    return fragsRemoved;
}


bool CFGCompressor::compressFallthroughs(ProcCFG *cfg)
{
    std::unordered_set<IRFragment *> visited;
    UniqueStack<IRFragment *> toVisit;

    IRFragment *entry = cfg->getEntryFragment();
    if (!entry) {
        return false;
    }

    bool change = false;
    toVisit.push(entry);

    while (!toVisit.empty()) {
        IRFragment *current = toVisit.pop();

        if (visited.find(current) != visited.end()) {
            continue;
        }

        visited.insert(current);

        if (current->getNumSuccessors() != 1) {
            for (IRFragment *succ : current->getSuccessors()) {
                toVisit.push(succ);
            }
            continue;
        }

        IRFragment *succ = current->getSuccessor(0);
        if (succ->getNumPredecessors() != 1) {
            toVisit.push(succ);
            continue;
        }
        else if (!current->isEmpty() && !current->getLastStmt()->isAssignment()) {
            toVisit.push(succ);
            continue;
        }

        SharedStmt succFirst = succ->getFirstStmt();
        if (succFirst->isPhi() || succFirst->isImplicit()) {
            toVisit.push(succ);
            continue;
        }
        else if (succ->getBB() != current->getBB()) {
            toVisit.push(succ);
            continue;
        }

        std::unique_ptr<RTLList> combined(new RTLList);
        for (auto &rtl : *current->getRTLs()) {
            combined->push_back(std::move(rtl));
        }
        current->getRTLs()->clear();

        for (auto &rtl : *succ->getRTLs()) {
            combined->push_back(std::move(rtl));
        }
        succ->getRTLs()->clear();

        IRFragment *combinedFrag = cfg->createFragment(succ->getType(), std::move(combined),
                                                       succ->getBB());

        for (IRFragment *pred : current->getPredecessors()) {
            for (int i = 0; i < pred->getNumSuccessors(); ++i) {
                if (pred->getSuccessor(i) == current) {
                    pred->setSuccessor(i, combinedFrag);
                    combinedFrag->addPredecessor(pred);
                }
            }
        }

        for (IRFragment *succ2 : succ->getSuccessors()) {
            for (int i = 0; i < succ2->getNumPredecessors(); ++i) {
                if (succ2->getPredecessor(i) == succ) {
                    succ2->setPredecessor(i, combinedFrag);
                    combinedFrag->addSuccessor(succ2);
                }
            }
        }

        IRFragment::RTLIterator rit;
        RTL::iterator sit;

        for (SharedStmt s = combinedFrag->getFirstStmt(rit, sit); s != nullptr;
             s            = combinedFrag->getNextStmt(rit, sit)) {
            s->setFragment(combinedFrag);
        }

        if (current == cfg->getEntryFragment()) {
            cfg->setEntryAndExitFragment(combinedFrag);
        }

        visited.erase(current);
        visited.erase(succ);
        toVisit.erase(current);
        toVisit.erase(succ);
        toVisit.push(combinedFrag);

        cfg->removeFragment(current);
        cfg->removeFragment(succ);

        change = true;
    }

    return change;
}
