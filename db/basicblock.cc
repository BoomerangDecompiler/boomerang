/*
 * Copyright (C) 1997-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
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

#include "exp.h"
#include "cfg.h"
#include "rtl.h"

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
        m_first(0),
        m_nodeType(INVALID),
        m_pRtls(0),
        m_iLabelNum(0),
        m_bIncomplete(true),
        m_bJumpReqd(false),
        m_iNumInEdges(0),
        m_iNumOutEdges(0),
        m_iTraversed(0)
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
    }
}


/*==============================================================================
 * FUNCTION:        BasicBlock::BasicBlock
 * OVERVIEW:        Copy constructor.    
 * PARAMETERS:      bb - the BB to copy from
 * RETURNS:         <nothing>
 *============================================================================*/
BasicBlock::BasicBlock(const BasicBlock& bb)
    :   m_first(0),
        m_nodeType(bb.m_nodeType),
        m_iLabelNum(bb.m_iLabelNum),
        m_bIncomplete(bb.m_bIncomplete),
        m_bJumpReqd(bb.m_bJumpReqd),
        m_InEdges(bb.m_InEdges),
        m_OutEdges(bb.m_OutEdges),
        m_iNumInEdges(bb.m_iNumInEdges),
        m_iNumOutEdges(bb.m_iNumOutEdges),
        m_iTraversed(0),
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
    :   m_first(0),
        m_nodeType(bbType),
        m_iLabelNum(0),
        m_bIncomplete(false),
        m_bJumpReqd(false),
        m_iNumInEdges(0),
        m_iNumOutEdges(iNumOutEdges),
        m_iTraversed(0)
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
 *		    done by inserting the used/defined places for each register
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
    m_iTraversed = 1;
    
    assert(usedDefineStruct != NULL);
    // First do any alias propagations in this BasicBlock
    usedDefineStruct->propagateRegWithinBB();

    tempPrevBB = prevBlock;
/*
    for (std::vector<PBB>::iterator it = m_OutEdges.begin(); it != m_OutEdges.end();
         it++) {
        PBB child = *it;

	if (child->m_iTraversed == 0) {
	    child->propagateType(this);
	}	
    }        
*/	

    if (!endCase) {	
	for (std::vector<PBB>::iterator it = m_OutEdges.begin(); it != m_OutEdges.end();
             it++) {

	    PBB child = *it;	
	    if (child->m_iTraversed == 0) {
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
    m_OutEdges[i] = pNewOutEdge;
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
    m_first = first;

    unsigned numTraversed = 1;
    m_iTraversed = 1;

    for (std::vector<PBB>::iterator it = m_OutEdges.begin(); it != m_OutEdges.end();
         it++) {

        PBB child = *it;
        if (child->m_iTraversed == 0)
            numTraversed = numTraversed + child->DFTOrder(first,last);
    }

    last++;
    m_last = last;

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
    m_revfirst = first;

    unsigned numTraversed = 1;
    m_iTraversed = 1;

    for (std::vector<PBB>::iterator it = m_InEdges.begin(); it != m_InEdges.end();
         it++) {

        PBB parent = *it;
        if (parent->m_iTraversed == 0)
            numTraversed = numTraversed + parent->RevDFTOrder(first,last);
    }

    last++;
    m_revlast = last;

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
 * FUNCTION:        BasicBlock::lessFirstDFS
 * OVERVIEW:        Static comparison function that returns true if the first BB
 *                  has DFS first order less than the second BB.
 * PARAMETERS:      bb1 - first BB
 *                  bb2 - last BB
 * RETURNS:         bb1.first_DFS < bb2.first_DFS
 *============================================================================*/
bool BasicBlock::lessFirstDFS(PBB bb1, PBB bb2)
{
    return bb1->m_first < bb2->m_first;
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
    if (m_iTraversed != 0) {
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
    m_iTraversed = 1;
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
 * FUNCTION:        BasicBlock::lessLastDFS
 * OVERVIEW:        Static comparison function that returns true if the first BB
 *                  has DFS first order less than the second BB.
 * PARAMETERS:      bb1 - first BB
 *                  bb2 - last BB
 * RETURNS:         bb1.last_DFS < bb2.last_DFS
 *============================================================================*/
bool BasicBlock::lessLastDFS(PBB bb1, PBB bb2)
{
    return bb1->m_last < bb2->m_last;
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

    m_iTraversed = 1;

    for (std::vector<PBB>::iterator it = m_OutEdges.begin(); it != m_OutEdges.end();
         it++)
    {
        PBB child = *it;
        if (child->m_iTraversed == 0)
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

/*==============================================================================
 * FUNCTION:        BasicBlock::getCallDest
 * OVERVIEW:        Get the destination of the call, if this is a CALL BB with
 *                    a fixed dest. Otherwise, return -1
 * PARAMETERS:      <none>
 * RETURNS:         Native destination of the call, or -1
 *============================================================================*/
ADDRESS BasicBlock::getCallDest() const
{
    if (m_nodeType != CALL)
        return 0;
    std::list<RTL*>::reverse_iterator it;
    for (it = m_pRtls->rbegin(); it != m_pRtls->rend(); it++) {
        if ((*it)->getKind() == CALL_RTL)
            return ((HLCall*)(*it))->getFixedDest();
    }
    assert(0);
    return 0;           // Should never happen
}
#endif
