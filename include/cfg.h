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
 * 18 Apr 02 - Mike: Mods for boomerang
 * 04 Dec 02 - Mike: Added isJmpZ
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
    void print(std::ostream& os, bool withDF = false);
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
     * Resets the DFA sets of this BB.
     */
    void resetDFASets();

	/* is this the latch node */
	bool isLatchNode();

	/* get the condition */
	Exp *getCond();

	/* set the condition */
	void setCond(Exp *e);

	/* Check if there is a jump if equals relation */
	bool isJmpZ(PBB dest);

	/* get the loop body */
	BasicBlock *getLoopBody();

	// establish if this bb has a back edge to the given destination
	bool hasBackEdgeTo(BasicBlock *dest);

	// establish if this bb is an ancestor of another BB
	bool isAncestorOf(BasicBlock *other);

	/* Simplify all the expressions in this BB
	 */
	void simplify();


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

public:

	/* stuff for new data flow analysis */
	void getLiveInAt(Statement *stmt, std::set<Statement*> &livein);
	void getLiveIn(std::set<Statement*> &livein);
	void calcLiveOut(std::set<Statement*> &live);
        std::set<Statement*> &getLiveOut() { return liveout; }

	/* set the return value */
	void setReturnVal(Exp *e);
	Exp *getReturnVal() { return m_returnVal; }

protected:
        std::set<Statement*> liveout;

    Exp* m_returnVal;

    /*
     * This field is used to test our assumption that the
     * substitution for a register in this BB is the same no matter
     * which entry to this BB is taken.
     */
    std::map<Exp*,Exp*> regSubs;

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
     * Set the return value for this CFG 
     * (assumes there is only one exit bb)
     */
    void setReturnVal(Exp *e);
    Exp *getReturnVal();

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
    void updateLiveness();
    std::set<Statement*> &getLiveOut() { return liveout; }

	/*
	 * Virtual Function Call analysis
	 */
	void virtualFunctionCalls(Prog* prog);

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

	/* Simplify all the expressions in the CFG
	 */
	void simplify();

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
     * Intersection of all statements live at the end of all the ret bbs.
     */
    std::set<Statement*> liveout;

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
    void print(std::ostream &out, bool withDF = false);
  
};              /* Cfg */

#endif
