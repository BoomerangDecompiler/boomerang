/*
 * Copyright (C) 1997-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       basicblock.cc
 * OVERVIEW:   Implementation of the BasicBlock class.
 *============================================================================*/

/*
 * $Revision$
 * Dec 97 - created by Mike
 * 18 Apr 02 - Mike: Changes for boomerang
*/


/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include "types.h"
#include "exp.h"
#include "cfg.h"
#include "register.h"
#include "rtl.h"
#include "hllcode.h"
#include "proc.h"
#include "prog.h"
#include "dataflow.h"
#include "util.h"

/**********************************
 * BasicBlock methods
 **********************************/

/*==============================================================================
 * FUNCTION:        BasicBlock::BasicBlock
 * OVERVIEW:        Constructor.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
BasicBlock::BasicBlock()
    :
        m_DFTfirst(0),
		m_structType(NONE),	m_loopCondType(NONE),
		m_loopHead(NULL), m_caseHead(NULL),
		m_condFollow(NULL), m_loopFollow(NULL),
		m_latchNode(NULL),
        m_nodeType(INVALID),
        m_pRtls(NULL),
        m_iLabelNum(0),
		m_labelneeded(false),
        m_bIncomplete(true),
        m_bJumpReqd(false),
        m_iNumInEdges(0),
        m_iNumOutEdges(0),
        m_iTraversed(false)
{
}

/*==============================================================================
 * FUNCTION:        BasicBlock::~BasicBlock
 * OVERVIEW:        Destructor.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
BasicBlock::~BasicBlock()
{
    if (m_pRtls)
    {
        // Delete the RTLs
        for (std::list<RTL*>::iterator it = m_pRtls->begin(); it != m_pRtls->end(); it++)
        {
            if (*it)
            {
                delete *it;
            }
        }

        // and delete the list
        delete m_pRtls;
        m_pRtls = NULL;
    }
}


/*==============================================================================
 * FUNCTION:        BasicBlock::BasicBlock
 * OVERVIEW:        Copy constructor.    
 * PARAMETERS:      bb - the BB to copy from
 * RETURNS:         <nothing>
 *============================================================================*/
BasicBlock::BasicBlock(const BasicBlock& bb)
    :   m_DFTfirst(0),
		m_structType(bb.m_structType), m_loopCondType(bb.m_loopCondType),
		m_loopHead(bb.m_loopHead), m_caseHead(bb.m_caseHead),
		m_condFollow(bb.m_condFollow), m_loopFollow(bb.m_loopFollow),
		m_latchNode(bb.m_latchNode),
        m_nodeType(bb.m_nodeType),
        m_pRtls(NULL),
        m_iLabelNum(bb.m_iLabelNum),
		m_labelneeded(false),
        m_bIncomplete(bb.m_bIncomplete),
        m_bJumpReqd(bb.m_bJumpReqd),
        m_InEdges(bb.m_InEdges),
        m_OutEdges(bb.m_OutEdges),
        m_iNumInEdges(bb.m_iNumInEdges),
        m_iNumOutEdges(bb.m_iNumOutEdges),
        m_iTraversed(false),
        returnLoc(bb.returnLoc)
{
    setRTLs(bb.m_pRtls);
}

/*==============================================================================
 * FUNCTION:        BasicBlock::BasicBlock
 * OVERVIEW:        Private constructor.
 * PARAMETERS:      pRtls -
 *                  bbType -
 *                  iNumOutEdges -
 * RETURNS:         <nothing>
 *============================================================================*/
BasicBlock::BasicBlock(std::list<RTL*>* pRtls, BBTYPE bbType, int iNumOutEdges)
    :   m_DFTfirst(0),
		m_structType(NONE),	m_loopCondType(NONE),
		m_loopHead(NULL), m_caseHead(NULL),
		m_condFollow(NULL), m_loopFollow(NULL),
		m_latchNode(NULL),  
        m_nodeType(bbType),
        m_pRtls(NULL),
        m_iLabelNum(0),
		m_labelneeded(false),
        m_bIncomplete(false),
        m_bJumpReqd(false),
        m_iNumInEdges(0),
        m_iNumOutEdges(iNumOutEdges),
        m_iTraversed(false)
{
    m_OutEdges.reserve(iNumOutEdges);               // Reserve the space;
                                                    // values added with
                                                    // AddOutEdge()

    // Set the RTLs
    setRTLs(pRtls);

}

#if 0
/*==============================================================================
 * FUNCTION:        BasicBlock::storeUseDefineStruct
 * OVERVIEW:        Stores the type info of every register in this BB. This is 
 *          done by inserting the used/defined places for each register
 * PARAMETERS:      BBBlock 
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::storeUseDefineStruct(BBBlock& inBlock) 
{
    if (m_pRtls)                    // Can be zero if e.g. INVALID
    {

//      RTL_CIT rit;
        std::list<RTL*>_CIT rit;
        for (rit = m_pRtls->begin(); rit != m_pRtls->end(); rit++) {
        (*rit)->storeUseDefineStruct(inBlock);
        }
    }
}

/*==============================================================================
 * FUNCTION:        BasicBlock::propagateType
 * OVERVIEW:        Recursively propagates type information from one 
 *                  basic block to the next. Depth first traversal
 * PARAMETERS:      BasicBlock * prevBlock, used to find the parent BB 
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::propagateType(BasicBlock * prevBlock, bool endCase = false){

    // Remember to clear it before and after type propagation
    m_iTraversed = true;
    
    assert(usedDefineStruct != NULL);
    // First do any alias propagations in this BasicBlock
    usedDefineStruct->propagateRegWithinBB();

    tempPrevBB = prevBlock;
/*
    for (std::vector<PBB>::iterator it = m_OutEdges.begin(); it != m_OutEdges.end();
         it++) {
        PBB child = *it;

    if (child->m_iTraversed == false) {
        child->propagateType(this);
    }   
    }        
*/  

    if (!endCase) { 
    for (std::vector<PBB>::iterator it = m_OutEdges.begin(); it != m_OutEdges.end();
             it++) {

        PBB child = *it;    
        if (child->m_iTraversed == false) {
        child->propagateType(this);
        }
        else {
        child->propagateType(this, true);
        }
    }
    }    

    // We reached a back branch, propagate ONCE
    // and then leave
    // Ignore for now, implement later

    std::map<Byte, UDChain*>::iterator noDefMapIt;
    std::map<Byte, UDChain*>::iterator prevLastDefMapIt;

    std::map<Byte, UDChain*> noDefMap = usedDefineStruct->returnNoDefRegChains();

    // Iterate through the map of no def chains     
    for (noDefMapIt = noDefMap.begin(); noDefMapIt != noDefMap.end(); ++noDefMapIt){

    // Reset the forwardLooking BB to the immediate previous BB 
    BasicBlock * forwardLookingBB = tempPrevBB;

        int timeOut = 0;
    while (forwardLookingBB) {
        // Get a map of all the defined registers in the prev BB 
            std::map<Byte, UDChain*> prevLastDefMap = 
             forwardLookingBB->usedDefineStruct->returnLastDefRegChains();

        // See if it contains one of the reg that we don't have a def for
        prevLastDefMapIt = prevLastDefMap.find(noDefMapIt->first);

        if (prevLastDefMapIt != prevLastDefMap.end()){
        // Found a match! Propagate type. 

        if (prevLastDefMapIt->second->chainType > noDefMapIt->second->chainType){
            noDefMapIt->second->chainType = prevLastDefMapIt->second->chainType;
        }
        else{
            prevLastDefMapIt->second->chainType = noDefMapIt->second->chainType ;
        }
        break;
        }
        else {
        // 50 is currently an arbitary number   
        // If we look 50 basic blocks up and still don't find it 
        // where the register is defined, give up
        if (timeOut++ > 50)
           break;
                
        forwardLookingBB = forwardLookingBB->tempPrevBB;                

        // Complete loop, will go around until timeout. 
        // Break now to save time
        if (forwardLookingBB == this->tempPrevBB)
            break;          

        }
     }
    }  
}
#endif

/*==============================================================================
 * FUNCTION:        BasicBlock::getLabel
 * OVERVIEW:        Returns nonzero if this BB has a label, in the sense that a
 *                  label is required in the translated source code
 * PARAMETERS:      <none>
 * RETURNS:         An integer unique to this BB, or zero
 *============================================================================*/
int BasicBlock::getLabel() {
    return m_iLabelNum;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::isTraversed
 * OVERVIEW:        Returns nonzero if this BB has been traversed
 * PARAMETERS:      <none>
 * RETURNS:         True if traversed
 *============================================================================*/
bool BasicBlock::isTraversed() {
    return m_iTraversed;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::setTraversed
 * OVERVIEW:        Sets the traversed flag
 * PARAMETERS:      bTraversed: true to set this BB to traversed
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::setTraversed(bool bTraversed) {
    m_iTraversed = bTraversed;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::setRTLs
 * OVERVIEW:        Sets the RTLs for a basic block. This is the only place that
 *                  the RTLs for a block must be set as we need to add the back
 *                  link for a call instruction to its enclosing BB.
 * PARAMETERS:      rtls - a list of RTLs
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::setRTLs(std::list<RTL*>* rtls)
{
    // should we delete old ones here?  breaks some things - trent
    m_pRtls = rtls;

    // Set the link between the last instruction (a call) and this BB
    // if this is a call BB
    HLCall* call = (HLCall*)*(m_pRtls->rbegin());
    if (call->getKind() == CALL_RTL)
        call->setBB(this);
}

/*==============================================================================
 * FUNCTION:        BasicBlock::getType
 * OVERVIEW:        Return the type of the basic block.
 * PARAMETERS:      <none>
 * RETURNS:         the type of the basic block
 *============================================================================*/
BBTYPE BasicBlock::getType() 
{
    return m_nodeType;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::updateType
 * OVERVIEW:        Update the type and number of out edges. Used for example
 *                  where a COMPJUMP type is updated to an NWAY when a switch
 *                  idiom is discovered.
 * PARAMETERS:      bbType - the new type
 *                  iNumOutEdges - new number of inedges
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::updateType(BBTYPE bbType, int iNumOutEdges)
{
    m_nodeType = bbType;
    m_iNumOutEdges = iNumOutEdges;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::setJumpReqd
 * OVERVIEW:        Sets the "jump required" bit. This means that this BB is
 *                  an orphan (not generated from input code), and that the
 *                  "fall through" out edge needs to be implemented as a jump
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::setJumpReqd()
{
    m_bJumpReqd = true;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::isJumpReqd
 * OVERVIEW:        Returns the "jump required" bit. See above for details
 * PARAMETERS:      <none>
 * RETURNS:         True if a jump is required
 *============================================================================*/
bool BasicBlock::isJumpReqd() 
{
    return m_bJumpReqd;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::print()
 * OVERVIEW:        Display the whole BB to the given stream
 *                  Used for "-R" option, and handy for debugging
 * PARAMETERS:      os - stream to output to
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::print(std::ostream& os) 
{
    if (m_iLabelNum) os << "L" << std::dec << m_iLabelNum << ": ";
    switch(m_nodeType)
    {
        case ONEWAY:    os << "Oneway BB"; break;
        case TWOWAY:    os << "Twoway BB"; break;
        case NWAY:      os << "Nway BB"; break;
        case CALL:      os << "Call BB"; break;
        case RET:       os << "Ret BB"; break;
        case FALL:      os << "Fall BB"; break;
        case COMPJUMP:  os << "Computed jump BB"; break;
        case COMPCALL:  os << "Computed call BB"; break;
        case INVALID:   os << "Invalid BB"; break;
    }
    // Printing the address is bad for unit testing, since address will
    // be arbitrary
    //os << " (0x" << std::hex << (unsigned int)this << "):\n";
    os << ":\n";
    if (m_pRtls)                    // Can be zero if e.g. INVALID
    {
        std::list<RTL*>::iterator rit;
        for (rit = m_pRtls->begin(); rit != m_pRtls->end(); rit++) {
            (*rit)->print(os);
        }
    }
    if (m_bJumpReqd) {
        os << "Synthetic out edge(s) to ";
        for (int i=0; i < m_iNumOutEdges; i++) {
            PBB outEdge = m_OutEdges[i];
            if (outEdge && outEdge->m_iLabelNum)
                os << "L" << std::dec << outEdge->m_iLabelNum << " ";
        }
        os << std::endl;
    }
}

// These are now the only two user level functions that are in the
// BasicBlock class. Perhaps these will be moved to the Cfg class
// for uniformity, or there will be other reasons - MVE

/*==============================================================================
 * FUNCTION:        BasicBlock::addInterProcOutEdge
 * OVERVIEW:        Adds an interprocedural out-edge to the basic block pBB that
 *                  represents this address.  The local inter-procedural boolean
 *                  is updated as well.  The mapping between addresses and basic
 *                  blocks is done when the  graph is well-formed.
 *                  NOTE: this code was changed to only decrease the # of out
 *                  edges by one so to avoid interprocedural links that are not
 *                  supported by the well-form graph routine.  We need to think
 *                  how to support this in a nice way, so for the moment,
 *                  decrease out-edges works ok.
 * PARAMETERS:      addr -
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::addInterProcOutEdge(ADDRESS addr)
{
//  m_OutEdges.push_back((PBB)addr);
//  m_OutEdgeInterProc[m_OutEdges.size()-1] = true;
    m_iNumOutEdges--;
}


/*==============================================================================
 * FUNCTION:        BasicBlock::addProcOutEdge 
 * OVERVIEW:        This function was proposed but never completed
 * PARAMETERS:      uAddr -
 * RETURNS:         <nothing>
 *============================================================================*/
bool BasicBlock::addProcOutEdge (ADDRESS uAddr)
{
    // This is a bit problematic. I tried writing one at one stage,
    // and ended up with circular dependencies... Perhaps we don't
    // need this function; you can just use getProcAddr() to find
    // the destintion of the call
    std::cerr << "AddProcOutEdge not implemented yet\n";
    return true;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::getLowAddr
 * OVERVIEW:        Get the lowest real address associated with this BB. Note
 *                  that although this is usually the address of the first RTL,
 *                  it is not always so. For example, if the BB contains just a
 *                  delayed branch, and the delay instruction for the branch
 *                  does not affect the branch, so the delay instruction is
 *                  copied in  front of the branch instruction. It's address
 *                  will be UpdateAddress()d to 0, since it is "not really
 *                  there", so the low address for this BB will be the address
 *                  of the branch.
 * PARAMETERS:      <none>
 * RETURNS:         the lowest real address associated with this BB
 *============================================================================*/
ADDRESS BasicBlock::getLowAddr() 
{
    assert(m_pRtls != NULL);
    ADDRESS a = m_pRtls->front()->getAddress();
    if ((a == 0) && (m_pRtls->size() > 1))
    {
        std::list<RTL*>::iterator it = m_pRtls->begin();
        ADDRESS add2 = (*++it)->getAddress();
        // This is a bit of a hack for 286 programs, whose main actually starts
        // at offset 0. A better solution would be to change orphan BBs'
        // addresses to NO_ADDRESS, but I suspect that this will cause many
        // problems. MVE
        if (add2 < 0x10)
            // Assume that 0 is the real address
            return 0;
        return add2;
    }
    return a;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::getHiAddr
 * OVERVIEW:        Get the highest address associated with this BB. This is
 *                  always the address associated with the last RTL.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
ADDRESS BasicBlock::getHiAddr() 
{
    assert(m_pRtls != NULL);
    return m_pRtls->back()->getAddress();
}

/*==============================================================================
 * FUNCTION:        BasicBlock::getRTLs
 * OVERVIEW:        Get pointer to the list of RTL*.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
std::list<RTL*>* BasicBlock::getRTLs() 
{
    return m_pRtls;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::getInEdges
 * OVERVIEW:        Get a constant reference to the vector of in edges.
 * PARAMETERS:      <none>
 * RETURNS:         a constant reference to the vector of in edges
 *============================================================================*/
std::vector<PBB>& BasicBlock::getInEdges()
{
    return m_InEdges;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::getOutEdges
 * OVERVIEW:        Get a constant reference to the vector of out edges.
 * PARAMETERS:      <none>
 * RETURNS:         a constant reference to the vector of out edges
 *============================================================================*/
std::vector<PBB>& BasicBlock::getOutEdges()
{
    return m_OutEdges;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::setInEdge
 * OVERVIEW:        Change the given in-edge (0 is first) to the given value
 *                  Needed for example when duplicating BBs
 * PARAMETERS:      i: index (0 based) of in-edge to change
 *                  pNewInEdge: pointer to BB that will be a new parent
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::setInEdge(int i, PBB pNewInEdge)
{
    m_InEdges[i] = pNewInEdge;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::setOutEdge
 * OVERVIEW:        Change the given out-edge (0 is first) to the given value
 *                  Needed for example when duplicating BBs
 * NOTE:            Cannot add an additional out-edge with this function; use
 *                  addOutEdge for this rare case
 * PARAMETERS:      i: index (0 based) of out-edge to change
 *                  pNewOutEdge: pointer to BB that will be the new successor
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::setOutEdge(int i, PBB pNewOutEdge)
{
	if (m_OutEdges.size() == 0) {
		assert(i == 0);
		m_OutEdges.push_back(pNewOutEdge);
	} else {
		assert(i < (int)m_OutEdges.size());
		m_OutEdges[i] = pNewOutEdge;
	}
}

/*==============================================================================
 * FUNCTION:        BasicBlock::getOutEdge
 * OVERVIEW:        Returns the i-th out edge of this BB; counting starts at 0
 * NOTE:            
 * PARAMETERS:      i: index (0 based) of the desired out edge
 * RETURNS:         the i-th out edge; 0 if there is no such out edge
 *============================================================================*/
PBB BasicBlock::getOutEdge(unsigned int i)
{
  if (i < m_OutEdges.size()) return m_OutEdges[i];
  else return 0;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::getCorrectOutEdge
 * OVERVIEW:        given an address this method returns the corresponding
 *                  out edge
 * NOTE:            
 * PARAMETERS:      a: the address
 * RETURNS:         
 *============================================================================*/

PBB BasicBlock::getCorrectOutEdge(ADDRESS a) 
{
  std::vector<PBB>::iterator it;
  for (it = m_OutEdges.begin(); it != m_OutEdges.end(); it++) {
    if ((*it)->getLowAddr() == a) return *it; 
  }
  return 0;
}


/*==============================================================================
 * FUNCTION:        BasicBlock::addInEdge
 * OVERVIEW:        Add the given in-edge
 *                  Needed for example when duplicating BBs
 * PARAMETERS:      pNewInEdge: pointer to BB that will be a new parent
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::addInEdge(PBB pNewInEdge)
{
    m_InEdges.push_back(pNewInEdge);
    m_iNumInEdges++;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::deleteInEdge
 * OVERVIEW:        Delete the in-edge from the given BB
 *                  Needed for example when duplicating BBs
 * PARAMETERS:      it: iterator to BB that will no longer be a parent
 * SIDE EFFECTS:    The iterator argument is incremented.
 * USAGE:           It should be used like this:
 *                    if (pred) deleteInEdge(it) else it++;
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::deleteInEdge(std::vector<PBB>::iterator& it)
{
    it = m_InEdges.erase(it);
    m_iNumInEdges--;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::getCoverage
 * OVERVIEW:        
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
unsigned BasicBlock::getCoverage()
{
    assert(m_pRtls != NULL);
    unsigned uTotal = 0;
    std::list<RTL*>::iterator it;
    for (it = m_pRtls->begin(); it != m_pRtls->end(); it++) {
        if ((*it)->getAddress())
            uTotal += (unsigned)(*it)->getNumBytes();
    }
    return uTotal;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::DFTOrder
 * OVERVIEW:        Traverse this node and recurse on its children in a depth
 *                  first manner. Records the times at which this node was first
 *                  visited and last visited
 * PARAMETERS:      first - the number of nodes that have been visited
 *                  last - the number of nodes that have been visited for the
 *                    last time during this traversal
 * RETURNS:         the number of nodes (including this one) that were traversed
 *                  from this node
 *============================================================================*/
unsigned BasicBlock::DFTOrder(int& first, int& last)
{
    first++;
    m_DFTfirst = first;

    unsigned numTraversed = 1;
    m_iTraversed = true;

    for (std::vector<PBB>::iterator it = m_OutEdges.begin(); it != m_OutEdges.end();
         it++) {

        PBB child = *it;
        if (child->m_iTraversed == false)
            numTraversed = numTraversed + child->DFTOrder(first,last);
    }

    last++;
    m_DFTlast = last;

    return numTraversed;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::DFTOrder
 * OVERVIEW:        Traverse this node and recurse on its parents in a reverse 
 *          depth first manner. Records the times at which this node was 
 *          first visited and last visited
 * PARAMETERS:      first - the number of nodes that have been visited
 *                  last - the number of nodes that have been visited for the
 *                    last time during this traversal
 * RETURNS:         the number of nodes (including this one) that were traversed
 *                  from this node
 *============================================================================*/
unsigned BasicBlock::RevDFTOrder(int& first, int& last)
{
    first++;
    m_DFTrevfirst = first;

    unsigned numTraversed = 1;
    m_iTraversed = true;

    for (std::vector<PBB>::iterator it = m_InEdges.begin(); it != m_InEdges.end();
         it++) {

        PBB parent = *it;
        if (parent->m_iTraversed == false)
            numTraversed = numTraversed + parent->RevDFTOrder(first,last);
    }

    last++;
    m_DFTrevlast = last;

    return numTraversed;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::lessAddress
 * OVERVIEW:        Static comparison function that returns true if the first BB
 *                  has an address less than the second BB.
 * PARAMETERS:      bb1 - first BB
 *                  bb2 - last BB
 * RETURNS:         bb1.address < bb2.address
 *============================================================================*/
bool BasicBlock::lessAddress(PBB bb1, PBB bb2)
{
    return bb1->getLowAddr() < bb2->getLowAddr();
}

/*==============================================================================
 * FUNCTION:        BasicBlock::lessFirstDFT
 * OVERVIEW:        Static comparison function that returns true if the first BB
 *                  has DFT first order less than the second BB.
 * PARAMETERS:      bb1 - first BB
 *                  bb2 - last BB
 * RETURNS:         bb1.first_DFS < bb2.first_DFS
 *============================================================================*/
bool BasicBlock::lessFirstDFT(PBB bb1, PBB bb2)
{
    return bb1->m_DFTfirst < bb2->m_DFTfirst;
}


/*==============================================================================
 * FUNCTION:        BasicBlock::subAXP
 * OVERVIEW:        Given a map from registers to expressions, follow
 *                  the control flow of the CFG replacing every use of a
 *                  register in this map with the corresponding
 *                  expression. Then for every definition of such a
 *                  register, update its expression to be the RHS of
 *                  the definition after the first type of substitution
 *                  has been performed and remove that definition from
 *                  the CFG.
 * PARAMETERS:      subMap - a map from register to expressions
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::subAXP(std::map<Exp*,Exp*> subMap)
{
    // Check that if this BB has already been visited, that the expressions
    // in the map regSubs are the same now as they were when last visited.
    // This is equivalent to checking that the stack "height" is consistent
    // for all entry paths to this BB
    if (m_iTraversed) {
        for (std::map<Exp*,Exp*>::iterator it = subMap.begin();
          it != subMap.end(); it++) {

            std::map<Exp*,Exp*>::iterator val = regSubs.find(it->first);
            if (val != regSubs.end()) {
                if (!(val->second == it->second)) {
                    std::cerr << "stack type assumption violated in BB @ ";
                    std::cerr << std::hex << getLowAddr() << std::dec << ":\n";
                    std::cerr << "\tprevious: " << it->first << " -> ";
                    std::cerr << val->second << "\n\tcurrent: " << it->first;
                    std::cerr << " -> " << it->second << std::endl;
                }
            }
        }
        return;
    }

    // Do some interBB forward substitutions involving %afp. In particular,
    // pa-risc is fond of putting say %afp-64 into r19, then using m[r[19]+4]
    // later on. We need these transformed in many cases
    // Note that for simplicity, we do the transformations twice. The first time
    // is to change raw register numbers into %afp relative expressions; we then
    // do the forward substitution and if there was a change this RTL, the
    // substitutions are done again.
    std::map<Exp*, Exp*, lessExpStar> subs; // A map from left hand sides to right
                                // hand sides, suitable for substition
                                // The map exists for one BB only; we assume
                                // that this is enough
    std::map<Exp*, Exp*, lessExpStar>::iterator mm;
    Exp* srch;
    regSubs = subMap;
    for (std::list<RTL*>::iterator it = m_pRtls->begin(); it != m_pRtls->end(); it++)
    {
        RTL& rtl = **it;
        // First, do the actual substitutions for this rtl
        rtl.subAXP(subMap);

        bool change = false;
        int n = rtl.getNumExp();
        for (int i=0; i < n; i++) {
            Exp* pe = rtl.elementAt(i);
            if (pe->isAssign()) continue;
            Exp* lhs = ((Binary*)pe)->getSubExp1();
            Exp* rhs = ((Binary*)pe)->getSubExp2();
            // Substitute the RHS, and LHS if in m[] etc
            for (mm = subs.begin(); mm != subs.end(); mm++) {
//              if (mm->second.len() == 0)
//              // This temp assignment has been disabled by a clear() (below)
//              continue;
                srch = mm->first;
                bool ch;
                if ((rhs = rhs->searchReplaceAll(srch, mm->second, ch)), ch) {
                    // Need to simplify. For example, m[r[19]+4] might end up as
                    // m[%afp-64+4], and we want m[%afp-60] so it will match the
                    // pattern m[%afp +- k] for later analyses
                    rhs = rhs->simplify();
                    change = true;
                }
                if (!(*lhs == *srch)) {
                    if ((lhs = lhs->searchReplaceAll(srch, mm->second, ch)), ch)
                    {
                        lhs = lhs->simplify();
                        change = true;
                    }
                }
            }
            // We are looking on the RHS for any of these three:
            // 1) %afp
            // 2) %afp + k
            // 3) %afp - k
            // In some architectures (e.g. pa-risc), we will see a[m[%afp +- K]]
            // If so, treat these as %afp +- K
            if (rhs->isAfpTerm() && lhs->isRegOfK()) {
                // We have a suitable assignment. Add it to the map.
                // (If it already exists, then the mapping is updated rather
                //  than added)
                // The map has to be of SemStr, not SemStr*, for this to work.
                // Mike: CHECK THIS!
                if (rhs->isAddrOf()) {
                    // We have a[m[ %afp...]]. For the purposes of forward
                    // substitution, discard the a[m[
                    Exp* temp = ((Unary*)rhs)->getSubExp1();    // Discard a[
                    temp = ((Unary*)temp)->getSubExp1();        // Discard m[
                    subs[lhs] = temp;
                } else
                    subs[lhs] = rhs;      // Ordinary mapping
            } else if (lhs->isRegOfK()) {
                // This is assigning to a register. Must check whether any regs
                // already in the map are now invalidated, for the purpose of
                // substiution, by this assignment
                for (mm = subs.begin(); mm != subs.end(); mm++) {
                    if (*mm->first == *lhs)
                        // This reg no longer contains an AFP expression, and
                        // so is usable for forward substitutions
                        // Note: std::map<>::erase doesn't return an iterator!
                        subs.erase(mm);
                }
            }
        }
        // If there was a change this RTL, do another substitutuion pass
        if (change) rtl.subAXP(subMap);
    }

    // Recurse to all the out-edges
    m_iTraversed = true;
    for (std::vector<PBB>::iterator it1 = m_OutEdges.begin();
      it1 != m_OutEdges.end(); it1++)
        (*it1)->subAXP(subMap);
}

#if 0
/*==============================================================================
 * FUNCTION:        BasicBlock::resetDFASets
 * OVERVIEW:        Resets the DFA sets of this BB.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::resetDFASets()
{
    liveOut.reset();
    defSet.reset();
    useSet.reset();
    useUndefSet.reset();
    recursiveUseUndefSet.reset();
}
#endif

/*==============================================================================
 * FUNCTION:        BasicBlock::lessLastDFT
 * OVERVIEW:        Static comparison function that returns true if the first BB
 *                  has DFT first order less than the second BB.
 * PARAMETERS:      bb1 - first BB
 *                  bb2 - last BB
 * RETURNS:         bb1.last_DFS < bb2.last_DFS
 *============================================================================*/
bool BasicBlock::lessLastDFT(PBB bb1, PBB bb2)
{
    return bb1->m_DFTlast < bb2->m_DFTlast;
}

#if 0
/*==============================================================================
 * FUNCTION:        BasicBlock::buildDefinedSet
 * OVERVIEW:        Build the set of locations that are defined by this BB as
 *                  well as the set of locations that are used before definition
 *                  or don't have a definition in this BB.
 *                  Also initialises the liveOut set to be equal to the defined
 *                  set.
 * PARAMETERS:      locMap - a map between locations and integers
 *                  filter - a filter to restrict which locations are
 *                    considered
 *                  proc - Proc object that this BB is contained in
 * RETURNS:         <nothing>
 *============================================================================*/
void BasicBlock::buildUseDefSets(LocationMap& locMap, LocationFilter* filter,
    Proc* proc)
{
    assert(!m_bIncomplete);
    assert(m_pRtls != NULL);

    // Summarise the set of location definitions and uses for all the RTLs.
    for (std::list<RTL*>::const_iterator it = m_pRtls->begin(); it != m_pRtls->end(); it++)
    {
        (*it)->getUseDefLocations(locMap,filter,defSet,useSet,useUndefSet,proc);
    }

    // Set the live out set to be equal to the just built defined set.
    liveOut = defSet;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::buildLiveInOutSets
 * OVERVIEW:        Build the set of locations that are live into and out of
 *                  this basic block.
 *                  This implements the inner loop of Algorithm 10.3 given on
 *                  page 631 of "Compilers: Principles, Techniques and Tools"
 *                  (i.e the 'dragon book').
 *                  This method assumes that all definitions (recall
 *                  that we are only interested in locations that
 *                  could be parameters or retrun values) are killed
 *                  by a call.
 * PARAMETERS:      callDefines - this is the set of locations that will
 *                    be considered as defined by a call. This will
 *                    typically be only the possible return locations
 *                    and it will only be non-NULL when
 *                    analysing a procedure as a callee so that we
 *                    don't determine uses of a return location after
 *                    a call as a
 *                    use-without-defintion. If this parameter is
 *                    NULL, then we assume the enclosing procedure is
 *                    being analysed as a caller and as such a call
 *                    kills all definitions (since we are only
 *                    considering parameter and return locations).
 * RETURNS:         true if there was a change in the live out set
 *============================================================================*/
bool BasicBlock::buildLiveInOutSets(const BITSET* callDefines /*= NULL*/)
{
    std::vector<PBB>::iterator it = m_InEdges.begin();
    while (it != m_InEdges.end()) {

        // Reset the live ins to empty if the predecessor is a call
        // and we are analysing the enclosing procedure as a caller
        if (callDefines == NULL &&
          ((*it)->m_nodeType == CALL || (*it)->m_nodeType == COMPCALL)) {
            // Don't reset it to empty; set it the the return location (if any)
            liveIn = (*it)->returnLoc;
        }

        // Set liveIn to be the intersection of itself and the
        // live outs of the current predecessor
        liveIn = (liveIn & (*it)->liveOut);

        it++;
    }

    BITSET oldOut = liveOut;
    liveOut = defSet | liveIn;

    // Add the call defs to live out if analysing as a callee
    if ((callDefines != NULL) &&
      ((m_nodeType == CALL) || (m_nodeType == COMPCALL)))
        liveOut |= (*callDefines);

    return liveOut != oldOut;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::buildRecursiveUseUndefSet
 * OVERVIEW:        Build the set of locations used before definition in the
 *                  subgraph headed by this BB. 
 * NOTE:            This has the word "Recursive" in it because when examining a
 *                   recursive function, there will be BBs where the answer will
 *                   depend on whether this function returns a value, and that's
 *                   what we hope to answer with this set. But recursive calls
 *                   always have parallel paths where the return value is used,
 *                   so that's why this set has the OR of all the child sets
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>, but changes member recursiveUseUndefSet
 *============================================================================*/
void BasicBlock::buildRecursiveUseUndefSet()
{
    // Note that "child" is used in the sense of a "child in the CFG", i.e. a
    // successor or out-edge BB (not in the sense of being called)
    recursiveUseUndefSet = useUndefSet;

    m_iTraversed = true;

    for (std::vector<PBB>::iterator it = m_OutEdges.begin(); it != m_OutEdges.end();
         it++)
    {
        PBB child = *it;
        if (child->m_iTraversed == false)
            child->buildRecursiveUseUndefSet();

        // We stop considering locations used before definition after this
        // BB if it is a call BB as a call kills all definitions.
        // If this is not a call BB, then we add to the
        // used-but-not-defined set all the locations of the child in
        // the same category that are not in this BB's live out set.
        if (m_nodeType != CALL && m_nodeType != COMPCALL)
            recursiveUseUndefSet |= 
                (child->recursiveUseUndefSet & ~liveOut);
    }
}

/*==============================================================================
 * FUNCTION:        BasicBlock::getLiveOuts
 * OVERVIEW:        Return a reference to the liveOut set of this BB.
 * PARAMETERS:      <none>
 * RETURNS:         the set of live outs
 *============================================================================*/
BITSET& BasicBlock::getLiveOuts()
{
    return liveOut;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::getLiveOutUnuseds
 * OVERVIEW:        Return a set of locations that are live but not used
 * PARAMETERS:      <none>
 * RETURNS:         the set as above
 *============================================================================*/
BITSET BasicBlock::getLiveOutUnuseds() 
{
    return liveOut & ~useSet;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::getRecursiveUseUndefSet()
 * OVERVIEW:        Return a reference to the set of locations used 
 *                  before definition in the subgraph headed by this BB.
 * PARAMETERS:      <none>
 * RETURNS:         the set of locations used before definition in the subgraph
 *                  headed by this BB
 *============================================================================*/
BITSET& BasicBlock::getRecursiveUseUndefSet()
{
    return recursiveUseUndefSet;
}

/*==============================================================================
 * FUNCTION:        BasicBlock::setReturnLoc
 * OVERVIEW:        Set the returnLoc BITSET appropriately, given the analysed
 *                    return location
 * NOTE:            Only makes sense if this is a CALL (or COMPCALL) BB
 * NOTE:            Do we really need this?
 * PARAMETERS:      loc: reference to the return location
 * RETURNS:         nothing
 *============================================================================*/
void BasicBlock::setReturnLoc(Exp* loc)
{
    returnLoc = loc;
}
#endif

#if 0
/*==============================================================================
 * FUNCTION:        BasicBlock::isDefined
 * OVERVIEW:        Return true if the location represented by the given bit
 *                    is defined in this BB
 * PARAMETERS:      bit - bit representing the location to check
 * RETURNS:         True if the location is defined in this BB
 *============================================================================*/
bool BasicBlock::isDefined(int bit)
{
    return defSet.test(bit);
}
#endif

#if 0
/*==============================================================================
 * FUNCTION:        BasicBlock::printDFAInfo
 * OVERVIEW:        Display the defined, live in and live out sets of this BB.
 * PARAMETERS:      os - the stream to use
 * RETURNS:         the given output stream
 *============================================================================*/
std::ostream& BasicBlock::printDFAInfo(std::ostream& os) 
{
    assert(m_pRtls != NULL);

//  os << "Basic block: " << std::hex << getLowAddr() << " .. " << getHiAddr() << std::endl;
    os << "===================" << std::endl;
    for (std::list<RTL*>::iterator it = m_pRtls->begin(); it != m_pRtls->end(); it++)
        (*it)->print(os);
    os << "                  defined: ";
    locMap.printBitset(cout,defSet);
    cout << std::endl;
    os << "                  live-in: ";
    locMap.printBitset(cout,liveIn);
    cout << std::endl;
    os << "                 live-out: ";
    locMap.printBitset(cout,liveOut);
    cout << std::endl;
    os << "                     used: ";
    locMap.printBitset(cout,useSet);
    cout << std::endl;
    os << "      used undefd (local): ";
    locMap.printBitset(cout,useUndefSet);
    cout << std::endl;
//    os << "     used undefd (global): ";
//    locMap.printBitset(cout,useUndefSet & ~liveIn);
//    cout << std::endl;
    // Note: there may be a bit of a bug with this, or at least the result as
    // printed is confusing; when the return value has been found, then
    // it defines the return location in the call, so the return location is
    // live on output, so the bit is not turned on for printing!
    // It may still be useful for calling from a debugger, so this code stays
    // MVE: Not sure if the above is valid now that I have removed "& ~liveIn"
    os << "used undefd (recursively): ";
    locMap.printBitset(cout,recursiveUseUndefSet /*& ~liveIn*/);
    cout << std::endl;
    return os;
}
#endif

/*==============================================================================
 * FUNCTION:        BasicBlock::getCallDest
 * OVERVIEW:        Get the destination of the call, if this is a CALL BB with
 *                    a fixed dest. Otherwise, return -1
 * PARAMETERS:      <none>
 * RETURNS:         Native destination of the call, or -1
 *============================================================================*/
ADDRESS BasicBlock::getCallDest()
{
    if (m_nodeType != CALL)
        return (ADDRESS)-1;
    std::list<RTL*>::reverse_iterator it;
    for (it = m_pRtls->rbegin(); it != m_pRtls->rend(); it++) {
        if ((*it)->getKind() == CALL_RTL)
            return ((HLCall*)(*it))->getFixedDest();
    }
    return (ADDRESS)-1;
}

// serialize this BB
bool BasicBlock::serialize(std::ostream &ouf, int &len)
{
	std::streampos st = ouf.tellp();

	saveFID(ouf, FID_BB_TYPE);
	saveValue(ouf, (char)m_nodeType);

	int ncount = 0;
	for (std::vector<PBB>::iterator it = m_OutEdges.begin(); it != m_OutEdges.end(); it++) 
		ncount++;
	if (ncount) {
		saveFID(ouf, FID_BB_OUTEDGES);
		saveLen(ouf, ncount * 4);
		saveValue(ouf, ncount, false);
		for (
#ifndef WIN32
			std::vector<PBB>::iterator 
#endif
			it = m_OutEdges.begin(); it != m_OutEdges.end(); it++) {
			saveValue(ouf, (*it)->m_nindex, false);
		}
	}

	if (m_pRtls) {
		for (std::list<RTL*>::iterator it = m_pRtls->begin(); it != m_pRtls->end(); it++) {
			saveFID(ouf, FID_BB_RTL);
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
	}

    saveFID(ouf, FID_BB_END);
    saveLen(ouf, 0);

    len = ouf.tellp() - st;
    return true;
}

// deserialize a BB
bool BasicBlock::deserialize(std::istream &inf)
{
    int fid;
    int i, l, j;
    char ch;

	m_nOutEdges.clear();

    while ((fid = loadFID(inf)) != -1 && fid != FID_BB_END) {
        switch (fid) {
            case FID_BB_TYPE:
                loadValue(inf, ch);
                assert(ch <= INVALID);
                m_nodeType = (BBTYPE)ch;
                break;
            case FID_BB_OUTEDGES:
				l = loadLen(inf);
				loadValue(inf, i, false);
				assert(l == i * 4);
				for (j = 0; j < i; j++) {
					int o;
					loadValue(inf, o, false);
					m_nOutEdges.push_back(o);
				}
                break;
            case FID_BB_RTL:
                {
                    int len = loadLen(inf);
                    std::streampos pos = inf.tellg();
                    RTL *rtl = RTL::deserialize(inf);
                    if (rtl) {
                        assert((int)(inf.tellg() - pos) == len);
                        if (m_pRtls == NULL) m_pRtls = new std::list<RTL*>();
                        assert(m_pRtls);
                        m_pRtls->push_back(rtl);
                    } else {
                        // unknown rtl type, skip it
                        inf.seekg(pos + (std::streamoff)len);
                    }
                }
                break;
    /*
    TODO:

    std::list<RTL*>*     m_pRtls;        // Ptr to list of RTLs
    int             m_iLabelNum;    // Nonzero if start of BB needs label
    bool            m_bIncomplete:1;// True if not yet complete
    bool            m_bJumpReqd:1;  // True if jump required for "fall through"
    
    std::vector<PBB>     m_InEdges;      // Vector of in-edges
    std::vector<PBB>     m_OutEdges;     // Vector of out-edges
    int             m_iNumInEdges;  // We need these two because GCC doesn't

    bool            m_iTraversed;   // traversal marker

    Exp* returnLoc;

	*/
			default:
				skipFID(inf, fid);
		}
	}
	assert(loadLen(inf) == 0);

	return true;
}

/*
 * Structuring and code generation.
 *
 * This code is whole heartly based on AST by Doug Simon.  Portions may be
 * copyright to him and are available under a BSD style license.  
 *
 * Adapted for Boomerang by Trent Waddington, 20 June 2002.
 *
 */

/* is this the loop latch node */
bool BasicBlock::isLatchNode() 
{ 
	return (m_loopHead != NULL) && (m_loopHead->m_latchNode == this); 
}

/* get the condition */
Exp *BasicBlock::getCond()
{
	// the condition will be in the last rtl
	assert(m_pRtls);
	RTL *last = m_pRtls->back();
	// it should be a HLJcond
	assert(last->getKind() == JCOND_RTL);
	HLJcond *j = (HLJcond*)last;
	Exp *e = j->getCondExpr();	
	assert(e != NULL);
	return e;
}

void BasicBlock::setCond(Exp *e)
{
	// the condition will be in the last rtl
	assert(m_pRtls);
	RTL *last = m_pRtls->back();
	// it should be a HLJcond
	assert(last->getKind() == JCOND_RTL);
	HLJcond *j = (HLJcond*)last;
	j->setCondExpr(e);
}

/* get the loop body */
BasicBlock *BasicBlock::getLoopBody()
{
	assert(m_structType == PRETESTLOOP || m_structType == POSTTESTLOOP || m_structType == ENDLESSLOOP);
	assert(m_iNumOutEdges == 2);
	if (m_OutEdges[0] != m_loopFollow)
		return m_OutEdges[0];
	return m_OutEdges[1];
}

bool BasicBlock::isAncestorOf(BasicBlock *other)
{           
    return (m_DFTfirst < other->m_DFTfirst && m_DFTlast > other->m_DFTlast) ||
    		(m_DFTrevlast < other->m_DFTrevlast && m_DFTrevfirst > other->m_DFTrevfirst);
}

void BasicBlock::simplify()
{
	for (std::list<RTL*>::iterator it = m_pRtls->begin(); it != m_pRtls->end(); it++)
		(*it)->simplify();
}
        
bool BasicBlock::hasBackEdgeTo(BasicBlock* dest)
{
//	assert(HasEdgeTo(dest) || dest == this);
	return dest == this || dest->isAncestorOf(this);
}

bool BasicBlock::allParentsTraversed()
{
	for (int i = 0; i < m_iNumInEdges; i++)
		if (!m_InEdges[i]->m_iTraversed && m_InEdges[i]->hasBackEdgeTo(this))
			return false;
	return true;
}

// generate code for the body of this BB
void BasicBlock::generateBodyCode(HLLCode &hll, bool dup)
{
	if (!dup)
		hll.AddLabel(this);	
	if (m_pRtls)
		for (std::list<RTL*>::iterator it = m_pRtls->begin(); it != m_pRtls->end(); it++) 
			(*it)->generateCode(hll, this);
}


// generate code for this BB, belonging to the loop specified by latch (initially NULL) 
void BasicBlock::generateCode(HLLCode &hll, BasicBlock *latch, bool loopCond)
{
	if (hll.gotoSetContains(this) && !isLatchNode() && 
		((latch && this == latch->m_loopHead->m_loopFollow) || !allParentsTraversed())) {
		hll.AddGoto(this, this);
		return;
	} else if (hll.followSetContains(this)) {
		if (this != hll.getEnclFollow()) {
			hll.AddGoto(this, this);
		}
		return;
	}

	if (m_iTraversed) {
//		assert(m_sType == Loop && m_lType == PostTested && m_latchNode == this);
		return;
	}
	m_iTraversed = true;

	if (isLatchNode()) {
		// some stuff about indentation level.. this is a little too language specific, so I need to
		// abstract this - trent.
		generateBodyCode(hll); // or generate a goto
		return;
	}

	if (m_structType == NONE && m_iNumOutEdges > 1) {
		// we know it has to be a conditional
		m_structType = IFGOTO;
		m_condFollow = m_OutEdges[1];
	}

	switch(loopCond ? m_loopCondType : m_structType) {
		case PRETESTLOOP:
			// add the follow of the loop (if it exists) to the follow set
			if (m_loopFollow)
				hll.addtoFollowSet(m_loopFollow);

			assert(m_latchNode->m_OutEdges.size() == 1);
			// add anything before the loop test
			generateBodyCode(hll);
			// add the header
			hll.AddPretestedLoopHeader(this, getCond());
			// add the code for the body of the loop
			getLoopBody()->generateCode(hll, m_latchNode);
			// add the code for the latch if not already added
			if (!m_latchNode->isTraversed()) {
				m_latchNode->setTraversed(true);
				m_latchNode->generateBodyCode(hll);
			}
			// add the stuff before the loop again (code duplication, yum)
			generateBodyCode(hll, true);
			// close the loop
			hll.AddPretestedLoopEnd(this);

			// write the code for the follow of the loop (if it exists)
			if (m_loopFollow)
			{
				// remove the follow from the follow set
				hll.removeLastFollow();

				if (!m_loopFollow->isTraversed())
					m_loopFollow->generateCode(hll, latch);
				else
					hll.AddGoto(this, m_loopFollow);
			}
			break;
		case POSTTESTLOOP:
			// add the follow of the loop (if it exists) to the follow set
			if (m_loopFollow)
				hll.addtoFollowSet(m_loopFollow);

			// add the header
			hll.AddPosttestedLoopHeader(this);
			// ensure there is a conditional if more than one out edge
			if (m_OutEdges.size() == 2 && m_loopCondType == NONE) {
				m_loopCondType = IFGOTO;
			}
			// add code for the conditional (if any)
			if (m_loopCondType != NONE) {
				// a conditional inside a loop is a pain
				setTraversed(false);
				generateCode(hll, m_latchNode, true);
			} else {
				// add code for loop body
				generateBodyCode(hll);
				getLoopBody()->generateCode(hll, m_latchNode);
			}

			// add the code for the latch if not already added
			if (!m_latchNode->isTraversed()) {
				m_latchNode->setTraversed(true);
				m_latchNode->generateBodyCode(hll);
			}
			// close the loop
			hll.AddPosttestedLoopEnd(this, getCond());				

			// write the code for the follow of the loop (if it exists)
			if (m_loopFollow)
			{
				// remove the follow from the follow set
				hll.removeLastFollow();

				if (!m_loopFollow->isTraversed())
					m_loopFollow->generateCode(hll, latch);
				else
					hll.AddGoto(this, m_loopFollow);
			}
			break;
		case ENDLESSLOOP:
			// add the follow of the loop (if it exists) to the follow set
			if (m_loopFollow)
				hll.addtoFollowSet(m_loopFollow);
			// add the header
			hll.AddEndlessLoopHeader(this);
			// add code for the conditional (if any)
			if (m_loopCondType != NONE) {
				// a conditional inside a loop is a pain
				setTraversed(false);
				generateCode(hll, m_latchNode, true);
			} else {
				// add code for loop body
				generateBodyCode(hll);
				getLoopBody()->generateCode(hll, m_latchNode);
			}
			// add the code for the latch if not already added
			if (!m_latchNode->isTraversed()) {
				m_latchNode->setTraversed(true);
				m_latchNode->generateBodyCode(hll);
			}
			// close the loop
			hll.AddEndlessLoopEnd(this);

			// write the code for the follow of the loop (if it exists)
			if (m_loopFollow)
			{
				// remove the follow from the follow set
				hll.removeLastFollow();

				if (!m_loopFollow->isTraversed())
					m_loopFollow->generateCode(hll, latch);
				else
					hll.AddGoto(this, m_loopFollow);
			}
			break;
		case JUMPINOUTLOOP:
			break;
		case JUMPINTOCASE:
			break;
		case IFGOTO:
			// add code for the header
			generateBodyCode(hll);
			// add the if header
			hll.AddIfCondHeader(this, getCond());
			// ensure that the follow is one of the out edges
			if (m_condFollow != m_OutEdges[0] && m_condFollow != m_OutEdges[1]) {
				// hopefully the condition hasn't been incorrectly flipped.
				m_condFollow = m_OutEdges[1];
			}
			// ensure out edge 0 is not the follow
			if (m_condFollow == m_OutEdges[0]) {
				// swap the condition and the out edges
				Exp *e = new Unary(opLNot, getCond()->clone());
				e = e->simplify();
				setCond(e);
				PBB tmp = m_OutEdges[0];
				m_OutEdges[0] = m_OutEdges[1];
				m_OutEdges[1] = tmp;
			}
			// add a goto for the body
			hll.AddGoto(this, m_OutEdges[0]);
			// add if end
			hll.AddIfCondEnd(this);
			// add code for the follow
			m_condFollow->generateCode(hll, latch);
			break;
		case IFTHEN:
			// add code for the header
			generateBodyCode(hll);		
			// ensure out edge 0 is not the follow
			if (m_condFollow == m_OutEdges[0]) {
				// swap the condition and the out edges
				Exp *e = new Unary(opLNot, getCond()->clone());
				e = e->simplify();
				setCond(e);
				PBB tmp = m_OutEdges[0];
				m_OutEdges[0] = m_OutEdges[1];
				m_OutEdges[1] = tmp;
			}
			// add the if header
			hll.AddIfCondHeader(this, getCond());
			// add code or goto for the body of the if
			hll.addtoFollowSet(m_condFollow);
			assert(m_condFollow != m_OutEdges[0]);
			if (m_OutEdges[0]->isTraversed() || (m_loopHead && m_OutEdges[0] == m_loopHead->m_loopFollow))
				hll.AddGoto(this, m_OutEdges[0]);
			else {
				m_OutEdges[0]->generateCode(hll, latch);
			}
			// add if end
			hll.AddIfCondEnd(this);
			// remove the follow
			assert(hll.getEnclFollow() == m_condFollow);
			hll.removeLastFollow();
			// add code for the follow
			m_condFollow->generateCode(hll, latch);
			break;
		case IFTHENELSE:
			// add code for the header
			generateBodyCode(hll);
			// add the if header
			hll.AddIfElseCondHeader(this, getCond());
			// add code or goto for the then body of the if
			assert(m_condFollow != m_OutEdges[0]);
			hll.addtoFollowSet(m_condFollow);
			if (m_OutEdges[0]->isTraversed() || (m_loopHead && m_OutEdges[0] == m_loopHead->m_loopFollow))
				hll.AddGoto(this, m_OutEdges[0]);
			else {
				m_OutEdges[0]->generateCode(hll, latch);
			}
			// add if else
			hll.AddIfElseCondOption(this);
			// add code or goto for the then body of the if
			assert(m_condFollow != m_OutEdges[1]);
			if (m_OutEdges[1]->isTraversed() || (m_loopHead && m_OutEdges[1] == m_loopHead->m_loopFollow))
				hll.AddGoto(this, m_OutEdges[1]);
			else {
				m_OutEdges[1]->generateCode(hll, latch);
			}
			// add if end
			hll.AddIfElseCondEnd(this);
			// remove the follow
			assert(hll.getEnclFollow() == m_condFollow);
			hll.removeLastFollow();
			// add code for the follow
			m_condFollow->generateCode(hll, latch);
			break;
		case IFELSE:
			assert(false);
			break;
		case CASE:
			assert(false);
			hll.addtoFollowSet(m_condFollow);

			break;
		case NONE:
			// add code for the body
			generateBodyCode(hll);

			// generate a ret if this is one
			if (m_nodeType == RET) {
				hll.AddReturnStatement(this, NULL);
				return;
			}

			// get child
			BasicBlock *child = getOutEdge(0);
			if (!child) return;

			// if the child has already been traversed
			if (child->isTraversed()) {
				hll.AddGoto(this, child);
				return;
			}

			// or the child is the latch
			if (latch && latch->m_loopHead->m_loopFollow == child) {
				hll.AddGoto(this, child);
				return;
			}

			// or the child is not in the same loop
			if (child->m_loopHead != m_loopHead) {
				// but has more than one untraversed parent or is a follow
				if (!child->allParentsTraversed() || hll.followSetContains(child)) {
					hll.AddGoto(this, child);
					return;
				}
			}

			// or the child is not in the same case
			if (child->m_caseHead != m_caseHead) {
				hll.AddGoto(this, child);
				return;
			}

			// or the child is the follow of this case, add a goto
			if (m_caseHead && child == m_caseHead->m_condFollow) {
				hll.AddGoto(this, child);
				return;
			}

			// otherwise, add the code
			child->generateCode(hll, latch);
			
			break;
	}	
}

void BasicBlock::getUsesAfterDef(Exp *def, UseSet &uses, bool start)
{
	assert(def->isAssign());
	if (m_iTraversed) return;
	m_iTraversed = true;

	if (m_pRtls) {
		std::list<RTL*>::iterator rit = m_pRtls->begin();
		bool rtlstart = false;
		if (start) {
			// must find start RTL containing def
			for (; rit != m_pRtls->end(); rit++) 
				if ((*rit)->containsDef(def))
					break;
			rtlstart = true;
		}
		for (; rit != m_pRtls->end(); rit++) {
			RTL *rtl = *rit;
			if (start && rtlstart) {
				rtl->getUsesAfterDef(def, uses);
				rtlstart = false;
			} else
				rtl->getUsesOf(uses, def->getSubExp1());
		}
	}

	// TODO: recurse
}

bool BasicBlock::getSSADefs(DefSet &defs)
{	
	bool ssa = true;
	for (std::list<RTL*>::iterator it = m_pRtls->begin(); it != m_pRtls->end(); it++) 
		ssa &= (*it)->getSSADefs(defs, ssa);
	return ssa;
}

void BasicBlock::SSAsubscript(SSACounts counts)
{
	if (isTraversed()) return;
	setTraversed(true);

	for (std::list<RTL*>::iterator it = m_pRtls->begin(); it != m_pRtls->end(); it++) 
		(*it)->SSAsubscript(counts);

	for (std::vector<PBB>::iterator bit = m_OutEdges.begin(); bit != m_OutEdges.end(); bit++) {
		(*bit)->SSAsetPhiParams(counts);
		(*bit)->SSAsubscript(counts);
	}
}

bool BasicBlock::isUsedInPhi(Exp *e)
{
	for (std::list<RTL*>::iterator it = m_pRtls->begin(); it != m_pRtls->end(); it++) {
		RTL *rtl = *it;		
		if (rtl->isUsedInPhi(e)) return true;
	}
	return false;
}

void BasicBlock::SSAsetPhiParams(SSACounts &counts)
{
	RTL *rtl = m_pRtls->front();
	
	for (std::list<Exp*>::iterator it = rtl->getList().begin(); it != rtl->getList().end(); it++) {
		if (!(*it)->isAssign()) break;
		// get left and right of assign
		Exp *left = (*it)->getSubExp1();
		Exp *right = (*it)->getSubExp2();
		// right should be phi
		if (right->getOper() != opPhi) break;
		// left should be the unsubscripted definition
		if (left->getOper() == opSubscript) left = left->getSubExp1();
		// get a count for this definition
		int count = counts.getSubscriptFor(left);
		// make a new subscripted use of this definition
		Exp *e = new Binary(opSubscript, left->clone(), new Const(count));
		// get the params of the phi
		Exp *params = right->getSubExp1();
		// append it to the param list
		if (params->getOper() == opNil) {
			// if only param then we just add it
			right->setSubExp1(new Binary(opList, e, new Terminal(opNil)));
		} else {
			// otherwise we traverse the list
			assert(params->getOper() == opList);
			Exp *l;
			for (l = params; l->getOper() == opList && l->getSubExp2()->getOper() != opNil; l = l->getSubExp2())
				;
			assert(l->getOper() == opList && l->getSubExp2()->getOper() == opNil);
			// till we get to the last element and add it there
			l->setSubExp2(new Binary(opList, e, new Terminal(opNil)));
		}
	}
}

void BasicBlock::SSAaddPhiFunctions(std::set<Exp*> &defs)
{
	if (m_InEdges.size() < 2)
		return;

	// get the first RTL or make one
	RTL *rtl;
	if (m_pRtls != NULL && m_pRtls->front()->getKind() == HL_NONE)
		rtl = m_pRtls->front();
	else {
		rtl = new RTL();
		if (m_pRtls == NULL)
			m_pRtls = new std::list<RTL*>();
		m_pRtls->push_front(rtl);
	}
	assert(rtl);

	// add an empty phi function for every definition
	for (std::set<Exp*>::iterator it = defs.begin(); it != defs.end(); it++) {		
		Exp *phi = new AssignExp((*it)->clone(), new Unary(opPhi, new Terminal(opNil)));
		rtl->getList().push_front(phi);
	}
}

bool BasicBlock::minimizeSSAForm()
{
	bool change = false;
	RTL *rtl = m_pRtls->front();
	for (std::list<Exp*>::iterator it = rtl->getList().begin(); it != rtl->getList().end(); it++) {
		Exp *e = *it;
		if (!e->isAssign()) continue;
		if (e->getSubExp2()->getOper() != opPhi) break;

		// two patterns:
		// * if all the params are the same, delete it
		// * if there is only 1 unique param, replace with a rename
		// * otherwise, make sure params are unique

		bool unique = true;
		std::set<Exp*> uparams;
		Exp *params = e->getSubExp2()->getSubExp1(); 
		assert(params->getOper() == opList);
		for (Exp *l = params; l->getOper() != opNil; l = l->getSubExp2()) {
			bool found = false;
			for (std::set<Exp*>::iterator uit = uparams.begin(); uit != uparams.end(); uit++)
				if (**uit == *l->getSubExp1()) {
					found = true;
					unique = false;
					break;
				}
			if (!found)
				uparams.insert(l->getSubExp1()->clone());
		}

		// erase
		if (uparams.size() == 0) {
			delete *it;
			it = rtl->getList().erase(it);
			change = true;
			continue;
		}
		
		// rename
		if (uparams.size() == 1) {
			(*it)->setSubExp2(*(uparams.begin()));
			change = true;
			continue;
		}

		if (!unique) {
			// replace list
			Exp *nl = new Terminal(opNil);
			for (std::set<Exp*>::iterator uit = uparams.begin(); uit != uparams.end(); uit++)
				nl = new Binary(opList, *uit, nl);
			(*it)->getSubExp2()->setSubExp1(nl);
			change = true;
		}
	}
	return change;
}

void BasicBlock::revSSATransform()
{
	if (m_pRtls == NULL) return;

	// remove the phi's from this node, propogating them back up the in edges.
	for (std::list<RTL*>::iterator it = m_pRtls->begin(); it != m_pRtls->end(); it++) {
		RTL *rtl = *it;
		std::list<Exp*> &exps = rtl->getList();
		for (std::list<Exp*>::iterator eit = exps.begin(); eit != exps.end();) {
			Exp *e = *eit;
			if (e->isAssign() && e->getSubExp2()->getOper() == opPhi) {
				// add an assign to each in edge.
				// remove phi
				eit = exps.erase(eit);
				continue;
			}
			eit++;
		}
	}
}

void BasicBlock::getDefs(DefSet &defs, Exp *before_use)
{
	if (m_pRtls == NULL) return;

	for (std::list<RTL*>::iterator it = m_pRtls->begin(); it != m_pRtls->end(); it++) {
		RTL *rtl = *it;		
		rtl->getDefs(defs, before_use);
	}
}

void BasicBlock::getUses(UseSet &uses)
{
	if (m_pRtls == NULL) return;

	for (std::list<RTL*>::iterator it = m_pRtls->begin(); it != m_pRtls->end(); it++) {
		RTL *rtl = *it;
		rtl->getUses(uses);
	}
}

void BasicBlock::getUsesOf(UseSet &uses, Exp *e)
{
	if (m_pRtls == NULL) return;

	for (std::list<RTL*>::iterator it = m_pRtls->begin(); it != m_pRtls->end(); it++) {
		RTL *rtl = *it;
		rtl->getUsesOf(uses, e);
	}
}

void BasicBlock::getKilled(DefSet &killed)
{
	DefSet defs;
	getDefs(defs);
	DefSet liveIn;
	getLiveIn(liveIn);

	for (DefSet::iterator it = defs.begin(); it != defs.end(); it++) {
		Def d;
		if (liveIn.find(*((*it).getLeft()), d))
			killed.insert(d);
	}
}

void BasicBlock::getLiveIn(DefSet &liveIn)
{
	for (int i = 0; i < m_iNumInEdges; i++)
		m_InEdges[i]->getLiveOut(liveIn);
}

void BasicBlock::getLiveOut(DefSet &liveOut)
{
	DefSet killed;
	getKilled(killed);

	getLiveIn(liveOut);

	liveOut.remove(killed);

	getDefs(liveOut);
}

void BasicBlock::getLiveInAt(AssignExp *asgn, std::set<AssignExp*> &livein)
{
	livein.clear();
	for (std::list<RTL*>::iterator rit = m_pRtls->begin(); rit != m_pRtls->end(); rit++) {
		RTL *rtl = *rit;
		switch(rtl->getKind()) {
			case HL_NONE:
				{
					for (std::list<Exp*>::iterator it = rtl->getList().begin(); it != rtl->getList().end(); it++) {
						Exp *e = *it;
						if (e->isAssign()) {
							((AssignExp*)e)->calcLive(livein);
						}
					}
				}
				break;
			case CALL_RTL:
				// a call kills everything (VERY conservative)
				livein.clear();		
				break;
		}
	}
}

