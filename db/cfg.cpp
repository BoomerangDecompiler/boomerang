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

/*==============================================================================
 * FILE:       cfg.cc
 * OVERVIEW:   Implementation of the CFG class.
 *============================================================================*/

/*
 * $Revision$
 * 18 Apr 01 - Mike: Mods for boomerang
 */


/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include <algorithm>        // For find()
#include <fstream>
#include <sstream>
#include "types.h"
#include "dataflow.h"
#include "exp.h"
#include "cfg.h"
#include "register.h"
#include "rtl.h"
#include "proc.h"           // For Proc::setTailCaller()
#include "prog.h"           // For findProc()
#include "util.h"
#include "hllcode.h"
#include "boomerang.h"

void delete_lrtls(std::list<RTL*>* pLrtl);
void erase_lrtls(std::list<RTL*>* pLrtl, std::list<RTL*>::iterator begin,
    std::list<RTL*>::iterator end);

/**********************************
 * Cfg methods.
 **********************************/

/*==============================================================================
 * FUNCTION:        Cfg::Cfg
 * OVERVIEW:        
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
Cfg::Cfg()
    : entryBB(NULL), m_bWellFormed(false), m_uExtraCover(0), lastLabel(0)
{}

/*==============================================================================
 * FUNCTION:        Cfg::~Cfg
 * OVERVIEW:        Destructor. Note: destructs the component BBs as well
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
Cfg::~Cfg()
{
    // Delete the BBs
    BB_IT it;
    for (it = m_listBB.begin(); it != m_listBB.end(); it++)
        if (*it) {
            delete *it;
        }
}

/*==============================================================================
 * FUNCTION:        Cfg::setProc
 * OVERVIEW:        Set the pointer to the owning UserProc object
 * PARAMETERS:      proc - pointer to the owning UserProc object
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::setProc(UserProc* proc)
{
    myProc = proc;
}

/*==============================================================================
 * FUNCTION:        Cfg::clear
 * OVERVIEW:        Clear the CFG of all basic blocks, ready for decode
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::clear()
{
	for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end(); it++)
		delete *it;
	m_listBB.clear();
	m_mapBB.clear();
	entryBB = NULL;
	m_bWellFormed = false;
	callSites.clear();
	lastLabel = 0;    
}

/*==============================================================================
 * FUNCTION:        Cfg::operator=
 * OVERVIEW:        
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
const Cfg& Cfg::operator=(const Cfg& other)
{
    m_listBB = other.m_listBB;
    m_mapBB = other.m_mapBB;
    m_bWellFormed = other.m_bWellFormed;
    return *this;
}

/*==============================================================================
 * FUNCTION:        checkEntryBB
 * OVERVIEW:        Check the entry BB pointer; if zero, emit error message
 *                    and return true
 * PARAMETERS:      <none>
 * RETURNS:         true if was null
 *============================================================================*/
bool Cfg::checkEntryBB()
{
    if (entryBB == NULL) {
        std::cerr << "No entry BB for ";
        if (myProc)
            std::cerr << myProc->getName() << std::endl;
        else
            std::cerr << "unknown proc\n";
        return true;
    }
    return false;
}

/*==============================================================================
 * FUNCTION:        Cfg::newBB
 * OVERVIEW:        Add a new basic block to this cfg 
 * PARAMETERS:      pRtls: list of pointers to RTLs to initialise the BB with
 *                  bbType: the type of the BB (e.g. TWOWAY)
 *                  iNumOutEdges: number of out edges this BB will eventually
 *                    have
 * RETURNS:         Pointer to the newly created BB, or 0 if there is already
 *                    an incomplete BB with the same address
 *============================================================================*/
PBB Cfg::newBB(std::list<RTL*>* pRtls, BBTYPE bbType, int iNumOutEdges)
{
    MAPBB::iterator mi;
    PBB pBB;

    // First find the native address of the first RTL
    // Can't use BasicBlock::GetLowAddr(), since we don't yet have a BB!
    ADDRESS addr = pRtls->front()->getAddress();
    // If this is zero, try the next RTL (only). This may be necessary
    // if e.g. there is a BB with a delayed branch only, with its delay
    // instruction moved in front of it (with 0 address).
    // Note: it is possible to see two RTLs with zero address with
    // Sparc: jmpl %o0, %o1. There will be one for the delay instr (if
    // not a NOP), and one for the side effect of copying %o7 to %o1.
    // Note that orphaned BBs (for which we must compute addr here to
    // to be 0) must not be added to the map, but they have no RTLs with
    // a non zero address.
    if ((addr == 0) && (pRtls->size() > 1))
    {
        std::list<RTL*>::iterator next = pRtls->begin();
        addr = (*++next)->getAddress();
    }

    // If this addr is non zero, check the map to see if we have a
    // (possibly incomplete) BB here already
    // If it is zero, this is a special BB for handling delayed
    // branches or the like
    bool bDone = false;
    if (addr != 0)
    {
        mi = m_mapBB.find(addr);
        if (mi != m_mapBB.end() && (*mi).second)
        {
            pBB = (*mi).second;
            // It should be incomplete, or the pBB there should be zero
            // (we have called Label but not yet created the BB for it).
            // Else we have duplicated BBs. Note: this can happen with
            // forward jumps into the middle of a loop, so not error
            if (!pBB->m_bIncomplete)
            {
                // This list of RTLs is not needed now
                delete_lrtls(pRtls);
                return 0;
            }
            else
            {
                // Fill in the details, and return it
                pBB->setRTLs(pRtls);
                pBB->m_nodeType = bbType;
                pBB->m_iNumOutEdges = iNumOutEdges;
                pBB->m_bIncomplete = false;
            }
            bDone = true;
        }
    }
    if (!bDone)
    {
        // Else add a new BB to the back of the current list.
        pBB = new BasicBlock(pRtls, bbType, iNumOutEdges);
        m_listBB.push_back(pBB);

        // Also add the address to the map from native (source) address to
        // pointer to BB, unless it's zero
        if (addr != 0)
        {
            m_mapBB[addr] = pBB;            // Insert the mapping
            mi = m_mapBB.find(addr);
        }
    }

    if (addr != 0 && (mi != m_mapBB.end()))
    {
        // Existing New         +---+ Top of new
        //          +---+       +---+
        //  +---+   |   |       +---+ Fall through
        //  |   |   |   | =>    |   |
        //  |   |   |   |       |   | Existing; rest of new discarded
        //  +---+   +---+       +---+
        //  
        // Check for overlap of the just added BB with the next BB
        // (address wise). If there is an overlap, truncate the std::list<Exp*> for
        // the new BB to not overlap, and make this a fall through BB
        // We still want to do this even if the new BB overlaps with an
        // incomplete BB, though in this case, splitBB needs to fill in
        // the details for the "bottom" BB of the split. Also, in this
        // case, we return a pointer to the newly completed BB, so it
        // will get out edges added (if required). In the other case
        // (i.e. we overlap with an exising, completed BB), we want to
        // return 0, since the out edges are already created.
        if (++mi != m_mapBB.end())
        {
            PBB pNextBB = (*mi).second;
            ADDRESS uNext = (*mi).first;
            bool bIncomplete = pNextBB->m_bIncomplete;
            if (uNext <= pRtls->back()->getAddress())
            {
                // Need to truncate the current BB. We use splitBB(), but
                // pass it pNextBB so it doesn't create a new BB for the
                // "bottom" BB of the split pair
                splitBB(pBB, uNext, pNextBB);
                // If the overlapped BB was incomplete, return the
                // "bottom" part of the BB, so adding out edges will work
                // properly.
                if (bIncomplete)
                {
                    return pNextBB;
                }
                // However, if the overlapping BB was already
                // complete, return 0, so out edges won't be added twice
                else return 0;
            }
        }

        // Existing New         +---+ Top of existing
        //  +---+               +---+
        //  |   |   +---+       +---+ Fall through
        //  |   |   |   | =>    |   |
        //  |   |   |   |       |   | New; rest of existing discarded
        //  +---+   +---+       +---+
        // Note: no need to check the other way around, because in this
        // case, we will have called Cfg::Label(), and it will have
        // split the existing BB already.
    }
    return pBB;
}

// Use this function when there are outedges to BBs that are not created
// yet. Usually used via addOutEdge()
/*==============================================================================
 * FUNCTION:        Cfg::newIncompleteBB
 * OVERVIEW:        
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
PBB Cfg::newIncompleteBB(ADDRESS addr)
{
    // Create a new (basically empty) BB
    PBB pBB = new BasicBlock();
    // Add it to the list
    m_listBB.push_back(pBB);
    m_mapBB[addr] = pBB;                // Insert the mapping
    return pBB;
}

/*==============================================================================
 * FUNCTION:        Cfg::addOutEdge
 * OVERVIEW:        Add an out edge to this BB (and the in-edge to the dest BB)
 *                  May also set a label
 * NOTE:            Overloaded with address as 2nd argument (calls this proc
 *                    in the end)
 * NOTE ALSO:       Does not increment m_iNumOutEdges; this is supposed to be
 *                    constant for a BB. (But see BasicBlock::addNewOutEdge())
 * PARAMETERS:      pBB: source BB (to have the out edge added to)
 *                  pDestBB: destination BB (to have the out edge point to)
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::addOutEdge(PBB pBB, PBB pDestBB, bool bSetLabel /* = false */)
{
    // Add the given BB pointer to the list of out edges
    pBB->m_OutEdges.push_back(pDestBB);
    // Note that the number of out edges is set at constructor time,
    // not incremented here.
    // Add the in edge to the destination BB
    pDestBB->m_InEdges.push_back(pBB);
    pDestBB->m_iNumInEdges++;           // Inc the count
    if (bSetLabel) setLabel(pDestBB);   // Indicate "label required"
}

/*==============================================================================
 * FUNCTION:        Cfg::addOutEdge
 * OVERVIEW:        Add an out edge to this BB (and the in-edge to the dest BB)
 *                  May also set a label
 * NOTE:            Calls the above
 * PARAMETERS:      pBB: source BB (to have the out edge added to) 
 *                  addr: source address of destination (the out edge is to
 *                      point to the BB whose lowest address is addr)
 *                  bSetLabel: if true, set a label at the destination address.
 *                      Set true on "true" branches of labels
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::addOutEdge(PBB pBB, ADDRESS addr, bool bSetLabel /* = false */)
{
    // Check to see if the address is in the map, i.e. we already have
    // a BB for this address
    MAPBB::iterator it = m_mapBB.find(addr);
    PBB pDestBB;
    if (it != m_mapBB.end() && (*it).second) {
        // Just add this PBB to the list of out edges
        pDestBB = (*it).second;
    }
    else {
        // Else, create a new incomplete BB, add that to the map, and add
        // the new BB as the out edge
        pDestBB = newIncompleteBB(addr);
    }
    addOutEdge(pBB, pDestBB, bSetLabel);
}

/*==============================================================================
 * FUNCTION:        Cfg::isLabel 
 * OVERVIEW:        Return true if the given address is the start of a basic
 *                    block, complete or not
 * PARAMETERS:      uNativeAddr: native address to look up
 * RETURNS:         True if uNativeAddr starts a BB
 *============================================================================*/
// Note: must ignore entries with a null pBB, since these are caused by
// calls to Label that failed, i.e. the instruction is not decoded yet.
bool Cfg::isLabel (ADDRESS uNativeAddr)
{
    MAPBB::iterator mi;
    mi = m_mapBB.find (uNativeAddr);
    return (mi != m_mapBB.end() && (*mi).second);
}

/*==============================================================================
 * FUNCTION:    Cfg::splitBB (private)
 * OVERVIEW:    Split the given basic block at the RTL associated with
 *              uNativeAddr. The first node's type becomes fall-through
 *              and ends at the RTL prior to that associated with uNativeAddr.
 *              The second node's type becomes the type of the original basic
 *              block (pBB), and its out-edges are those of the original
 *              basic block. In edges of the new BB's descendants are changed.
 * PRECONDITION: assumes uNativeAddr is an address within the boundaries
 *              of the given basic block.
 * PARAMETERS:  pBB -  pointer to the BB to be split
 *              uNativeAddr - address of RTL to become the start of the new BB
 *              pNewBB -  if non zero, it remains as the "bottom" part of the
 *              BB, and splitBB only modifies the top part to not overlap.
 *              bDelRtls - if true, deletes the RTLs removed from the existing
 *              BB after the split point. Only used if there is an overlap with
 *              existing instructions
 * RETURNS:     Returns a pointer to the "bottom" (new) part of the split BB.
 *============================================================================*/
PBB Cfg::splitBB (PBB pBB, ADDRESS uNativeAddr, PBB pNewBB /* = 0 */,
    bool bDelRtls /* = false */)
{ 
    std::list<RTL*>::iterator ri;

    // First find which RTL has the split address; note that this
    // could fail (e.g. label in the middle of an instruction, or
    // some weird delay slot effects)
    for (ri = pBB->m_pRtls->begin(); ri != pBB->m_pRtls->end(); ri++) {
        if ((*ri)->getAddress() == uNativeAddr)
            break;
    }
    if (ri == pBB->m_pRtls->end())
    {
        std::cerr << "could not split BB at " << std::hex;
        std::cerr << pBB->getLowAddr() << " at splt address " << uNativeAddr
            << std::endl;
        return pBB;
    }

    // If necessary, set up a new basic block with information from the
    // original bb
    if (pNewBB == 0)
    {
        pNewBB = new BasicBlock(*pBB);
        // But we don't want the top BB's in edges; our only in-edge should
        // be the out edge from the top BB
        pNewBB->m_iNumInEdges = 0;
        pNewBB->m_InEdges.erase(pNewBB->m_InEdges.begin(),
            pNewBB->m_InEdges.end());
        // The "bottom" BB now starts at the implicit label, so we create
        // a new list that starts at ri. We need a new list, since it is
        // different from the original BB's list. We don't have to "deep
        // copy" the RTLs themselves, since they will never overlap
        pNewBB->setRTLs(new std::list<RTL*>(ri, pBB->m_pRtls->end()));
        // Put it in the graph
        m_listBB.push_back(pNewBB);
        // Put the implicit label into the map. Need to do this before the
        // addOutEdge() below
        m_mapBB[uNativeAddr] = pNewBB;
        // There must be a label here; else would not be splitting.
        // Give it a new label
        pNewBB->m_iLabelNum = ++lastLabel;
    }
    else if (pNewBB->m_bIncomplete)
    {
        // We have an existing BB and a map entry, but no details except
        // for in-edges and m_bHasLabel.
        // First save the in-edges and m_iLabelNum
        std::vector<PBB> ins(pNewBB->m_InEdges);
        int label = pNewBB->m_iLabelNum;
        // Copy over the details now, completing the bottom BB
        *pNewBB = *pBB;                 // Assign the BB, copying fields
                                        // This will set m_bIncomplete false
        // Replace the in edges (likely only one)
        pNewBB->m_InEdges = ins;
        pNewBB->m_iNumInEdges = ins.size();
        // Replace the label (must be one, since we are splitting this BB!)
        pNewBB->m_iLabelNum = label;
        // The "bottom" BB now starts at the implicit label
        // We need to create a new list of RTLs, as per above
        pNewBB->setRTLs(new std::list<RTL*>(ri, pBB->m_pRtls->end()));
    }
    // else pNewBB exists and is complete. We don't want to change the
    // complete BB in any way, except to later add one in-edge

    // Update original ("top") basic block's info and make it a fall-through
    pBB->m_nodeType = FALL;
    // Fix the in-edges of pBB's descendants. They are now pNewBB
    // Note: you can't believe m_iNumOutEdges at the time that this function
    // may get called
    for (unsigned j=0; j < pBB->m_OutEdges.size(); j++)
    {
        PBB pDescendant = pBB->m_OutEdges[j];
        // Search through the in edges for pBB (old ancestor)
        unsigned k;
        for (k=0; k < pDescendant->m_InEdges.size(); k++)
        {
            if (pDescendant->m_InEdges[k] == pBB)
            {
                // Replace with a pointer to the new ancestor
                pDescendant->m_InEdges[k] = pNewBB;
                break;
            }
        }
        // That pointer should have been found!
        assert (k < pDescendant->m_InEdges.size());
    }
    // The old BB needs to have part of its list of RTLs erased, since
    // the instructions overlap
    if (bDelRtls)
    {
        // Delete the list of pointers, and also the RTLs they point to
        erase_lrtls(pBB->m_pRtls, ri, pBB->m_pRtls->end());
    }
    else
    {
        // Delete the list of pointers, but not the RTLs they point to
        pBB->m_pRtls->erase(ri, pBB->m_pRtls->end());
    }
    // Erase any existing out edges
    pBB->m_OutEdges.erase(pBB->m_OutEdges.begin(), pBB->m_OutEdges.end());
    pBB->m_iNumOutEdges = 1;
    addOutEdge (pBB, uNativeAddr);  
    return pNewBB;
}

/*==============================================================================
 * FUNCTION:        Cfg::getFirstBB
 * OVERVIEW:        Get the first BB of this cfg
 * PARAMETERS:      it: set to an value that must be passed to getNextBB
 * RETURNS:         Pointer to the first BB this cfg, or NULL if none
 *============================================================================*/
PBB Cfg::getFirstBB(BB_IT& it)
{
    if ((it = m_listBB.begin()) == m_listBB.end()) return 0;
    return *it;
}

/*==============================================================================
 * FUNCTION:        Cfg::getNextBB
 * OVERVIEW:        Get the next BB this cfg. Basically increments the given
 *                  iterator and returns it
 * PARAMETERS:      iterator from a call to getFirstBB or getNextBB
 * RETURNS:         pointer to the BB, or NULL if no more
 *============================================================================*/
PBB Cfg::getNextBB(BB_IT& it)
{
    if (++it == m_listBB.end()) return 0;
    return *it;
}

/*==============================================================================
 * FUNCTION:    Cfg::label
 * OVERVIEW:    Checks whether the given native address is a label (explicit
 *              or non explicit) or not. Returns false for incomplete BBs.
 *              So it returns true iff the address has already been decoded
 *              in some BB. If it was not already a label (i.e. the first
 *              instruction of some BB), the BB is split so that it becomes
 *              a label.
 *              Explicit labels are addresses that have already been tagged
 *              as being labels due to transfers of control to that address,
 *              and are therefore the start of some BB.  Non explicit labels
 *              are those that belong to basic blocks that have already been
 *              constructed (i.e. have previously been parsed) and now need
 *              to be made explicit labels.  In the case of non explicit
 *              labels, the basic block is split into two and types and
 *              edges are adjusted accordingly. If pCurBB is the BB that
 *              gets split, it is changed to point to the address of the
 *              new (lower) part of the split BB.
 *              If there is an incomplete entry in the table for this
 *              address which overlaps with a completed address, the completed
 *              BB is split and the BB for this address is completed.
 * PARAMETERS:  uNativeAddress - native (source) address to check
 *              pCurBB - See above
 * RETURNS:     True if uNativeAddr is a label, i.e. (now) the start of a BB
 *              Note: pCurBB may be modified (as above)
 *============================================================================*/
bool Cfg::label ( ADDRESS uNativeAddr, PBB& pCurBB )
{ 
    MAPBB::iterator mi, newi;

    // check if the native address is in the map already (explicit label)
    mi = m_mapBB.find (uNativeAddr);
    
    if (mi == m_mapBB.end())        // not in the map
    {
        // If not an explicit label, temporarily add the address to the map
        m_mapBB[uNativeAddr] = (PBB) 0;     // no PBB yet

        // get an iterator to the new native address and check if the
        // previous element in the (sorted) map overlaps
        // this new native address; if so, it's a non-explicit label 
        // which needs to be made explicit by splitting the previous BB.
        mi = m_mapBB.find (uNativeAddr);

        newi = mi;
        bool bSplit = false;
        PBB pPrevBB;
        if (newi != m_mapBB.begin()) {
            pPrevBB = (*--mi).second;
            if (!pPrevBB->m_bIncomplete &&
                (pPrevBB->getLowAddr() < uNativeAddr) &&
                (pPrevBB->getHiAddr () >= uNativeAddr))
                    bSplit = true;
        }
        if (bSplit) {
            // Non-explicit label. Split the previous BB
            PBB pNewBB = splitBB (pPrevBB, uNativeAddr);    
            if (pCurBB == pPrevBB) {
                // This means that the BB that we are expecting to use, usually
                // to add out edges, has changed. We must change this pointer
                // so that the right BB gets the out edges. However, if the
                // new BB is not the BB of interest, we mustn't change pCurBB
                pCurBB = pNewBB;
            }
            return true;            // wasn't a label, but already parsed
        }
        else {                      // not a non-explicit label
            // We don't have to erase this map entry. Having a null
            // BasicBlock pointer is coped with in newBB() and addOutEdge();
            // when eventually the BB is created, it will replace this entry.
            // we should be currently processing this BB. The map will
            // be corrected when newBB is called with this address.
            return false;               // was not already parsed
        }
    }
    else            // We already have uNativeAddr in the map
    {
        if ((*mi).second && !(*mi).second->m_bIncomplete) {
            // There is a complete BB here. Return true.
            return true;
        }

        // We are finalising an incomplete BB. Still need to check previous
        // map entry to see if there is a complete BB overlapping
        bool bSplit = false;
        PBB pPrevBB, pBB = (*mi).second;
        if (mi != m_mapBB.begin())
        {
            pPrevBB = (*--mi).second;
            if (!pPrevBB->m_bIncomplete &&
                (pPrevBB->getLowAddr() < uNativeAddr) &&
                (pPrevBB->getHiAddr () >= uNativeAddr))
                    bSplit = true;
        }
        if (bSplit)
        {
            // Pass the third parameter to splitBB, because we already
            // have an (incomplete) BB for the "bottom" BB of the split
            splitBB (pPrevBB, uNativeAddr, pBB);    // non-explicit label
            return true;            // wasn't a label, but already parsed
        }
        // A non overlapping, incomplete entry is in the map.
        return false;
    }
}

// Return true if there is an incomplete BB already at this address
/*==============================================================================
 * FUNCTION:        Cfg::isIncomplete
 * OVERVIEW:        Return true if given address is the start of an incomplete
 *                    basic block
 * PARAMETERS:      uAddr: Address to look up
 * RETURNS:         True if uAddr starts an incomplete BB
 *============================================================================*/
bool Cfg::isIncomplete(ADDRESS uAddr)
{
    MAPBB::iterator mi = m_mapBB.find(uAddr);
    if (mi == m_mapBB.end())
        // No entry at all
        return false;
    // Else, there is a BB there. If it's incomplete, return true
    PBB pBB = (*mi).second;
    return pBB->m_bIncomplete;
}

/*==============================================================================
 * FUNCTION:        Cfg::sortByAddress
 * OVERVIEW:        Sorts the BBs in a cfg by first address. Just makes it more
 *                  convenient to read when BBs are iterated.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/

void Cfg::sortByAddress()
{
    m_listBB.sort(BasicBlock::lessAddress);
}

/*==============================================================================
 * FUNCTION:        Cfg::sortByFirstDFT
 * OVERVIEW:        Sorts the BBs in a cfg by their first DFT numbers.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::sortByFirstDFT()
{
#ifndef WIN32
    m_listBB.sort(BasicBlock::lessFirstDFT);
#else
	updateVectorBB();
    for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end(); it++)
		m_vectorBB[(*it)->m_DFTfirst-1] = *it;
	m_listBB.clear();
	for (int i = 0; i < m_vectorBB.size(); i++)
		m_listBB.push_back(m_vectorBB[i]);
#endif
}

/*==============================================================================
 * FUNCTION:        Cfg::sortByLastDFT
 * OVERVIEW:        Sorts the BBs in a cfg by their last DFT numbers.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::sortByLastDFT()
{
#ifndef WIN32
    m_listBB.sort(BasicBlock::lessLastDFT);
#else
	updateVectorBB();
    for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end(); it++)
		m_vectorBB[(*it)->m_DFTlast-1] = *it;
	m_listBB.clear();
	for (int i = 0; i < m_vectorBB.size(); i++)
		m_listBB.push_back(m_vectorBB[i]);
#endif
}

/*==============================================================================
 * FUNCTION:        Cfg::updateVectorBB
 * OVERVIEW:        Updates m_vectorBB to m_listBB
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::updateVectorBB()
{
    m_vectorBB.clear();
    for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end(); it++)
        m_vectorBB.push_back(*it);
}


/*==============================================================================
 * FUNCTION:        Cfg::wellFormCfg
 * OVERVIEW:        Checks that all BBs are complete, and all out edges are
 *                  valid. However, ADDRESSes that are interprocedural out edges
 *                  are not checked or changed.
 * PARAMETERS:      <none>
 * RETURNS:         transformation was successful
 *============================================================================*/
bool Cfg::wellFormCfg()
{
    m_bWellFormed = true;
    for (BB_IT it = m_listBB.begin(); it != m_listBB.end(); it++)
    {
        // it iterates through all BBs in the list
        // Check that it's complete
        if ((*it)->m_bIncomplete)
        {
            m_bWellFormed = false;
            MAPBB::iterator itm;
            for (itm = m_mapBB.begin(); itm != m_mapBB.end(); itm++)
                if ((*itm).second == *it) break;
            if (itm == m_mapBB.end())
                std::cerr << "WellFormCfg: incomplete BB not even in map!\n";
            else
            {
                std::cerr << "WellFormCfg: BB with native address ";
                std::cerr << std::hex << (*itm).first << " is incomplete\n";
            }
        }
        else
        {
            // Complete. Test the out edges
            assert((int)(*it)->m_OutEdges.size() == (*it)->m_iNumOutEdges);
            for (int i=0; i < (*it)->m_iNumOutEdges; i++) {
                // check if address is interprocedural
//              if ((*it)->m_OutEdgeInterProc[i] == false)
                {
                    // i iterates through the outedges in the BB *it
                    PBB pBB = (*it)->m_OutEdges[i];

                    // Check that the out edge has been written (i.e. nonzero)
                    if (pBB == 0) {
                        m_bWellFormed = false;  // At least one problem
                        ADDRESS addr = (*it)->getLowAddr();
                        std::cerr << "WellFormCfg: BB with native address ";
                        std::cerr << std::hex << addr << " is missing outedge ";
                        std::cerr << i << std::endl;
                    }
                    else {
                        // Check that there is a corresponding in edge from the
                        // child to here
                        std::vector<PBB>::iterator ii;
                        for (ii=pBB->m_InEdges.begin();
                                ii != pBB->m_InEdges.end(); ii++)
                            if (*ii == *it) break;
                        if (ii == pBB->m_InEdges.end()) {
                            std::cerr << "WellFormCfg: No in edge to BB at " << std::hex;
                            std::cerr << (*it)->getLowAddr();
                            std::cerr << " from successor BB at ";
                            std::cerr << pBB->getLowAddr() << std::endl;
                            m_bWellFormed = false;  // At least one problem
                        }
                    }
                }
            }
            // Also check that each in edge has a corresponding out edge to here
            // (could have an extra in-edge, for example)
            assert((int)(*it)->m_InEdges.size() == (*it)->m_iNumInEdges);
            std::vector<PBB>::iterator ii;
            for (ii = (*it)->m_InEdges.begin(); ii != (*it)->m_InEdges.end();
                    ii++) {
                std::vector<PBB>::iterator oo;
                for (oo=(*ii)->m_OutEdges.begin();
                        oo != (*ii)->m_OutEdges.end(); oo++)
                    if (*oo == *it) break;
                if (oo == (*ii)->m_OutEdges.end()) {
                    std::cerr << "WellFormCfg: No out edge to BB at " << std::hex;
                    std::cerr << (*it)->getLowAddr();
                    std::cerr << " from predecessor BB at ";
                    std::cerr << (*ii)->getLowAddr() << std::endl;
                    m_bWellFormed = false;  // At least one problem
                }
            }
        }
    }
    return m_bWellFormed;
}

/*==============================================================================
 * FUNCTION:        Cfg::mergeBBs
 * OVERVIEW:        
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
bool Cfg::mergeBBs( PBB pb1, PBB pb2)
{
    // Can only merge if pb1 has only one outedge to pb2, and pb2 has
    // only one in-edge, from pb1. This can only be done after the
    // in-edges are done, which can only be done on a well formed CFG.
    if (!m_bWellFormed) return false;
    if (pb1->m_iNumOutEdges != 1) return false;
    if (pb2->m_iNumInEdges != 1) return false;
    if (pb1->m_OutEdges[0] != pb2) return false;
    if (pb2->m_InEdges[0] != pb1) return false;

    // Merge them! We remove pb1 rather than pb2, since this is also
    // what is needed for many optimisations, e.g. jump to jump.
    completeMerge(pb1, pb2, true);
    return true;
}

/*==============================================================================
 * FUNCTION:        Cfg::completeMerge
 * OVERVIEW:        Complete the merge of two BBs by adjusting in and out edges.
 *                  If bDelete is true, delete pb1
 * PARAMETERS:      pb1, pb2: pointers to the two BBs to merge
 *                  bDelete: if true, pb1 is deleted as well
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::completeMerge(PBB pb1, PBB pb2, bool bDelete = false)
{
    // First we replace all of pb1's predecessors' out edges that used to
    // point to pb1 (usually only one of these) with pb2
    for (int i=0; i < pb1->m_iNumInEdges; i++)
    {
        PBB pPred = pb1->m_InEdges[i];
        for (int j=0; j < pPred->m_iNumOutEdges; j++)
        {
            if (pPred->m_OutEdges[j] == pb1)
                pPred->m_OutEdges[j] = pb2;
        }
    }

    // Now we replace pb2's in edges by pb1's inedges
    pb2->m_InEdges = pb1->m_InEdges;
    pb2->m_iNumInEdges = pb1->m_iNumInEdges;

    if (bDelete) {
        // Finally, we delete pb1 from the BB list. Note: remove(pb1)
        // should also work, but it would involve member comparison
        // (not implemented), and also would attempt to remove ALL
        // elements of the list with this value (so it has to
        // search the whole list, instead of an average of half the
        // list as we have here).
        for (BB_IT it = m_listBB.begin(); it != m_listBB.end(); it++)
        {
            if (*it == pb1)
            {
                m_listBB.erase(it);
                break;
            }
        }
    }
}

/*==============================================================================
 * FUNCTION:        Cfg::joinBB
 * OVERVIEW:        Amalgamate the RTLs for pb1 and pb2, and place the result
 *                    into pb2
 * PARAMETERS:      pb1, pb2: pointers to the BBs to join
 * ASSUMES:         Fallthrough of *pb1 is *pb2
 * RETURNS:         True if successful
 *============================================================================*/
bool Cfg::joinBB(PBB pb1, PBB pb2)
{
    // Ensure that the fallthrough case for pb1 is pb2
    std::vector<PBB>& v = pb1->getOutEdges();
    if (v.size() != 2 || v[1] != pb2)
        return false;
    // Prepend the RTLs for pb1 to those of pb2. Since they will be pushed to
    // the front of pb2, push them in reverse order
    std::list<RTL*>::reverse_iterator it;
    for (it = pb1->m_pRtls->rbegin(); it != pb1->m_pRtls->rend(); it++) {
        pb2->m_pRtls->push_front(*it);
    }
    completeMerge(pb1, pb2);                // Mash them together
    // pb1 no longer needed. Remove it from the list of BBs
    // This will also delete *pb1. It will be a shallow delete, but that's good
    // because we only did shallow copies to *pb2
    BB_IT bbit = std::find(m_listBB.begin(), m_listBB.end(), pb1);
    m_listBB.erase(bbit);
    return true;
}

#if 0           // Old code; doesn't work. May have good ideas, so keep here
bool Cfg::compressCfg()
{
    if (!m_bWellFormed) return false;
    for (BB_IT it = m_listBB.begin(); it != m_listBB.end(); it++)
    {
        for (int i=0; i < (*it)->m_iNumOutEdges; i++)
        {
            PBB pSucc = (*it)->m_OutEdges[i];
            // We have a BB (*it) and its successor (*pSucc).
            // Attempt the three optimisations on it.
            // 1) Jumps to jumps
            if (((*it)->m_nodeType == ONEWAY) && 
                (pSucc->m_nodeType == ONEWAY))
            {
                completeMerge(*it, pSucc, true);
            }
            // 2) Jump to next BB
            else if(((*it)->m_nodeType == ONEWAY) &&
                ((*it)->m_OutEdges[0] == pSucc) && pSucc->m_iNumInEdges == 1)
            {
                completeMerge(*it, pSucc, true);
            }
            // 3) Attempt ordinary merge
            else mergeBBs(*it, pSucc);
        }
    }
    return true;
}
#endif

/*==============================================================================
 * FUNCTION:        Cfg::compressCfg
 * OVERVIEW:        Compress the CFG. For now, it only removes BBs that are
 *                    just branches
 * PARAMETERS:      <none>
 * RETURNS:         False if not well formed; true otherwise
 *============================================================================*/
bool Cfg::compressCfg()
{
    // must be well formed
    if (!m_bWellFormed) return false;

#if 1
    // replace never taken branches with oneways
    bool change = true;
    while (change && !Boomerang::get()->noBranchSimplify) {
        change = false;
        for (BB_IT it = m_listBB.begin(); it != m_listBB.end(); it++) 
            if ((*it)->getType() == TWOWAY) {
                PBB bb = *it;
                PBB prev = NULL;
                while (bb->getInEdges().size() == 1) {
                    prev = bb;
                    bb = bb->getInEdges()[0];
                    if (bb->getType() == TWOWAY) break;
                }
                if (bb->getType() != TWOWAY || bb == *it)
                    continue;
                HLJcond *jcond = dynamic_cast<HLJcond*>((*it)->m_pRtls->back());
                HLJcond *prior = dynamic_cast<HLJcond*>(bb->m_pRtls->back());
                assert(jcond && prior);
                std::set<Statement*> live;
                jcond->getLiveIn(live);
                bool allLive = true;
                for (std::set<Statement*>::iterator sit = prior->getUses()->begin();
                     sit != prior->getUses()->end(); sit++) 
                    if (live.find(*sit) == live.end()) { allLive = false; break; }
                if (!allLive) continue;
                Exp *priorcond = prior->getCondExpr()->clone();
                Exp *revpriorcond = new Unary(opNot, prior->getCondExpr()->clone());
                Exp *cond = jcond->getCondExpr();
                revpriorcond = revpriorcond->simplify();
                if (bb->getOutEdges()[0] != prev) {
                    Exp *tmp = priorcond;
                    priorcond = revpriorcond;
                    revpriorcond = tmp;
                }
                std::cerr << "consider branch: ";
                cond->print(std::cerr);
                std::cerr << " inside ";
                priorcond->print(std::cerr);
                std::cerr << std::endl;
                bool alwaysTrue = (*cond == *priorcond);
                bool alwaysFalse = (*cond == *revpriorcond);
                // consider some other possibilities
                if ((cond->getOper() == opLess && 
                     priorcond->getOper() == opLessEq) ||
                    (cond->getOper() == opGtr && 
                     priorcond->getOper() == opGtrEq) ||
                    (cond->getOper() == opLessUns && 
                     priorcond->getOper() == opLessEqUns) ||
                    (cond->getOper() == opGtrUns && 
                     priorcond->getOper() == opGtrEqUns)) {
                    if (*cond->getSubExp1() == *priorcond->getSubExp1() &&
                        *cond->getSubExp2() == *priorcond->getSubExp2())
                        alwaysTrue = true;
                }
                assert(!(alwaysTrue && alwaysFalse));
                if (alwaysTrue) {
                    std::cerr << "found always true branch: ";
                    jcond->print(std::cerr, false);
                    std::cerr << std::endl;
                    (*it)->m_nodeType = ONEWAY;
                    (*it)->deleteEdge((*it)->m_OutEdges[1]);
                    *(--(*it)->m_pRtls->end()) = new HLJump(jcond->getAddress(),
                        (*it)->m_OutEdges[0]->getLowAddr());
                    delete jcond;
                    change = true;
                } else if (alwaysFalse) {
                    std::cerr << "found always false branch: ";
                    jcond->print(std::cerr, false);
                    std::cerr << std::endl;
                    (*it)->m_nodeType = ONEWAY;
                    (*it)->deleteEdge((*it)->m_OutEdges[0]);
                    *(--(*it)->m_pRtls->end()) = new HLJump(jcond->getAddress(),
                        (*it)->m_OutEdges[0]->getLowAddr());
                    delete jcond;
                    change = true;
                }
            }
    }
#endif
    
    // Find A -> J -> B  where J is a BB that is only a jump
    // Then A -> B
    for (BB_IT it = m_listBB.begin(); it != m_listBB.end(); it++)
    {
        for (std::vector<PBB>::iterator it1 = (*it)->m_OutEdges.begin();
          it1 != (*it)->m_OutEdges.end(); it1++) {
            PBB pSucc = (*it1);         // Pointer to J
            PBB bb = (*it);             // Pointer to A
            if (pSucc->m_InEdges.size()==1 && pSucc->m_OutEdges.size()==1 &&
              pSucc->m_pRtls->size()==1 &&
              pSucc->m_pRtls->front()->getKind()==JUMP_RTL) {
                // Found an out-edge to an only-jump BB
                /* cout << "outedge to jump detected at " << std::hex <<
                    bb->getLowAddr() << " to ";
                    cout << pSucc->getLowAddr() << " to " <<
                    pSucc->m_OutEdges.front()->getLowAddr() << dec << std::endl; */
                // Point this outedge of A to the dest of the jump (B)
                *it1=pSucc->m_OutEdges.front();
                // Now pSucc still points to J; *it1 points to B.
                // Almost certainly, we will need a jump in the low level C
                // that may be generated. Also force a label for B
                bb->m_bJumpReqd = true;
                setLabel(*it1);
                // Find the in-edge from B to J; replace this with an in-edge
                // to A
                std::vector<PBB>::iterator it2;
                for (it2 = (*it1)->m_InEdges.begin();
                  it2 != (*it1)->m_InEdges.end(); it2++) {
                    if (*it2==pSucc)
                        *it2 = bb;          // Point to A
                }
                // Remove the in-edge from J to A. First find the in-edge
                for (it2 = pSucc->m_InEdges.begin();
                  it2 != pSucc->m_InEdges.end(); it2++) {
                    if (*it2 == bb)
                        break;
                }
                assert(it2 != pSucc->m_InEdges.end());
                pSucc->deleteInEdge(it2);
                // If nothing else uses this BB (J), remove it from the CFG
                if (pSucc->m_iNumInEdges == 0) {
                    for (BB_IT it3 = m_listBB.begin(); it3 != m_listBB.end();
                      it3++) {
                        if (*it3==pSucc) {
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

/*==============================================================================
 * FUNCTION:        Cfg::unTraverse
 * OVERVIEW:        Reset all the traversed flags.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::unTraverse()
{
    for (BB_IT it = m_listBB.begin(); it != m_listBB.end(); it++)
    {
        (*it)->m_iTraversed = false;
    }
}
    
/*==============================================================================
 * FUNCTION:        Cfg::establishDFTOrder
 * OVERVIEW:        Given a well-formed cfg graph, a partial ordering is
 *                  established between the nodes. The ordering is based on the
 *                  final visit to each node during a depth first traversal such
 *                  that if node n1 was visited for the last time before node n2
 *                  was visited for the last time, n1 will be less than n2. The
 *                  return value indicates if all nodes where ordered. This will
 *                  not be the case for incomplete CFGs (e.g. switch table not
 *                  completely recognised) or where there are nodes unreachable
 *                  from the entry node.
 * PARAMETERS:      <none>
 * RETURNS:         all nodes where ordered
 *============================================================================*/
bool Cfg::establishDFTOrder()
{
    // Must be well formed.
    if (!m_bWellFormed) return false;

    // Reset all the traversed flags
    unTraverse();

    int first = 0;
    int last = 0;
    unsigned numTraversed;

    if (checkEntryBB()) return false;

    numTraversed = entryBB->DFTOrder(first,last);

    return numTraversed == m_listBB.size();
}

PBB Cfg::findRetNode()
{
    PBB retNode = NULL;
    for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end(); 
	 it++) {
        if ((*it)->getOutEdges().size() == 0 && (*it)->getType() == RET)
            retNode = *it;
    }
    return retNode;
}

/*==============================================================================
 * FUNCTION:        Cfg::establishRevDFTOrder
 * OVERVIEW:        Performs establishDFTOrder on the reverse (flip) of the 
 *          graph, assumes: establishDFTOrder has already been called
 * PARAMETERS:      <none>
 * RETURNS:         all nodes where ordered
 *============================================================================*/
bool Cfg::establishRevDFTOrder()
{
    // Must be well formed.
    if (!m_bWellFormed) return false;

    // WAS: sort by last dfs and grab the exit node
	// Why?  This does not seem like a the best way
	// What we need is the ret node, so let's find it.
	// If the CFG has more than one ret node then
	// it needs to be fixed.
    //sortByLastDFT();

	PBB retNode = findRetNode();

	if (retNode == NULL) return false;

    // Reset all the traversed flags
    unTraverse();

    int first = 0;
    int last = 0;
    unsigned numTraversed;

    numTraversed = retNode->RevDFTOrder(first,last);

    return numTraversed == m_listBB.size();
}

/*==============================================================================
 * FUNCTION:        Cfg::isWellFormed
 * OVERVIEW:        
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
bool Cfg::isWellFormed()
{
    return m_bWellFormed;
}

/*==============================================================================
 * FUNCTION:        Cfg::isOrphan
 * OVERVIEW:        
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
bool Cfg::isOrphan(ADDRESS uAddr)
{
    MAPBB::iterator mi = m_mapBB.find(uAddr);
    if (mi == m_mapBB.end())
        // No entry at all
        return false;
    // Return true if the first RTL at this address has an address set to 0
    PBB pBB = (*mi).second;
    // If it's incomplete, it can't be an orphan
    if (pBB->m_bIncomplete) return false;
    return pBB->m_pRtls->front()->getAddress() == 0;
}

/*==============================================================================
 * FUNCTION:        Cfg::pbbToIndex 
 * OVERVIEW:        Return an index for the given PBB
 * NOTE:            Linear search: O(N) complexity
 * PARAMETERS:      <none>
 * RETURNS:         Index, or -1 for unknown PBB
 *============================================================================*/
int Cfg::pbbToIndex (PBB pBB) {
    BB_IT it = m_listBB.begin();
    int i = 0;
    while (it != m_listBB.end()) {
        if (*it++ == pBB) return i;
        i++;
    }
    return -1;
}

/*==============================================================================
 * FUNCTION:        Cfg::getCoverage
 * OVERVIEW:        
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
unsigned Cfg::getCoverage()
{
    // Start with the extra coverage from nops, switch tables, and the like
    unsigned uTotal = m_uExtraCover;
    for (BB_IT it=m_listBB.begin(); it != m_listBB.end(); it++)
    {
        uTotal += (*it)->getCoverage();
    }
    return uTotal;
}

/*==============================================================================
 * FUNCTION:        Cfg::addCall
 * OVERVIEW:        Add a call to the set of calls within this procedure.
 * PARAMETERS:      call - a call instruction
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::addCall(HLCall* call)
{
    callSites.insert(call);
}

/*==============================================================================
 * FUNCTION:        Cfg::getCalls
 * OVERVIEW:        Get the set of calls within this procedure.
 * PARAMETERS:      <none>
 * RETURNS:         the set of calls within this procedure
 *============================================================================*/
std::set<HLCall*>& Cfg::getCalls()
{
    return callSites;
}

/*==============================================================================
 * FUNCTION:        Cfg::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::searchAndReplace(Exp* search, Exp* replace)
{
    for (BB_IT bb_it = m_listBB.begin(); bb_it != m_listBB.end(); bb_it++) {

        std::list<RTL*>& rtls = *((*bb_it)->getRTLs());
        for (std::list<RTL*>::iterator rtl_it = rtls.begin(); rtl_it != rtls.end();
          rtl_it++) {

            RTL& rtl = **rtl_it;
            rtl.searchAndReplace(search,replace);
        }
	if ((*bb_it)->getType() == RET && (*bb_it)->m_returnVal) {
            bool change;
	    (*bb_it)->m_returnVal = (*bb_it)->m_returnVal->searchReplaceAll(
                search, replace, change);
	}
    }
}

/*==============================================================================
 * FUNCTION:        Cfg::computeDominators
 * OVERVIEW:        computes the dominators of each BB 
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::computeDominators() {
    sortByLastDFT();

/*    int index=0;
    for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end(); it++) 
    {       
        (*it)->m_index = index++;
        (*it)->dominators.set();
    }
    
    if (checkEntryBB()) return;
    entryBB->dominators.reset();
    entryBB->dominators.set(entryBB->m_index);

    bool change = true;

    while(change) 
    {   
        change=false;
        for (std::list<PBB>::iterator it1 = m_listBB.begin(); it1 != m_listBB.end(); it1++)
            if ((*it1) != entryBB)
            {
                BITSET intersect,newd;
                intersect.set();
                newd.reset();
                newd.set((*it1)->m_index);
                for (std::vector<PBB>::iterator in = (*it1)->m_InEdges.begin(); in != (*it1)->m_InEdges.end(); in++)
                    intersect &= (*in)->dominators;
                newd |= intersect;
                if (newd != (*it1)->dominators) {               
                    change=true;
                    (*it1)->dominators = newd;
                }
            }
    }

    for (std::list<PBB>::iterator it2 = m_listBB.begin(); it2 != m_listBB.end(); it2++) 
    {
        (*it2)->m_dominatedBy.clear();
        for (std::list<PBB>::iterator it3 = m_listBB.begin(); it3 != m_listBB.end(); it3++)
            if ((*it2)->dominators.test((*it3)->m_index))
                (*it2)->m_dominatedBy.push_back(*it3);
    }*/
}

#if 0
/*==============================================================================
 * FUNCTION:        Cfg::computePostDominators
 * OVERVIEW:        computes the dominators of each BB 
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::computePostDominators() {
    sortByLastDFS();

    PBB retnode = NULL;

    int index=0;
    for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end(); it++) 
    {       
        (*it)->m_index = index++;
        (*it)->postdominators.set();
        if ((*it)->m_OutEdges.empty()) 
        {
            if (retnode!=NULL)
                cout << "eek, more than one ret node detected" << std::endl;
            retnode = *it;
        }
    }
    
    retnode->postdominators.reset();
    retnode->postdominators.set(retnode->m_index);

    bool change = true;

    while(change) 
    {   
        change=false;
        for (std::list<PBB>::iterator it1 = m_listBB.begin(); it1 != m_listBB.end(); it1++)
            if ((*it1) != retnode)
            {
                BITSET intersect,newd;
                intersect.set();
                newd.reset();
                newd.set((*it1)->m_index);
                for (std::vector<PBB>::iterator in = (*it1)->m_OutEdges.begin(); in != (*it1)->m_OutEdges.end(); in++)
                    intersect &= (*in)->postdominators;
                newd |= intersect;
                if (newd != (*it1)->postdominators) {               
                    change=true;
                    (*it1)->postdominators = newd;
                }
            }
    }

    for (std::list<PBB>::iterator it2 = m_listBB.begin(); it2 != m_listBB.end(); it2++) 
    {
        (*it2)->m_postdominatedBy.clear();
        for (std::list<PBB>::iterator it3 = m_listBB.begin(); it3 != m_listBB.end(); it3++)
            if ((*it2)->postdominators.test((*it3)->m_index))
                (*it2)->m_postdominatedBy.push_back(*it3);
    }
}
#endif

/*==============================================================================
 * FUNCTION:        Cfg::computeDataflow
 * OVERVIEW:        Computes the liveness/use information for every bb.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::computeDataflow()
{
    for (std::list<PBB>::iterator it = m_listBB.begin(); 
        it != m_listBB.end(); it++)
	    (*it)->liveout.clear();
    updateLiveness();
}

void Cfg::updateLiveness()
{
    bool change = true;
    while(change) {
        change = false;
        for (std::list<PBB>::iterator it = m_listBB.begin(); 
             it != m_listBB.end(); it++) {
            std::set<Statement*> out;
            (*it)->calcLiveOut(out);
            if (out != (*it)->liveout) {
                (*it)->liveout.clear();
		if ((*it)->getType() == RET)
		    liveout.clear();
                for (std::set<Statement*>::iterator it1 = out.begin();
                     it1 != out.end(); it1++) {
                    (*it)->liveout.insert(*it1);
		    if ((*it)->getType() == RET)
		        liveout.insert(*it1);
                }
                change = true;
            }
        }
    }
}

/*==============================================================================
 * FUNCTION:    delete_lrtls
 * OVERVIEW:    "deep" delete for a list of pointers to RTLs
 * PARAMETERS:  pLrtl - the list
 * RETURNS:     <none>
 *============================================================================*/
void delete_lrtls(std::list<RTL*>* pLrtl)
{
    std::list<RTL*>::iterator it;
    for (it = pLrtl->begin(); it != pLrtl->end(); it++) {
        delete (*it);
    }
}

/*==============================================================================
 * FUNCTION:    erase_lrtls
 * OVERVIEW:    "deep" erase for a list of pointers to RTLs
 * PARAMETERS:  pLrtls - the list
 *              begin - iterator to first (inclusive) item to delete
 *              end - iterator to last (exclusive) item to delete
 * RETURNS:     <none>
 *============================================================================*/
void erase_lrtls(std::list<RTL*>* pLrtl, std::list<RTL*>::iterator begin,
    std::list<RTL*>::iterator end)
{
    std::list<RTL*>::iterator it;
    for (it = begin; it != end; it++) {
        delete (*it);
    }
    pLrtl->erase(begin, end);
}

/*==============================================================================
 * FUNCTION:        Cfg::setLabel
 * OVERVIEW:        Sets a flag indicating that this BB has a label, in the
 *                  sense that a label is required in the translated source code
 * PARAMETERS:      pBB: Pointer to the BB whose label will be set
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::setLabel(PBB pBB) {
    if (pBB->m_iLabelNum == 0)
        pBB->m_iLabelNum = ++lastLabel;
}

/*==============================================================================
 * FUNCTION:        Cfg::addNewOutEdge
 * OVERVIEW:        Append a new out-edge from the given BB to the other
 *                    given BB
 *                  Needed for example when converting a one-way BB to a two-
 *                  way BB
 * NOTE:            Use BasicBlock::setOutEdge() for the common case where
 *                    an existing out edge is merely changed
 * NOTE ALSO:       Use Cfg::addOutEdge for ordinary BB creation; this is for
 *                    unusual cfg manipulation (e.g. analysis.cc)
 * PARAMETERS:      pFromBB: pointer to the BB getting the new out edge
 *                  pNewOutEdge: pointer to BB that will be the new successor
 * SIDE EFFECTS:    Increments m_iNumOutEdges
 * RETURNS:         <nothing>
 *============================================================================*/
void Cfg::addNewOutEdge(PBB pFromBB, PBB pNewOutEdge)
{
    pFromBB->m_OutEdges.push_back(pNewOutEdge);
    pFromBB->m_iNumOutEdges++;
    // Since this is a new out-edge, set the "jump required" flag
    pFromBB->m_bJumpReqd = true;
    // Make sure that there is a label there
    setLabel(pNewOutEdge);
}

// serialize the CFG
bool Cfg::serialize(std::ostream &ouf, int &len)
{
	std::streampos st = ouf.tellp();

	saveFID(ouf, FID_CFG_WELLFORMED);
	saveValue(ouf, m_bWellFormed);

	// save BBs
	int n = 0;
	for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end(); it++)
		(*it)->m_nindex = n++;
	for (
#ifndef WIN32
		std::list<PBB>::iterator 
#endif
		it = m_listBB.begin(); it != m_listBB.end(); it++) {
		saveFID(ouf, FID_CFG_BB);
		std::streampos pos = ouf.tellp();
		int len = -1;
		saveLen(ouf, -1, true);
		std::streampos posa = ouf.tellp();

		assert((*it)->serialize(ouf, len));

		std::streampos now = ouf.tellp();
		assert((int)(now - posa) == len);
		ouf.seekp(pos);
		saveLen(ouf, len, true);
		ouf.seekp(now);
	}

	saveFID(ouf, FID_CFG_ENTRYBB);
	saveValue(ouf, entryBB->m_nindex);

	saveFID(ouf, FID_CFG_END);
	saveLen(ouf, 0);

	len = ouf.tellp() - st;
	return true;
}

// deserialize a CFG
bool Cfg::deserialize(std::istream &inf)
{
	int fid;
	int entry_idx = -1;

	while ((fid = loadFID(inf)) != -1 && fid != FID_CFG_END) {
		switch (fid) {
			case FID_CFG_WELLFORMED:
				loadValue(inf, m_bWellFormed);
				break;
			case FID_CFG_BB:
				{
					int len = loadLen(inf);
					std::streampos pos = inf.tellg();
					PBB bb = new BasicBlock();
					assert(bb);					
					assert(bb->deserialize(inf));
					assert((int)(inf.tellg() - pos) == len);
					m_listBB.push_back(bb);
				}
				break;
			case FID_CFG_ENTRYBB:
				{
					loadValue(inf, entry_idx);
				}
				break;
    /*
	TODO:

    MAPBB m_mapBB;
    unsigned m_uExtraCover;
    std::set<HLCall*> callSites;
    int lastLabel;
	*/
			default:
				skipFID(inf, fid);
		}
	}
	assert(loadLen(inf) == 0);

	// resolve the call graph
	std::vector<PBB> ntobb;
	int n = 0;
	entryBB = NULL;
	for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end(); it++) {
		(*it)->getInEdges().clear();
		(*it)->getOutEdges().clear();
		if (n == entry_idx)
			entryBB = *it;
		ntobb.push_back(*it);
		assert(ntobb[n] == *it);
		m_mapBB[(*it)->getLowAddr()] = *it;
		n++;
	}
	assert(entryBB);

	for (
#ifndef WIN32
		std::list<PBB>::iterator 
#endif
		it = m_listBB.begin(); it != m_listBB.end(); it++) {
		for (std::vector<int>::iterator nit = (*it)->m_nOutEdges.begin(); nit != (*it)->m_nOutEdges.end(); nit++) {
			PBB dest = ntobb[*nit];
			(*it)->getOutEdges().push_back(dest);
			dest->getInEdges().push_back(*it);
		}
	}

	for (
#ifndef WIN32
		std::list<PBB>::iterator 
#endif
		it = m_listBB.begin(); it != m_listBB.end(); it++) {
		(*it)->m_iNumInEdges = (*it)->getInEdges().size();
		(*it)->m_iNumOutEdges = (*it)->getOutEdges().size();
	}

	return true;
}


void Cfg::makeCallRet(PBB head, Proc *p)
{
	head->m_nodeType = CALL;
	HLCall *call = new HLCall(NO_ADDRESS);
	call->setDest(p->getNativeAddress());
	if (head->m_pRtls)
		delete head->m_pRtls;
	head->m_pRtls = new std::list<RTL*>();
	head->m_pRtls->push_back(call);
	PBB pret = new BasicBlock();
	pret->m_nodeType = RET;
	HLReturn *ret = new HLReturn(NO_ADDRESS);
	pret->m_pRtls = new std::list<RTL*>();
	pret->m_pRtls->push_back(ret);
	head->m_OutEdges.clear();
	head->m_OutEdges.push_back(pret);
	pret->m_InEdges.push_back(head);
	m_listBB.push_back(pret);

	// find orphans, delete em
}

void Cfg::simplify()
{
	for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end(); it++) 
		(*it)->simplify();
}

// print this cfg, mainly for debugging
void Cfg::print(std::ostream &out, bool withDF) {
    for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end(); it++) 
        (*it)->print(out, withDF);
    out << "cfg liveout: ";
    for (std::set<Statement*>::iterator it = liveout.begin(); 
         it != liveout.end(); it++) {
        (*it)->printAsUse(out);
        out << ", ";
    }
    out << std::endl;
}

void Cfg::setReturnVal(Exp *e)
{
    bool onlyOneReturnBB = true;
    for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end(); it++) 
        if ((*it)->getType() == RET) {
		assert(onlyOneReturnBB);
		(*it)->setReturnVal(e);
		onlyOneReturnBB = false;
	}
}

Exp *Cfg::getReturnVal()
{
    Exp *e = NULL;
    bool onlyOneReturnBB = true;
    for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end(); it++) 
        if ((*it)->getType() == RET) {
		assert(onlyOneReturnBB);
		e = (*it)->getReturnVal();
		onlyOneReturnBB = false;
	}
    return e;
}

void Cfg::setTimeStamps()
{
    // set DFS tag
    for (std::list<PBB>::iterator it = m_listBB.begin(); it != m_listBB.end();
         it++) (*it)->traversed = DFS_TAG;

    // set the parenthesis for the nodes as well as setting
    // the post-order ordering between the nodes
    int time = 1;
    Ordering.clear();
    entryBB->setLoopStamps(time, Ordering);

    // set the reverse parenthesis for the nodes
    time = 1;
    entryBB->setRevLoopStamps(time);

    PBB retNode = findRetNode();
    assert(retNode);
    revOrdering.clear();
    retNode->setRevOrder(revOrdering);
}

// Finds the common post dominator of the current immediate post dominator
// and its successor's immediate post dominator
PBB Cfg::commonPDom(PBB curImmPDom, PBB succImmPDom)
{
    if (!curImmPDom)
        return succImmPDom;
    if (!succImmPDom)
        return curImmPDom;

    while (curImmPDom && succImmPDom && (curImmPDom != succImmPDom))
        if (curImmPDom->revOrd > succImmPDom->revOrd)
            succImmPDom = succImmPDom->immPDom;
        else
            curImmPDom = curImmPDom->immPDom;

    return curImmPDom;
}

/* Finds the immediate post dominator of each node in the graph PROC->cfg.
 * Adapted version of the dominators algorithm by Hecht and Ullman; finds
 * immediate post dominators only.
 * Note: graph should be reducible */
void Cfg::findImmedPDom()
{
    PBB curNode, succNode;	// the current Node and its successor

    // traverse the nodes in order (i.e from the bottom up)
    for (int i = revOrdering.size() - 1; i >= 0; i--) {
        curNode = revOrdering[i];
        std::vector<PBB> &oEdges = curNode->getOutEdges();
        for (unsigned int j = 0; j < oEdges.size(); j++) {
            succNode = oEdges[j];
	    if (succNode->revOrd > curNode->revOrd)
	        curNode->immPDom = commonPDom(curNode->immPDom, succNode);
	}
    }

    // make a second pass but consider the original CFG ordering this time
    for (unsigned int i = 0; i < Ordering.size(); i++) {
        curNode = Ordering[i];
        std::vector<PBB> &oEdges = curNode->getOutEdges();
        if (oEdges.size() > 1)
            for (unsigned int j = 0; j < oEdges.size(); j++) {
                succNode = oEdges[j];
	        curNode->immPDom = commonPDom(curNode->immPDom, succNode);
	    }
    }

    // one final pass to fix up nodes involved in a loop
    for (unsigned int i = 0; i < Ordering.size(); i++) {
        curNode = Ordering[i];
        std::vector<PBB> &oEdges = curNode->getOutEdges();
        if (oEdges.size() > 1)
            for (unsigned int j = 0; j < oEdges.size(); j++) {
                succNode = oEdges[j];	
                if (curNode->hasBackEdgeTo(succNode) && 
                    curNode->getOutEdges().size() > 1 &&
                    succNode->immPDom->ord < curNode->immPDom->ord)
                    curNode->immPDom = 
                        commonPDom(succNode->immPDom, curNode->immPDom);
                else
                    curNode->immPDom = 
                        commonPDom(curNode->immPDom, succNode);
            }
    }
}

// Structures all conditional headers (i.e. nodes with more than one outedge)
void Cfg::structConds()
{
    // Process the nodes in order
    for (unsigned int i = 0; i < Ordering.size(); i++) {
        PBB curNode = Ordering[i];

        // does the current node have more than one out edge?
        if (curNode->getOutEdges().size() > 1) {
	    // if the current conditional header is a two way node and has a 
            // back edge, then it won't have a follow
            if (curNode->hasBackEdge() && curNode->getType() == TWOWAY) {
                curNode->setStructType(Cond);
                continue;
            }
		
            // set the follow of a node to be its immediate post dominator
            curNode->setCondFollow(curNode->immPDom);

            // set the structured type of this node
            curNode->setStructType(Cond);

            // if this is an nway header, then we have to tag each of the nodes
            // within the body of the nway subgraph
            if (curNode->getCondType() == Case)
                curNode->setCaseHead(curNode,curNode->getCondFollow());
        }
    }
}

// Pre: The loop induced by (head,latch) has already had all its member nodes 
//      tagged
// Post: The type of loop has been deduced
void Cfg::determineLoopType(PBB header, bool* &loopNodes)
{
    assert(header->getLatchNode());

    // if the latch node is a two way node then this must be a post tested 
    // loop
    if (header->getLatchNode()->getType() == TWOWAY) {
        header->setLoopType(PostTested);

        // if the head of the loop is a two way node and the loop spans more 
        // than one block  then it must also be a conditional header
        if (header->getType() == TWOWAY && header != header->getLatchNode())
            header->setStructType(LoopCond);
    }

    // otherwise it is either a pretested or endless loop
    else if (header->getType() == TWOWAY) {
        // if the header is a two way node then it must have a conditional 
        // follow (since it can't have any backedges leading from it). If this 
        // follow is within the loop then this must be an endless loop
        assert(header->getCondFollow());
        if (loopNodes[header->getCondFollow()->ord]) {
            header->setLoopType(Endless);

            // retain the fact that this is also a conditional header
            header->setStructType(LoopCond);
        } else
            header->setLoopType(PreTested);
    }

    // both the header and latch node are one way nodes so this must be an 
    // endless loop
    else
        header->setLoopType(Endless);
}

// Pre: The loop headed by header has been induced and all it's member nodes 
//      have been tagged
// Post: The follow of the loop has been determined.
void Cfg::findLoopFollow(PBB header, bool* &loopNodes)
{
    assert(header->getStructType() == Loop || 
           header->getStructType() == LoopCond);
    loopType lType = header->getLoopType();
    PBB latch = header->getLatchNode();

    if (lType == PreTested) {
        // if the 'while' loop's true child is within the loop, then its false 
        // child is the loop follow
        if (loopNodes[header->getOutEdges()[0]->ord])
            header->setLoopFollow(header->getOutEdges()[1]);
        else
	    header->setLoopFollow(header->getOutEdges()[0]);
    } else if (lType == PostTested) {
        // the follow of a post tested ('repeat') loop is the node on the end 
        // of the non-back edge from the latch node
        if (latch->getOutEdges()[0] == header)
            header->setLoopFollow(latch->getOutEdges()[1]);
        else
            header->setLoopFollow(latch->getOutEdges()[0]);
    } else { // endless loop
        PBB follow = NULL;
	
        // traverse the ordering array between the header and latch nodes.
        PBB latch = header->getLatchNode();
	for (int i = header->ord - 1; i > latch->ord; i--) {
	    PBB &desc = Ordering[i];
	    // the follow for an endless loop will have the following 
            // properties:
            //   i) it will have a parent that is a conditional header inside 
            //      the loop whose follow is outside the loop
	    //  ii) it will be outside the loop according to its loop stamp 
            //      pair
            // iii) have the highest ordering of all suitable follows (i.e. 
            //      highest in the graph)
	
	    if (desc->getStructType() == Cond && desc->getCondFollow() && 
                desc->getLoopHead() == header) {
	        if (loopNodes[desc->getCondFollow()->ord]) {
	            // if the conditional's follow is in the same loop AND is 
                    // lower in the loop, jump to this follow
                    if (desc->ord > desc->getCondFollow()->ord)
		        i = desc->getCondFollow()->ord;
                    // otherwise there is a backward jump somewhere to a node 
                    // earlier in this loop. We don't need to any nodes below 
                    // this one as they will all have a conditional within the 
                    // loop.  
                    else break;
                } else {
                    // otherwise find the child (if any) of the conditional 
                    // header that isn't inside the same loop 
                    PBB succ = desc->getOutEdges()[0];
		    if (loopNodes[succ->ord])
		        if (!loopNodes[desc->getOutEdges()[1]->ord])
                            succ = desc->getOutEdges()[1];
                        else
                            succ = NULL;
		    // if a potential follow was found, compare its ordering 
                    // with the currently found follow
                    if (succ && (!follow || succ->ord > follow->ord))
                        follow = succ;
                }
            }
        } 
	// if a follow was found, assign it to be the follow of the loop under 
        // investigation
	if (follow)
	    header->setLoopFollow(follow);
    }
}

// Pre: header has been detected as a loop header and has the details of the 
//      latching node
// Post: the nodes within the loop have been tagged
void Cfg::tagNodesInLoop(PBB header, bool* &loopNodes)
{
    assert(header->getLatchNode());

    // traverse the ordering structure from the header to the latch node 
    // tagging the nodes determined to be within the loop. These are nodes 
    // that satisfy the following:
    //  i) header.loopStamps encloses curNode.loopStamps and 
    //     curNode.loopStamps encloses latch.loopStamps
    //  OR
    //  ii) latch.revLoopStamps encloses curNode.revLoopStamps and 
    //      curNode.revLoopStamps encloses header.revLoopStamps
    //  OR
    //  iii) curNode is the latch node

    PBB latch = header->getLatchNode();
    for (int i = header->ord - 1; i >= latch->ord; i--)
        if (Ordering[i]->inLoop(header, latch)) {
            // update the membership map to reflect that this node is within 
            // the loop
            loopNodes[i] = true;

	    Ordering[i]->setLoopHead(header);
	}
}

// Pre: The graph for curProc has been built.
// Post: Each node is tagged with the header of the most nested loop of which 
//       it is a member (possibly none).
// The header of each loop stores information on the latching node as well as 
// the type of loop it heads.
void Cfg::structLoops()
{
    for (int i = Ordering.size() - 1; i >= 0; i--) {
        PBB curNode = Ordering[i];	// the current node under investigation
        PBB latch = NULL;	        // the latching node of the loop

        // If the current node has at least one back edge into it, it is a 
        // loop header. If there are numerous back edges into the header, 
        // determine which one comes form the proper latching node.
        // The proper latching node is defined to have the following 
        // properties:
        //   i) has a back edge to the current node
        //  ii) has the same case head as the current node
        // iii) has the same loop head as the current node
        //  iv) is not an nway node
        //   v) is not the latch node of an enclosing loop
        //  vi) has a lower ordering than all other suitable candiates
        // If no nodes meet the above criteria, then the current node is not a 
        // loop header

        std::vector<PBB> &iEdges = curNode->getInEdges();
        for (unsigned int j = 0; j < iEdges.size(); j++) {
            PBB pred = iEdges[j];
            if (pred->getCaseHead() == curNode->getCaseHead() &&  // ii)
                pred->getLoopHead() == curNode->getLoopHead() &&  // iii)
                (!latch || latch->ord > pred->ord) &&             // vi)
                !(pred->getLoopHead() && 
                  pred->getLoopHead()->getLatchNode() == pred) && // v)
                pred->hasBackEdgeTo(curNode))                     // i)
                latch = pred;
        }

        // if a latching node was found for the current node then it is a loop 
        // header. 
        if (latch) {
            // define the map that maps each node to whether or not it is 
            // within the current loop
            bool* loopNodes = new bool[Ordering.size()];
            for (unsigned int j = 0; j < Ordering.size(); j++)
                loopNodes[j] = false;

            curNode->setLatchNode(latch);

            // the latching node may already have been structured as a 
            // conditional header. If it is not also the loop header (i.e. the 
            // loop is over more than one block) then reset it to be a 
            // sequential node otherwise it will be correctly set as a loop 
            // header only later
            if (latch != curNode && latch->getStructType() == Cond)
                latch->setStructType(Seq);
	
            // set the structured type of this node
            curNode->setStructType(Loop);

            // tag the members of this loop
            tagNodesInLoop(curNode, loopNodes);

            // calculate the type of this loop
            determineLoopType(curNode, loopNodes);

            // calculate the follow node of this loop
            findLoopFollow(curNode, loopNodes);

            // delete the space taken by the loopnodes map
            delete[] loopNodes;
        }
    }
}

// This routine is called after all the other structuring has been done. It 
// detects conditionals that are in fact the head of a jump into/outof a loop 
// or into a case body.  Only forward jumps are considered as unstructured 
// backward jumps will always be generated nicely.
void Cfg::checkConds()
{
    for (unsigned int i = 0; i < Ordering.size(); i++) {
        PBB curNode = Ordering[i];
        std::vector<PBB> &oEdges = curNode->getOutEdges();
		
        // consider only conditional headers that have a follow and aren't 
        // case headers
        if ((curNode->getStructType() == Cond || 
             curNode->getStructType() == LoopCond) &&
            curNode->getCondFollow() && curNode->getCondType() != Case) {
            // define convenient aliases for the relevant loop and case heads 
            // and the out edges
	    PBB myLoopHead = (curNode->getStructType() == LoopCond ? 
                              curNode : curNode->getLoopHead());
            PBB follLoopHead = curNode->getCondFollow()->getLoopHead();

            // analyse whether this is a jump into/outof a loop
            if (myLoopHead != follLoopHead) {
                // we want to find the branch that the latch node is on for a 
                // jump out of a loop
                if (myLoopHead) {
                    PBB myLoopLatch = myLoopHead->getLatchNode();

                    // does the then branch goto the loop latch?
                    if (oEdges[BTHEN]->isAncestorOf(myLoopLatch) || 
                        oEdges[BTHEN] == myLoopLatch) {
                        curNode->setUnstructType(JumpInOutLoop);
                        curNode->setCondType(IfElse);
                    }
		    // does the else branch goto the loop latch?
		    else if (oEdges[BELSE]->isAncestorOf(myLoopLatch) || 
                             oEdges[BELSE] == myLoopLatch) {
                        curNode->setUnstructType(JumpInOutLoop);
                        curNode->setCondType(IfThen);
                    }
                }

                if (curNode->getUnstructType() == Structured && follLoopHead) { 
                    // find the branch that the loop head is on for a jump 
                    // into a loop body. If a branch has already been found, 
                    // then it will match this one anyway

                    // does the else branch goto the loop head?
                    if (oEdges[BTHEN]->isAncestorOf(follLoopHead) || 
                        oEdges[BTHEN] == follLoopHead) {
                        curNode->setUnstructType(JumpInOutLoop);
                        curNode->setCondType(IfElse);
                    }

                    // does the else branch goto the loop head?
                    else if (oEdges[BELSE]->isAncestorOf(follLoopHead) || 
                             oEdges[BELSE] == follLoopHead) {
                        curNode->setUnstructType(JumpInOutLoop);
                        curNode->setCondType(IfThen);
                    }
                }
            }

            // this is a jump into a case body if either of its children don't 
            // have the same same case header as itself
            if (curNode->getUnstructType() == Structured &&
                (curNode->getCaseHead() != 
                     curNode->getOutEdges()[BTHEN]->getCaseHead() ||
                 curNode->getCaseHead() != 
                     curNode->getOutEdges()[BELSE]->getCaseHead())) {
                PBB myCaseHead = curNode->getCaseHead();
                PBB thenCaseHead = curNode->getOutEdges()[BTHEN]->getCaseHead();
                PBB elseCaseHead = curNode->getOutEdges()[BELSE]->getCaseHead();

                if (thenCaseHead == myCaseHead && 
                    (!myCaseHead || 
                     elseCaseHead != myCaseHead->getCondFollow())) {
                    curNode->setUnstructType(JumpIntoCase);
                    curNode->setCondType(IfElse);
                } else if (elseCaseHead == myCaseHead && 
                           (!myCaseHead || 
                            thenCaseHead != myCaseHead->getCondFollow())) {
                    curNode->setUnstructType(JumpIntoCase);
                    curNode->setCondType(IfThen);
                }
            }	
        }

        // for 2 way conditional headers that don't have a follow (i.e. are 
        // the source of a back edge) and haven't been structured as latching 
        // nodes, set their follow to be the non-back edge child.
        if (curNode->getStructType() == Cond && !curNode->getCondFollow() &&
            curNode->getUnstructType() == Structured && 
            curNode->getCondType() != Case) {
            // latching nodes will already have been reset to Seq structured 
            // type
            assert(curNode->hasBackEdge());

            if (curNode->hasBackEdgeTo(curNode->getOutEdges()[BTHEN])) {
                curNode->setCondType(IfThen);
                curNode->setCondFollow(curNode->getOutEdges()[BELSE]);
            } else {
                curNode->setCondType(IfElse);
                curNode->setCondFollow(curNode->getOutEdges()[BTHEN]);
            }
        }
    }
}

void Cfg::structure()
{
    setTimeStamps();
    findImmedPDom();
    structConds();
    structLoops();
    checkConds();
}

void Cfg::removeUnneededLabels(HLLCode *hll)
{
    for (unsigned int i = 0; i < Ordering.size(); i++)
        if (!Ordering[i]->hllLabel)
            hll->RemoveLabel(Ordering[i]->ord);
}

void Cfg::generateDotFile(const char *str)
{
    assert(str);
    std::ofstream of(str);
    of << "digraph Cfg {" << std::endl;
    for (unsigned int i = 0; i < Ordering.size(); i++) {
        of << "    " << i << " [";
        of << "label=\"";
        switch(Ordering[i]->getType()) {
            case ONEWAY: of << "oneway"; break;
            case TWOWAY: 
                if (Ordering[i]->getCond())
                    Ordering[i]->getCond()->print(of); 
                else
                    of << "twoway";
                break;
            case NWAY: of << "nway"; break;
            case CALL: of << "call"; break;
            case RET: of << "ret"; break;
            case FALL: of << "fall"; break;
            case COMPJUMP: of << "compjump"; break;
            case COMPCALL: of << "compcall"; break;
            case INVALID: of << "invalid"; break;
        }
        of << "\"];\n";
    }
    for (unsigned int i = 0; i < Ordering.size(); i++) {
        for (unsigned int j = 0; j < Ordering[i]->getOutEdges().size(); j++) {
            of << "    " << i << " -> " << Ordering[i]->getOutEdges()[j]->ord;
            if (Ordering[i]->getType() == TWOWAY) {
                if (j == 0)
                    of << "[label=\"true\"]";
                else
                    of << "[label=\"false\"]";
            }
            of << ";\n";
        }
    }
    of << "}";
    of.close();
}

