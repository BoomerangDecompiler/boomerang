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
#include "boomerang/db/CFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/RTL.h"
#include "boomerang/util/Address.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/util/Log.h"


StringInstructionProcessor::StringInstructionProcessor(UserProc* proc)
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

        for (RTL *rtl : *bbRTLs) {
            prev = addr;
            addr = rtl->getAddress();

            if (!rtl->empty()) {
                Statement *firstStmt = rtl->front();
                if (firstStmt->isAssign()) {
                    SharedExp lhs = ((Assign *)firstStmt)->getLeft();

                    if (lhs->isMachFtr()) {
                        QString str = lhs->access<Const, 1>()->getStr();

                        if (str.startsWith("%SKIP")) {
                            stringInstructions.push_back({ rtl, bb });

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
        RTL *skipRTL = p.first;
        BasicBlock *bb = p.second;

        BranchStatement *br1 = new BranchStatement;

        assert(skipRTL->size() >= 4); // They vary; at least 5 or 6

        Statement *s1 = *skipRTL->begin();
        Statement *s6 = *(--skipRTL->end());
        if (s1->isAssign()) {
            br1->setCondExpr(((Assign *)s1)->getRight());
        }
        else {
            br1->setCondExpr(nullptr);
        }
        br1->setDest(skipRTL->getAddress() + 2);

        BranchStatement *br2 = new BranchStatement;
        if (s6->isAssign()) {
            br2->setCondExpr(((Assign *)s6)->getRight());
        }
        else {
            br2->setCondExpr(nullptr);
        }
        br2->setDest(skipRTL->getAddress());

        splitForBranch(bb, skipRTL, br1, br2);
    }

    return !stringInstructions.empty();
}



BasicBlock *StringInstructionProcessor::splitForBranch(BasicBlock *bb, RTL *skipRTL, BranchStatement *br1, BranchStatement *br2)
{
    Address skipAddr = skipRTL->getAddress();
    RTLList::iterator ri = std::find(bb->getRTLs()->begin(), bb->getRTLs()->end(), skipRTL);
    assert(ri != bb->getRTLs()->end());

    const bool haveA = (ri != bb->getRTLs()->begin());

    // Make a BB for the br1 instruction

    // Don't give this "instruction" the same address as the rest of the string instruction (causes problems when
    // creating the rptBB). Or if there is no A, temporarily use 0
    Address    a        = (haveA) ? skipAddr : Address::ZERO;
    RTL        *skipRtl = new RTL(a, new std::list<Statement *> { br1 }); // list initializer in braces
    std::unique_ptr<RTLList> bbRTL(new RTLList({ skipRtl }));
    BasicBlock *skipBB  = m_proc->getCFG()->createBB(BBType::Twoway, std::move(bbRTL));
    skipRTL->setAddress(skipAddr + 1);

    if (!haveA) {
        skipRtl->setAddress(skipAddr);
        // Address addr now refers to the splitBB
        m_proc->getCFG()->setBBStart(skipBB, skipAddr);

        // Fix all predecessors of pBB to point to splitBB instead
        for (int i = 0; i < bb->getNumPredecessors(); i++) {
            BasicBlock *pred = bb->getPredecessor(i);

            for (int j = 0; j < pred->getNumSuccessors(); j++) {
                BasicBlock *succ = pred->getSuccessor(j);

                if (succ == bb) {
                    pred->setSuccessor(j, skipBB);
                    skipBB->addPredecessor(pred);
                    break;
                }
            }
        }
    }

    // Remove the SKIP from the start of the string instruction RTL
    assert(skipRTL->size() >= 4);
    skipRTL->pop_front();
    // Replace the last statement with br2
    skipRTL->back() = br2;

    // Move the remainder of the string RTL into a new BB
    bbRTL.reset(new RTLList({ *ri }));
    BasicBlock *rptBB = m_proc->getCFG()->createBB(BBType::Twoway, std::move(bbRTL));
    ri = bb->getRTLs()->erase(ri);

    // Move the remaining RTLs (if any) to a new list of RTLs
    BasicBlock *newBB;
    int    oldOutEdges = 0;
    bool       haveB       = true;

    if (ri != bb->getRTLs()->end()) {
        std::unique_ptr<RTLList> pRtls(new RTLList);

        while (ri != bb->getRTLs()->end()) {
            pRtls->push_back(*ri);
            ri = bb->getRTLs()->erase(ri);
        }

        oldOutEdges = bb->getNumSuccessors();
        newBB       = m_proc->getCFG()->createBB(bb->getType(), std::move(pRtls));

        // Transfer the out edges from A to B (pBB to newBB)
        for (int i = 0; i < oldOutEdges; i++) {
            // Don't use addOutEdge, since it will also add in-edges back to the BB
            newBB->addSuccessor(bb->getSuccessor(i));
        }

        // addOutEdge(newBB, pBB->getSuccessor(i));
    }
    else {
        // The "B" part of the above diagram is empty.
        // Don't create a new BB; just point newBB to the successor of this BB
        haveB = false;
        newBB = bb->getSuccessor(0);
    }

    // Change pBB to a FALL bb
    bb->setType(BBType::Fall);

    // Set the first out-edge to be skipBB
    bb->removeAllSuccessors();
    m_proc->getCFG()->addEdge(bb, skipBB);

    // Set the out edges for skipBB. First is the taken (true) leg.
    m_proc->getCFG()->addEdge(skipBB, newBB);
    m_proc->getCFG()->addEdge(skipBB, rptBB);

    // Set the out edges for the rptBB
    m_proc->getCFG()->addEdge(rptBB, skipBB);
    m_proc->getCFG()->addEdge(rptBB, newBB);

    // For each out edge of newBB, change any in-edges from pBB to instead come from newBB
    if (haveB) {
        for (int i = 0; i < oldOutEdges; i++) {
            BasicBlock *succ = newBB->getSuccessor(i);

            for (int j = 0; j < succ->getNumPredecessors(); j++) {
                if (succ->getPredecessor(j) == bb) {
                    succ->setPredecessor(j, newBB);
                    break;
                }
            }
        }
    }
    else {
        // There is no "B" bb (newBB is just the successor of pBB) Fix that one out-edge to point to rptBB
        for (int i = 0; i < newBB->getNumPredecessors(); i++) {
            if (newBB->getPredecessor(i) == bb) {
                newBB->setPredecessor(i, rptBB);
                break;
            }
        }
    }

    if (!haveA) {
        // There is no A any more. All A's in-edges have been copied to the skipBB. It is possible that the original BB
        // had a self edge (branch to start of self). If so, this edge, now in to skipBB, must now come from newBB (if
        // there is a B) or rptBB if none.  Both of these will already exist, so delete it.
        for (int i = 0; i < skipBB->getNumPredecessors(); i++) {
            if (skipBB->getPredecessor(i) == bb) {
                skipBB->removePredecessor(bb);
                break;
            }
        }

#if DEBUG_SPLIT_FOR_BRANCH
        LOG_VERBOSE("About to delete pBB: %1", bb->prints());
        dumpBB(bb);
        dumpBB(skipBB);
        dumpBB(rptBB);
        dumpBB(newBB);
#endif

        // Must delete bb.
        m_proc->getCFG()->removeBB(bb);
        assert(false);
    }

    return newBB;
}
