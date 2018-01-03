#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ControlFlowAnalyzer.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/CFG.h"

ControlFlowAnalyzer::ControlFlowAnalyzer(Cfg* cfg)
    : m_cfg(cfg)
{
}


void ControlFlowAnalyzer::structureCFG()
{
    if (m_structured) {
        m_cfg->unTraverse();
        return;
    }

    if (m_cfg->findRetNode() == nullptr) {
        return;
    }

    setTimeStamps();
    updateImmedPDom();

    if (!SETTING(noDecompile)) {
        structConds();
        structLoops();
        checkConds();
    }

    m_structured = true;
}


void ControlFlowAnalyzer::setTimeStamps()
{
    // set the parenthesis for the nodes as well as setting the post-order ordering between the nodes
    int time = 1;
    m_ordering.clear();
    m_cfg->getEntryBB()->setLoopStamps(time, m_ordering);

    // set the reverse parenthesis for the nodes
    time = 1;
    m_cfg->getEntryBB()->setRevLoopStamps(time);

    BasicBlock *retNode = m_cfg->findRetNode();
    assert(retNode);
    m_revOrdering.clear();
    retNode->setRevOrder(m_revOrdering);
}


void ControlFlowAnalyzer::updateImmedPDom()
{
    // traverse the nodes in order (i.e from the bottom up)
    for (int i = m_revOrdering.size() - 1; i >= 0; i--) {
        BasicBlock *curNode = m_revOrdering[i];

        for (BasicBlock *succNode : curNode->getSuccessors()) {
            if (succNode->getRevOrd() > curNode->getRevOrd()) {
                curNode->setImmPDom(commonPDom(curNode->getImmPDom(), succNode));
            }
        }
    }

    // make a second pass but consider the original CFG ordering this time
    for (BasicBlock *curNode : m_ordering) {
        if (curNode->getNumSuccessors() <= 1) {
            continue;
        }

        for (auto& oEdge : curNode->getSuccessors()) {
            BasicBlock *succNode = oEdge;
            curNode->setImmPDom(commonPDom(curNode->getImmPDom(), succNode));
        }
    }

    // one final pass to fix up nodes involved in a loop
    for (BasicBlock *curNode : m_ordering) {
        if (curNode->getNumSuccessors() > 1) {
            for (auto& oEdge : curNode->getSuccessors()) {
                BasicBlock *succNode = oEdge;

                if (curNode->hasBackEdgeTo(succNode) && (curNode->getNumSuccessors() > 1) && succNode->getImmPDom() &&
                    (succNode->getImmPDom()->getOrdering() < curNode->getImmPDom()->getOrdering())) {
                    curNode->setImmPDom(commonPDom(succNode->getImmPDom(), curNode->getImmPDom()));
                }
                else {
                    curNode->setImmPDom(commonPDom(curNode->getImmPDom(), succNode));
                }
            }
        }
    }
}


BasicBlock *ControlFlowAnalyzer::commonPDom(BasicBlock *curImmPDom, BasicBlock *succImmPDom)
{
    if (!curImmPDom) {
        return succImmPDom;
    }

    if (!succImmPDom) {
        return curImmPDom;
    }

    if (curImmPDom->getRevOrd() == succImmPDom->getRevOrd()) {
        return curImmPDom; // ordering hasn't been done
    }

    BasicBlock *oldCurImmPDom  = curImmPDom;
    BasicBlock *oldSuccImmPDom = succImmPDom;

    int giveup = 0;
#define GIVEUP    10000

    while (giveup < GIVEUP && curImmPDom && succImmPDom && (curImmPDom != succImmPDom)) {
        if (curImmPDom->getRevOrd() > succImmPDom->getRevOrd()) {
            succImmPDom = succImmPDom->getImmPDom();
        }
        else {
            curImmPDom = curImmPDom->getImmPDom();
        }

        giveup++;
    }

    if (giveup >= GIVEUP) {
        LOG_VERBOSE("Failed to find commonPDom for %1 and %2",
                    oldCurImmPDom->getLowAddr(), oldSuccImmPDom->getLowAddr());

        return oldCurImmPDom; // no change
    }

    return curImmPDom;
}


void ControlFlowAnalyzer::structConds()
{
    // Process the nodes in order
    for (BasicBlock *curNode : m_ordering) {
        // does the current node have more than one out edge?
        if (curNode->getNumSuccessors()) {
            // if the current conditional header is a two way node and has a back edge, then it won't have a follow
            if (curNode->hasBackEdge() && (curNode->getType() == BBType::Twoway)) {
                curNode->setStructType(StructType::Cond);
                continue;
            }

            // set the follow of a node to be its immediate post dominator
            curNode->setCondFollow(curNode->getImmPDom());

            // set the structured type of this node
            curNode->setStructType(StructType::Cond);

            // if this is an nway header, then we have to tag each of the nodes within the body of the nway subgraph
            if (curNode->getCondType() == CondType::Case) {
                curNode->setCaseHead(curNode, curNode->getCondFollow());
            }
        }
    }
}


void ControlFlowAnalyzer::determineLoopType(BasicBlock *header, bool *& loopNodes)
{
    assert(header->getLatchNode());

    // if the latch node is a two way node then this must be a post tested loop
    if (header->getLatchNode()->getType() == BBType::Twoway) {
        header->setLoopType(LoopType::PostTested);

        // if the head of the loop is a two way node and the loop spans more than one block  then it must also be a
        // conditional header
        if ((header->getType() == BBType::Twoway) && (header != header->getLatchNode())) {
            header->setStructType(StructType::LoopCond);
        }
    }

    // otherwise it is either a pretested or endless loop
    else if (header->getType() == BBType::Twoway) {
        // if the header is a two way node then it must have a conditional follow (since it can't have any backedges
        // leading from it). If this follow is within the loop then this must be an endless loop
        if (header->getCondFollow() && loopNodes[header->getCondFollow()->getOrdering()]) {
            header->setLoopType(LoopType::Endless);

            // retain the fact that this is also a conditional header
            header->setStructType(StructType::LoopCond);
        }
        else {
            header->setLoopType(LoopType::PreTested);
        }
    }
    // both the header and latch node are one way nodes so this must be an endless loop
    else {
        header->setLoopType(LoopType::Endless);
    }
}


void ControlFlowAnalyzer::findLoopFollow(BasicBlock *header, bool *& loopNodes)
{
    assert(header->getStructType() == StructType::Loop || header->getStructType() == StructType::LoopCond);
    LoopType   lType  = header->getLoopType();
    BasicBlock *latch = header->getLatchNode();

    if (lType == LoopType::PreTested) {
        // if the 'while' loop's true child is within the loop, then its false child is the loop follow
        if (loopNodes[header->getSuccessor(0)->getOrdering()]) {
            header->setLoopFollow(header->getSuccessor(1));
        }
        else {
            header->setLoopFollow(header->getSuccessor(0));
        }
    }
    else if (lType == LoopType::PostTested) {
        // the follow of a post tested ('repeat') loop is the node on the end of the non-back edge from the latch node
        if (latch->getSuccessor(0) == header) {
            header->setLoopFollow(latch->getSuccessor(1));
        }
        else {
            header->setLoopFollow(latch->getSuccessor(0));
        }
    }
    else {
        // endless loop
        BasicBlock *follow = nullptr;

        // traverse the ordering array between the header and latch nodes.
        // BasicBlock * latch = header->getLatchNode(); initialized at function start
        for (int i = header->getOrdering() - 1; i > latch->getOrdering(); i--) {
            BasicBlock *& desc = m_ordering[i];
            // the follow for an endless loop will have the following
            // properties:
            //   i) it will have a parent that is a conditional header inside the loop whose follow is outside the
            //        loop
            //  ii) it will be outside the loop according to its loop stamp pair
            // iii) have the highest ordering of all suitable follows (i.e. highest in the graph)

            if ((desc->getStructType() == StructType::Cond) && desc->getCondFollow() && (desc->getLoopHead() == header)) {
                if (loopNodes[desc->getCondFollow()->getOrdering()]) {
                    // if the conditional's follow is in the same loop AND is lower in the loop, jump to this follow
                    if (desc->getOrdering() > desc->getCondFollow()->getOrdering()) {
                        i = desc->getCondFollow()->getOrdering();
                    }
                    // otherwise there is a backward jump somewhere to a node earlier in this loop. We don't need to any
                    //  nodes below this one as they will all have a conditional within the loop.
                    else {
                        break;
                    }
                }
                else {
                    // otherwise find the child (if any) of the conditional header that isn't inside the same loop
                    BasicBlock *succ = desc->getSuccessor(0);

                    if (loopNodes[succ->getOrdering()]) {
                        if (!loopNodes[desc->getSuccessor(1)->getOrdering()]) {
                            succ = desc->getSuccessor(1);
                        }
                        else {
                            succ = nullptr;
                        }
                    }

                    // if a potential follow was found, compare its ordering with the currently found follow
                    if (succ && (!follow || (succ->getOrdering() > follow->getOrdering()))) {
                        follow = succ;
                    }
                }
            }
        }

        // if a follow was found, assign it to be the follow of the loop under
        // investigation
        if (follow) {
            header->setLoopFollow(follow);
        }
    }
}


void ControlFlowAnalyzer::tagNodesInLoop(BasicBlock *header, bool *& loopNodes)
{
    assert(header->getLatchNode());

    // traverse the ordering structure from the header to the latch node tagging the nodes determined to be within the
    // loop. These are nodes that satisfy the following:
    //  i)   header.loopStamps encloses curNode.loopStamps and curNode.loopStamps encloses latch.loopStamps
    //    OR
    //  ii)  latch.revLoopStamps encloses curNode.revLoopStamps and curNode.revLoopStamps encloses header.revLoopStamps
    //    OR
    //  iii) curNode is the latch node

    BasicBlock *latch = header->getLatchNode();

    for (int i = header->getOrdering() - 1; i >= latch->getOrdering(); i--) {
        if (m_ordering[i]->inLoop(header, latch)) {
            // update the membership map to reflect that this node is within the loop
            loopNodes[i] = true;

            m_ordering[i]->setLoopHead(header);
        }
    }
}


void ControlFlowAnalyzer::structLoops()
{
    for (int i = m_ordering.size() - 1; i >= 0; i--) {
        BasicBlock *curNode = m_ordering[i]; // the current node under investigation
        BasicBlock *latch   = nullptr;       // the latching node of the loop

        // If the current node has at least one back edge into it, it is a loop header. If there are numerous back edges
        // into the header, determine which one comes form the proper latching node.
        // The proper latching node is defined to have the following properties:
        //     i) has a back edge to the current node
        //    ii) has the same case head as the current node
        // iii) has the same loop head as the current node
        //    iv) is not an nway node
        //     v) is not the latch node of an enclosing loop
        //    vi) has a lower ordering than all other suitable candiates
        // If no nodes meet the above criteria, then the current node is not a loop header

        for (BasicBlock *pred : curNode->getPredecessors()) {
            if ((pred->getCaseHead() == curNode->getCaseHead()) &&                         // ii)
                (pred->getLoopHead() == curNode->getLoopHead()) &&                         // iii)
                (!latch || (latch->getOrdering() > pred->getOrdering())) &&                // vi)
                !(pred->getLoopHead() && (pred->getLoopHead()->getLatchNode() == pred)) && // v)
                pred->hasBackEdgeTo(curNode)) {                                            // i)
                latch = pred;
            }
        }

        // if a latching node was found for the current node then it is a loop header.
        if (latch) {
            // define the map that maps each node to whether or not it is within the current loop
            bool *loopNodes = new bool[m_ordering.size()];

            for (unsigned int j = 0; j < m_ordering.size(); j++) {
                loopNodes[j] = false;
            }

            curNode->setLatchNode(latch);

            // the latching node may already have been structured as a conditional header. If it is not also the loop
            // header (i.e. the loop is over more than one block) then reset it to be a sequential node otherwise it
            // will be correctly set as a loop header only later
            if ((latch != curNode) && (latch->getStructType() == StructType::Cond)) {
                latch->setStructType(StructType::Seq);
            }

            // set the structured type of this node
            curNode->setStructType(StructType::Loop);

            // tag the members of this loop
            tagNodesInLoop(curNode, loopNodes);

            // calculate the type of this loop
            determineLoopType(curNode, loopNodes);

            // calculate the follow node of this loop
            findLoopFollow(curNode, loopNodes);
        }
    }
}


void ControlFlowAnalyzer::checkConds()
{
    for (auto& elem : m_ordering) {
        BasicBlock *curNode = elem;

        // consider only conditional headers that have a follow and aren't case headers
        if (((curNode->getStructType() == StructType::Cond) || (curNode->getStructType() == StructType::LoopCond)) && curNode->getCondFollow() &&
            (curNode->getCondType() != CondType::Case)) {
            // define convenient aliases for the relevant loop and case heads and the out edges
            BasicBlock *myLoopHead   = (curNode->getStructType() == StructType::LoopCond ? curNode : curNode->getLoopHead());
            BasicBlock *follLoopHead = curNode->getCondFollow()->getLoopHead();
            BasicBlock *bbThen = curNode->getSuccessor(BTHEN);
            BasicBlock *bbElse = curNode->getSuccessor(BELSE);

            // analyse whether this is a jump into/outof a loop
            if (myLoopHead != follLoopHead) {
                // we want to find the branch that the latch node is on for a jump out of a loop
                if (myLoopHead) {
                    BasicBlock *myLoopLatch = myLoopHead->getLatchNode();

                    // does the then branch goto the loop latch?
                    if (bbThen->hasBackEdgeTo(myLoopLatch)) {
                        curNode->setUnstructType(UnstructType::JumpInOutLoop);
                        curNode->setCondType(CondType::IfElse);
                    }
                    // does the else branch goto the loop latch?
                    else if (bbElse->hasBackEdgeTo(myLoopLatch)) {
                        curNode->setUnstructType(UnstructType::JumpInOutLoop);
                        curNode->setCondType(CondType::IfThen);
                    }
                }

                if ((curNode->getUnstructType() == UnstructType::Structured) && follLoopHead) {
                    // find the branch that the loop head is on for a jump into a loop body. If a branch has already
                    // been found, then it will match this one anyway

                    // does the else branch goto the loop head?
                    if (bbThen->hasBackEdgeTo(follLoopHead)) {
                        curNode->setUnstructType(UnstructType::JumpInOutLoop);
                        curNode->setCondType(CondType::IfElse);
                    }
                    // does the else branch goto the loop head?
                    else if (bbElse->hasBackEdgeTo(follLoopHead)) {
                        curNode->setUnstructType(UnstructType::JumpInOutLoop);
                        curNode->setCondType(CondType::IfThen);
                    }
                }
            }

            // this is a jump into a case body if either of its children don't have the same same case header as itself
            if ((curNode->getUnstructType() == UnstructType::Structured) &&
                ((curNode->getCaseHead() != bbThen->getCaseHead()) ||
                 (curNode->getCaseHead() != bbElse->getCaseHead()))) {
                BasicBlock *myCaseHead   = curNode->getCaseHead();
                BasicBlock *thenCaseHead = bbThen->getCaseHead();
                BasicBlock *elseCaseHead = bbElse->getCaseHead();

                if ((thenCaseHead == myCaseHead) && (!myCaseHead || (elseCaseHead != myCaseHead->getCondFollow()))) {
                    curNode->setUnstructType(UnstructType::JumpIntoCase);
                    curNode->setCondType(CondType::IfElse);
                }
                else if ((elseCaseHead == myCaseHead) && (!myCaseHead || (thenCaseHead != myCaseHead->getCondFollow()))) {
                    curNode->setUnstructType(UnstructType::JumpIntoCase);
                    curNode->setCondType(CondType::IfThen);
                }
            }
        }

        // for 2 way conditional headers that don't have a follow (i.e. are the source of a back edge) and haven't been
        // structured as latching nodes, set their follow to be the non-back edge child.
        if ((curNode->getStructType() == StructType::Cond) && !curNode->getCondFollow() && (curNode->getCondType() != CondType::Case) &&
            (curNode->getUnstructType() == UnstructType::Structured)) {
            // latching nodes will already have been reset to Seq structured type
            if (curNode->hasBackEdge()) {
                if (curNode->hasBackEdgeTo(curNode->getSuccessor(BTHEN))) {
                    curNode->setCondType(CondType::IfThen);
                    curNode->setCondFollow(curNode->getSuccessor(BELSE));
                }
                else {
                    curNode->setCondType(CondType::IfElse);
                    curNode->setCondFollow(curNode->getSuccessor(BTHEN));
                }
            }
        }
    }
}
