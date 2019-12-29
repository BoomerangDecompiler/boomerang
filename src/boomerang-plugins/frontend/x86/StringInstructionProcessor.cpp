#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StringInstructionProcessor.h"

#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/util/Address.h"
#include "boomerang/util/log/Log.h"


StringInstructionProcessor::StringInstructionProcessor(UserProc *proc)
    : m_proc(proc)
{
}


bool StringInstructionProcessor::processStringInstructions()
{
    std::list<std::pair<RTL *, IRFragment *>> stringInstructions;

    for (IRFragment *frag : *m_proc->getCFG()) {
        RTLList *fragRTLs = frag->getRTLs();

        if (fragRTLs == nullptr) {
            continue;
        }

        Address prev, addr = Address::ZERO;

        for (auto &rtl : *fragRTLs) {
            prev = addr;
            addr = rtl->getAddress();

            if (!rtl->empty()) {
                SharedStmt firstStmt = rtl->front();
                if (firstStmt->isAssign()) {
                    SharedExp lhs = firstStmt->as<Assign>()->getLeft();

                    if (lhs->isMachFtr()) {
                        QString str = lhs->access<Const, 1>()->getStr();

                        if (str.startsWith("%SKIP")) {
                            stringInstructions.push_back({ rtl.get(), frag });

                            // Assume there is only 1 string instruction per fragment
                            // This might not be true, but can be worked around
                            // by calling processStringInstructions multiple times
                            // to catch all string instructions.
                            break;
                        }
                    }
                }
            }
        }
    }

    for (auto p : stringInstructions) {
        RTL *skipRTL     = p.first;
        IRFragment *frag = p.second;

        std::shared_ptr<BranchStatement> skipBranch(new BranchStatement);

        assert(skipRTL->size() >= 4); // They vary; at least 5 or 6

        SharedStmt s1 = *skipRTL->begin();
        SharedStmt s6 = *(--skipRTL->end());
        if (s1->isAssign()) {
            skipBranch->setCondExpr(s1->as<Assign>()->getRight());
        }
        else {
            skipBranch->setCondExpr(nullptr);
        }
        skipBranch->setDest(skipRTL->getAddress() + 2);

        std::shared_ptr<BranchStatement> rptBranch(new BranchStatement);
        if (s6->isAssign()) {
            rptBranch->setCondExpr(s6->as<Assign>()->getRight());
        }
        else {
            rptBranch->setCondExpr(nullptr);
        }
        rptBranch->setDest(skipRTL->getAddress());

        splitForBranch(frag, skipRTL, skipBranch, rptBranch);
    }

    return !stringInstructions.empty();
}


IRFragment *StringInstructionProcessor::splitForBranch(IRFragment *frag, RTL *stringRTL,
                                                       std::shared_ptr<BranchStatement> skipBranch,
                                                       std::shared_ptr<BranchStatement> rptBranch)
{
    Address stringAddr         = stringRTL->getAddress();
    RTLList::iterator stringIt = std::find_if(
        frag->getRTLs()->begin(), frag->getRTLs()->end(),
        [stringRTL](const std::unique_ptr<RTL> &ptr) { return stringRTL == ptr.get(); });

    assert(stringIt != frag->getRTLs()->end());

    const bool haveA  = (stringIt != frag->getRTLs()->begin());
    const bool haveB  = (std::next(stringIt) != frag->getRTLs()->end());
    IRFragment *aFrag = nullptr;
    IRFragment *bFrag = nullptr;

    const std::vector<IRFragment *> oldPredecessors = frag->getPredecessors();
    const std::vector<IRFragment *> oldSuccessors   = frag->getSuccessors();

    if (haveA) {
        aFrag = frag;
        frag  = m_proc->getCFG()->splitFragment(aFrag, stringAddr);
        assert(aFrag->getLowAddr() < frag->getLowAddr());
    }
    stringIt = frag->getRTLs()->begin();
    if (haveB) {
        Address splitAddr = (*std::next(stringIt))->getAddress();
        bFrag             = m_proc->getCFG()->splitFragment(frag, splitAddr);
        assert(frag->getLowAddr() < bFrag->getLowAddr());
    }
    else {
        // this means the original fragment has a fallthrough branch to its successor.
        // Just pretend the successor is the split off B fragment.
        bFrag = frag->getSuccessor(0);
    }

    assert(frag->getRTLs()->size() == 1); // only the string instruction
    assert(frag->getRTLs()->front()->getAddress() == stringAddr);

    // Make an RTL for the skip and the rpt branch instructions.
    std::unique_ptr<RTLList> skipFragRTLs(new RTLList);
    std::unique_ptr<RTLList> rptFragRTLs(new RTLList);
    skipFragRTLs->push_back(std::unique_ptr<RTL>(new RTL(stringAddr, { skipBranch })));
    rptFragRTLs->push_back(std::unique_ptr<RTL>(new RTL(**stringIt)));

    rptFragRTLs->front()->setAddress(stringAddr + 1);
    rptFragRTLs->front()->pop_front();
    rptFragRTLs->front()->back() = rptBranch;

    // remove the original string instruction from the CFG.
    BasicBlock *origBB = frag->getBB();
    frag->removeAllPredecessors();

    // remove connection between the string instruction and the B part
    for (IRFragment *succ : oldSuccessors) {
        frag->removeSuccessor(succ);
        succ->removePredecessor(frag);
    }

    const bool entryFragNeedsUpdate = !haveA && frag == m_proc->getCFG()->getEntryFragment();
    m_proc->getCFG()->removeFragment(frag);

    IRFragment *skipFrag = m_proc->getCFG()->createFragment(FragType::Twoway,
                                                            std::move(skipFragRTLs), origBB);
    IRFragment *rptFrag = m_proc->getCFG()->createFragment(FragType::Twoway, std::move(rptFragRTLs),
                                                           origBB);

    assert(skipFrag && rptFrag);

    if (haveA) {
        aFrag->removeAllSuccessors();
        aFrag->setType(FragType::Fall);
        m_proc->getCFG()->addEdge(aFrag, skipFrag);
    }
    else {
        for (IRFragment *pred : oldPredecessors) {
            for (int i = 0; i < pred->getNumSuccessors(); i++) {
                if (pred->getSuccessor(i) == frag) {
                    pred->setSuccessor(i, skipFrag);
                    skipFrag->addPredecessor(pred);
                }
            }
        }
    }

    bFrag->removePredecessor(frag);
    m_proc->getCFG()->addEdge(skipFrag, bFrag);   // BTHEN
    m_proc->getCFG()->addEdge(skipFrag, rptFrag); // BELSE
    m_proc->getCFG()->addEdge(rptFrag, rptFrag);  // BTHEN
    m_proc->getCFG()->addEdge(rptFrag, bFrag);    // BELSE

    if (entryFragNeedsUpdate) {
        m_proc->getCFG()->setEntryAndExitFragment(skipFrag);
    }

    return haveB ? bFrag : rptFrag;
}
