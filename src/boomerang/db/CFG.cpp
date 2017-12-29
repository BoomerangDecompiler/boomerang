#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CFG.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/util/Types.h"
#include "boomerang/util/Log.h"
#include "boomerang/codegen/ICodeGenerator.h"
#include "boomerang/util/Util.h"

#include <algorithm>
#include <cassert>
#include <cstring>


Cfg::Cfg(UserProc *proc)
    : m_myProc(proc)
    , m_wellFormed(false)
    , m_structured(false)
    , m_implicitsDone(false)
    , m_entryBB(nullptr)
    , m_exitBB(nullptr)
{
}


Cfg::~Cfg()
{
    for (BasicBlock *bb : m_listBB) {
        delete bb;
    }
}


void Cfg::clear()
{
    // Don't delete the BBs; this will delete any CaseStatements we want to save for the re-decode.
    // Just let them leak since we do not use a garbage collection any more.
    // A better idea would be to save the CaseStatements explicitly and delete the BBs afterwards.
    // But this has to wait until the decoder redesign.

    m_listBB.clear();
    m_mapBB.clear();
    m_implicitMap.clear();
    m_entryBB    = nullptr;
    m_exitBB     = nullptr;
    m_wellFormed = false;
}

void Cfg::setEntryAndExitBB(BasicBlock *entryBB)
{
    m_entryBB = entryBB;

    for (BasicBlock *bb : m_listBB) {
        if (bb->getType() == BBType::Ret) {
            m_exitBB = bb;
            return;
        }
    }

    // It is possible that there is no exit BB
}


void Cfg::setExitBB(BasicBlock *bb)
{
    m_exitBB = bb;
}


bool Cfg::hasNoEntryBB()
{
    if (m_entryBB != nullptr) {
        return false;
    }

    if (m_myProc) {
        LOG_WARN("No entry BB for %1", m_myProc->getName());
    }
    else {
        LOG_WARN("No entry BB for unknown proc");
    }

    return true;
}


BasicBlock *Cfg::createBB(std::unique_ptr<RTLList> pRtls, BBType bbType)
{
    MAPBB::iterator mi         = m_mapBB.end();
    BasicBlock      *currentBB = nullptr;

    // First find the native address of the first RTL
    // Can't use BasicBlock::GetLowAddr(), since we don't yet have a BB!
    Address startAddr = pRtls->front()->getAddress();

    // If this is zero, try the next RTL (only). This may be necessary if e.g. there is a BB with a delayed branch only,
    // with its delay instruction moved in front of it (with 0 address).
    // Note: it is possible to see two RTLs with zero address with Sparc: jmpl %o0, %o1. There will be one for the delay
    // instr (if not a NOP), and one for the side effect of copying %o7 to %o1.
    // Note that orphaned BBs (for which we must compute addr here to to be 0) must not be added to the map, but they
    // have no RTLs with a non zero address.
    if (startAddr.isZero() && (pRtls->size() > 1)) {
        std::list<RTL *>::iterator next = std::next(pRtls->begin());
        startAddr = (*next)->getAddress();
    }

    // If this addr is non zero, check the map to see if we have a (possibly incomplete) BB here already
    // If it is zero, this is a special BB for handling delayed branches or the like
    bool bDone = false;

    if (!startAddr.isZero()) {
        mi = m_mapBB.find(startAddr);

        if ((mi != m_mapBB.end()) && (*mi).second) {
            currentBB = (*mi).second;

            // It should be incomplete, or the pBB there should be zero (we have called Label but not yet created the BB
            // for it).  Else we have duplicated BBs. Note: this can happen with forward jumps into the middle of a
            // loop, so not error
            if (!currentBB->m_incomplete) {
                // This list of RTLs is not needed now
                qDeleteAll(*pRtls);

                LOG_VERBOSE("throwing BBAlreadyExistsError");

                throw BBAlreadyExistsError(currentBB);
            }
            else {
                // Fill in the details, and return it
                currentBB->setRTLs(std::move(pRtls));
                currentBB->m_incomplete = false;
                currentBB->setType(bbType);
            }

            bDone = true;
        }
    }

    if (!bDone) {
        // Else add a new BB to the back of the current list.
        currentBB = new BasicBlock(bbType, std::move(pRtls), m_myProc);
        m_listBB.push_back(currentBB);

        // Also add the address to the map from native (source) address to
        // pointer to BB, unless it's zero
        if (!startAddr.isZero()) {
            m_mapBB[startAddr] = currentBB; // Insert the mapping
            mi = m_mapBB.find(startAddr);
        }
    }

    if (!startAddr.isZero() && (mi != m_mapBB.end())) {
        //
        //  Existing   New         +---+ Top of new
        //            +---+        +---+
        //            |   |          \/ Fall through
        //    +---+   |   | =>     +---+
        //    |   |   |   |        |   | Existing; rest of new discarded
        //    +---+   +---+        +---+
        //
        // Check for overlap of the just added BB with the next BB (address wise).  If there is an overlap, truncate the
        // std::list<Exp*> for the new BB to not overlap, and make this a fall through BB.
        // We still want to do this even if the new BB overlaps with an incomplete BB, though in this case,
        // splitBB needs to fill in the details for the "bottom" BB of the split.
        // Also, in this case, we return a pointer to the newly completed BB, so it will get out edges added
        // (if required). In the other case (i.e. we overlap with an existing, completed BB), we want to return 0, since
        // the out edges are already created.
        //
        mi = std::next(mi);

        if (mi != m_mapBB.end()) {
            BasicBlock *nextBB          = (*mi).second;
            Address    nextAddr         = (*mi).first;
            bool       nextIsIncomplete = nextBB->m_incomplete;

            if (nextAddr <= currentBB->getRTLs()->back()->getAddress()) {
                // Need to truncate the current BB. We use splitBB(), but pass it pNextBB so it doesn't create a new BB
                // for the "bottom" BB of the split pair
                splitBB(currentBB, nextAddr, nextBB);

                // If the overlapped BB was incomplete, return the "bottom" part of the BB, so adding out edges will
                // work properly.
                if (nextIsIncomplete) {
                    assert(nextBB);
                    return nextBB;
                }

                // However, if the overlapping BB was already complete, return 0, so out edges won't be added twice
                throw BBAlreadyExistsError(nextBB);
            }
        }

        //  Existing    New        +---+ Top of existing
        //    +---+                +---+
        //    |   |    +---+       +---+ Fall through
        //    |   |    |   | =>    |   |
        //    |   |    |   |       |   | New; rest of existing discarded
        //    +---+    +---+       +---+
        //
        // Note: no need to check the other way around, because in this case, we will have called Cfg::Label(), and it
        // will have split the existing BB already.
    }

    assert(currentBB);
    return currentBB;
}


BasicBlock *Cfg::createIncompleteBB(Address addr)
{
    // Create a new (basically empty) BB
    BasicBlock *pBB = new BasicBlock(m_myProc);

    // Add it to the list
    m_listBB.push_back(pBB);
    m_mapBB[addr] = pBB; // Insert the mapping
    return pBB;
}


void Cfg::addOutEdge(BasicBlock *sourceBB, BasicBlock *destBB, bool destRequiresLabel /* = false */)
{
    // Wire up edges
    sourceBB->m_successors.push_back(destBB);
    destBB->m_predecessors.push_back(sourceBB);

    // special handling for upgrading oneway BBs to twoway BBs
    if ((sourceBB->getType() == BBType::Oneway) && (sourceBB->getSuccessors().size() > 1)) {
        sourceBB->setType(BBType::Twoway);
        destRequiresLabel = true;
    }

    if (destRequiresLabel) {
        setLabelRequired(destBB); // Indicate "label required"
    }
}


void Cfg::addOutEdge(BasicBlock *sourceBB, Address addr, bool requiresLabel /* = false */)
{
    // If we already have a BB for this address, add the edge to it.
    // If not, create a new incomplete BB at the destination address.
    BasicBlock *destBB = getBB(addr);

    if (!destBB) {
        destBB = createIncompleteBB(addr);
    }

    this->addOutEdge(sourceBB, destBB, requiresLabel);
}


bool Cfg::existsBB(Address addr) const
{
    return getBB(addr) != nullptr;
}


BasicBlock *Cfg::splitBB(BasicBlock *bb, Address splitAddr, BasicBlock *_newBB /* = 0 */,
                         bool deleteRTLs /* = false */)
{
    std::list<RTL *>::iterator ri;

    // First find which RTL has the split address; note that this could fail (e.g. label in the middle of an
    // instruction, or some weird delay slot effects)
    for (ri = bb->m_listOfRTLs->begin(); ri != bb->m_listOfRTLs->end(); ri++) {
        if ((*ri)->getAddress() == splitAddr) {
            break;
        }
    }

    if (ri == bb->m_listOfRTLs->end()) {
        LOG_WARN("Cannot split BB at address %1 at split address %2", bb->getLowAddr(), splitAddr);
        return bb;
    }

    // If necessary, set up a new basic block with information from the original bb
    if (_newBB == nullptr) {
        _newBB = new BasicBlock(*bb);
        // But we don't want the top BB's in edges; our only in-edge should be the out edge from the top BB
        _newBB->m_predecessors.clear();

        // The "bottom" BB now starts at the implicit label, so we create a new list
        // that starts at ri. We need a new list, since it is different from the
        // original BB's list. We don't have to "deep copy" the RTLs themselves,
        // since they will never overlap
        _newBB->setRTLs(Util::makeUnique<RTLList>(ri, bb->m_listOfRTLs->end()));
        m_listBB.push_back(_newBB); // Put it in the graph
        // Put the implicit label into the map. Need to do this before the addOutEdge() below
        m_mapBB[splitAddr] = _newBB;
        // There must be a label here; else would not be splitting. Give it a new label
        _newBB->m_labelNeeded = true;
    }
    else if (_newBB->m_incomplete) {
        // We have an existing BB and a map entry, but no details except for
        // in-edges and m_bHasLabel.
        // First save the in-edges and m_iLabelNum
        std::vector<BasicBlock *> ins(_newBB->m_predecessors);

        // Copy over the details now, completing the bottom BB
        *_newBB = *bb;              // Assign the BB, copying fields. This will set m_bIncomplete false
                                    // Replace the in edges (likely only one)
        _newBB->m_predecessors  = ins;
        _newBB->m_labelNeeded = true;
        _newBB->setRTLs(Util::makeUnique<RTLList>(ri, bb->m_listOfRTLs->end()));
    }

    // else pNewBB exists and is complete. We don't want to change the complete
    // BB in any way, except to later add one in-edge
    bb->setType(BBType::Fall); // Update original ("top") basic block's info and make it a fall-through

    // Fix the in-edges of pBB's descendants. They are now pNewBB
    // Note: you can't believe m_iNumOutEdges at the time that this function may
    // get called
    for (BasicBlock *pDescendant : bb->m_successors) {
        // Search through the in edges for pBB (old ancestor)
        unsigned k;

        for (k = 0; k < pDescendant->m_predecessors.size(); k++) {
            if (pDescendant->m_predecessors[k] == bb) {
                // Replace with a pointer to the new ancestor
                pDescendant->m_predecessors[k] = _newBB;
                break;
            }
        }

        // That pointer should have been found!
        assert(k < pDescendant->m_predecessors.size());
    }

    // The old BB needs to have part of its list of RTLs erased, since the
    // instructions overlap
    if (deleteRTLs) {
        // Delete the list of pointers, and also the RTLs they point to
        qDeleteAll(ri, bb->m_listOfRTLs->end());
        bb->m_listOfRTLs->erase(ri, bb->m_listOfRTLs->end());
    }
    else {
        // Delete the list of pointers, but not the RTLs they point to
        bb->m_listOfRTLs->erase(ri, bb->m_listOfRTLs->end());
    }

    // Erase any existing out edges
    bb->m_successors.erase(bb->m_successors.begin(), bb->m_successors.end());
    addOutEdge(bb, splitAddr);
    return _newBB;
}


BasicBlock *Cfg::getFirstBB(iterator& it)
{
    it = m_listBB.begin();

    if (it == m_listBB.end()) {
        return nullptr;
    }

    return *it;
}


const BasicBlock *Cfg::getFirstBB(const_iterator& it) const
{
    it = m_listBB.begin();

    if (it == m_listBB.end()) {
        return nullptr;
    }

    return *it;
}


BasicBlock *Cfg::getNextBB(iterator& it)
{
    if (++it == m_listBB.end()) {
        return nullptr;
    }

    return *it;
}


const BasicBlock *Cfg::getNextBB(const_iterator& it) const
{
    if (++it == m_listBB.end()) {
        return nullptr;
    }

    return *it;
}


bool Cfg::label(Address uNativeAddr, BasicBlock *& pCurBB)
{
    MAPBB::iterator mi, newi;

    mi = m_mapBB.find(uNativeAddr);     // check if the native address is in the map already (explicit label)

    if (mi == m_mapBB.end()) {          // not in the map
                                        // If not an explicit label, temporarily add the address to the map
        m_mapBB[uNativeAddr] = nullptr; // no PBB yet
                                        // get an iterator to the new native address and check if the previous
                                        // element in the (sorted) map overlaps this new native address; if so,
                                        // it's a non-explicit label which needs to be made explicit by
                                        // splitting the previous BB.
        mi   = m_mapBB.find(uNativeAddr);
        newi = mi;
        bool       bSplit   = false;
        BasicBlock *pPrevBB = nullptr;

        if (newi != m_mapBB.begin()) {
            pPrevBB = (*--mi).second;

            if (!pPrevBB->m_incomplete && (pPrevBB->getLowAddr() < uNativeAddr) &&
                (pPrevBB->getHiAddr() >= uNativeAddr)) {
                bSplit = true;
            }
        }

        if (bSplit) {
            // Non-explicit label. Split the previous BB
            BasicBlock *pNewBB = splitBB(pPrevBB, uNativeAddr);

            if (pCurBB == pPrevBB) {
                // This means that the BB that we are expecting to use, usually to add
                // out edges, has changed. We must change this pointer so that the right
                // BB gets the out edges. However, if the new BB is not the BB of
                // interest, we mustn't change pCurBB
                pCurBB = pNewBB;
            }

            return true;  // wasn't a label, but already parsed
        }
        else {            // not a non-explicit label
                          // We don't have to erase this map entry. Having a null BasicBlock
                          // pointer is coped with in newBB() and addOutEdge(); when eventually
                          // the BB is created, it will replace this entry.  We should be
                          // currently processing this BB. The map will be corrected when newBB is
                          // called with this address.
            return false; // was not already parsed
        }
    }
    else {               // We already have uNativeAddr in the map
        if ((*mi).second && !(*mi).second->m_incomplete) {
            return true; // There is a complete BB here. Return true.
        }

        // We are finalising an incomplete BB. Still need to check previous map
        // entry to see if there is a complete BB overlapping
        bool       bSplit = false;
        BasicBlock *pPrevBB = nullptr, *pBB = (*mi).second;

        if (mi != m_mapBB.begin()) {
            pPrevBB = (*--mi).second;

            if (!pPrevBB->m_incomplete && (pPrevBB->getLowAddr() < uNativeAddr) &&
                (pPrevBB->getHiAddr() >= uNativeAddr)) {
                bSplit = true;
            }
        }

        if (bSplit) {
            // Pass the third parameter to splitBB, because we already have an
            // (incomplete) BB for the "bottom" BB of the split
            splitBB(pPrevBB, uNativeAddr, pBB); // non-explicit label
            return true;                        // wasn't a label, but already parsed
        }

        // A non overlapping, incomplete entry is in the map.
        return false;
    }
}


bool Cfg::isIncomplete(Address uAddr) const
{
    const BasicBlock *bb = getBB(uAddr);

    return bb && bb->m_incomplete;
}


void Cfg::sortByAddress()
{
    m_listBB.sort([] (const BasicBlock *bb1, const BasicBlock *bb2) {
        return bb1->getLowAddr() < bb2->getLowAddr();
    });
}


bool Cfg::isWellFormed() const
{
    m_wellFormed = true;

    for (const BasicBlock *elem : m_listBB) {
        // it iterates through all BBs in the list
        // Check that it's complete
        const BasicBlock *current = elem;

        if (current->m_incomplete) {
            m_wellFormed = false;
            MAPBB::const_iterator itm;

            for (itm = m_mapBB.begin(); itm != m_mapBB.end(); itm++) {
                if ((*itm).second == elem) {
                    break;
                }
            }

            if (itm == m_mapBB.end()) {
                LOG_ERROR("Incomplete BB not even in map!");
            }
            else {
                LOG_ERROR("BB with native address %1 is incomplete", (*itm).first);
            }
        }
        else {
            // Complete. Test the out edges
            // assert(current->m_OutEdges.size() == current->m_iTargetOutEdges);
            for (size_t i = 0; i < current->m_successors.size(); i++) {
                // check if address is interprocedural
                //                if ((*it)->m_OutEdgeInterProc[i] == false)
                {
                    // i iterates through the outedges in the BB *it
                    BasicBlock *pBB = current->m_successors[i];

                    // Check that the out edge has been written (i.e. nonzero)
                    if (pBB == nullptr) {
                        m_wellFormed = false;                   // At least one problem
                        Address addr = current->getLowAddr();
                        LOG_ERROR("BB with native address %1 is missing outedge %2", addr, i);
                    }
                    else {
                        // Check that there is a corresponding in edge from the child to here
                        auto ii = std::find(pBB->m_predecessors.begin(), pBB->m_predecessors.end(), elem);

                        if (ii == pBB->m_predecessors.end()) {
                            LOG_ERROR("No in edge to BB at %1 from successor BB at %2",
                                      (elem)->getLowAddr(), pBB->getLowAddr());
                            m_wellFormed = false;                      // At least one problem
                        }
                    }
                }
            }

            // Also check that each in edge has a corresponding out edge to here (could have an extra in-edge, for
            // example)
            std::vector<BasicBlock *>::iterator ii;

            for (BasicBlock *elem_inedge : elem->m_predecessors) {
                auto oo = std::find(elem_inedge->m_successors.begin(), elem_inedge->m_successors.end(), elem);

                if (oo == elem_inedge->m_successors.end()) {
                    LOG_ERROR("No out edge to BB at %1 from predecessor BB at %2",
                              elem->getLowAddr(), elem_inedge->getLowAddr());
                    m_wellFormed = false;                // At least one problem
                }
            }
        }
    }

    return m_wellFormed;
}


bool Cfg::mergeBBs(BasicBlock *pb1, BasicBlock *pb2)
{
    // Can only merge if pb1 has only one outedge to pb2, and pb2 has only one in-edge, from pb1. This can only be done
    // after the in-edges are done, which can only be done on a well formed CFG.
    if (!m_wellFormed) {
        return false;
    }

    if (pb1->m_successors.size() != 1) {
        return false;
    }

    if (pb2->m_predecessors.size() != 1) {
        return false;
    }

    if (pb1->m_successors[0] != pb2) {
        return false;
    }

    if (pb2->m_predecessors[0] != pb1) {
        return false;
    }

    // Merge them! We remove pb1 rather than pb2, since this is also what is needed for many optimisations, e.g. jump to
    // jump.
    completeMerge(pb1, pb2, true);
    return true;
}


void Cfg::completeMerge(BasicBlock *bb1, BasicBlock *bb2, bool bDelete)
{
    // First we replace all of pb1's predecessors' out edges that used to point to pb1 (usually only one of these) with
    // pb2
    for (BasicBlock *pPred : bb1->m_predecessors) {
        for (BasicBlock *& pred_out : pPred->m_successors) {
            if (pred_out == bb1) {
                pred_out = bb2;
            }
        }
    }

    // Now we replace pb2's in edges by pb1's inedges
    bb2->m_predecessors = bb1->m_predecessors;

    if (!bDelete) {
        return;
    }

    // Finally, we delete bb1 from the CFG.
    removeBB(bb1);
}


bool Cfg::joinBB(BasicBlock *bb1, BasicBlock *bb2)
{
    // Ensure that the fallthrough case for bb1 is bb2
    const std::vector<BasicBlock *>& v = bb1->getSuccessors();

    if ((v.size() != 2) || (v[BELSE] != bb2)) {
        return false;
    }

    // Prepend the RTLs for pb1 to those of pb2.
    // Since they will be pushed to the front of pb2,
    // push them in reverse order
    for (std::list<RTL *>::reverse_iterator it = bb1->m_listOfRTLs->rbegin();
         it != bb1->m_listOfRTLs->rend(); it++) {
        bb2->m_listOfRTLs->push_front(*it);
    }

    completeMerge(bb1, bb2); // Mash them together

    // pb1 no longer needed. Remove it from the list of BBs.
    // This will also delete *pb1. It will be a shallow delete,
    // but that's good because we only did shallow copies to *pb2
    removeBB(bb1);
    return true;
}


void Cfg::removeBB(BasicBlock *bb)
{
    iterator bbIt = std::find(m_listBB.begin(), m_listBB.end(), bb);

    assert(bbIt != m_listBB.end()); // must not delete BBs of other CFGs

    if ((*bbIt)->getLowAddr() != Address::ZERO) {
        m_mapBB.erase((*bbIt)->getLowAddr());
    }

    m_listBB.erase(bbIt);

    // Actually, removed BBs should be deleted; however,
    // doing so deletes the statements of the BB that seem to be still in use.
    // So don't do it for now.
}


bool Cfg::compressCfg()
{
    // must be well formed
    if (!m_wellFormed) {
        return false;
    }

    // FIXME: The below was working while we still had reaching definitions. It seems to me that it would be easy to
    // search the BB for definitions between the two branches (so we don't need reaching defs, just the SSA property of
    //  unique definition).
    //
    // Look in CVS for old code.

    // Find A -> J -> B where J is a BB that is only a jump and replace it by A -> B
    for (iterator it = m_listBB.begin(); it != m_listBB.end(); it++) {
        BasicBlock *a = *it;

        for (size_t i = 0; i < a->getSuccessors().size(); i++) {
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

            setLabelRequired(b);

            if (jmpBB->getNumPredecessors() == 0) {
                jmpBB->removeAllSuccessors(); // now we can remove the successors
                removeBB(jmpBB);
            }
        }
    }

    return true;
}


bool Cfg::removeOrphanBBs()
{
    std::deque<BasicBlock *> orphans;

    for (iterator it = m_listBB.begin(); it != m_listBB.end(); it++) {
        if (*it == this->m_entryBB) { // don't remove entry BasicBlock
            continue;
        }

        BasicBlock *bb = *it;

        if (bb->m_predecessors.empty()) {
            orphans.push_back(bb);
        }
    }

    const bool bbsRemoved = !orphans.empty();

    while (!orphans.empty()) {
        BasicBlock *b = orphans.front();
        orphans.pop_front();

        for (BasicBlock *child : b->m_successors) {
            child->removePredecessor(b);

            if (child->m_predecessors.empty()) {
                orphans.push_back(child);
            }
        }

        removeBB(b);
    }

    return bbsRemoved;
}


void Cfg::unTraverse()
{
    for (BasicBlock *bb : m_listBB) {
        bb->m_travType       = TravType::Untraversed;
    }
}


BasicBlock *Cfg::findRetNode()
{
    BasicBlock *retNode = nullptr;

    for (BasicBlock *bb : m_listBB) {
        if (bb->getType() == BBType::Ret) {
            return bb;
        }
        else if (bb->getType() == BBType::Call) {
            Function *p = bb->getCallDestProc();

            if (p && !p->getName().compare("exit")) { // TODO: move this into check Proc::noReturn();
                retNode = bb;
            }
        }
    }

    return retNode;
}


bool Cfg::isOrphan(Address uAddr)
{
    BasicBlock *pBB = getBB(uAddr);

    // If it's incomplete, it can't be an orphan
    return pBB && !pBB->m_incomplete &&
           pBB->m_listOfRTLs->front()->getAddress().isZero();
}


void Cfg::searchAndReplace(const Exp& search, const SharedExp& replace)
{
    for (BasicBlock *bb : m_listBB) {
        bb->searchAndReplace(search, replace);
    }
}


bool Cfg::searchAll(const Exp& search, std::list<SharedExp>& result)
{
    bool ch = false;

    for (BasicBlock *bb : m_listBB) {
        ch |= bb->searchAll(search, result);
    }

    return ch;
}


void Cfg::setLabelRequired(BasicBlock *pBB)
{
    pBB->m_labelNeeded = true;
}


void Cfg::simplify()
{
    LOG_VERBOSE("Simplifying CFG ...");

    for (BasicBlock *bb : m_listBB) {
        bb->simplify();
    }
}


void Cfg::print(QTextStream& out, bool html)
{
    out << "Control Flow Graph:\n";

    for (BasicBlock *bb : m_listBB) {
        bb->print(out, html);
    }

    out << '\n';
}


void Cfg::dump()
{
    QTextStream q_cerr(stderr);

    print(q_cerr);
}


void Cfg::dumpImplicitMap()
{
    QTextStream q_cerr(stderr);

    for (auto it : m_implicitMap) {
        q_cerr << it.first << " -> " << it.second << "\n";
    }
}


void Cfg::printToLog()
{
    QString     tgt;
    QTextStream ost(&tgt);

    print(ost);
    LOG_MSG(tgt);
}


void Cfg::setTimeStamps()
{
    // set DFS tag
    for (BasicBlock *it : m_listBB) {
        it->m_travType = TravType::DFS_Tag;
    }

    // set the parenthesis for the nodes as well as setting the post-order ordering between the nodes
    int time = 1;
    m_ordering.clear();
    m_entryBB->setLoopStamps(time, m_ordering);

    // set the reverse parenthesis for the nodes
    time = 1;
    m_entryBB->setRevLoopStamps(time);

    BasicBlock *retNode = findRetNode();
    assert(retNode);
    m_revOrdering.clear();
    retNode->setRevOrder(m_revOrdering);
}


BasicBlock *Cfg::commonPDom(BasicBlock *curImmPDom, BasicBlock *succImmPDom)
{
    if (!curImmPDom) {
        return succImmPDom;
    }

    if (!succImmPDom) {
        return curImmPDom;
    }

    if (curImmPDom->m_revOrd == succImmPDom->m_revOrd) {
        return curImmPDom; // ordering hasn't been done
    }

    BasicBlock *oldCurImmPDom  = curImmPDom;
    BasicBlock *oldSuccImmPDom = succImmPDom;

    int giveup = 0;
#define GIVEUP    10000

    while (giveup < GIVEUP && curImmPDom && succImmPDom && (curImmPDom != succImmPDom)) {
        if (curImmPDom->m_revOrd > succImmPDom->m_revOrd) {
            succImmPDom = succImmPDom->m_immPDom;
        }
        else {
            curImmPDom = curImmPDom->m_immPDom;
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


void Cfg::updateImmedPDom()
{
    // traverse the nodes in order (i.e from the bottom up)
    for (int i = m_revOrdering.size() - 1; i >= 0; i--) {
        BasicBlock *curNode = m_revOrdering[i];
        const std::vector<BasicBlock *>& oEdges = curNode->getSuccessors();

        for (auto& oEdge : oEdges) {
            BasicBlock *succNode = oEdge;

            if (succNode->m_revOrd > curNode->m_revOrd) {
                curNode->m_immPDom = commonPDom(curNode->m_immPDom, succNode);
            }
        }
    }

    // make a second pass but consider the original CFG ordering this time
    for (BasicBlock *curNode : m_ordering) {
        const std::vector<BasicBlock *>& oEdges = curNode->getSuccessors();

        if (oEdges.size() <= 1) {
            continue;
        }

        for (auto& oEdge : oEdges) {
            BasicBlock *succNode = oEdge;
            curNode->m_immPDom = commonPDom(curNode->m_immPDom, succNode);
        }
    }

    // one final pass to fix up nodes involved in a loop
    for (BasicBlock *curNode : m_ordering) {
        const std::vector<BasicBlock *>& oEdges = curNode->getSuccessors();

        if (oEdges.size() > 1) {
            for (auto& oEdge : oEdges) {
                BasicBlock *succNode = oEdge;

                if (curNode->hasBackEdgeTo(succNode) && (curNode->getSuccessors().size() > 1) && succNode->m_immPDom &&
                    (succNode->m_immPDom->m_ord < curNode->m_immPDom->m_ord)) {
                    curNode->m_immPDom = commonPDom(succNode->m_immPDom, curNode->m_immPDom);
                }
                else {
                    curNode->m_immPDom = commonPDom(curNode->m_immPDom, succNode);
                }
            }
        }
    }
}


void Cfg::structConds()
{
    // Process the nodes in order
    for (BasicBlock *curNode : m_ordering) {
        // does the current node have more than one out edge?
        if (curNode->getSuccessors().size() > 1) {
            // if the current conditional header is a two way node and has a back edge, then it won't have a follow
            if (curNode->hasBackEdge() && (curNode->getType() == BBType::Twoway)) {
                curNode->setStructType(StructType::Cond);
                continue;
            }

            // set the follow of a node to be its immediate post dominator
            curNode->setCondFollow(curNode->m_immPDom);

            // set the structured type of this node
            curNode->setStructType(StructType::Cond);

            // if this is an nway header, then we have to tag each of the nodes within the body of the nway subgraph
            if (curNode->getCondType() == CondType::Case) {
                curNode->setCaseHead(curNode, curNode->getCondFollow());
            }
        }
    }
}


void Cfg::determineLoopType(BasicBlock *header, bool *& loopNodes)
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
        if (header->getCondFollow() && loopNodes[header->getCondFollow()->m_ord]) {
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


void Cfg::findLoopFollow(BasicBlock *header, bool *& loopNodes)
{
    assert(header->getStructType() == StructType::Loop || header->getStructType() == StructType::LoopCond);
    LoopType   lType  = header->getLoopType();
    BasicBlock *latch = header->getLatchNode();

    if (lType == LoopType::PreTested) {
        // if the 'while' loop's true child is within the loop, then its false child is the loop follow
        if (loopNodes[header->getSuccessors()[0]->m_ord]) {
            header->setLoopFollow(header->getSuccessors()[1]);
        }
        else {
            header->setLoopFollow(header->getSuccessors()[0]);
        }
    }
    else if (lType == LoopType::PostTested) {
        // the follow of a post tested ('repeat') loop is the node on the end of the non-back edge from the latch node
        if (latch->getSuccessors()[0] == header) {
            header->setLoopFollow(latch->getSuccessors()[1]);
        }
        else {
            header->setLoopFollow(latch->getSuccessors()[0]);
        }
    }
    else {
        // endless loop
        BasicBlock *follow = nullptr;

        // traverse the ordering array between the header and latch nodes.
        // BasicBlock * latch = header->getLatchNode(); initialized at function start
        for (int i = header->m_ord - 1; i > latch->m_ord; i--) {
            BasicBlock *& desc = m_ordering[i];
            // the follow for an endless loop will have the following
            // properties:
            //   i) it will have a parent that is a conditional header inside the loop whose follow is outside the
            //        loop
            //  ii) it will be outside the loop according to its loop stamp pair
            // iii) have the highest ordering of all suitable follows (i.e. highest in the graph)

            if ((desc->getStructType() == StructType::Cond) && desc->getCondFollow() && (desc->getLoopHead() == header)) {
                if (loopNodes[desc->getCondFollow()->m_ord]) {
                    // if the conditional's follow is in the same loop AND is lower in the loop, jump to this follow
                    if (desc->m_ord > desc->getCondFollow()->m_ord) {
                        i = desc->getCondFollow()->m_ord;
                    }
                    // otherwise there is a backward jump somewhere to a node earlier in this loop. We don't need to any
                    //  nodes below this one as they will all have a conditional within the loop.
                    else {
                        break;
                    }
                }
                else {
                    // otherwise find the child (if any) of the conditional header that isn't inside the same loop
                    BasicBlock *succ = desc->getSuccessors()[0];

                    if (loopNodes[succ->m_ord]) {
                        if (!loopNodes[desc->getSuccessors()[1]->m_ord]) {
                            succ = desc->getSuccessors()[1];
                        }
                        else {
                            succ = nullptr;
                        }
                    }

                    // if a potential follow was found, compare its ordering with the currently found follow
                    if (succ && (!follow || (succ->m_ord > follow->m_ord))) {
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


void Cfg::tagNodesInLoop(BasicBlock *header, bool *& loopNodes)
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

    for (int i = header->m_ord - 1; i >= latch->m_ord; i--) {
        if (m_ordering[i]->inLoop(header, latch)) {
            // update the membership map to reflect that this node is within the loop
            loopNodes[i] = true;

            m_ordering[i]->setLoopHead(header);
        }
    }
}


void Cfg::structLoops()
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

        std::vector<BasicBlock *>& iEdges = curNode->getPredecessors();

        for (auto& iEdge : iEdges) {
            BasicBlock *pred = iEdge;

            if ((pred->getCaseHead() == curNode->getCaseHead()) &&                         // ii)
                (pred->getLoopHead() == curNode->getLoopHead()) &&                         // iii)
                (!latch || (latch->m_ord > pred->m_ord)) &&                                // vi)
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

            // delete the space taken by the loopnodes map
            // delete[] loopNodes;
        }
    }
}


void Cfg::checkConds()
{
    for (auto& elem : m_ordering) {
        BasicBlock *curNode = elem;
        const std::vector<BasicBlock *>& oEdges = curNode->getSuccessors();

        // consider only conditional headers that have a follow and aren't case headers
        if (((curNode->getStructType() == StructType::Cond) || (curNode->getStructType() == StructType::LoopCond)) && curNode->getCondFollow() &&
            (curNode->getCondType() != CondType::Case)) {
            // define convenient aliases for the relevant loop and case heads and the out edges
            BasicBlock *myLoopHead   = (curNode->getStructType() == StructType::LoopCond ? curNode : curNode->getLoopHead());
            BasicBlock *follLoopHead = curNode->getCondFollow()->getLoopHead();

            // analyse whether this is a jump into/outof a loop
            if (myLoopHead != follLoopHead) {
                // we want to find the branch that the latch node is on for a jump out of a loop
                if (myLoopHead) {
                    BasicBlock *myLoopLatch = myLoopHead->getLatchNode();

                    // does the then branch goto the loop latch?
                    if (oEdges[BTHEN]->isAncestorOf(myLoopLatch) || (oEdges[BTHEN] == myLoopLatch)) {
                        curNode->setUnstructType(UnstructType::JumpInOutLoop);
                        curNode->setCondType(CondType::IfElse);
                    }
                    // does the else branch goto the loop latch?
                    else if (oEdges[BELSE]->isAncestorOf(myLoopLatch) || (oEdges[BELSE] == myLoopLatch)) {
                        curNode->setUnstructType(UnstructType::JumpInOutLoop);
                        curNode->setCondType(CondType::IfThen);
                    }
                }

                if ((curNode->getUnstructType() == UnstructType::Structured) && follLoopHead) {
                    // find the branch that the loop head is on for a jump into a loop body. If a branch has already
                    // been found, then it will match this one anyway

                    // does the else branch goto the loop head?
                    if (oEdges[BTHEN]->isAncestorOf(follLoopHead) || (oEdges[BTHEN] == follLoopHead)) {
                        curNode->setUnstructType(UnstructType::JumpInOutLoop);
                        curNode->setCondType(CondType::IfElse);
                    }

                    // does the else branch goto the loop head?
                    else if (oEdges[BELSE]->isAncestorOf(follLoopHead) || (oEdges[BELSE] == follLoopHead)) {
                        curNode->setUnstructType(UnstructType::JumpInOutLoop);
                        curNode->setCondType(CondType::IfThen);
                    }
                }
            }

            // this is a jump into a case body if either of its children don't have the same same case header as itself
            if ((curNode->getUnstructType() == UnstructType::Structured) &&
                ((curNode->getCaseHead() != curNode->getSuccessors()[BTHEN]->getCaseHead()) ||
                 (curNode->getCaseHead() != curNode->getSuccessors()[BELSE]->getCaseHead()))) {
                BasicBlock *myCaseHead   = curNode->getCaseHead();
                BasicBlock *thenCaseHead = curNode->getSuccessors()[BTHEN]->getCaseHead();
                BasicBlock *elseCaseHead = curNode->getSuccessors()[BELSE]->getCaseHead();

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
                if (curNode->hasBackEdgeTo(curNode->getSuccessors()[BTHEN])) {
                    curNode->setCondType(CondType::IfThen);
                    curNode->setCondFollow(curNode->getSuccessors()[BELSE]);
                }
                else {
                    curNode->setCondType(CondType::IfElse);
                    curNode->setCondFollow(curNode->getSuccessors()[BTHEN]);
                }
            }
        }
    }
}


void Cfg::structure()
{
    if (m_structured) {
        unTraverse();
        return;
    }

    if (findRetNode() == nullptr) {
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


void Cfg::removeUnneededLabels(ICodeGenerator *gen)
{
    gen->removeUnusedLabels(m_ordering.size());
}


void Cfg::generateDotFile(QTextStream& of)
{
    Address returnAddress = Address::INVALID;

    // The nodes
    for (BasicBlock *bb : m_listBB) {
        of << "       "
           << "bb" << bb->getLowAddr() << " ["
           << "label=\"" << bb->getLowAddr() << " ";

        switch (bb->getType())
        {
        case BBType::Oneway:
            of << "oneway";
            break;

        case BBType::Twoway:

            if (bb->getCond()) {
                of << "\\n";
                bb->getCond()->print(of);
                of << "\" shape=diamond];\n";
                continue;
            }
            else {
                of << "twoway";
            }

            break;

        case BBType::Nway:
            {
                of << "nway";
                SharedExp de = bb->getDest();

                if (de) {
                    of << "\\n";
                    of << de;
                }

                of << "\" shape=trapezium];\n";
                continue;
            }

        case BBType::Call:
            {
                of << "call";
                Function *dest = bb->getDestProc();

                if (dest) {
                    of << "\\n" << dest->getName();
                }

                break;
            }

        case BBType::Ret:
            of << "ret\" shape=triangle];\n";
            // Remember the (unique) return BB's address
            returnAddress = bb->getLowAddr();
            continue;

        case BBType::Fall:
            of << "fall";
            break;

        case BBType::CompJump:
            of << "compjump";
            break;

        case BBType::CompCall:
            of << "compcall";
            break;

        case BBType::Invalid:
            of << "invalid";
            break;
        }

        of << "\"];\n";
    }

    // Force the one return node to be at the bottom (max rank). Otherwise, with all its in-edges, it will end up in the
    // middle
    if (!returnAddress.isZero()) {
        of << "{rank=max; bb" << returnAddress << "}\n";
    }

    // Close the subgraph
    of << "}\n";

    // Now the edges
    for (BasicBlock *srcBB : m_listBB) {
        const std::vector<BasicBlock *>& outEdges = srcBB->getSuccessors();

        for (unsigned int j = 0; j < outEdges.size(); j++) {
            BasicBlock *dstBB = outEdges[j];
            of << "       bb" << srcBB->getLowAddr() << " -> ";
            of << "bb" << dstBB->getLowAddr();

            if (srcBB->getType() == BBType::Twoway) {
                if (j == 0) {
                    of << " [color=\"green\"]"; // cond == true
                }
                else {
                    of << " [color=\"red\"]"; // cond == false
                }
            }
            else {
                of << " [color=\"black\"];\n"; // normal connection
            }
        }
    }

#if PRINT_BACK_EDGES
    for (it = m_listBB.begin(); it != m_listBB.end(); it++) {
        std::vector<PBB>& inEdges = (*it)->getInEdges();

        for (unsigned int j = 0; j < inEdges.size(); j++) {
            of << "       "
               << "bb" << std::hex << (*it)->getLowAddr() << " -> ";
            of << "bb" << std::hex << inEdges[j]->getLowAddr();
            of << " [color = \"green\"];\n";
        }
    }
#endif
}


void updateWorkListRev(BasicBlock *currBB, std::list<BasicBlock *>& workList, std::set<BasicBlock *>& workSet)
{
    // Insert inedges of currBB into the worklist, unless already there
    for (BasicBlock *currIn : currBB->getPredecessors()) {
        if (workSet.find(currIn) == workSet.end()) {
            workList.push_front(currIn);
            workSet.insert(currIn);
        }
    }
}


void Cfg::findInterferences(ConnectionGraph& cg)
{
    if (m_listBB.empty()) {
        return;
    }

    std::list<BasicBlock *> workList; // List of BBs still to be processed
    // Set of the same; used for quick membership test
    std::set<BasicBlock *> workSet;
    appendBBs(workList, workSet);

    int count = 0;

    while (!workList.empty() && count < 100000) {
        count++; // prevent infinite loop

        BasicBlock *currBB = workList.back();
        workList.erase(--workList.end());
        workSet.erase(currBB);
        // Calculate live locations and interferences
        bool change = currBB->calcLiveness(cg, m_myProc);

        if (!change) {
            continue;
        }

        if (DEBUG_LIVENESS) {
            Statement *last = nullptr;

            if (!currBB->m_listOfRTLs->empty()) {
                RTL *lastRtl = currBB->m_listOfRTLs->back();

                if (lastRtl->size()) {
                    last = lastRtl->back();
                }
            }

            LOG_MSG("Revisiting BB ending with stmt %1 due to change",
                    last ? QString::number(last->getNumber(), 10) : "<none>");
        }

        updateWorkListRev(currBB, workList, workSet);
    }
}


void Cfg::appendBBs(std::list<BasicBlock *>& worklist, std::set<BasicBlock *>& workset)
{
    // Append my list of BBs to the worklist
    worklist.insert(worklist.end(), m_listBB.begin(), m_listBB.end());
    // Do the same for the workset
    std::copy(m_listBB.begin(), m_listBB.end(), std::inserter(workset, workset.end()));
}


void dumpBB(BasicBlock *bb)
{
    LOG_MSG("For BB at %1:", HostAddress(bb).toString());
    LOG_MSG("  In edges:");

    std::vector<BasicBlock *> ins = bb->getPredecessors();
    std::vector<BasicBlock *> outs = bb->getSuccessors();
    size_t i, n = ins.size();

    for (i = 0; i < n; i++) {
        LOG_MSG("    %1", HostAddress(ins[i]).toString());
    }

    LOG_MSG("  Out edges:");
    n = outs.size();

    for (i = 0; i < n; i++) {
        LOG_MSG("    %1", HostAddress(outs[i]).toString());
    }
}


BasicBlock *Cfg::splitForBranch(BasicBlock *bb, RTL *rtl, BranchStatement *br1, BranchStatement *br2, iterator& it)
{
    std::list<RTL *>::iterator ri;

    // First find which RTL has the split address
    for (ri = bb->m_listOfRTLs->begin(); ri != bb->m_listOfRTLs->end(); ri++) {
        if ((*ri) == rtl) {
            break;
        }
    }

    assert(ri != bb->m_listOfRTLs->end());

    bool haveA = (ri != bb->m_listOfRTLs->begin());

    Address addr = rtl->getAddress();

    // Make a BB for the br1 instruction

    // Don't give this "instruction" the same address as the rest of the string instruction (causes problems when
    // creating the rptBB). Or if there is no A, temporarily use 0
    Address    a        = (haveA) ? addr : Address::ZERO;
    RTL        *skipRtl = new RTL(a, new std::list<Statement *> { br1 }); // list initializer in braces
    std::unique_ptr<RTLList> bbRTL(new RTLList({skipRtl}));
    BasicBlock *skipBB  = createBB(std::move(bbRTL), BBType::Twoway);
    rtl->setAddress(addr + 1);

    if (!haveA) {
        skipRtl->setAddress(addr);
        // Address addr now refers to the splitBB
        m_mapBB[addr] = skipBB;

        // Fix all predecessors of pBB to point to splitBB instead
        for (size_t i = 0; i < bb->m_predecessors.size(); i++) {
            BasicBlock *pred = bb->m_predecessors[i];

            for (size_t j = 0; j < pred->m_successors.size(); j++) {
                BasicBlock *succ = pred->m_successors[j];

                if (succ == bb) {
                    pred->m_successors[j] = skipBB;
                    skipBB->addPredecessor(pred);
                    break;
                }
            }
        }
    }

    // Remove the SKIP from the start of the string instruction RTL
    assert(rtl->size() >= 4);
    rtl->pop_front();
    // Replace the last statement with br2
    rtl->back() = br2;

    // Move the remainder of the string RTL into a new BB
    bbRTL.reset(new RTLList({ *ri }));
    BasicBlock *rptBB = createBB(std::move(bbRTL), BBType::Twoway);
    ri = bb->m_listOfRTLs->erase(ri);

    // Move the remaining RTLs (if any) to a new list of RTLs
    BasicBlock *newBB;
    size_t     oldOutEdges = 0;
    bool       haveB       = true;

    if (ri != bb->m_listOfRTLs->end()) {
        std::unique_ptr<RTLList> pRtls(new RTLList);

        while (ri != bb->m_listOfRTLs->end()) {
            pRtls->push_back(*ri);
            ri = bb->m_listOfRTLs->erase(ri);
        }

        oldOutEdges = bb->getNumSuccessors();
        newBB       = this->createBB(std::move(pRtls), bb->getType());

        // Transfer the out edges from A to B (pBB to newBb)
        for (size_t i = 0; i < oldOutEdges; i++) {
            // Don't use addOutEdge, since it will also add in-edges back to the BB
            newBB->m_successors.push_back(bb->getSuccessor(i));
        }

        // addOutEdge(newBb, pBB->getOutEdge(i));
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
    bb->m_successors.erase(bb->m_successors.begin(), bb->m_successors.end());
    addOutEdge(bb, skipBB);
    // Set the out edges for skipBB. First is the taken (true) leg.
    addOutEdge(skipBB, newBB);
    addOutEdge(skipBB, rptBB);
    // Set the out edges for the rptBB
    addOutEdge(rptBB, skipBB);
    addOutEdge(rptBB, newBB);

    // For each out edge of newBb, change any in-edges from pBB to instead come from newBb
    if (haveB) {
        for (size_t i = 0; i < oldOutEdges; i++) {
            BasicBlock *succ = newBB->m_successors[i];

            for (auto& elem : succ->m_predecessors) {
                BasicBlock *pred = elem;

                if (pred == bb) {
                    elem = newBB;
                    break;
                }
            }
        }
    }
    else {
        // There is no "B" bb (newBb is just the successor of pBB) Fix that one out-edge to point to rptBB
        for (auto& elem : newBB->m_predecessors) {
            BasicBlock *pred = elem;

            if (pred == bb) {
                elem = rptBB;
                break;
            }
        }
    }

    if (!haveA) {
        // There is no A any more. All A's in-edges have been copied to the skipBB. It is possible that the original BB
        // had a self edge (branch to start of self). If so, this edge, now in to skipBB, must now come from newBb (if
        // there is a B) or rptBB if none.  Both of these will already exist, so delete it.
        for (size_t j = 0; j < skipBB->m_predecessors.size(); j++) {
            BasicBlock *pred = skipBB->m_predecessors[j];

            if (pred == bb) {
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

        // Must delete bb. Note that this effectively "increments" iterator it
        it = m_listBB.erase(it);
        bb = nullptr;
    }
    else {
        it++;
    }

    return newBB;
}


bool Cfg::decodeIndirectJmp(UserProc *proc)
{
    bool res = false;

    for (BasicBlock *bb : m_listBB) {
        res |= bb->decodeIndirectJmp(proc);
    }

    return res;
}


void Cfg::undoComputedBB(Statement *stmt)
{
    for (BasicBlock *bb : m_listBB) {
        if (bb->undoComputedBB(stmt)) {
            break;
        }
    }
}


Statement *Cfg::findImplicitAssign(SharedExp x)
{
    Statement *def;

    std::map<SharedExp, Statement *, lessExpStar>::iterator it = m_implicitMap.find(x);

    if (it == m_implicitMap.end()) {
        // A use with no explicit definition. Create a new implicit assignment
        x   = x->clone(); // In case the original gets changed
        def = new ImplicitAssign(x);
        m_entryBB->prependStmt(def, m_myProc);

        // Remember it for later so we don't insert more than one implicit assignment for any one location
        // We don't clone the copy in the map. So if the location is a m[...], the same type information is available in
        // the definition as at all uses
        m_implicitMap[x] = def;
    }
    else {
        // Use an existing implicit assignment
        def = it->second;
    }

    assert(def);
    return def;
}


Statement *Cfg::findTheImplicitAssign(const SharedExp& x)
{
    // As per the above, but don't create an implicit if it doesn't already exist
    auto it = m_implicitMap.find(x);

    if (it == m_implicitMap.end()) {
        return nullptr;
    }

    return it->second;
}


Statement *Cfg::findImplicitParamAssign(Parameter *param)
{
    // As per the above, but for parameters (signatures don't get updated with opParams)
    SharedExp n = param->getExp();

    // TODO: implicitMap contains subscripted values -> m[r28{0}+4]
    // but the Parameter expresions are not subscripted, so, they are not found
    // with a simple:
    // auto it = implicitMap.find(n);
    ExpStatementMap::iterator it;

    // search the map by hand, and compare without subscripts.
    for (it = m_implicitMap.begin(); it != m_implicitMap.end(); ++it) {
        if ((*(it->first)) *= *n) {
            break;
        }
    }

    if (it == m_implicitMap.end()) {
        SharedExp eParam = Location::param(param->getName());
        it = m_implicitMap.find(eParam);
    }

    if (it == m_implicitMap.end()) {
        return nullptr;
    }

    return it->second;
}


void Cfg::removeImplicitAssign(SharedExp x)
{
    auto it = m_implicitMap.find(x);

    assert(it != m_implicitMap.end());
    Statement *ia = it->second;
    m_implicitMap.erase(it);          // Delete the mapping
    m_myProc->removeStatement(ia);    // Remove the actual implicit assignment statement as well
}
