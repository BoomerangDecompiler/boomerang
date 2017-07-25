/*
 * Copyright (C) 1997-2000, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file    cfg.cpp
 * \brief   Implementation of the CFG class.
 ******************************************************************************/

#include "boomerang/db/CFG.h"


#include "boomerang/db/Signature.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/Proc.h" // For Proc::setTailCaller()
#include "boomerang/db/Prog.h" // For findProc()
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/exp/Location.h"

#include "boomerang/util/Types.h"
#include "boomerang/util/Log.h"
#include "boomerang/codegen/ICodeGenerator.h"

#include "boomerang/util/Util.h"

#include <QtCore/QDebug>
#include <cassert>
#include <algorithm> // For find()
#include <cstring>


void delete_lrtls(std::list<RTL *>& pLrtl);
void erase_lrtls(std::list<RTL *>& pLrtl, std::list<RTL *>::iterator begin, std::list<RTL *>::iterator end);

static int progress = 0;


/**********************************
* Cfg methods.
**********************************/

Cfg::Cfg()
    : m_wellFormed(false)
    , m_structured(false)
    , m_implicitsDone(false)
    , m_lastLabel(0)
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


void Cfg::setProc(UserProc *proc)
{
    m_myProc = proc;
}


void Cfg::clear()
{
    // Don't delete the BBs; this will delete any CaseStatements we want to save for the re-decode. Just let the garbage
    // collection take care of it.
    // for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end(); it++)
    //    delete *it;
    m_listBB.clear();
    m_mapBB.clear();
    m_implicitMap.clear();
    m_entryBB    = nullptr;
    m_exitBB     = nullptr;
    m_wellFormed = false;
    m_callSites.clear();
    m_lastLabel = 0;
}


Cfg& Cfg::operator=(const Cfg& other)
{
    m_listBB     = other.m_listBB;
    m_mapBB      = other.m_mapBB;
    m_wellFormed = other.m_wellFormed;
    return *this;
}


void Cfg::setEntryBB(BasicBlock *bb)
{
    m_entryBB = bb;

    for (BasicBlock *it : m_listBB) {
        if (it->getType() == BBType::Ret) {
            m_exitBB = it;
            return;
        }
    }

    // It is possible that there is no exit BB
}


void Cfg::setExitBB(BasicBlock *bb)
{
    m_exitBB = bb;
}


bool Cfg::checkEntryBB()
{
    if (m_entryBB != nullptr) {
        return false;
    }

    if (m_myProc) {
        qWarning() << "No entry BB for " << m_myProc->getName();
    }
    else {
        qWarning() << "No entry BB for "
                   << "unknown proc";
    }

    return true;
}


BasicBlock *Cfg::newBB(std::list<RTL *> *pRtls, BBType bbType, uint32_t iNumOutEdges)
{
    MAPBB::iterator mi;
    BasicBlock      *pBB = nullptr;

    // First find the native address of the first RTL
    // Can't use BasicBlock::GetLowAddr(), since we don't yet have a BB!
       Address addr = pRtls->front()->getAddress();

    // If this is zero, try the next RTL (only). This may be necessary if e.g. there is a BB with a delayed branch only,
    // with its delay instruction moved in front of it (with 0 address).
    // Note: it is possible to see two RTLs with zero address with Sparc: jmpl %o0, %o1. There will be one for the delay
    // instr (if not a NOP), and one for the side effect of copying %o7 to %o1.
    // Note that orphaned BBs (for which we must compute addr here to to be 0) must not be added to the map, but they
    // have no RTLs with a non zero address.
    if (addr.isZero() && (pRtls->size() > 1)) {
        std::list<RTL *>::iterator next = pRtls->begin();
        addr = (*++next)->getAddress();
    }

    // If this addr is non zero, check the map to see if we have a (possibly incomplete) BB here already
    // If it is zero, this is a special BB for handling delayed branches or the like
    bool bDone = false;

    if (!addr.isZero()) {
        mi = m_mapBB.find(addr);

        if ((mi != m_mapBB.end()) && (*mi).second) {
            pBB = (*mi).second;

            // It should be incomplete, or the pBB there should be zero (we have called Label but not yet created the BB
            // for it).  Else we have duplicated BBs. Note: this can happen with forward jumps into the middle of a
            // loop, so not error
            if (!pBB->m_incomplete) {
                // This list of RTLs is not needed now
                delete_lrtls(*pRtls);

                if (VERBOSE) {
                    LOG << "throwing BBAlreadyExistsError\n";
                }

                throw BBAlreadyExistsError(pBB);
            }
            else {
                // Fill in the details, and return it
                pBB->setRTLs(pRtls);
                pBB->m_nodeType       = bbType;
                pBB->m_targetOutEdges = iNumOutEdges;
                pBB->m_incomplete     = false;
            }

            bDone = true;
        }
    }

    if (!bDone) {
        // Else add a new BB to the back of the current list.
        pBB = new BasicBlock(m_myProc, pRtls, bbType, iNumOutEdges);
        m_listBB.push_back(pBB);

        // Also add the address to the map from native (source) address to
        // pointer to BB, unless it's zero
        if (!addr.isZero()) {
            m_mapBB[addr] = pBB; // Insert the mapping
            mi            = m_mapBB.find(addr);
        }
    }

    if (!addr.isZero() && (mi != m_mapBB.end())) {
        // Existing New            +---+ Top of new
        //            +---+        +---+
        //    +---+   |   |        +---+ Fall through
        //    |   |   |   | =>     |   |
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
        if (++mi != m_mapBB.end()) {
            BasicBlock *pNextBB    = (*mi).second;
                     Address    uNext       = (*mi).first;
            bool       bIncomplete = pNextBB->m_incomplete;

            if (uNext <= pRtls->back()->getAddress()) {
                // Need to truncate the current BB. We use splitBB(), but pass it pNextBB so it doesn't create a new BB
                // for the "bottom" BB of the split pair
                splitBB(pBB, uNext, pNextBB);

                // If the overlapped BB was incomplete, return the "bottom" part of the BB, so adding out edges will
                // work properly.
                if (bIncomplete) {
                    return pNextBB;
                }

                // However, if the overlapping BB was already complete, return 0, so out edges won't be added twice
                throw BBAlreadyExistsError(pNextBB);
            }
        }

        // Existing New            +---+ Top of existing
        //    +---+                +---+
        //    |   |    +---+       +---+ Fall through
        //    |   |    |   | =>    |   |
        //    |   |    |   |       |   | New; rest of existing discarded
        //    +---+    +---+       +---+
        // Note: no need to check the other way around, because in this case, we will have called Cfg::Label(), and it
        // will have split the existing BB already.
    }

    assert(pBB);
    return pBB;
}


BasicBlock *Cfg::newIncompleteBB(Address addr)
{
    // Create a new (basically empty) BB
    BasicBlock *pBB = new BasicBlock(m_myProc);

    // Add it to the list
    m_listBB.push_back(pBB);
    m_mapBB[addr] = pBB; // Insert the mapping
    return pBB;
}


void Cfg::addOutEdge(BasicBlock *pBB, BasicBlock *pDestBB, bool bSetLabel /* = false */)
{
    // Add the given BB pointer to the list of out edges
    pBB->m_outEdges.push_back(pDestBB);
    // Add the in edge to the destination BB
    pDestBB->m_inEdges.push_back(pBB);

    if (bSetLabel) {
        setLabel(pDestBB); // Indicate "label required"
    }
}


void Cfg::addOutEdge(BasicBlock *pBB, Address addr, bool bSetLabel /* = false */)
{
    // Check to see if the address is in the map, i.e. we already have a BB for this address
    MAPBB::iterator it = m_mapBB.find(addr);
    BasicBlock      *pDestBB;

    if ((it != m_mapBB.end()) && (*it).second) {
        // Just add this PBB to the list of out edges
        pDestBB = (*it).second;
    }
    else {
        // Else, create a new incomplete BB, add that to the map, and add the new BB as the out edge
        pDestBB = newIncompleteBB(addr);
    }

    addOutEdge(pBB, pDestBB, bSetLabel);
}


bool Cfg::existsBB(Address uNativeAddr) const
{
    auto mi = m_mapBB.find(uNativeAddr);

    return(mi != m_mapBB.end() && (*mi).second);
}


BasicBlock *Cfg::splitBB(BasicBlock *pBB, Address uNativeAddr, BasicBlock *pNewBB /* = 0 */,
                         bool bDelRtls /* = false */)
{
    std::list<RTL *>::iterator ri;

    // First find which RTL has the split address; note that this could fail (e.g. label in the middle of an
    // instruction, or some weird delay slot effects)
    for (ri = pBB->m_listOfRTLs->begin(); ri != pBB->m_listOfRTLs->end(); ri++) {
        if ((*ri)->getAddress() == uNativeAddr) {
            break;
        }
    }

    if (ri == pBB->m_listOfRTLs->end()) {
        LOG_STREAM() << "could not split BB at " << pBB->getLowAddr() << " at split address " << uNativeAddr;
        return pBB;
    }

    // If necessary, set up a new basic block with information from the original bb
    if (pNewBB == nullptr) {
        pNewBB = new BasicBlock(*pBB);
        // But we don't want the top BB's in edges; our only in-edge should be the out edge from the top BB
        pNewBB->m_inEdges.clear();
        // The "bottom" BB now starts at the implicit label, so we create a new list
        // that starts at ri. We need a new list, since it is different from the
        // original BB's list. We don't have to "deep copy" the RTLs themselves,
        // since they will never overlap
        pNewBB->setRTLs(new std::list<RTL *>(ri, pBB->m_listOfRTLs->end()));
        m_listBB.push_back(pNewBB); // Put it in the graph
        // Put the implicit label into the map. Need to do this before the addOutEdge() below
        m_mapBB[uNativeAddr] = pNewBB;
        // There must be a label here; else would not be splitting. Give it a new label
        pNewBB->m_labelNum = ++m_lastLabel;
    }
    else if (pNewBB->m_incomplete) {
        // We have an existing BB and a map entry, but no details except for
        // in-edges and m_bHasLabel.
        // First save the in-edges and m_iLabelNum
        std::vector<BasicBlock *> ins(pNewBB->m_inEdges);
        int label = pNewBB->m_labelNum;
        // Copy over the details now, completing the bottom BB
        *pNewBB = *pBB;             // Assign the BB, copying fields. This will set m_bIncomplete false
                                    // Replace the in edges (likely only one)
        pNewBB->m_inEdges  = ins;
        pNewBB->m_labelNum = label; // Replace the label (must be one, since we are splitting this BB!)
                                    // The "bottom" BB now starts at the implicit label
                                    // We need to create a new list of RTLs, as per above
        pNewBB->setRTLs(new std::list<RTL *>(ri, pBB->m_listOfRTLs->end()));
    }

    // else pNewBB exists and is complete. We don't want to change the complete
    // BB in any way, except to later add one in-edge
    pBB->m_nodeType = BBType::Fall; // Update original ("top") basic block's info and make it a fall-through

    // Fix the in-edges of pBB's descendants. They are now pNewBB
    // Note: you can't believe m_iNumOutEdges at the time that this function may
    // get called
    for (BasicBlock *pDescendant : pBB->m_outEdges) {
        // Search through the in edges for pBB (old ancestor)
        unsigned k;

        for (k = 0; k < pDescendant->m_inEdges.size(); k++) {
            if (pDescendant->m_inEdges[k] == pBB) {
                // Replace with a pointer to the new ancestor
                pDescendant->m_inEdges[k] = pNewBB;
                break;
            }
        }

        // That pointer should have been found!
        assert(k < pDescendant->m_inEdges.size());
    }

    // The old BB needs to have part of its list of RTLs erased, since the
    // instructions overlap
    if (bDelRtls) {
        // Delete the list of pointers, and also the RTLs they point to
        erase_lrtls(*pBB->m_listOfRTLs, ri, pBB->m_listOfRTLs->end());
    }
    else {
        // Delete the list of pointers, but not the RTLs they point to
        pBB->m_listOfRTLs->erase(ri, pBB->m_listOfRTLs->end());
    }

    // Erase any existing out edges
    pBB->m_outEdges.erase(pBB->m_outEdges.begin(), pBB->m_outEdges.end());
    addOutEdge(pBB, uNativeAddr);
    pBB->m_targetOutEdges = 1;
    return pNewBB;
}


BasicBlock *Cfg::getFirstBB(BB_IT& it)
{
    it = m_listBB.begin();

    if (it == m_listBB.end()) {
        return nullptr;
    }

    return *it;
}


const BasicBlock *Cfg::getFirstBB(BBC_IT& it) const
{
    it = m_listBB.begin();

    if (it == m_listBB.end()) {
        return nullptr;
    }

    return *it;
}


BasicBlock *Cfg::getNextBB(BB_IT& it)
{
    if (++it == m_listBB.end()) {
        return nullptr;
    }

    return *it;
}


const BasicBlock *Cfg::getNextBB(BBC_IT& it) const
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
    auto mi = m_mapBB.find(uAddr);

    if (mi == m_mapBB.end()) {
        return false; // No entry at all
    }

    // Else, there is a BB there. If it's incomplete, return true
    BasicBlock *pBB = (*mi).second;
    return pBB->m_incomplete;
}


void Cfg::sortByAddress()
{
    m_listBB.sort(BasicBlock::lessAddress);
}


void Cfg::sortByFirstDFT()
{
    m_listBB.sort(BasicBlock::lessFirstDFT);
}


void Cfg::sortByLastDFT()
{
    m_listBB.sort(BasicBlock::lessLastDFT);
}


bool Cfg::wellFormCfg() const
{
    QTextStream q_cerr(stderr);

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
                q_cerr << "WellFormCfg: incomplete BB not even in map!\n";
            }
            else {
                q_cerr << "WellFormCfg: BB with native address " << (*itm).first << " is incomplete\n";
            }
        }
        else {
            // Complete. Test the out edges
            // assert(current->m_OutEdges.size() == current->m_iTargetOutEdges);
            for (size_t i = 0; i < current->m_outEdges.size(); i++) {
                // check if address is interprocedural
                //                if ((*it)->m_OutEdgeInterProc[i] == false)
                {
                    // i iterates through the outedges in the BB *it
                    BasicBlock *pBB = current->m_outEdges[i];

                    // Check that the out edge has been written (i.e. nonzero)
                    if (pBB == nullptr) {
                        m_wellFormed = false;                   // At least one problem
                                          Address addr = current->getLowAddr();
                        q_cerr << "WellFormCfg: BB with native address " << addr << " is missing outedge " << i << '\n';
                    }
                    else {
                        // Check that there is a corresponding in edge from the child to here
                        auto ii = std::find(pBB->m_inEdges.begin(), pBB->m_inEdges.end(), elem);

                        if (ii == pBB->m_inEdges.end()) {
                            q_cerr << "WellFormCfg: No in edge to BB at " << (elem)->getLowAddr()
                                   << " from successor BB at " << pBB->getLowAddr() << '\n';
                            m_wellFormed = false;                      // At least one problem
                        }
                    }
                }
            }

            // Also check that each in edge has a corresponding out edge to here (could have an extra in-edge, for
            // example)
            std::vector<BasicBlock *>::iterator ii;

            for (BasicBlock *elem_inedge : elem->m_inEdges) {
                auto oo = std::find(elem_inedge->m_outEdges.begin(), elem_inedge->m_outEdges.end(), elem);

                if (oo == elem_inedge->m_outEdges.end()) {
                    q_cerr << "WellFormCfg: No out edge to BB at " << elem->getLowAddr()
                           << " from predecessor BB at " << elem_inedge->getLowAddr() << '\n';
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

    if (pb1->m_outEdges.size() != 1) {
        return false;
    }

    if (pb2->m_inEdges.size() != 1) {
        return false;
    }

    if (pb1->m_outEdges[0] != pb2) {
        return false;
    }

    if (pb2->m_inEdges[0] != pb1) {
        return false;
    }

    // Merge them! We remove pb1 rather than pb2, since this is also what is needed for many optimisations, e.g. jump to
    // jump.
    completeMerge(pb1, pb2, true);
    return true;
}


void Cfg::completeMerge(BasicBlock *pb1, BasicBlock *pb2, bool bDelete)
{
    // First we replace all of pb1's predecessors' out edges that used to point to pb1 (usually only one of these) with
    // pb2
    for (BasicBlock *pPred : pb1->m_inEdges) {
        assert(pPred->m_targetOutEdges == pPred->m_outEdges.size());

        for (BasicBlock *& pred_out : pPred->m_outEdges) {
            if (pred_out == pb1) {
                pred_out = pb2;
            }
        }
    }

    // Now we replace pb2's in edges by pb1's inedges
    pb2->m_inEdges = pb1->m_inEdges;

    if (!bDelete) {
        return;
    }

    // Finally, we delete pb1 from the BB list. Note: remove(pb1) should also work, but it would involve member
    // comparison (not implemented), and also would attempt to remove ALL elements of the list with this value (so
    // it has to search the whole list, instead of an average of half the list as we have here).
    for (BB_IT it = m_listBB.begin(); it != m_listBB.end(); it++) {
        if (*it != pb1) {
            continue;
        }

        if ((*it)->getLowAddr() != Address::ZERO) {
            m_mapBB.erase((*it)->getLowAddr());
        }

        m_listBB.erase(it);
        break;
    }
}


bool Cfg::joinBB(BasicBlock *pb1, BasicBlock *pb2)
{
    // Ensure that the fallthrough case for pb1 is pb2
    const std::vector<BasicBlock *>& v = pb1->getOutEdges();

    if ((v.size() != 2) || (v[1] != pb2)) {
        return false;
    }

    // Prepend the RTLs for pb1 to those of pb2. Since they will be pushed to the front of pb2, push them in reverse
    // order
    std::list<RTL *>::reverse_iterator it;

    for (it = pb1->m_listOfRTLs->rbegin(); it != pb1->m_listOfRTLs->rend(); it++) {
        pb2->m_listOfRTLs->push_front(*it);
    }

    completeMerge(pb1, pb2); // Mash them together
    // pb1 no longer needed. Remove it from the list of BBs.  This will also delete *pb1. It will be a shallow delete,
    // but that's good because we only did shallow copies to *pb2
    BB_IT bbit = std::find(m_listBB.begin(), m_listBB.end(), pb1);

    if ((*bbit)->getLowAddr() != Address::ZERO) {
        m_mapBB.erase((*bbit)->getLowAddr());
    }

    m_listBB.erase(bbit);
    return true;
}


void Cfg::removeBB(BasicBlock *bb)
{
    BB_IT bbit = std::find(m_listBB.begin(), m_listBB.end(), bb);

    if ((*bbit)->getLowAddr() != Address::ZERO) {
        m_mapBB.erase((*bbit)->getLowAddr());
    }

    m_listBB.erase(bbit);
}


bool Cfg::compressCfg()
{
    // must be well formed
    if (!m_wellFormed) {
        return false;
    }

    // FIXME: The below was working while we still had reaching definitions.  It seems to me that it would be easy to
    // search the BB for definitions between the two branches (so we don't need reaching defs, just the SSA property of
    //  unique definition).
    //
    // Look in CVS for old code.

    // Find A -> J -> B     where J is a BB that is only a jump
    // Then A -> B
    for (BB_IT it = m_listBB.begin(); it != m_listBB.end(); it++) {
        for (auto it1 = (*it)->m_outEdges.begin(); it1 != (*it)->m_outEdges.end(); it1++) {
            BasicBlock *pSucc = (*it1); // Pointer to J
            BasicBlock *bb    = (*it);  // Pointer to A

            if ((pSucc->m_inEdges.size() == 1) && (pSucc->m_outEdges.size() == 1) && (pSucc->m_listOfRTLs->size() == 1) &&
                (pSucc->m_listOfRTLs->front()->size() == 1) && pSucc->m_listOfRTLs->front()->front()->isGoto()) {
                // Found an out-edge to an only-jump BB

                /* std::cout << "outedge to jump detected at " << std::hex << bb->getLowAddr() << " to ";
                 *                      std::cout << pSucc->getLowAddr() << " to " <<
                 * pSucc->m_OutEdges.front()->getLowAddr() << std::dec <<
                 *                      '\n'; */
                // Point this outedge of A to the dest of the jump (B)
                *it1 = pSucc->m_outEdges.front();
                // Now pSucc still points to J; *it1 points to B.  Almost certainly, we will need a jump in the low
                // level C that may be generated. Also force a label for B
                bb->m_jumpRequired = true;
                setLabel(*it1);
                // Find the in-edge from B to J; replace this with an in-edge to A
                std::vector<BasicBlock *>::iterator it2;

                for (it2 = (*it1)->m_inEdges.begin(); it2 != (*it1)->m_inEdges.end(); it2++) {
                    if (*it2 == pSucc) {
                        *it2 = bb; // Point to A
                    }
                }

                // Remove the in-edge from J to A. First find the in-edge
                for (it2 = pSucc->m_inEdges.begin(); it2 != pSucc->m_inEdges.end(); it2++) {
                    if (*it2 == bb) {
                        break;
                    }
                }

                assert(it2 != pSucc->m_inEdges.end());
                pSucc->deleteInEdge(it2);

                // If nothing else uses this BB (J), remove it from the CFG
                if (pSucc->m_inEdges.empty()) {
                    for (BB_IT it3 = m_listBB.begin(); it3 != m_listBB.end(); it3++) {
                        if (*it3 == pSucc) {
                            if ((*it3)->getLowAddr() != Address::ZERO) {
                                m_mapBB.erase((*it3)->getLowAddr());
                            }

                            m_listBB.erase(it3);
                            // And delete the BB
                            delete pSucc;
                            break;
                        }
                    }
                }
            }
        }
    }

    return true;
}


bool Cfg::removeOrphanBBs()
{
    std::deque<BasicBlock *> orphans;

    for (BB_IT it = m_listBB.begin(); it != m_listBB.end(); it++) {
        if (*it == this->m_entryBB) { // don't remove entry BasicBlock
            continue;
        }

        BasicBlock *b = *it;

        if (b->m_inEdges.empty()) {
            orphans.push_back(b);
        }
    }

    bool res = !orphans.empty();

    while (!orphans.empty()) {
        BasicBlock *b = orphans.front();
        orphans.pop_front();

        for (BasicBlock *child : b->m_outEdges) {
            child->deleteInEdge(b);

            if (child->m_inEdges.empty()) {
                orphans.push_back(child);
            }
        }

        removeBB(b);
    }

    return res;
}


void Cfg::unTraverse()
{
    for (BasicBlock *it : m_listBB) {
        it->m_traversedMarker = false;
        it->m_traversed       = TravType::Untraversed;
    }
}


bool Cfg::establishDFTOrder()
{
    // Must be well formed.
    if (!m_wellFormed) {
        return false;
    }

    // Reset all the traversed flags
    unTraverse();

    int      first = 0;
    int      last  = 0;
    unsigned numTraversed;

    if (checkEntryBB()) {
        return false;
    }

    numTraversed = m_entryBB->getDFTOrder(first, last);

    return numTraversed == m_listBB.size();
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


bool Cfg::establishRevDFTOrder()
{
    // Must be well formed.
    if (!m_wellFormed) {
        return false;
    }

    // WAS: sort by last dfs and grab the exit node
    // Why?     This does not seem like a the best way. What we need is the ret node, so let's find it.
    // If the CFG has more than one ret node then it needs to be fixed.
    // sortByLastDFT();

    BasicBlock *retNode = findRetNode();

    if (retNode == nullptr) {
        return false;
    }

    // Reset all the traversed flags
    unTraverse();

    int      first        = 0;
    int      last         = 0;
    unsigned numTraversed = retNode->getRevDFTOrder(first, last);

    return numTraversed == m_listBB.size();
}


bool Cfg::isWellFormed()
{
    return m_wellFormed;
}


bool Cfg::isOrphan(Address uAddr)
{
    MAPBB::iterator mi = m_mapBB.find(uAddr);

    if (mi == m_mapBB.end()) {
        return false; // No entry at all
    }

    // Return true if the first RTL at this address has an address set to 0
    BasicBlock *pBB = (*mi).second;

    // If it's incomplete, it can't be an orphan
    if (pBB->m_incomplete) {
        return false;
    }

    return pBB->m_listOfRTLs->front()->getAddress().isZero();
}


int Cfg::pbbToIndex(const BasicBlock *pBB)
{
    BB_IT it = m_listBB.begin();
    int   i  = 0;

    while (it != m_listBB.end()) {
        if (*it++ == pBB) {
            return i;
        }

        i++;
    }

    return -1;
}


void Cfg::addCall(CallStatement *call)
{
    m_callSites.insert(call);
}


Cfg::CallStatementSet& Cfg::getCalls()
{
    return m_callSites;
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


/***************************************************************************/ /**
 * \brief    "deep" delete for a list of pointers to RTLs
 * \param pLrtl - the list
 ******************************************************************************/
void delete_lrtls(std::list<RTL *>& pLrtl)
{
    for (RTL *it : pLrtl) {
        delete it;
    }
}


/***************************************************************************/ /**
 *
 * \brief   "deep" erase for a list of pointers to RTLs
 * \param   pLrtl - the list
 * \param   begin - iterator to first (inclusive) item to delete
 * \param   end - iterator to last (exclusive) item to delete
 *
 ******************************************************************************/
void erase_lrtls(std::list<RTL *>& pLrtl, std::list<RTL *>::iterator begin, std::list<RTL *>::iterator end)
{
    for (auto it = begin; it != end; it++) {
        delete (*it);
    }

    pLrtl.erase(begin, end);
}


void Cfg::setLabel(BasicBlock *pBB)
{
    if (pBB->m_labelNum == 0) {
        pBB->m_labelNum = ++m_lastLabel;
    }
}


void Cfg::addNewOutEdge(BasicBlock *pFromBB, BasicBlock *pNewOutEdge)
{
    pFromBB->m_outEdges.push_back(pNewOutEdge);
    // Since this is a new out-edge, set the "jump required" flag
    pFromBB->m_jumpRequired = true;
    // Make sure that there is a label there
    setLabel(pNewOutEdge);
}


void Cfg::simplify()
{
    LOG_VERBOSE(1) << "simplifying...\n";

    for (BasicBlock *bb : m_listBB) {
        bb->simplify();
    }
}


void Cfg::print(QTextStream& out, bool html)
{
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
    LOG << tgt;
}


void Cfg::setTimeStamps()
{
    // set DFS tag
    for (BasicBlock *it : m_listBB) {
        it->m_traversed = TravType::DFS_Tag;
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
        if (VERBOSE) {
            LOG << "failed to find commonPDom for " << oldCurImmPDom->getLowAddr() << " and "
                << oldSuccImmPDom->getLowAddr() << "\n";
        }

        return oldCurImmPDom; // no change
    }

    return curImmPDom;
}


void Cfg::findImmedPDom()
{
    BasicBlock *curNode, *succNode; // the current Node and its successor

    // traverse the nodes in order (i.e from the bottom up)
    for (int i = m_revOrdering.size() - 1; i >= 0; i--) {
        curNode = m_revOrdering[i];
        const std::vector<BasicBlock *>& oEdges = curNode->getOutEdges();

        for (auto& oEdge : oEdges) {
            succNode = oEdge;

            if (succNode->m_revOrd > curNode->m_revOrd) {
                curNode->m_immPDom = commonPDom(curNode->m_immPDom, succNode);
            }
        }
    }

    // make a second pass but consider the original CFG ordering this time
    unsigned u;

    for (u = 0; u < m_ordering.size(); u++) {
        curNode = m_ordering[u];
        const std::vector<BasicBlock *>& oEdges = curNode->getOutEdges();

        if (oEdges.size() <= 1) {
            continue;
        }

        for (auto& oEdge : oEdges) {
            succNode           = oEdge;
            curNode->m_immPDom = commonPDom(curNode->m_immPDom, succNode);
        }
    }

    // one final pass to fix up nodes involved in a loop
    for (u = 0; u < m_ordering.size(); u++) {
        curNode = m_ordering[u];
        const std::vector<BasicBlock *>& oEdges = curNode->getOutEdges();

        if (oEdges.size() > 1) {
            for (auto& oEdge : oEdges) {
                succNode = oEdge;

                if (curNode->hasBackEdgeTo(succNode) && (curNode->getOutEdges().size() > 1) && succNode->m_immPDom &&
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
        if (curNode->getOutEdges().size() > 1) {
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
        if (loopNodes[header->getOutEdges()[0]->m_ord]) {
            header->setLoopFollow(header->getOutEdges()[1]);
        }
        else {
            header->setLoopFollow(header->getOutEdges()[0]);
        }
    }
    else if (lType == LoopType::PostTested) {
        // the follow of a post tested ('repeat') loop is the node on the end of the non-back edge from the latch node
        if (latch->getOutEdges()[0] == header) {
            header->setLoopFollow(latch->getOutEdges()[1]);
        }
        else {
            header->setLoopFollow(latch->getOutEdges()[0]);
        }
    }
    else {   // endless loop
        BasicBlock *follow = nullptr;

        // traverse the ordering array between the header and latch nodes.
        // BasicBlock * latch = header->getLatchNode(); initialized at function start
        for (int i = header->m_ord - 1; i > latch->m_ord; i--) {
            BasicBlock *& desc = m_ordering[i];
            // the follow for an endless loop will have the following
            // properties:
            //     i) it will have a parent that is a conditional header inside the loop whose follow is outside the
            //        loop
            //    ii) it will be outside the loop according to its loop stamp pair
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
                    BasicBlock *succ = desc->getOutEdges()[0];

                    if (loopNodes[succ->m_ord]) {
                        if (!loopNodes[desc->getOutEdges()[1]->m_ord]) {
                            succ = desc->getOutEdges()[1];
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
    //    i) header.loopStamps encloses curNode.loopStamps and curNode.loopStamps encloses latch.loopStamps
    //    OR
    //    ii) latch.revLoopStamps encloses curNode.revLoopStamps and curNode.revLoopStamps encloses header.revLoopStamps
    //    OR
    //    iii) curNode is the latch node

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

        std::vector<BasicBlock *>& iEdges = curNode->getInEdges();

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
        const std::vector<BasicBlock *>& oEdges = curNode->getOutEdges();

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
                ((curNode->getCaseHead() != curNode->getOutEdges()[BTHEN]->getCaseHead()) ||
                 (curNode->getCaseHead() != curNode->getOutEdges()[BELSE]->getCaseHead()))) {
                BasicBlock *myCaseHead   = curNode->getCaseHead();
                BasicBlock *thenCaseHead = curNode->getOutEdges()[BTHEN]->getCaseHead();
                BasicBlock *elseCaseHead = curNode->getOutEdges()[BELSE]->getCaseHead();

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
                if (curNode->hasBackEdgeTo(curNode->getOutEdges()[BTHEN])) {
                    curNode->setCondType(CondType::IfThen);
                    curNode->setCondFollow(curNode->getOutEdges()[BELSE]);
                }
                else {
                    curNode->setCondType(CondType::IfElse);
                    curNode->setCondFollow(curNode->getOutEdges()[BTHEN]);
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
    findImmedPDom();

    if (!Boomerang::get()->noDecompile) {
        structConds();
        structLoops();
        checkConds();
    }

    m_structured = true;
}


void Cfg::removeJunctionStatements()
{
    for (BasicBlock *pbb : m_listBB) {
        if (pbb->getFirstStmt() && pbb->getFirstStmt()->isJunction()) {
            assert(pbb->getRTLs());
            pbb->getRTLs()->front()->pop_front();
        }
    }
}


void Cfg::removeUnneededLabels(ICodeGenerator *hll)
{
    hll->removeUnusedLabels(m_ordering.size());
}


void Cfg::generateDotFile(QTextStream& of)
{
       Address aret = Address::INVALID;

    // The nodes
    // std::list<PBB>::iterator it;
    for (BasicBlock *pbb : m_listBB) {
        of << "       "
           << "bb" << pbb->getLowAddr() << " ["
           << "label=\"";
        char *p = pbb->getStmtNumber();
#if PRINT_BBINDEX
        of << std::dec << indices[*it];

        if (p[0] != 'b') {
            // If starts with 'b', no statements (something like bb8101c3c).
            of << ":";
        }
#endif
        of << p << " ";

        switch (pbb->getType())
        {
        case BBType::Oneway:
            of << "oneway";
            break;

        case BBType::Twoway:

            if (pbb->getCond()) {
                of << "\\n";
                pbb->getCond()->print(of);
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
                SharedExp de = pbb->getDest();

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
                Function *dest = pbb->getDestProc();

                if (dest) {
                    of << "\\n" << dest->getName();
                }

                break;
            }

        case BBType::Ret:
            of << "ret\" shape=triangle];\n";
            // Remember the (unbique) return BB's address
            aret = pbb->getLowAddr();
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
    if (!aret.isZero()) {
        of << "{rank=max; bb" << aret << "}\n";
    }

    // Close the subgraph
    of << "}\n";

    // Now the edges
    for (BasicBlock *pbb : m_listBB) {
        const std::vector<BasicBlock *>& outEdges = pbb->getOutEdges();

        for (unsigned int j = 0; j < outEdges.size(); j++) {
            of << "       bb" << pbb->getLowAddr() << " -> ";
            of << "bb" << outEdges[j]->getLowAddr();

            if (pbb->getType() == BBType::Twoway) {
                if (j == 0) {
                    of << " [label=\"true\"]";
                }
                else {
                    of << " [label=\"false\"]";
                }
            }

            of << " [color = \"blue\"];\n";
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
    for (BasicBlock *currIn : currBB->getInEdges()) {
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

    while (workList.size() && count < 100000) {
        count++; // prevent infinite loop

        if (++progress > 20) {
            LOG_STREAM() << "i";
            LOG_STREAM().flush();
            progress = 0;
        }

        BasicBlock *currBB = workList.back();
        workList.erase(--workList.end());
        workSet.erase(currBB);
        // Calculate live locations and interferences
        bool change = currBB->calcLiveness(cg, m_myProc);

        if (!change) {
            continue;
        }

        if (DEBUG_LIVENESS) {
            LOG << "Revisiting BB ending with stmt ";
            Instruction *last = nullptr;

            if (!currBB->m_listOfRTLs->empty()) {
                RTL *lastRtl = currBB->m_listOfRTLs->back();

                if (lastRtl->size()) {
                    last = lastRtl->back();
                }
            }

            if (last) {
                LOG << last->getNumber();
            }
            else {
                LOG << "<none>";
            }

            LOG << " due to change\n";
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
    LOG_STREAM() << "For BB at " << bb << ":\nIn edges: ";
    std::vector<BasicBlock *> ins = bb->getInEdges();
    std::vector<BasicBlock *> outs = bb->getOutEdges();
    size_t i, n = ins.size();

    for (i = 0; i < n; i++) {
        LOG_STREAM() << ins[i] << " ";
    }

    LOG_STREAM() << "\nOut Edges: ";
    n = outs.size();

    for (i = 0; i < n; i++) {
        LOG_STREAM() << outs[i] << " ";
    }

    LOG_STREAM() << "\n";
}


BasicBlock *Cfg::splitForBranch(BasicBlock *pBB, RTL *rtl, BranchStatement *br1, BranchStatement *br2, BB_IT& it)
{
    std::list<RTL *>::iterator ri;

    // First find which RTL has the split address
    for (ri = pBB->m_listOfRTLs->begin(); ri != pBB->m_listOfRTLs->end(); ri++) {
        if ((*ri) == rtl) {
            break;
        }
    }

    assert(ri != pBB->m_listOfRTLs->end());

    bool haveA = (ri != pBB->m_listOfRTLs->begin());

       Address addr = rtl->getAddress();

    // Make a BB for the br1 instruction

    // Don't give this "instruction" the same address as the rest of the string instruction (causes problems when
    // creating the rptBB). Or if there is no A, temporarily use 0
       Address    a        = (haveA) ? addr : Address::ZERO;
    RTL        *skipRtl = new RTL(a, new std::list<Instruction *> { br1 }); // list initializer in braces
    BasicBlock *skipBB  = newBB(new std::list<RTL *> { skipRtl }, BBType::Twoway, 2);
    rtl->setAddress(addr + 1);

    if (!haveA) {
        skipRtl->setAddress(addr);
        // Address addr now refers to the splitBB
        m_mapBB[addr] = skipBB;

        // Fix all predecessors of pBB to point to splitBB instead
        for (size_t i = 0; i < pBB->m_inEdges.size(); i++) {
            BasicBlock *pred = pBB->m_inEdges[i];

            for (size_t j = 0; j < pred->m_outEdges.size(); j++) {
                BasicBlock *succ = pred->m_outEdges[j];

                if (succ == pBB) {
                    pred->m_outEdges[j] = skipBB;
                    skipBB->addInEdge(pred);
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
    BasicBlock *rptBB = newBB(new std::list<RTL *> { *ri }, BBType::Twoway, 2);
    ri = pBB->m_listOfRTLs->erase(ri);

    // Move the remaining RTLs (if any) to a new list of RTLs
    BasicBlock *newBb;
    size_t     oldOutEdges = 0;
    bool       haveB       = true;

    if (ri != pBB->m_listOfRTLs->end()) {
        auto pRtls = new std::list<RTL *>;

        while (ri != pBB->m_listOfRTLs->end()) {
            pRtls->push_back(*ri);
            ri = pBB->m_listOfRTLs->erase(ri);
        }

        oldOutEdges = pBB->getNumOutEdges();
        newBb       = newBB(pRtls, pBB->getType(), oldOutEdges);

        // Transfer the out edges from A to B (pBB to newBb)
        for (size_t i = 0; i < oldOutEdges; i++) {
            // Don't use addOutEdge, since it will also add in-edges back to pBB
            newBb->m_outEdges.push_back(pBB->getOutEdge(i));
        }

        // addOutEdge(newBb, pBB->getOutEdge(i));
    }
    else {
        // The "B" part of the above diagram is empty.
        // Don't create a new BB; just point newBB to the successor of pBB
        haveB = false;
        newBb = pBB->getOutEdge(0);
    }

    // Change pBB to a FALL bb
    pBB->updateType(BBType::Fall, 1);
    // Set the first out-edge to be skipBB
    pBB->m_outEdges.erase(pBB->m_outEdges.begin(), pBB->m_outEdges.end());
    addOutEdge(pBB, skipBB);
    // Set the out edges for skipBB. First is the taken (true) leg.
    addOutEdge(skipBB, newBb);
    addOutEdge(skipBB, rptBB);
    // Set the out edges for the rptBB
    addOutEdge(rptBB, skipBB);
    addOutEdge(rptBB, newBb);

    // For each out edge of newBb, change any in-edges from pBB to instead come from newBb
    if (haveB) {
        for (size_t i = 0; i < oldOutEdges; i++) {
            BasicBlock *succ = newBb->m_outEdges[i];

            for (auto& elem : succ->m_inEdges) {
                BasicBlock *pred = elem;

                if (pred == pBB) {
                    elem = newBb;
                    break;
                }
            }
        }
    }
    else {
        // There is no "B" bb (newBb is just the successor of pBB) Fix that one out-edge to point to rptBB
        for (auto& elem : newBb->m_inEdges) {
            BasicBlock *pred = elem;

            if (pred == pBB) {
                elem = rptBB;
                break;
            }
        }
    }

    if (!haveA) {
        // There is no A any more. All A's in-edges have been copied to the skipBB. It is possible that the original BB
        // had a self edge (branch to start of self). If so, this edge, now in to skipBB, must now come from newBb (if
        // there is a B) or rptBB if none.  Both of these will already exist, so delete it.
        for (size_t j = 0; j < skipBB->m_inEdges.size(); j++) {
            BasicBlock *pred = skipBB->m_inEdges[j];

            if (pred == pBB) {
                skipBB->deleteInEdge(pBB);
                break;
            }
        }

#if DEBUG_SPLIT_FOR_BRANCH
        LOG_STREAM() << "About to delete pBB: " << std::hex << pBB << "\n";
        dumpBB(pBB);
        dumpBB(skipBB);
        dumpBB(rptBB);
        dumpBB(newBb);
#endif

        // Must delete pBB. Note that this effectively "increments" iterator it
        it  = m_listBB.erase(it);
        pBB = nullptr;
    }
    else {
        it++;
    }

    return newBb;
}


bool Cfg::decodeIndirectJmp(UserProc *proc)
{
    bool res = false;

    for (BasicBlock *bb : m_listBB) {
        res |= bb->decodeIndirectJmp(proc);
    }

    return res;
}


void Cfg::undoComputedBB(Instruction *stmt)
{
    for (BasicBlock *bb : m_listBB) {
        if (bb->undoComputedBB(stmt)) {
            break;
        }
    }
}


Instruction *Cfg::findImplicitAssign(SharedExp x)
{
    Instruction *def;

    std::map<SharedExp, Instruction *, lessExpStar>::iterator it = m_implicitMap.find(x);

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

    return def;
}


Instruction *Cfg::findTheImplicitAssign(const SharedExp& x)
{
    // As per the above, but don't create an implicit if it doesn't already exist
    auto it = m_implicitMap.find(x);

    if (it == m_implicitMap.end()) {
        return nullptr;
    }

    return it->second;
}


Instruction *Cfg::findImplicitParamAssign(Parameter *param)
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
    Instruction *ia = it->second;
    m_implicitMap.erase(it);          // Delete the mapping
    m_myProc->removeStatement(ia);    // Remove the actual implicit assignment statement as well
}
