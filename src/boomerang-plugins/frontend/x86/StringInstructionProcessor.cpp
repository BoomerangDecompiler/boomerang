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
    std::list<std::pair<RTL *, BasicBlock *>> stringInstructions;

    for (BasicBlock *bb : *m_proc->getCFG()) {
        RTLList *bbRTLs = bb->getRTLs();

        if (bbRTLs == nullptr) {
            continue;
        }

        Address prev, addr = Address::ZERO;

        for (auto &rtl : *bbRTLs) {
            prev = addr;
            addr = rtl->getAddress();

            if (!rtl->empty()) {
                SharedStmt firstStmt = rtl->front();
                if (firstStmt->isAssign()) {
                    SharedExp lhs = firstStmt->as<Assign>()->getLeft();

                    if (lhs->isMachFtr()) {
                        QString str = lhs->access<Const, 1>()->getStr();

                        if (str.startsWith("%SKIP")) {
                            stringInstructions.push_back({ rtl.get(), bb });

                            // Assume there is only 1 string instruction per BB
                            // This might not be true, but can be migitated
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
        RTL *skipRTL   = p.first;
        BasicBlock *bb = p.second;

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

        splitForBranch(bb, skipRTL, skipBranch, rptBranch);
    }

    return !stringInstructions.empty();
}


BasicBlock *StringInstructionProcessor::splitForBranch(BasicBlock *bb, RTL *stringRTL,
                                                       std::shared_ptr<BranchStatement> skipBranch,
                                                       std::shared_ptr<BranchStatement> rptBranch)
{
    Address stringAddr         = stringRTL->getAddress();
    RTLList::iterator stringIt = std::find_if(
        bb->getRTLs()->begin(), bb->getRTLs()->end(),
        [stringRTL](const std::unique_ptr<RTL> &ptr) { return stringRTL == ptr.get(); });

    assert(stringIt != bb->getRTLs()->end());

    const bool haveA = (stringIt != bb->getRTLs()->begin());
    const bool haveB = (std::next(stringIt) != bb->getRTLs()->end());
    BasicBlock *aBB  = nullptr;
    BasicBlock *bBB  = nullptr;

    const std::vector<BasicBlock *> oldPredecessors = bb->getPredecessors();
    const std::vector<BasicBlock *> oldSuccessors   = bb->getSuccessors();

    if (haveA) {
        aBB = bb;
        bb  = m_proc->getCFG()->splitBB(aBB, stringAddr);
        assert(aBB->getLowAddr() < bb->getLowAddr());
    }
    stringIt = bb->getRTLs()->begin();
    if (haveB) {
        Address splitAddr = (*std::next(stringIt))->getAddress();
        bBB               = m_proc->getCFG()->splitBB(bb, splitAddr);
        assert(bb->getLowAddr() < bBB->getLowAddr());
    }
    else {
        // this means the original BB has a fallthrough branch to its successor.
        // Just pretend the successor is the split off B bb.
        bBB = bb->getSuccessor(0);
    }

    assert(bb->getRTLs()->size() == 1); // only the string instruction
    assert(bb->getRTLs()->front()->getAddress() == stringAddr);

    // Make an RTL for the skip and the rpt branch instructions.
    std::unique_ptr<RTLList> skipBBRTLs(new RTLList);
    std::unique_ptr<RTLList> rptBBRTLs(new RTLList);
    skipBBRTLs->push_back(std::unique_ptr<RTL>(new RTL(stringAddr, { skipBranch })));
    rptBBRTLs->push_back(std::unique_ptr<RTL>(new RTL(**stringIt)));

    rptBBRTLs->front()->setAddress(stringAddr + 1);
    rptBBRTLs->front()->pop_front();
    rptBBRTLs->front()->back() = rptBranch;

    // remove the original string instruction from the CFG.
    bb->removeAllPredecessors();

    // remove connection between the string instruction and the B part
    for (BasicBlock *succ : oldSuccessors) {
        bb->removeSuccessor(succ);
        succ->removePredecessor(bb);
    }

    const bool entryBBNeedsUpdate = !haveA && bb == m_proc->getCFG()->getEntryBB();
    m_proc->getCFG()->removeBB(bb);

    BasicBlock *skipBB = m_proc->getCFG()->createBB(BBType::Twoway, std::move(skipBBRTLs));
    BasicBlock *rptBB  = m_proc->getCFG()->createBB(BBType::Twoway, std::move(rptBBRTLs));

    assert(skipBB && rptBB);

    if (haveA) {
        aBB->removeAllSuccessors();
        aBB->setType(BBType::Fall);
        m_proc->getCFG()->addEdge(aBB, skipBB);
    }
    else {
        for (BasicBlock *pred : oldPredecessors) {
            for (int i = 0; i < pred->getNumSuccessors(); i++) {
                if (pred->getSuccessor(i) == bb) {
                    pred->setSuccessor(i, skipBB);
                    skipBB->addPredecessor(pred);
                }
            }
        }
    }

    bBB->removePredecessor(bb);
    m_proc->getCFG()->addEdge(skipBB, bBB);
    m_proc->getCFG()->addEdge(skipBB, rptBB);
    m_proc->getCFG()->addEdge(rptBB, bBB);
    m_proc->getCFG()->addEdge(rptBB, rptBB);

    if (entryBBNeedsUpdate) {
        m_proc->getCFG()->setEntryAndExitBB(skipBB);
    }

    return haveB ? bBB : rptBB;
}
