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

#include "boomerang/db/IRFragment.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/util/log/Log.h"


// index of the "then" branch of conditional jumps
#define BTHEN 0

// index of the "else" branch of conditional jumps
#define BELSE 1


ControlFlowAnalyzer::ControlFlowAnalyzer()
{
}


void ControlFlowAnalyzer::structureCFG(ProcCFG *cfg)
{
    m_cfg = cfg;

    if (m_cfg->findRetFragment() == nullptr) {
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

    updateLoopStamps(findEntryFragment(), time);

    // set the reverse parenthesis for the nodes
    time = 1;
    updateRevLoopStamps(findEntryFragment(), time);

    IRFragment *retNode = findExitFragment();
    assert(retNode);
    m_revPostOrdering.clear();
    updateRevOrder(retNode);
}


void ControlFlowAnalyzer::updateImmedPDom()
{
    // traverse the nodes in order (i.e from the bottom up)
    for (int i = m_revPostOrdering.size() - 1; i >= 0; i--) {
        const IRFragment *frag = m_revPostOrdering[i];

        for (IRFragment *succ : frag->getSuccessors()) {
            if (getRevOrd(succ) > getRevOrd(frag)) {
                setImmPDom(frag, findCommonPDom(getImmPDom(frag), succ));
            }
        }
    }

    // make a second pass but consider the original CFG ordering this time
    for (const IRFragment *frag : m_postOrdering) {
        if (frag->getNumSuccessors() <= 1) {
            continue;
        }

        for (auto &succ : frag->getSuccessors()) {
            IRFragment *succNode = succ;
            setImmPDom(frag, findCommonPDom(getImmPDom(frag), succNode));
        }
    }

    // one final pass to fix up nodes involved in a loop
    for (const IRFragment *frag : m_postOrdering) {
        if (frag->getNumSuccessors() > 1) {
            for (const IRFragment *succ : frag->getSuccessors()) {
                if (isBackEdge(frag, succ) && (frag->getNumSuccessors() > 1) && getImmPDom(succ) &&
                    (getPostOrdering(getImmPDom(succ)) < getPostOrdering(getImmPDom(frag)))) {
                    setImmPDom(frag, findCommonPDom(getImmPDom(succ), getImmPDom(frag)));
                }
                else {
                    setImmPDom(frag, findCommonPDom(getImmPDom(frag), succ));
                }
            }
        }
    }
}


const IRFragment *ControlFlowAnalyzer::findCommonPDom(const IRFragment *currImmPDom,
                                                      const IRFragment *succImmPDom)
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

    const IRFragment *oldCurImmPDom  = currImmPDom;
    const IRFragment *oldSuccImmPDom = succImmPDom;

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
    for (const IRFragment *currNode : m_postOrdering) {
        if (currNode->getNumSuccessors() <= 1) {
            // not an if/case condition
            continue;
        }

        // if the current conditional header is a two way node and has a back edge,
        // then it won't have a follow
        if (hasBackEdge(currNode) && currNode->isType(FragType::Twoway)) {
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


void ControlFlowAnalyzer::determineLoopType(const IRFragment *header, bool *&loopNodes)
{
    assert(getLatchNode(header));

    // if the latch node is a two way node then this must be a post tested loop
    if (getLatchNode(header)->isType(FragType::Twoway)) {
        setLoopType(header, LoopType::PostTested);

        // if the head of the loop is a two way node and the loop spans more than one block  then it
        // must also be a conditional header
        if (header->isType(FragType::Twoway) && (header != getLatchNode(header))) {
            setStructType(header, StructType::LoopCond);
        }
    }
    // otherwise it is either a pretested or endless loop
    else if (header->isType(FragType::Twoway)) {
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


void ControlFlowAnalyzer::findLoopFollow(const IRFragment *header, bool *&loopNodes)
{
    assert(getStructType(header) == StructType::Loop ||
           getStructType(header) == StructType::LoopCond);
    const LoopType loopType = getLoopType(header);
    const IRFragment *latch = getLatchNode(header);

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
        const IRFragment *follow = nullptr;

        // traverse the ordering array between the header and latch nodes.
        // IRFragment * latch = header->getLatchNode(); initialized at function start
        for (int i = getPostOrdering(header) - 1; i > getPostOrdering(latch); i--) {
            const IRFragment *&desc = m_postOrdering[i];
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
                    const IRFragment *succ = desc->getSuccessor(BTHEN);

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


void ControlFlowAnalyzer::tagNodesInLoop(const IRFragment *header, bool *&loopNodes)
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

    const IRFragment *latch = getLatchNode(header);
    assert(latch);

    for (int i = getPostOrdering(header) - 1; i >= getPostOrdering(latch); i--) {
        if (isFragInLoop(m_postOrdering[i], header, latch)) {
            // update the membership map to reflect that this node is within the loop
            loopNodes[i] = true;

            setLoopHead(m_postOrdering[i], header);
        }
    }
}


void ControlFlowAnalyzer::structLoops()
{
    for (int i = m_postOrdering.size() - 1; i >= 0; i--) {
        const IRFragment *currFrag = m_postOrdering[i]; // the current node under investigation
        const IRFragment *latch    = nullptr;           // the latching node of the loop

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

        for (const IRFragment *pred : currFrag->getPredecessors()) {
            if ((getCaseHead(pred) == getCaseHead(currFrag)) &&                      // ii)
                (getLoopHead(pred) == getLoopHead(currFrag)) &&                      // iii)
                (!latch || (getPostOrdering(latch) > getPostOrdering(pred))) &&      // vi)
                !(getLoopHead(pred) && (getLatchNode(getLoopHead(pred)) == pred)) && // v)
                isBackEdge(pred, currFrag)) {                                        // i)
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

        setLatchNode(currFrag, latch);

        // the latching node may already have been structured as a conditional header. If it is
        // not also the loop header (i.e. the loop is over more than one block) then reset it to
        // be a sequential node otherwise it will be correctly set as a loop header only later
        if ((latch != currFrag) && (getStructType(latch) == StructType::Cond)) {
            setStructType(latch, StructType::Seq);
        }

        // set the structured type of this node
        setStructType(currFrag, StructType::Loop);

        // tag the members of this loop
        tagNodesInLoop(currFrag, loopNodes);

        // calculate the type of this loop
        determineLoopType(currFrag, loopNodes);

        // calculate the follow node of this loop
        findLoopFollow(currFrag, loopNodes);

        delete[] loopNodes;
    }
}


void ControlFlowAnalyzer::checkConds()
{
    for (const IRFragment *currNode : m_postOrdering) {
        // consider only conditional headers that have a follow and aren't case headers
        if (((getStructType(currNode) == StructType::Cond) ||
             (getStructType(currNode) == StructType::LoopCond)) &&
            getCondFollow(currNode) && (getCondType(currNode) != CondType::Case)) {
            // define convenient aliases for the relevant loop and case heads and the out edges
            const IRFragment *myLoopHead = (getStructType(currNode) == StructType::LoopCond)
                                               ? currNode
                                               : getLoopHead(currNode);
            const IRFragment *follLoopHead = getLoopHead(getCondFollow(currNode));
            const IRFragment *fragThen     = currNode->getSuccessor(BTHEN);
            const IRFragment *fragElse     = currNode->getSuccessor(BELSE);

            // analyse whether this is a jump into/outof a loop
            if (myLoopHead != follLoopHead) {
                // we want to find the branch that the latch node is on for a jump out of a loop
                if (myLoopHead) {
                    // this is a jump out of a loop (break or return)
                    if (getLoopHead(fragThen) != nullptr) {
                        // the "else" branch jumps out of the loop. (e.g. "if (!foo) break;")
                        setUnstructType(currNode, UnstructType::JumpInOutLoop);
                        setCondType(currNode, CondType::IfElse);
                    }
                    else {
                        assert(getLoopHead(fragElse) != nullptr);
                        // the "then" branch jumps out of the loop
                        setUnstructType(currNode, UnstructType::JumpInOutLoop);
                        setCondType(currNode, CondType::IfThen);
                    }
                }

                if ((getUnstructType(currNode) == UnstructType::Structured) && follLoopHead) {
                    // find the branch that the loop head is on for a jump into a loop body. If a
                    // branch has already been found, then it will match this one anyway

                    // does the else branch goto the loop head?
                    if (isBackEdge(fragThen, follLoopHead)) {
                        setUnstructType(currNode, UnstructType::JumpInOutLoop);
                        setCondType(currNode, CondType::IfElse);
                    }
                    // does the else branch goto the loop head?
                    else if (isBackEdge(fragElse, follLoopHead)) {
                        setUnstructType(currNode, UnstructType::JumpInOutLoop);
                        setCondType(currNode, CondType::IfThen);
                    }
                }
            }

            // this is a jump into a case body if either of its children don't have the same same
            // case header as itself
            if ((getUnstructType(currNode) == UnstructType::Structured) &&
                ((getCaseHead(currNode) != getCaseHead(fragThen)) ||
                 (getCaseHead(currNode) != getCaseHead(fragElse)))) {
                const IRFragment *myCaseHead   = getCaseHead(currNode);
                const IRFragment *thenCaseHead = getCaseHead(fragThen);
                const IRFragment *elseCaseHead = getCaseHead(fragElse);

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


bool ControlFlowAnalyzer::isBackEdge(const IRFragment *source, const IRFragment *dest) const
{
    return dest == source || isAncestorOf(dest, source);
}


bool ControlFlowAnalyzer::isCaseOption(const IRFragment *frag) const
{
    if (!getCaseHead(frag)) {
        return false;
    }

    for (int i = 0; i < getCaseHead(frag)->getNumSuccessors() - 1; i++) {
        if (getCaseHead(frag)->getSuccessor(i) == frag) {
            return true;
        }
    }

    return false;
}


bool ControlFlowAnalyzer::isAncestorOf(const IRFragment *frag, const IRFragment *other) const
{
    return (m_info[frag].m_preOrderID < m_info[other].m_preOrderID &&
            m_info[frag].m_postOrderID > m_info[other].m_postOrderID) ||
           (m_info[frag].m_revPreOrderID < m_info[other].m_revPreOrderID &&
            m_info[frag].m_revPostOrderID > m_info[other].m_revPostOrderID);
}


void ControlFlowAnalyzer::updateLoopStamps(const IRFragment *frag, int &time)
{
    // timestamp the current node with the current time
    // and set its traversed flag
    setTravType(frag, TravType::DFS_LNum);
    m_info[frag].m_preOrderID = time;

    // recurse on unvisited children and set inedges for all children
    for (const IRFragment *succ : frag->getSuccessors()) {
        // set the in edge from this child to its parent (the current node)
        // (not done here, might be a problem)
        // outEdges[i]->inEdges.Add(this);

        // recurse on this child if it hasn't already been visited
        if (getTravType(succ) != TravType::DFS_LNum) {
            updateLoopStamps(succ, ++time);
        }
    }

    // set the the second loopStamp value
    m_info[frag].m_postOrderID = ++time;

    // add this node to the ordering structure as well as recording its position within the ordering
    m_info[frag].m_postOrderIndex = static_cast<int>(m_postOrdering.size());
    m_postOrdering.push_back(frag);
}


void ControlFlowAnalyzer::updateRevLoopStamps(const IRFragment *frag, int &time)
{
    // timestamp the current node with the current time and set its traversed flag
    setTravType(frag, TravType::DFS_RNum);
    m_info[frag].m_revPreOrderID = time;

    // recurse on the unvisited children in reverse order
    for (int i = frag->getNumSuccessors() - 1; i >= 0; i--) {
        // recurse on this child if it hasn't already been visited
        if (getTravType(frag->getSuccessor(i)) != TravType::DFS_RNum) {
            updateRevLoopStamps(frag->getSuccessor(i), ++time);
        }
    }

    m_info[frag].m_revPostOrderID = ++time;
}


void ControlFlowAnalyzer::updateRevOrder(const IRFragment *frag)
{
    // Set this node as having been traversed during the post domimator DFS ordering traversal
    setTravType(frag, TravType::DFS_PDom);

    // recurse on unvisited children
    for (const IRFragment *pred : frag->getPredecessors()) {
        if (getTravType(pred) != TravType::DFS_PDom) {
            updateRevOrder(pred);
        }
    }

    // add this node to the ordering structure and record the post dom. order of this node as its
    // index within this ordering structure
    m_info[frag].m_revPostOrderIndex = static_cast<int>(m_revPostOrdering.size());
    m_revPostOrdering.push_back(frag);
}


void ControlFlowAnalyzer::setCaseHead(const IRFragment *frag, const IRFragment *head,
                                      const IRFragment *follow)
{
    assert(!getCaseHead(frag));

    setTravType(frag, TravType::DFS_Case);

    // don't tag this node if it is the case header under investigation
    if (frag != head) {
        m_info[frag].m_caseHead = head;
    }

    // if this is a nested case header, then it's member nodes
    // will already have been tagged so skip straight to its follow
    if (frag->isType(FragType::Nway) && (frag != head)) {
        if (getCondFollow(frag) && (getTravType(getCondFollow(frag)) != TravType::DFS_Case) &&
            (getCondFollow(frag) != follow)) {
            setCaseHead(frag, head, follow);
        }
    }
    else {
        // traverse each child of this node that:
        //   i) isn't on a back-edge,
        //  ii) hasn't already been traversed in a case tagging traversal and,
        // iii) isn't the follow node.
        for (IRFragment *succ : frag->getSuccessors()) {
            if (!isBackEdge(frag, succ) && (getTravType(succ) != TravType::DFS_Case) &&
                (succ != follow)) {
                setCaseHead(succ, head, follow);
            }
        }
    }
}


void ControlFlowAnalyzer::setStructType(const IRFragment *frag, StructType structType)
{
    // if this is a conditional header, determine exactly which type of conditional header it is
    // (i.e. switch, if-then, if-then-else etc.)
    if (structType == StructType::Cond) {
        if (frag->isType(FragType::Nway)) {
            m_info[frag].m_conditionHeaderType = CondType::Case;
        }
        else if (getCondFollow(frag) == frag->getSuccessor(BELSE)) {
            m_info[frag].m_conditionHeaderType = CondType::IfThen;
        }
        else if (getCondFollow(frag) == frag->getSuccessor(BTHEN)) {
            m_info[frag].m_conditionHeaderType = CondType::IfElse;
        }
        else {
            m_info[frag].m_conditionHeaderType = CondType::IfThenElse;
        }
    }

    m_info[frag].m_structuringType = structType;
}


void ControlFlowAnalyzer::setUnstructType(const IRFragment *frag, UnstructType unstructType)
{
    assert((m_info[frag].m_structuringType == StructType::Cond ||
            m_info[frag].m_structuringType == StructType::LoopCond) &&
           m_info[frag].m_conditionHeaderType != CondType::Case);
    m_info[frag].m_unstructuredType = unstructType;
}


UnstructType ControlFlowAnalyzer::getUnstructType(const IRFragment *frag) const
{
    assert((m_info[frag].m_structuringType == StructType::Cond ||
            m_info[frag].m_structuringType == StructType::LoopCond));
    // fails when cenerating code for switches; not sure if actually needed TODO
    // assert(m_conditionHeaderType != CondType::Case);

    return m_info[frag].m_unstructuredType;
}


void ControlFlowAnalyzer::setLoopType(const IRFragment *frag, LoopType l)
{
    assert(getStructType(frag) == StructType::Loop || getStructType(frag) == StructType::LoopCond);
    m_info[frag].m_loopHeaderType = l;

    // set the structured class (back to) just Loop if the loop type is PreTested OR it's PostTested
    // and is a single block loop
    if ((m_info[frag].m_loopHeaderType == LoopType::PreTested) ||
        ((m_info[frag].m_loopHeaderType == LoopType::PostTested) && (frag == getLatchNode(frag)))) {
        setStructType(frag, StructType::Loop);
    }
}


LoopType ControlFlowAnalyzer::getLoopType(const IRFragment *frag) const
{
    assert(getStructType(frag) == StructType::Loop || getStructType(frag) == StructType::LoopCond);
    return m_info[frag].m_loopHeaderType;
}


void ControlFlowAnalyzer::setCondType(const IRFragment *frag, CondType condType)
{
    assert(getStructType(frag) == StructType::Cond || getStructType(frag) == StructType::LoopCond);
    m_info[frag].m_conditionHeaderType = condType;
}


CondType ControlFlowAnalyzer::getCondType(const IRFragment *frag) const
{
    assert(getStructType(frag) == StructType::Cond || getStructType(frag) == StructType::LoopCond);
    return m_info[frag].m_conditionHeaderType;
}


bool ControlFlowAnalyzer::isFragInLoop(const IRFragment *frag, const IRFragment *header,
                                       const IRFragment *latch) const
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
    return frag == latch ||
           (m_info[header].m_preOrderID < m_info[frag].m_preOrderID &&
            m_info[frag].m_postOrderID < m_info[header].m_postOrderID &&
            m_info[frag].m_preOrderID < m_info[latch].m_preOrderID &&
            m_info[latch].m_postOrderID < m_info[frag].m_postOrderID) ||
           (m_info[header].m_revPreOrderID < m_info[frag].m_revPreOrderID &&
            m_info[frag].m_revPostOrderID < m_info[header].m_revPostOrderID &&
            m_info[frag].m_revPreOrderID < m_info[latch].m_revPreOrderID &&
            m_info[latch].m_revPostOrderID < m_info[frag].m_revPostOrderID);
}


bool ControlFlowAnalyzer::hasBackEdge(const IRFragment *frag) const
{
    return std::any_of(frag->getSuccessors().begin(), frag->getSuccessors().end(),
                       [this, frag](const IRFragment *succ) { return isBackEdge(frag, succ); });
}


void ControlFlowAnalyzer::unTraverse()
{
    for (auto &elem : m_info) {
        elem.second.m_travType = TravType::Untraversed;
    }
}


IRFragment *ControlFlowAnalyzer::findEntryFragment() const
{
    return m_cfg->getEntryFragment();
}


IRFragment *ControlFlowAnalyzer::findExitFragment() const
{
    return m_cfg->findRetFragment();
}
