/*
 * Copyright (C) 1997-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       cfg.h
 * OVERVIEW:   Interface for a control flow graph, based on basic block nodes.
 *============================================================================*/

/*
 * $Revision$
 * Dec 97 - created by Mike based on Cristina's initial implementation
 * 19 Feb 98  - Cristina 
 *  changed RTLLIT for RTL_IT as this name is defined in the new rtl.h. 
 * 25 Feb 98 - Cristina 
 *  made RTL iterator arguments in Init() and NewBB() constant iterators.
 *  moved enumerated type BBTYPE from the RTL files to this file as a BBTYPE
 *      is a property of a basic block, not of a register transfer list.
 * 3 Mar 98 - Cristina
 *  replaced ADDR for ADDRESS.
 *  use of ADDRESS and RTL types based on whether the Cfg object is used
 *      on its own or as part of the uqbt tool. 
 * 11 Mar 98 - Cristina  
 *  replaced BOOL for bool type (C++'s), same for TRUE and FALSE.
 * 24 Mar 98 - Cristina
 *  replaced driver include to global.h. 
 * 26 Mar 98 - Cristina
 *  added AddInterProcOutEdge().
 * 7 Aug 98 - Mike
 *  Changed Init() to a constructor - decided it's OK to have constructors
    that may fail
 * 13 Aug 98 - Mike
 *  Removed the ADDRESS parameter from NewBB - can get it from itFirst
 * 14 Aug 98 - Mike
 *  m_listBB is a list of pointers now, rather than BBs. Needed so that
 *  changes made to this list happen to all BBs
 * 19 Aug 98 - Mike
 *  Out edges are all pointers to BBs now; added m_bIncomplete to BasicBlock
 * 1 Sep 98 - Mike
 *  NewBB returns "bottom" part of BB if split, and returns 0 if BB already
 *  exists
 * 1 Sep 98 - Mike
 *  splitBB takes another parameter, in case we already have a BB for the
 *  "bottom" half
 * 23 Sep 98 - Mike
 *  Added GetFirstRtl() and GetLastRtl() to class BasicBlock
 * 29 Sep 98 - Mike
 *  Added GetOutEdges() to class BasicBlock
 * 10 Dec 98 - Mike: changes for WriteCfgFile() and WriteBBFile()
 * 12 Dec 98 - Mike: changes for AddCoverage()
 * 22 Dec 98 - Mike: itFirstRtl etc are not const now
 * 22 Jan 99 - Mike: Replaced m_it[First,Last]Rtl with m_pRtls
 * 27 Jan 99 - Mike: Use COMPJUMP and COMPCALL BBTYPEs now
 * 04 Feb 99 - Mike: added GetInEdges()
 * 25 Mar 99 - Mike: added Cfg::JoinBB()
 * 07 Apr 99 - Mike: attempted to isolate from other .h files
 * 09 Apr 99 - Mike: WriteDotFile() takes the entry address now
 * 27 Apr 99 - Doug: Added liveness sets for analysis.
 * 28 Apr 99 - Doug: Removed AddInEdges (was enclosed in #if 0)
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 25 Aug 99 - Mike: addCoverage() -> addExCoverage()
 * 15 Mar 00 - Cristina: BasicBlock::setAFP and Cfg::setAFP transformed 
 *              to setAXP
 * 05 May 00 - Mike: Changed BasicBlock::addOutEdge() to addNewOutEdge()
 * 19 May 00 - Mike: Moved addNewOutEdge() from BasicBlock to Cfg class
 * 28 Sep 00 - Mike: Added setTraversed() and isTraversed(); joinBB has only
 *              2 parameters now
 * 19 Sep 00 - Mike: Added isDefined()
 * 14 Feb 01 - Mike: Trivial documentation update
 * 31 Mar 01 - Mike: getCallDest returns -1 if not a fixed address now
 * 19 Jun 01 - Mike: added getOutEdge
 * 12 Jul 01 - Mike: added getCorrectOutEdge method
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList,
 *              getLrtls to getHRTLs, setRTLs to setHRTLs,
 *              and RTL_IT to HRTLList_IT.
 * 13 Aug 01 - Bernard: Added support for type analysis
 * 16 Oct 01 - Mike: Added BITSET returnLoc to class BasicBlock
 * 18 Apr 02 - Mike: Mods for boomerang
 */
 
#ifndef _CFG_H_
#define _CFG_H_

#define COVERAGE 0      // Attempt to save memory

#include <stdio.h>      // For FILE
#include <list>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <string>
#include "types.h"

//#include "bitset.h"     // Saves time. Otherwise, any implementation file that 
                        // defines say a BB, will need to #include this file

class Exp;
class AssignExp;
class Proc;
class UserProc;
class UseSet;
class DefSet;
class SSACounts;
class BinaryFile;
// For Type Analysis
class BBBlock;
class BasicBlock;
typedef BasicBlock* PBB;
class HLLCode;
class HLCall;
class RTL;

// Kinds of basic block nodes
// reordering these will break the save files - trent
enum BBTYPE {
    ONEWAY,                  // unconditional branch
    TWOWAY,                  // conditional branch
    NWAY,                    // case branch
    CALL,                    // procedure call
    RET,                     // return
    FALL,                    // fall-through node
    COMPJUMP,                // computed jump
    COMPCALL,                // computed call
    INVALID                  // invalid instruction
};

enum SBBTYPE {
	NONE,					 // not structured
	PRETESTLOOP,			 // header of a loop
	POSTTESTLOOP,
	ENDLESSLOOP,
	JUMPINOUTLOOP,			 // an unstructured jump in or out of a loop
	JUMPINTOCASE,			 // an unstructured jump into a case statement
	IFGOTO,					 // unstructured conditional
	IFTHEN,				 	 // conditional with then clause
	IFTHENELSE,				 // conditional with then and else clauses
	IFELSE,					 // conditional with else clause only
	CASE					 // case statement (switch)
};

typedef std::list<PBB>::iterator BB_IT;
/*==============================================================================
 * BasicBlock class. <more comments>
 *============================================================================*/
class BasicBlock {
    /*
     * Objects of class Cfg can access the internals of a BasicBlock object.
     */
    friend class Cfg;

public:
    /*
     * Constructor.
     */
    BasicBlock();

    /*
     * Destructor.
     */
    ~BasicBlock();

    /*
     * Copy constructor.
     */
    BasicBlock(const BasicBlock& bb);

    /*
     * Return the type of the basic block.
     */
    BBTYPE getType();

    /*
     * Check if this BB has a label. If so, return the numeric value of
     * the label (nonzero integer). If not, returns zero.
     * See also Cfg::setLabel()
     */
    int getLabel();

	std::string &getLabelStr() { return m_labelStr; }
	void setLabelStr(std::string &s) { m_labelStr = s; }
	bool isLabelNeeded() { return m_labelneeded; }
	void setLabelNeeded(bool b) { m_labelneeded = b; }

    /*
     * Return whether this BB has been traversed or not
     */
    bool isTraversed();

    /*
     * Set the traversed flag
     */
    void setTraversed(bool bTraversed);

    /*
     * Print the BB. For -R and for debugging
	 * Don't use = std::cout, because gdb doesn't know about std::
     */
    void print(std::ostream& os);
	void print() {print(std::cout);}

    /*
     * Set the type of the basic block.
     */
    void updateType(BBTYPE bbType, int iNumOutEdges);

    /*
     * Set the "jump reqd" bit. This means that this is an orphan BB (it is
     * generated, not part of the original program), and that the "fall through"
     * out edge (m_OutEdges[1]) has to be implemented as a jump. The back end
     * needs to take heed of this bit
     */
    void setJumpReqd();

    /*
     * Check if jump is required (see above).
     */
    bool isJumpReqd();

    /*
     * Adds an interprocedural out-edge to the basic block pBB that 
     * represents this address.  The mapping between addresses and 
     * basic blocks is done when the graph is well-formed. 
     * Returns true if successful.
     */
    void addInterProcOutEdge(ADDRESS adr);
    bool addProcOutEdge (ADDRESS addr);

    /*
     * Get the address associated with the BB
     * Note that this is not always the same as getting the address
     * of the first RTL (e.g. if the first RTL is a delay instruction of
     * a DCTI instruction; then the address of this RTL will be 0)
     */
    ADDRESS getLowAddr();
    ADDRESS getHiAddr();

    /*
     * Get ptr to the list of RTLs.
     */
    std::list<RTL*>* getRTLs();

    /*
     * Get the set of in edges.
     */
    std::vector<PBB>& getInEdges();

    /*
     * Get the set of out edges.
     */
    std::vector<PBB>& getOutEdges();

    /*
     * Set an in edge to a new value; same number of in edges as before
     */
    void setInEdge(int i, PBB newIn);

    /*
     * Set an out edge to a new value; same number of out edges as before
     */
    void setOutEdge(int i, PBB newInEdge);

    /*
     * Get the n-th out edge or 0 if it does not exist
     */
    PBB getOutEdge(unsigned int i);

    /*
     * Add an in-edge
     */
    void addInEdge(PBB newInEdge);

    /*
     * Delete an in-edge
     */
    void deleteInEdge(std::vector<PBB>::iterator& it);

    /*
     * If this is a call BB, find the fixed destination (if any)
     * Returns -1 otherwise
     */
    ADDRESS getCallDest();

    /*
     * Get the coverage (total number of bytes for this BB).
     */
    unsigned getCoverage();

    /*
     * Traverse this node and recurse on its children in a depth first manner.
     * Records the times at which this node was first visited and last visited.
     * Returns the number of nodes traversed.
     */
    unsigned DFTOrder(int& first, int& last);

    /*
     * Traverse this node and recurse on its parents in a reverse depth first manner.
     * Records the times at which this node was first visited and last visited.
     * Returns the number of nodes traversed.
     */
    unsigned RevDFTOrder(int& first, int& last);

    /*
     * Static comparison function that returns true if the first BB has an
     * address less than the second BB.
     */
    static bool lessAddress(PBB bb1, PBB bb2);

    /*
     * Static comparison function that returns true if the first BB has an
     * DFT first number less than the second BB.
     */
    static bool lessFirstDFT(PBB bb1, PBB bb2);

    /*
     * Static comparison function that returns true if the first BB has an
     * DFT last less than the second BB.
     */
    static bool lessLastDFT(PBB bb1, PBB bb2);

    /*
     * (see comment for Proc::subAXP)
     */
    void subAXP(std::map<Exp*, Exp*> subMap);

    /*
     * Resets the DFA sets of this BB.
     */
    void resetDFASets();

	/* is this the latch node */
	bool isLatchNode();

	/* get the condition */
	Exp *getCond();

	/* set the condition */
	void setCond(Exp *e);

	/* get the loop body */
	BasicBlock *getLoopBody();

	// establish if this bb has a back edge to the given destination
	bool hasBackEdgeTo(BasicBlock *dest);

	// establish if this bb is an ancestor of another BB
	bool isAncestorOf(BasicBlock *other);

	/* Simplify all the expressions in this BB
	 */
	void simplify();

#if 0
    /*
     * Build the set of locations that are defined by this BB as well as the set
     * of locations that are used before definition or don't have a definition
     * in this BB.
     */
    void buildUseDefSets(LocationMap& locMap, LocationFilter* filter,
        Proc* proc);

    /*
     * Build the set of locations used before definition in the subgraph headed
     * by this BB.
     */
    void buildRecursiveUseUndefSet();
#endif

#if 0
    /*
     * Return a reference to the liveOut set of this BB.
     */
    BITSET& getLiveOuts();

    /*
     * Return a set which is liveOut & !useSet
     */
    BITSET getLiveOutUnuseds() const;

    /*
     * Return a reference to the set of locations used before definition in the
     * subgraph headed by this BB.
     */
    BITSET& getRecursiveUseUndefSet();
#endif

    /*
     * Print any data flow analysis info gathered for this BB.
     */
    std::ostream& printDFAInfo(std::ostream& os);

#if 0
    /*
     * Set the returnLoc member appropriately given the return location
     */
    void setReturnLoc(LocationMap& locMap, Exp* loc);

    /*
     * Return true if this node is dominated by that node
     */
    bool isDominatedBy(PBB pbb) { return dominators.test(pbb->m_index); }

    /*
     * Return true if the location represented by this bit is defined in this BB
     */
    bool isDefined(int bit);
#endif

    /*
     *  given an address, returns the outedge which corresponds to that address
     *  or 0 if there was no such outedge
     */

    PBB getCorrectOutEdge(ADDRESS a);
    
	/*
	 * Depth first traversal of all bbs, numbering as we go and as we come back,
	 * forward and reverse passes.  Use Cfg::establishDFTOrder() and 
	 * CFG::establishRevDFTOrder to create these values.
	 */
    int         m_DFTfirst;        // depth-first traversal first visit
    int         m_DFTlast;         // depth-first traversal last visit
    int         m_DFTrevfirst;     // reverse depth-first traversal first visit
    int         m_DFTrevlast;      // reverse depth-first traversal last visit

#if 0
    BITSET& getPostDominatorSet() { return postdominators; }

        
    /*
     * The following are used for type analysis
     *
     * This function is use to gather and store the
     * use define chains for each register within
     * a basic block
     */     
    void storeUseDefineStruct(BBBlock& inBlock);
    
    /*
     * Propagates type information between basic blocks
     * within a function
     */    
    void propagateType(BasicBlock * prevBlock, bool endCase = false );
    
    /* 
     * Used by propagate type to back track the
     * parent BB. The value of this variable is 
     * meaningless otherwise
     */
    BasicBlock * tempPrevBB;
#endif

private:
    /*
     * Constructor. Called by Cfg::NewBB.
     */
    BasicBlock(std::list<RTL*>* pRtls, BBTYPE bbType, int iNumOutEdges);

    /*
     * Sets the RTLs for this BB. This is the only place that
     * the RTLs for a block must be set as we need to add the back
     * link for a call instruction to its enclosing BB.
     */
    void setRTLs(std::list<RTL*>* rtls);

#if 0
    /*
     * Build the sets of locations that are live into and out of this basic
     * block. Returns true if there was a change in the live out set.
     */
    bool buildLiveInOutSets(const BITSET* callDefines = NULL);
#endif

	// serialize the basic block
	bool serialize(std::ostream &ouf, int &len);

	// deserialize a basic block
	bool deserialize(std::istream &inf);

	// used during serialization
	std::vector<int> m_nOutEdges;     // numerical outedges
	int m_nindex;					// numerical index of this bb in list of bbs in CFG.

	// establish that all "parents" - ie. inedges that are not back edges - have been traversed
	bool allParentsTraversed();

public:

	// code generation
	void generateCode(HLLCode &hll, BasicBlock *latch, bool loopCond = false);
	void generateBodyCode(HLLCode &hll, bool dup = false);

/* high level structuring */
	SBBTYPE		m_structType;   // structured type of this node
	SBBTYPE		m_loopCondType;	// type of conditional to treat this loop header as (if any)
	PBB			m_loopHead;     // head of the most nested enclosing loop
	PBB			m_caseHead;		// head of the most nested enclosing case
	PBB			m_condFollow;	// follow of a conditional header
	PBB			m_loopFollow;	// follow of a loop header
	PBB			m_latchNode;	// latch node of a loop header  

protected:
/* general basic block information */
    BBTYPE          m_nodeType;     // type of basic block
    std::list<RTL*>*     m_pRtls;        // Ptr to list of RTLs
    int             m_iLabelNum;    // Nonzero if start of BB needs label
	std::string		m_labelStr;		// string label of this bb.
	bool			m_labelneeded;
    bool            m_bIncomplete;  // True if not yet complete
    bool            m_bJumpReqd;    // True if jump required for "fall through"

/* in-edges and out-edges */
    std::vector<PBB>     m_InEdges;      // Vector of in-edges
    std::vector<PBB>     m_OutEdges;     // Vector of out-edges
    int             m_iNumInEdges;  // We need these two because GCC doesn't
    int             m_iNumOutEdges; // support resize() of vectors!

/* for traversal */
    bool            m_iTraversed;   // traversal marker

/* Yet Another Implementation of dataflow analysis.  All but one of these
 * will go eventually, and we'll have something stable - trent 
 *
 * So how about some explanation of these things:
 *
 * * defs = set of assignment expressions in this BB defining a value
 * * uses = set of expressions in this BB where defs are used.
 * * liveIn = set of defs that reach this BB
 * * killed = set of expressions in liveIn which are overwritten by defs
 * * liveOut = liveIn - killed + defs
 *
 * This implementation tries to be a little more OO than the dragon book.
 */
public:

	/* Get all the definitions in this BB.  Returns true if the
	 * BB is in SSA form.
     */
	bool getSSADefs(DefSet &defs);

	/* Return true if this expression is used in a phi
	 */
	bool isUsedInPhi(Exp *e);

	/* stuff for new data flow analysis */
	void getLiveInAt(AssignExp *asgn, std::set<AssignExp*> &livein);
	void getLiveIn(std::set<AssignExp*> &livein);
	void calcLiveOut(std::set<AssignExp*> &live);
        void getLiveOut(std::set<AssignExp*> &liveout) {
            liveout = this->liveout;
        }
        void calcUses();

protected:
        std::set<AssignExp*> liveout;

	void getUses(UseSet &uses);
	void getUsesOf(UseSet &uses, Exp *e);
	void getDefs(DefSet &defs, Exp *before_use = NULL);
	void getKilled(DefSet &killed);
	void getLiveIn(DefSet &liveIn);
	void getLiveOut(DefSet &liveOut);

	/* Get all the uses of a given def after it's definition.
	 * This function recurses the remainder of the cfg and 
	 * does not assume ssa form.
	 */
	void getUsesAfterDef(Exp *def, UseSet &uses, bool start = false);

	/* Subscript the definitions in this BB to SSA form.
	 */
	void SSAsubscript(SSACounts counts);

	/* Set the parameters of a phi function based on current
	 * counts.
	 */
	void SSAsetPhiParams(SSACounts &counts);

	/* Add phi functions in every BB with more than one in edge for every def.
	 * takes a map of unique definitions (lhs of assigns) to subscript values.
	 */
	void SSAaddPhiFunctions(std::set<Exp*> &defs);

	/* Minimize the number of phi functions in this node.
	 * returns true if anything changed.
	 */
	bool minimizeSSAForm();

	/* Reverse the SSA transformation on this node.
	 */
	void revSSATransform();

/* others to come later for analysis purposes */

#if 0
    /*
     * The set of dominators of this bb.
     */
    BITSET dominators;

    /*
     * A vector of dominators of this bb.
     */
    std::vector<PBB> m_dominatedBy;

    /*
     * The set of post dominators of this bb.
     */
    BITSET postdominators;

    /*
     * A vector of postdominators of this bb.
     */
    std::vector<PBB> m_postdominatedBy;

    /*
     * The set of locations that are live on entry to this BB on any inedge.
     */
    BITSET liveIn;

    /*
     * The set of locations that are live on exit from this BB.
     */
    BITSET liveOut;

    /*
     * The set of locations that are assigned to in this BB.
     */
    BITSET defSet;

    /*
     * The set of locations that are used in this BB.
     */
    BITSET useSet;

    /*
     * The set of locations that are used before being defined in this BB.
     * Ignores the liveIn definitions.
     */
    BITSET useUndefSet;

    /*
     * The set of locations that are used before definition in the subgraph
     * headed by this BB.
     * = useUndefSet | Union(all outedge recursiveUseUndefSet & ~liveOut)
     * (see buildRecursiveUseUndefSet())
     */
    BITSET recursiveUseUndefSet;
#endif

    /*
     * If this is a CALL BB, (eventually including computed calls), this bitset
     * has the return location determined by analyseCaller
    * Do we want this here?
     */
    Exp* returnLoc;

    /*
     * This field is used to test our assumption that the
     * substitution for a register in this BB is the same no matter
     * which entry to this BB is taken.
     */
    std::map<Exp*,Exp*> regSubs;
    

#if 0
public:
    /*
     * Bernard: This data structure is used in each basic block in order
     * to store the used/define chains for each register
     */	 
    BBBlock * usedDefineStruct;
#endif
};

    // A type for the ADDRESS to BB map
typedef std::map<ADDRESS, PBB, std::less<ADDRESS> >   MAPBB;

/*==============================================================================
 * Control Flow Graph class. Contains all the BasicBlock objects for a
 * procedure. These BBs contain all the RTLs for the procedure, so by traversing
 * the Cfg, one traverses the whole procedure.
 *============================================================================*/
class Cfg {
public:
    /*
     * Constructor.
     */
    Cfg();

    /*
     * Destructor.
     */
    ~Cfg();

    /*
     * Set the pointer to the owning UserProc object
     */
    void setProc(UserProc* proc);

	/*
	 * clear this CFG of all basic blocks, ready for decode
	 */
	void clear();

    /*
     * Equality operator.
     */
    const Cfg& operator=(const Cfg& other); /* Copy constructor */

    /*
     * Checks to see if the address associated with pRtls is already
     * in the map as an incomplete BB; if so, it is completed now and
     * a pointer to that BB is returned. Otherwise,
     * allocates memory for a new basic block node, initializes its list
     * of RTLs with pRtls, its type to the given type, and allocates
     * enough space to hold pointers to the out-edges (based on given
     * numOutEdges).
     * The native address associated with the start of the BB is taken
     * from pRtls, and added to the map (unless 0).
     * NOTE: You cannot assume that the returned BB will have the RTL
     * associated with pStart as its first RTL, since the BB could be
     * split. You can however assume that the returned BB is suitable for
     * adding out edges (i.e. if the BB is split, you get the "bottom"
     * part of the BB, not the "top" (with lower addresses at the "top").
     * Returns NULL if not successful, or if there already exists a
     * completed BB at this address (this can happen with certain kinds of
     * forward branches).
     */
    PBB newBB ( std::list<RTL*>* pRtls, BBTYPE bbType, int iNumOutEdges);

    /*
     * Allocates space for a new, incomplete BB, and the given address is
     * added to the map. This BB will have to be completed before calling
     * WellFormCfg. This function will commonly be called via AddOutEdge()
     */
    PBB newIncompleteBB(ADDRESS addr);

    /*
     * Remove the incomplete BB at uAddr, if any. Was used when dealing
     * with the SKIP instruction, but no longer.
     */
    void removeIncBB(ADDRESS uAddr);

    /*
     * Adds an out-edge to the basic block pBB by filling in the first
     * slot that is empty.  Note: an address is given here; the out edge
     * will be filled in as a pointer to a BB. An incomplete BB will be
     * created if required. If bSetLabel is true, the destination BB will
     * have its "label required" bit set.
     */
    void addOutEdge(PBB pBB, ADDRESS adr, bool bSetLabel = false);

    /*
     * Adds an out-edge to the basic block pBB by filling in the first
     * slot that is empty.  Note: a pointer to a BB is given here.
     */
    void addOutEdge(PBB pBB, PBB pDestBB, bool bSetLabel = false);

    /*
     * Add a label for the given basicblock. The label number must be
     * a non-zero integer
     */
    void setLabel(PBB pBB);

    /*
     * Gets a pointer to the first BB this cfg. Also initialises `it' so
     * that calling GetNextBB will return the second BB, etc.
     * Also, *it is the first BB.
     * Returns 0 if there are no BBs this CFG.
     */
    PBB getFirstBB(BB_IT& it);

    /*
     * Gets a pointer to the next BB this cfg. `it' must be from a call to
     * GetFirstBB(), or from a subsequent call to GetNextBB().
     * Also, *it is the current BB.
     * Returns 0 if there are no more BBs this CFG.
     */
    PBB getNextBB(BB_IT& it);


    /*
     * Checks whether the given native address is a label (explicit or non
     * explicit) or not.  Explicit labels are addresses that have already 
     * been tagged as being labels due to transfers of control to that 
     * address.  Non explicit labels are those that belong to basic blocks 
     * that have already been constructed (i.e. have previously been parsed) 
     * and now need to be made explicit labels.  In the case of non explicit 
     * labels, the basic block is split into two and types and edges are 
     * adjusted accordingly. pNewBB is set to the lower part of the split BB.
     * Returns true if the native address is that of an explicit or non 
     * explicit label, false otherwise.
     */ 
    bool label ( ADDRESS uNativeAddr, PBB& pNewBB );

    /*
     * Checks whether the given native address is in the map. If not,
     *  returns false. If so, returns true if it is incomplete.
     *  Otherwise, returns false.
     */
    bool isIncomplete ( ADDRESS uNativeAddr );

    /*
     * Just checks to see if the native address is a label. If not, the
     *  address is not added to the map of Lables to BBs.
     */
    bool isLabel ( ADDRESS uNativeAddr );

    /*
     * Sorts the BBs in the CFG according to the low address of each BB.
     * Useful because it makes printouts easier, if they used iterators
     * to traverse the list of BBs.
     */
    void sortByAddress ();

    /*
     * Sorts the BBs in the CFG by their first DFT numbers.
     */
    void sortByFirstDFT();

    /*
     * Sorts the BBs in the CFG by their last DFT numbers.
     */
    void sortByLastDFT();

    /*
     * Updates m_vectorBB to m_listBB
     */
    void updateVectorBB();

    /*
     * Transforms the input machine-dependent cfg, which has ADDRESS labels for
     * each out-edge, into a machine-independent cfg graph (i.e. a well-formed
     * graph) which has references to basic blocks for each out-edge.
     * Returns false if not successful.
     */
    bool wellFormCfg ( );

    /*
     * Given two basic blocks that belong to a well-formed graph, merges the 
     * second block onto the first one and returns the new block.  The in and 
     * out edges links are updated accordingly. 
     * Note that two basic blocks can only be merged if each has a unique 
     * out-edge and in-edge respectively, and these edges correspond to each
     * other.  
     * Returns true if the blocks are merged.
     */
    bool mergeBBs ( PBB pb1, PBB pb2 );
 

    /*
     * Given a well-formed cfg graph, optimizations are performed on
     * the graph to reduce the number of basic blocks and edges.  
     * Optimizations performed are: removal of branch chains (i.e. jumps
     * to jumps), removal of redundant jumps (i.e. jump to the next 
     * instruction), merge basic blocks where possible, and remove
     * redundant basic blocks created by the previous optimizations.  
     * Returns false if not successful.
     */
    bool compressCfg ( );


    /*
     * Given a well-formed cfg graph, a partial ordering is established between
     * the nodes. The ordering is based on the final visit to each node during a
     * depth first traversal such that if node n1 was visited for the last time
     * before node n2 was visited for the last time, n1 will be less than n2.
     * The return value indicates if all nodes where ordered. This will not be
     * the case for incomplete CFGs (e.g. switch table not completely
     * recognised) or where there are nodes unreachable from the entry node.
     */
    bool establishDFTOrder();

    /*
     * Performs establishDFTOrder on the inverse of the graph (ie, flips the graph)
     */
    bool establishRevDFTOrder();

    /*
     * Writes to an already opened .dot (dotty) file a graph
     * for the current object's well-formed cfg. 
     * All node IDs are offset by iOffset
     * BinaryFile is used to find the names of library functions
     * Native entry address for this proc is uEntryAddr
     * Returns false if not successful.
     */
    bool writeDotFile (FILE* fi, const char* pName, int iOffset,
        BinaryFile* pBF, ADDRESS uEntryAddr);

    /*
     * Given a pointer to a basic block, return an index (e.g. 0 for the first
     * basic block, 1 for the next, ... n-1 for the last BB.
     */
    int pbbToIndex (PBB pBB);

    /*
     * Reset all the traversed flags.
     * To make this a useful public function, we need access to the
     * traversed flag with other public functions.
    */
    void unTraverse ( );

    /*
     * Return true if the CFG is well formed.
     */
    bool isWellFormed ( );

    /*
     * Return true if there is a BB at the address given whose first
     * RTL is an orphan, i.e. GetAddress() returns 0.
     */
    bool isOrphan ( ADDRESS uAddr);

    /*
     * Add the indicated number of bytes to the total coverage for the cfg.
     * Needed for example with NOP instuctions in delay slots, and for
     * switch tables, etc. Inlined for efficiency.
     */
    void addExCoverage(unsigned u)
    {
        m_uExtraCover += u;
    }

    /*
     * Return the number of bytes occupied by the instructions in this
     * Cfg.
     */
    unsigned getCoverage();

    /*
     * This is called where a two-way branch is deleted, thereby joining
     * a two-way BB with it's successor. This happens for example when
     * transforming Intel floating point branches, and a branch on parity
     * is deleted. The joined BB becomes the type of the successor.
     * Returns true if succeeds.
     */
    bool joinBB( PBB pb1, PBB pb2);

    /*
     * (see comment for Proc::subXFP)
     */
    void subAXP(std::map<Exp*,Exp*>& subMap);

    /*
     * Resets the DFA sets of all the BBs.
     */
    void resetDFASets();

    /*
     * Add a call to the set of calls within this procedure.
     */
    void addCall(HLCall* call);

    /*
     * Get the set of calls within this procedure.
     */
    std::set<HLCall*>& getCalls();

    /*
     * Replace all instances of search with replace.
     * Can be type sensitive if reqd
     */
    void searchAndReplace(Exp* search, Exp* replace);

    /*
     * Print any data flow analysis info gathered so far such as
     * the live-in and live-out sets for each BB.
     */
    std::ostream& printDFAInfo(std::ostream& os);

    /*
     * Set the return location for a geven procedure
     */
    void setReturnLoc(Proc* proc, Exp* loc);

    /*
     * Structures the control flow graph
     */
    void structure();

    /*
     * Computes dominators for each BB.
     */
    void computeDominators();

    /*
     * Computes postdominators for each BB.
     */
    void computePostDominators();

    /*
     * Compute liveness/use information
     */
    void computeDataflow();

    std::vector<PBB> m_vectorBB; // faster access

	// serialize the CFG
	bool serialize(std::ostream &ouf, int &len);

	// deserialize a CFG
	bool deserialize(std::istream &inf);

	/* make the given BB into a call followed by a ret 
	 * and remove all the orphaned nodes.
	 */
	void makeCallRet(PBB head, Proc *p);

	/* return a bb given an address */
	PBB bbForAddr(ADDRESS addr) { return m_mapBB[addr]; }

	/* Get all the definitions in this CFG.  Returns true if the
	 * CFG is in SSA form.
     */
	bool getSSADefs(DefSet &defs);

	/* Transform the CFG to SSA form.
	 */
	void SSATransform(DefSet &defs);

	/* Transform the CFG from SSA form.
	 */
	void revSSATransform();

	/* Minimize the CFG, returns true if anything changed
	 */
	bool minimizeSSAForm();

	/* Get all uses of a given expression
	 */
	void getAllUses(Exp *def, UseSet &uses);
	void getAllUses(UseSet &uses);

	/* Propogate a given expression forward in the CFG
	 */
	void propogateForward(Exp *e);

	/* Return true if this expression is used in a phi
	 */
	bool isUsedInPhi(Exp *e);

	/* Simplify all the expressions in the CFG
	 */
	void simplify();

	/* Find the basic block containing the given expression
	 */
	PBB findBBWith(Exp *exp);

	/* does a simple forward propogation
	 */
	void simplePropogate(Exp *def);

private:

    /*
     * Split the given basic block at the RTL associated with uNativeAddr.
     * The first node's type becomes fall-through and ends at the
     * RTL prior to that associated with uNativeAddr.  The second node's
     * type becomes the type of the original basic block (pBB), and its
     * out-edges are those of the original basic block.
     * Precondition: assumes uNativeAddr is an address within the boundaries
     * of the given basic block.
     * If pNewBB is non zero, it is retained as the "bottom" part of the
     * split, i.e. splitBB just changes the "top" BB to not overlap the
     * existing one.
     * Returns a pointer to the "bottom" (new) part of the BB.
     */
    PBB splitBB (PBB pBB, ADDRESS uNativeAddr, PBB pNewBB = 0,
        bool bDelRtls = false);

    /*
     * Completes the merge of pb1 and pb2 by adjusting out edges. No checks
     * are made that the merge is valid (hence this is a private function)
     * Deletes pb1 if bDelete is true
     */
    void completeMerge(PBB pb1, PBB pb2, bool bDelete);


    /*
     * checkEntryBB: emit error message if this pointer is null
     */
    bool checkEntryBB();

protected:

    /*
     * Pointer to the UserProc object that contains this CFG object
     */
    UserProc* myProc;

    /*
     * The list of pointers to BBs.
     */
    std::list<PBB> m_listBB;

    /*
     * The ADDRESS to PBB map.
     */
    MAPBB m_mapBB;

    /*
     * The entry BB.
     */
    BasicBlock* entryBB;

    /*
     * True if well formed.
     */
    bool m_bWellFormed;

    /*
     * Extra coverage for NOPs and switch tables.
     */
    unsigned m_uExtraCover;

    /*
     * Set of the call instructions in this procedure.
     */
    std::set<HLCall*> callSites;

    /*
     * Last label (positive integer) used by any BB this Cfg
     */
    int lastLabel;

public:
    /*
     * Get the entry-point BB
     */
    PBB getEntryBB() { return entryBB;}

    /*
     * Set the entry-point BB
     */
    void setEntryBB(PBB bb) { entryBB = bb;}

    /*
     * Set an additional new out edge to a given value
     */
    void addNewOutEdge(PBB fromBB, PBB newOutEdge);

    // print this cfg, mainly for debugging
    void print(std::ostream &out);
  
};              /* Cfg */

#endif
