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

#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/util/log/Log.h"


ControlFlowAnalyzer::ControlFlowAnalyzer()
{
}


void ControlFlowAnalyzer::structureCFG(ProcCFG *cfg)
{
    m_cfg = cfg;

    if (m_cfg->findRetNode() == nullptr) {
        return;
    }

    setTimeStamps();
    updateImmedPDom();

    structConds();
    structLoops();
    checkConds();

    unTraverse();
}


void ControlFlowAnalyzer::setTimeStamps()
{
    // set the parenthesis for the nodes as well as setting the post-order ordering between the
    // nodes
    int time = 1;
    m_postOrdering.clear();

    updateLoopStamps(findEntryBB(), time);

    // set the reverse parenthesis for the nodes
    time = 1;
    updateRevLoopStamps(findEntryBB(), time);

    BasicBlock *retNode = findExitBB();
    assert(retNode);
    m_revPostOrdering.clear();
    updateRevOrder(retNode);
}


void ControlFlowAnalyzer::updateImmedPDom()
{
    // traverse the nodes in order (i.e from the bottom up)
    for (int i = m_revPostOrdering.size() - 1; i >= 0; i--) {
        const BasicBlock *bb = m_revPostOrdering[i];

        for (BasicBlock *succ : bb->getSuccessors()) {
            if (getRevOrd(succ) > getRevOrd(bb)) {
                setImmPDom(bb, findCommonPDom(getImmPDom(bb), succ));
            }
        }
    }

    // make a second pass but consider the original CFG ordering this time
    for (const BasicBlock *bb : m_postOrdering) {
        if (bb->getNumSuccessors() <= 1) {
            continue;
        }

        for (auto &succ : bb->getSuccessors()) {
            BasicBlock *succNode = succ;
            setImmPDom(bb, findCommonPDom(getImmPDom(bb), succNode));
        }
    }

    // one final pass to fix up nodes involved in a loop
    for (const BasicBlock *bb : m_postOrdering) {
        if (bb->getNumSuccessors() > 1) {
            for (auto &succ : bb->getSuccessors()) {
                BasicBlock *succNode = succ;

                if (isBackEdge(bb, succNode) && (bb->getNumSuccessors() > 1) &&
                    getImmPDom(succNode) &&
                    (getPostOrdering(getImmPDom(succ)) < getPostOrdering(getImmPDom(bb)))) {
                    setImmPDom(bb, findCommonPDom(getImmPDom(succNode), getImmPDom(bb)));
                }
                else {
                    setImmPDom(bb, findCommonPDom(getImmPDom(bb), succNode));
                }
            }
        }
    }
}


const BasicBlock *ControlFlowAnalyzer::findCommonPDom(const BasicBlock *currImmPDom,
                                                      const BasicBlock *succImmPDom)
{
    if (!currImmPDom) {
        return succImmPDom;
    }

    if (!succImmPDom) {
        return currImmPDom;
    }

    if (getRevOrd(currImmPDom) == getRevOrd(succImmPDom)) {
        return currImmPDom; // ordering hasn't been done
    }

    const BasicBlock *oldCurImmPDom  = currImmPDom;
    const BasicBlock *oldSuccImmPDom = succImmPDom;

    int giveup = 0;
#define GIVEUP 10000

    while (giveup < GIVEUP && currImmPDom && succImmPDom && (currImmPDom != succImmPDom)) {
        if (getRevOrd(currImmPDom) > getRevOrd(succImmPDom)) {
            succImmPDom = getImmPDom(succImmPDom);
        }
        else {
            currImmPDom = getImmPDom(currImmPDom);
        }

        giveup++;
    }

    if (giveup >= GIVEUP) {
        LOG_VERBOSE("Failed to find commonPDom for %1 and %2", oldCurImmPDom->getLowAddr(),
                    oldSuccImmPDom->getLowAddr());

        return oldCurImmPDom; // no change
    }

    return currImmPDom;
}


void ControlFlowAnalyzer::structConds()
{
    // Process the nodes in order
    for (const BasicBlock *currNode : m_postOrdering) {
        if (currNode->getNumSuccessors() <= 1) {
            // not an if/case condition
            continue;
        }

        // if the current conditional header is a two way node and has a back edge, then it
        // won't have a follow
        if (hasBackEdge(currNode) && (currNode->getType() == BBType::Twoway)) {
            setStructType(currNode, StructType::Cond);
            continue;
        }

        // set the follow of a node to be its immediate post dominator
        setCondFollow(currNode, getImmPDom(currNode));

        // set the structured type of this node
        setStructType(currNode, StructType::Cond);

        // if this is an nway header, then we have to tag each of the nodes within the body of
        // the nway subgraph
        if (getCondType(currNode) == CondType::Case) {
            setCaseHead(currNode, currNode, getCondFollow(currNode));
        }
    }
}


void ControlFlowAnalyzer::determineLoopType(const BasicBlock *header, bool *&loopNodes)
{
    assert(getLatchNode(header));

    // if the latch node is a two way node then this must be a post tested loop
    if (getLatchNode(header)->getType() == BBType::Twoway) {
        setLoopType(header, LoopType::PostTested);

        // if the head of the loop is a two way node and the loop spans more than one block  then it
        // must also be a conditional header
        if ((header->getType() == BBType::Twoway) && (header != getLatchNode(header))) {
            setStructType(header, StructType::LoopCond);
        }
    }
    // otherwise it is either a pretested or endless loop
    else if (header->getType() == BBType::Twoway) {
        // if the header is a two way node then it must have a conditional follow (since it can't
        // have any backedges leading from it). If this follow is within the loop then this must be
        // an endless loop
        if (getCondFollow(header) && loopNodes[getPostOrdering(getCondFollow(header))]) {
            setLoopType(header, LoopType::Endless);

            // retain the fact that this is also a conditional header
            setStructType(header, StructType::LoopCond);
        }
        else {
            setLoopType(header, LoopType::PreTested);
        }
    }
    else {
        // both the header and latch node are one way nodes so this must be an endless loop
        setLoopType(header, LoopType::Endless);
    }
}


void ControlFlowAnalyzer::findLoopFollow(const BasicBlock *header, bool *&loopNodes)
{
    assert(getStructType(header) == StructType::Loop ||
           getStructType(header) == StructType::LoopCond);
    const LoopType loopType = getLoopType(header);
    const BasicBlock *latch = getLatchNode(header);

    if (loopType == LoopType::PreTested) {
        // if the 'while' loop's true child is within the loop, then its false child is the loop
        // follow
        if (loopNodes[getPostOrdering(header->getSuccessor(BTHEN))]) {
            setLoopFollow(header, header->getSuccessor(BELSE));
        }
        else {
            setLoopFollow(header, header->getSuccessor(BTHEN));
        }
    }
    else if (loopType == LoopType::PostTested) {
        // the follow of a post tested ('repeat') loop is the node on the end of the non-back edge
        // from the latch node
        if (latch->getSuccessor(BELSE) == header) {
            setLoopFollow(header, latch->getSuccessor(BTHEN));
        }
        else {
            setLoopFollow(header, latch->getSuccessor(BELSE));
        }
    }
    else {
        // endless loop
        const BasicBlock *follow = nullptr;

        // traverse the ordering array between the header and latch nodes.
        // BasicBlock * latch = header->getLatchNode(); initialized at function start
        for (int i = getPostOrdering(header) - 1; i > getPostOrdering(latch); i--) {
            const BasicBlock *&desc = m_postOrdering[i];
            // the follow for an endless loop will have the following
            // properties:
            //   i) it will have a parent that is a conditional header inside the loop whose follow
            //      is outside the loop
            //  ii) it will be outside the loop according to its loop stamp pair
            // iii) have the highest ordering of all suitable follows (i.e. highest in the graph)

            if ((getStructType(desc) == StructType::Cond) && getCondFollow(desc) &&
                (getLoopHead(desc) == header)) {
                if (loopNodes[getPostOrdering(getCondFollow(desc))]) {
                    // if the conditional's follow is in the same loop AND is lower in the loop,
                    // jump to this follow
                    if (getPostOrdering(desc) > getPostOrdering(getCondFollow(desc))) {
                        i = getPostOrdering(getCondFollow(desc));
                    }
                    else {
                        // otherwise there is a backward jump somewhere to a node earlier in this
                        // loop. We don't need to any nodes below this one as they will all have a
                        // conditional within the loop.
                        break;
                    }
                }
                else {
                    // otherwise find the child (if any) of the conditional header that isn't inside
                    // the same loop
                    const BasicBlock *succ = desc->getSuccessor(BTHEN);

                    if (loopNodes[getPostOrdering(succ)]) {
                        if (!loopNodes[getPostOrdering(desc->getSuccessor(BELSE))]) {
                            succ = desc->getSuccessor(BELSE);
                        }
                        else {
                            succ = nullptr;
                        }
                    }

                    // if a potential follow was found, compare its ordering with the currently
                    // found follow
                    if (succ && (!follow || (getPostOrdering(succ) > getPostOrdering(follow)))) {
                        follow = succ;
                    }
                }
            }
        }

        // if a follow was found, assign it to be the follow of the loop under
        // investigation
        if (follow) {
            setLoopFollow(header, follow);
        }
    }
}


void ControlFlowAnalyzer::tagNodesInLoop(const BasicBlock *header, bool *&loopNodes)
{
    // Traverse the ordering structure from the header to the latch node tagging the nodes
    // determined to be within the loop. These are nodes that satisfy the following:
    //    i)   header.loopStamps encloses curNode.loopStamps and curNode.loopStamps encloses
    //         latch.loopStamps
    //    OR
    //   ii)   latch.revLoopStamps encloses curNode.revLoopStamps and curNode.revLoopStamps encloses
    //         header.revLoopStamps
    //    OR
    //  iii) curNode is the latch node

    const BasicBlock *latch = getLatchNode(header);
    assert(latch);

    for (int i = getPostOrdering(header) - 1; i >= getPostOrdering(latch); i--) {
        if (isBBInLoop(m_postOrdering[i], header, latch)) {
            // update the membership map to reflect that this node is within the loop
            loopNodes[i] = true;

            setLoopHead(m_postOrdering[i], header);
        }
    }
}


void ControlFlowAnalyzer::structLoops()
{
    for (int i = m_postOrdering.size() - 1; i >= 0; i--) {
        const BasicBlock *currNode = m_postOrdering[i]; // the current node under investigation
        const BasicBlock *latch    = nullptr;           // the latching node of the loop

        // If the current node has at least one back edge into it, it is a loop header. If there are
        // numerous back edges into the header, determine which one comes form the proper latching
        // node. The proper latching node is defined to have the following properties:
        //     i) has a back edge to the current node
        //    ii) has the same case head as the current node
        //   iii) has the same loop head as the current node
        //    iv) is not an nway node
        //     v) is not the latch node of an enclosing loop
        //    vi) has a lower ordering than all other suitable candiates
        // If no nodes meet the above criteria, then the current node is not a loop header

        for (const BasicBlock *pred : currNode->getPredecessors()) {
            if ((getCaseHead(pred) == getCaseHead(currNode)) &&                      // ii)
                (getLoopHead(pred) == getLoopHead(currNode)) &&                      // iii)
                (!latch || (getPostOrdering(latch) > getPostOrdering(pred))) &&      // vi)
                !(getLoopHead(pred) && (getLatchNode(getLoopHead(pred)) == pred)) && // v)
                isBackEdge(pred, currNode)) {                                        // i)
                latch = pred;
            }
        }

        // if a latching node was found for the current node then it is a loop header.
        if (!latch) {
            continue;
        }

        // define the map that maps each node to whether or not it is within the current loop
        bool *loopNodes = new bool[m_postOrdering.size()];

        for (unsigned int j = 0; j < m_postOrdering.size(); j++) {
            loopNodes[j] = false;
        }

        setLatchNode(currNode, latch);

        // the latching node may already have been structured as a conditional header. If it is
        // not also the loop header (i.e. the loop is over more than one block) then reset it to
        // be a sequential node otherwise it will be correctly set as a loop header only later
        if ((latch != currNode) && (getStructType(latch) == StructType::Cond)) {
            setStructType(latch, StructType::Seq);
        }

        // set the structured type of this node
        setStructType(currNode, StructType::Loop);

        // tag the members of this loop
        tagNodesInLoop(currNode, loopNodes);

        // calculate the type of this loop
        determineLoopType(currNode, loopNodes);

        // calculate the follow node of this loop
        findLoopFollow(currNode, loopNodes);

        delete[] loopNodes;
    }
}


void ControlFlowAnalyzer::checkConds()
{
    for (const BasicBlock *currNode : m_postOrdering) {
        // consider only conditional headers that have a follow and aren't case headers
        if (((getStructType(currNode) == StructType::Cond) ||
             (getStructType(currNode) == StructType::LoopCond)) &&
            getCondFollow(currNode) && (getCondType(currNode) != CondType::Case)) {
            // define convenient aliases for the relevant loop and case heads and the out edges
            const BasicBlock *myLoopHead = (getStructType(currNode) == StructType::LoopCond)
                                               ? currNode
                                               : getLoopHead(currNode);
            const BasicBlock *follLoopHead = getLoopHead(getCondFollow(currNode));
            const BasicBlock *bbThen       = currNode->getSuccessor(BTHEN);
            const BasicBlock *bbElse       = currNode->getSuccessor(BELSE);

            // analyse whether this is a jump into/outof a loop
            if (myLoopHead != follLoopHead) {
                // we want to find the branch that the latch node is on for a jump out of a loop
                if (myLoopHead) {
                    // this is a jump out of a loop (break or return)
                    if (getLoopHead(bbThen) != nullptr) {
                        // the "else" branch jumps out of the loop. (e.g. "if (!foo) break;")
                        setUnstructType(currNode, UnstructType::JumpInOutLoop);
                        setCondType(currNode, CondType::IfElse);
                    }
                    else {
                        assert(getLoopHead(bbElse) != nullptr);
                        // the "then" branch jumps out of the loop
                        setUnstructType(currNode, UnstructType::JumpInOutLoop);
                        setCondType(currNode, CondType::IfThen);
                    }
                }

                if ((getUnstructType(currNode) == UnstructType::Structured) && follLoopHead) {
                    // find the branch that the loop head is on for a jump into a loop body. If a
                    // branch has already been found, then it will match this one anyway

                    // does the else branch goto the loop head?
                    if (isBackEdge(bbThen, follLoopHead)) {
                        setUnstructType(currNode, UnstructType::JumpInOutLoop);
                        setCondType(currNode, CondType::IfElse);
                    }
                    // does the else branch goto the loop head?
                    else if (isBackEdge(bbElse, follLoopHead)) {
                        setUnstructType(currNode, UnstructType::JumpInOutLoop);
                        setCondType(currNode, CondType::IfThen);
                    }
                }
            }

            // this is a jump into a case body if either of its children don't have the same same
            // case header as itself
            if ((getUnstructType(currNode) == UnstructType::Structured) &&
                ((getCaseHead(currNode) != getCaseHead(bbThen)) ||
                 (getCaseHead(currNode) != getCaseHead(bbElse)))) {
                const BasicBlock *myCaseHead   = getCaseHead(currNode);
                const BasicBlock *thenCaseHead = getCaseHead(bbThen);
                const BasicBlock *elseCaseHead = getCaseHead(bbElse);

                if ((thenCaseHead == myCaseHead) &&
                    (!myCaseHead || (elseCaseHead != getCondFollow(myCaseHead)))) {
                    setUnstructType(currNode, UnstructType::JumpIntoCase);
                    setCondType(currNode, CondType::IfElse);
                }
                else if ((elseCaseHead == myCaseHead) &&
                         (!myCaseHead || (thenCaseHead != getCondFollow(myCaseHead)))) {
                    setUnstructType(currNode, UnstructType::JumpIntoCase);
                    setCondType(currNode, CondType::IfThen);
                }
            }
        }

        // for 2 way conditional headers that don't have a follow (i.e. are the source of a back
        // edge) and haven't been structured as latching nodes, set their follow to be the non-back
        // edge child.
        if ((getStructType(currNode) == StructType::Cond) && !getCondFollow(currNode) &&
            (getCondType(currNode) != CondType::Case) &&
            (getUnstructType(currNode) == UnstructType::Structured)) {
            // latching nodes will already have been reset to Seq structured type
            if (hasBackEdge(currNode)) {
                if (isBackEdge(currNode, currNode->getSuccessor(BTHEN))) {
                    setCondType(currNode, CondType::IfThen);
                    setCondFollow(currNode, currNode->getSuccessor(BELSE));
                }
                else {
                    setCondType(currNode, CondType::IfElse);
                    setCondFollow(currNode, currNode->getSuccessor(BTHEN));
                }
            }
        }
    }
}


bool ControlFlowAnalyzer::isBackEdge(const BasicBlock *source, const BasicBlock *dest) const
{
    return dest == source || isAncestorOf(dest, source);
}


bool ControlFlowAnalyzer::isCaseOption(const BasicBlock *bb) const
{
    if (!getCaseHead(bb)) {
        return false;
    }

    for (int i = 0; i < getCaseHead(bb)->getNumSuccessors() - 1; i++) {
        if (getCaseHead(bb)->getSuccessor(i) == bb) {
            return true;
        }
    }

    return false;
}


bool ControlFlowAnalyzer::isAncestorOf(const BasicBlock *bb, const BasicBlock *other) const
{
    return (m_info[bb].m_preOrderID < m_info[other].m_preOrderID &&
            m_info[bb].m_postOrderID > m_info[other].m_postOrderID) ||
           (m_info[bb].m_revPreOrderID < m_info[other].m_revPreOrderID &&
            m_info[bb].m_revPostOrderID > m_info[other].m_revPostOrderID);
}


void ControlFlowAnalyzer::updateLoopStamps(const BasicBlock *bb, int &time)
{
    // timestamp the current node with the current time
    // and set its traversed flag
    setTravType(bb, TravType::DFS_LNum);
    m_info[bb].m_preOrderID = time;

    // recurse on unvisited children and set inedges for all children
    for (const BasicBlock *succ : bb->getSuccessors()) {
        // set the in edge from this child to its parent (the current node)
        // (not done here, might be a problem)
        // outEdges[i]->inEdges.Add(this);

        // recurse on this child if it hasn't already been visited
        if (getTravType(succ) != TravType::DFS_LNum) {
            updateLoopStamps(succ, ++time);
        }
    }

    // set the the second loopStamp value
    m_info[bb].m_postOrderID = ++time;

    // add this node to the ordering structure as well as recording its position within the ordering
    m_info[bb].m_postOrderIndex = static_cast<int>(m_postOrdering.size());
    m_postOrdering.push_back(bb);
}


void ControlFlowAnalyzer::updateRevLoopStamps(const BasicBlock *bb, int &time)
{
    // timestamp the current node with the current time and set its traversed flag
    setTravType(bb, TravType::DFS_RNum);
    m_info[bb].m_revPreOrderID = time;

    // recurse on the unvisited children in reverse order
    for (int i = bb->getNumSuccessors() - 1; i >= 0; i--) {
        // recurse on this child if it hasn't already been visited
        if (getTravType(bb->getSuccessor(i)) != TravType::DFS_RNum) {
            updateRevLoopStamps(bb->getSuccessor(i), ++time);
        }
    }

    m_info[bb].m_revPostOrderID = ++time;
}


void ControlFlowAnalyzer::updateRevOrder(const BasicBlock *bb)
{
    // Set this node as having been traversed during the post domimator DFS ordering traversal
    setTravType(bb, TravType::DFS_PDom);

    // recurse on unvisited children
    for (const BasicBlock *pred : bb->getPredecessors()) {
        if (getTravType(pred) != TravType::DFS_PDom) {
            updateRevOrder(pred);
        }
    }

    // add this node to the ordering structure and record the post dom. order of this node as its
    // index within this ordering structure
    m_info[bb].m_revPostOrderIndex = static_cast<int>(m_revPostOrdering.size());
    m_revPostOrdering.push_back(bb);
}


void ControlFlowAnalyzer::setCaseHead(const BasicBlock *bb, const BasicBlock *head,
                                      const BasicBlock *follow)
{
    assert(!getCaseHead(bb));

    setTravType(bb, TravType::DFS_Case);

    // don't tag this node if it is the case header under investigation
    if (bb != head) {
        m_info[bb].m_caseHead = head;
    }

    // if this is a nested case header, then it's member nodes
    // will already have been tagged so skip straight to its follow
    if (bb->isType(BBType::Nway) && (bb != head)) {
        if (getCondFollow(bb) && (getTravType(getCondFollow(bb)) != TravType::DFS_Case) &&
            (getCondFollow(bb) != follow)) {
            setCaseHead(bb, head, follow);
        }
    }
    else {
        // traverse each child of this node that:
        //   i) isn't on a back-edge,
        //  ii) hasn't already been traversed in a case tagging traversal and,
        // iii) isn't the follow node.
        for (BasicBlock *succ : bb->getSuccessors()) {
            if (!isBackEdge(bb, succ) && (getTravType(succ) != TravType::DFS_Case) &&
                (succ != follow)) {
                setCaseHead(succ, head, follow);
            }
        }
    }
}


void ControlFlowAnalyzer::setStructType(const BasicBlock *bb, StructType structType)
{
    // if this is a conditional header, determine exactly which type of conditional header it is
    // (i.e. switch, if-then, if-then-else etc.)
    if (structType == StructType::Cond) {
        if (bb->isType(BBType::Nway)) {
            m_info[bb].m_conditionHeaderType = CondType::Case;
        }
        else if (getCondFollow(bb) == bb->getSuccessor(BELSE)) {
            m_info[bb].m_conditionHeaderType = CondType::IfThen;
        }
        else if (getCondFollow(bb) == bb->getSuccessor(BTHEN)) {
            m_info[bb].m_conditionHeaderType = CondType::IfElse;
        }
        else {
            m_info[bb].m_conditionHeaderType = CondType::IfThenElse;
        }
    }

    m_info[bb].m_structuringType = structType;
}


void ControlFlowAnalyzer::setUnstructType(const BasicBlock *bb, UnstructType unstructType)
{
    assert((m_info[bb].m_structuringType == StructType::Cond ||
            m_info[bb].m_structuringType == StructType::LoopCond) &&
           m_info[bb].m_conditionHeaderType != CondType::Case);
    m_info[bb].m_unstructuredType = unstructType;
}


UnstructType ControlFlowAnalyzer::getUnstructType(const BasicBlock *bb) const
{
    assert((m_info[bb].m_structuringType == StructType::Cond ||
            m_info[bb].m_structuringType == StructType::LoopCond));
    // fails when cenerating code for switches; not sure if actually needed TODO
    // assert(m_conditionHeaderType != CondType::Case);

    return m_info[bb].m_unstructuredType;
}


void ControlFlowAnalyzer::setLoopType(const BasicBlock *bb, LoopType l)
{
    assert(getStructType(bb) == StructType::Loop || getStructType(bb) == StructType::LoopCond);
    m_info[bb].m_loopHeaderType = l;

    // set the structured class (back to) just Loop if the loop type is PreTested OR it's PostTested
    // and is a single block loop
    if ((m_info[bb].m_loopHeaderType == LoopType::PreTested) ||
        ((m_info[bb].m_loopHeaderType == LoopType::PostTested) && (bb == getLatchNode(bb)))) {
        setStructType(bb, StructType::Loop);
    }
}


LoopType ControlFlowAnalyzer::getLoopType(const BasicBlock *bb) const
{
    assert(getStructType(bb) == StructType::Loop || getStructType(bb) == StructType::LoopCond);
    return m_info[bb].m_loopHeaderType;
}


void ControlFlowAnalyzer::setCondType(const BasicBlock *bb, CondType condType)
{
    assert(getStructType(bb) == StructType::Cond || getStructType(bb) == StructType::LoopCond);
    m_info[bb].m_conditionHeaderType = condType;
}


CondType ControlFlowAnalyzer::getCondType(const BasicBlock *bb) const
{
    assert(getStructType(bb) == StructType::Cond || getStructType(bb) == StructType::LoopCond);
    return m_info[bb].m_conditionHeaderType;
}


bool ControlFlowAnalyzer::isBBInLoop(const BasicBlock *bb, const BasicBlock *header,
                                     const BasicBlock *latch) const
{
    assert(getLatchNode(header) == latch);
    assert(header == latch || ((m_info[header].m_preOrderID > m_info[latch].m_preOrderID &&
                                m_info[latch].m_postOrderID > m_info[header].m_postOrderID) ||
                               (m_info[header].m_preOrderID < m_info[latch].m_preOrderID &&
                                m_info[latch].m_postOrderID < m_info[header].m_postOrderID)));

    // this node is in the loop if it is the latch node OR
    // this node is within the header and the latch is within this when using the forward loop
    // stamps OR this node is within the header and the latch is within this when using the reverse
    // loop stamps
    return bb == latch ||
           (m_info[header].m_preOrderID < m_info[bb].m_preOrderID &&
            m_info[bb].m_postOrderID < m_info[header].m_postOrderID &&
            m_info[bb].m_preOrderID < m_info[latch].m_preOrderID &&
            m_info[latch].m_postOrderID < m_info[bb].m_postOrderID) ||
           (m_info[header].m_revPreOrderID < m_info[bb].m_revPreOrderID &&
            m_info[bb].m_revPostOrderID < m_info[header].m_revPostOrderID &&
            m_info[bb].m_revPreOrderID < m_info[latch].m_revPreOrderID &&
            m_info[latch].m_revPostOrderID < m_info[bb].m_revPostOrderID);
}


bool ControlFlowAnalyzer::hasBackEdge(const BasicBlock *bb) const
{
    return std::any_of(bb->getSuccessors().begin(), bb->getSuccessors().end(),
                       [this, bb](const BasicBlock *succ) { return isBackEdge(bb, succ); });
}


void ControlFlowAnalyzer::unTraverse()
{
    for (auto &elem : m_info) {
        elem.second.m_travType = TravType::Untraversed;
    }
}


BasicBlock *ControlFlowAnalyzer::findEntryBB() const
{
    return m_cfg->getEntryBB();
}


BasicBlock *ControlFlowAnalyzer::findExitBB() const
{
    return m_cfg->findRetNode();
}
