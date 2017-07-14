#pragma once

/*
 * Copyright (C) 1997-2005, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file    cfg.h
 * \brief   Interface for a control flow graph, based on basic block nodes.
 ******************************************************************************/

#include "boomerang/util/types.h"
#include "boomerang/db/exphelp.h" // For lessExpStar

#include <cstdio>                 // For FILE
#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>

#define DEBUG_LIVENESS    (Boomerang::get()->debugLiveness)

class Function;
class Prog;
class UserProc;
class UseSet;
class LocationSet;
class SSACounts;
class BasicBlock;
class ICodeGenerator;
class CallStatement;
class BranchStatement;
class RTL;
struct DOM;

class XMLProgParser;
class Global;
class Parameter;
class ConnectionGraph;
class Instruction;
enum class BBTYPE;

#define BTHEN    0
#define BELSE    1


/***************************************************************************/ /**
 * \class Cfg
 * Control Flow Graph class. Contains all the BasicBlock objects for a procedure.
 * These BBs contain all the RTLs for the procedure, so by traversing the Cfg,
 * one traverses the whole procedure.
 *
 * \var Cfg::myProc
 * Pointer to the UserProc object that contains this CFG object
 * \var Cfg::m_listBB
 * BasicBlock s contained in this CFG
 * \var Cfg::m_ordering
 * Ordering of BBs for control flow structuring
 * \var Cfg::m_revOrdering
 * Ordering of BBs for control flow structuring
 * \var Cfg::m_mapBB
 * The ADDRESS to PBB map.
 * \var Cfg::m_entryBB
 * The CFG entry BasicBlock.
 * \var Cfg::m_exitBB
 * The CFG exit BasicBlock.
 * \var Cfg::m_wellFormed
 * \var Cfg::m_structured
 * \var Cfg::m_callSites
 * Set of the call instructions in this procedure.
 * \var Cfg::m_lastLabel
 * Last label (positive integer) used by any BB this Cfg
 * \var Cfg::m_implicitMap
 * Map from expression to implicit assignment. The purpose is to prevent multiple implicit assignments
 * for the same location.
 * \var Cfg::m_implicitsDone
 * True when the implicits are done; they can cause problems (e.g. with ad-hoc global assignment)
 ******************************************************************************/
class Cfg
{
	friend class XMLProgParser;

	typedef std::set<CallStatement *>                              CallStatementSet;
	typedef std::map<SharedExp, Instruction *, lessExpStar>        ExpStatementMap;

	// A type for the ADDRESS to BB map
	typedef std::map<Address, BasicBlock *, std::less<Address> >   MAPBB;
	typedef std::list<BasicBlock *>::iterator                      BB_IT;
	typedef std::list<BasicBlock *>::const_iterator                BBC_IT;

private:
	mutable bool m_wellFormed;
	bool m_structured;
	bool m_implicitsDone;
	int m_lastLabel;
	UserProc *m_myProc;
	std::list<BasicBlock *> m_listBB;
	std::vector<BasicBlock *> m_ordering;
	std::vector<BasicBlock *> m_revOrdering;

	MAPBB m_mapBB;
	BasicBlock *m_entryBB;
	BasicBlock *m_exitBB;
	CallStatementSet m_callSites;
	ExpStatementMap m_implicitMap;

public:
	class BBAlreadyExistsError : public std::exception
	{
	public:
		BasicBlock *pBB;
		BBAlreadyExistsError(BasicBlock *_pBB)
			: pBB(_pBB) {}
	};

	Cfg();

	/***************************************************************************/ /**
	 * \brief        Destructor. Note: destructs the component BBs as well
	 ******************************************************************************/
	~Cfg();

	/***************************************************************************/ /**
	 * \brief   Set the pointer to the owning UserProc object
	 * \param   proc - pointer to the owning UserProc object
	 ******************************************************************************/
	void setProc(UserProc *proc);

	/***************************************************************************/ /**
	 * \brief        Clear the CFG of all basic blocks, ready for decode
	 ******************************************************************************/
	void clear();

	size_t getNumBBs() { return m_listBB.size(); } ///< Get the number of BBs

	/***************************************************************************/ /**
	 * \brief assignment operator for Cfg's, the BB's are shallow copied
	 * \param other - rhs
	 ******************************************************************************/
	Cfg& operator=(const Cfg& other);        /* Copy constructor */

	/***************************************************************************/ /**
	 * \brief        Add a new basic block to this cfg
	 *
	 * Checks to see if the address associated with pRtls is already in the map as an incomplete BB; if so, it is
	 * completed now and a pointer to that BB is returned. Otherwise, allocates memory for a new basic block node,
	 * initializes its list of RTLs with pRtls, its type to the given type, and allocates enough space to hold
	 * pointers to the out-edges (based on given numOutEdges).
	 * The native address associated with the start of the BB is taken from pRtls, and added to the map (unless 0).
	 *
	 * \note You cannot assume that the returned BB will have the RTL associated with pStart as its first RTL, since
	 * the BB could be split. You can however assume that the returned BB is suitable for adding out edges (i.e. if
	 * the BB is split, you get the "bottom" part of the BB, not the "top" (with lower addresses at the "top").
	 * Returns nullptr if not successful, or if there already exists a completed BB at this address (this can happen
	 * with certain kinds of forward branches).
	 *
	 * \param pRtls list of pointers to RTLs to initialise the BB with bbType: the type of the BB (e.g. TWOWAY)
	 * \param bbType - type of new BasicBlock
	 * \param iNumOutEdges number of out edges this BB will eventually have
	 * \returns Pointer to the newly created BB, or 0 if there is already an incomplete BB with the same address
	 ******************************************************************************/
	BasicBlock *newBB(std::list<RTL *> *pRtls, BBTYPE bbType, uint32_t iNumOutEdges) noexcept(false);

	/***************************************************************************/ /**
	 * \brief Allocates space for a new, incomplete BB, and the given address is
	 * added to the map. This BB will have to be completed before calling WellFormCfg.
	 *
	 * Use this function when there are outedges to BBs that are not created yet. Usually used via addOutEdge()
	 * This function will commonly be called via AddOutEdge()
	 * \returns           pointer to allocated BasicBlock
	 ******************************************************************************/
	BasicBlock *newIncompleteBB(Address addr);

	/***************************************************************************/ /**
	 * \brief Add an out edge to this BB (and the in-edge to the dest BB)
	 *        May also set a label
	 *
	 * Adds an out-edge to the basic block pBB by filling in the first slot that is empty.
	 * \note  a pointer to a BB is given here.
	 *
	 * \note    Overloaded with address as 2nd argument (calls this proc in the end)
	 * \param   pBB source BB (to have the out edge added to)
	 * \param   addr Start address of the BB reached by the out edge
	 * \param   bSetLabel - indicates that label is required in \a pDestBB
	 ******************************************************************************/
	void addOutEdge(BasicBlock *pBB, Address addr, bool bSetLabel = false);

	/***************************************************************************/ /**
	 * \brief        Add an out edge to this BB (and the in-edge to the dest BB)
	 * May also set a label
	 *
	 * Adds an out-edge to the basic block pBB by filling in the first slot that is empty.    Note: an address is
	 * given here; the out edge will be filled in as a pointer to a BB. An incomplete BB will be created if
	 * required. If bSetLabel is true, the destination BB will have its "label required" bit set.
	 *
	 * \note            Calls the above
	 * \param pBB source BB (to have the out edge added to)
	 * \param pDestBB   Destination BB of the out edge
	 * \param bSetLabel if true, set a label at the destination address.  Set true on "true" branches of labels
	 ******************************************************************************/
	void addOutEdge(BasicBlock *pBB, BasicBlock *pDestBB, bool bSetLabel = false);

	/***************************************************************************/ /**
	 *
	 * \brief Adds a label for the given basicblock. The label number will be a non-zero integer
	 *
	 *        Sets a flag indicating that this BB has a label, in the sense that a label is required in the
	 * translated source code
	 * \note         The label is only set if it was not set previously
	 * \param        pBB Pointer to the BB whose label will be set
	 ******************************************************************************/
	void setLabel(BasicBlock *pBB);

	/***************************************************************************/ /**
	 * \brief Get the first BB of this cfg
	 *
	 * Gets a pointer to the first BB this cfg. Also initialises `it' so that calling GetNextBB will return the
	 * second BB, etc.  Also, *it is the first BB.  Returns 0 if there are no BBs this CFG.
	 *
	 * \param       it set to an value that must be passed to getNextBB
	 * \returns     Pointer to the first BB this cfg, or nullptr if none
	 ******************************************************************************/
	BasicBlock *getFirstBB(BB_IT& it);
	const BasicBlock *getFirstBB(BBC_IT& it) const;

	/***************************************************************************/ /**
	 * \brief Get the next BB this cfg. Basically increments the given iterator and returns it
	 *
	 * Gets a pointer to the next BB this cfg. `it' must be from a call to GetFirstBB(), or from a subsequent call
	 * to GetNextBB().  Also, *it is the current BB.  Returns 0 if there are no more BBs this CFG.
	 *
	 * \param   it - iterator from a call to getFirstBB or getNextBB
	 * \returns pointer to the BB, or nullptr if no more
	 ******************************************************************************/
	BasicBlock *getNextBB(BB_IT& it);
	const BasicBlock *getNextBB(BBC_IT& it) const;

	/*
	 * An alternative to the above is to use begin() and end():
	 */
	typedef BB_IT iterator;
	iterator begin() { return m_listBB.begin(); }
	iterator end() { return m_listBB.end(); }

	/* Checks whether the given native address is a label (explicit or non explicit) or not.  Explicit labels are
	 * addresses that have already been tagged as being labels due to transfers of control to that address.
	 * Non explicit labels are those that belong to basic blocks that have already been constructed (i.e. have
	 * previously been parsed) and now need to be made explicit labels.     In the case of non explicit labels, the
	 * basic block is split into two and types and edges are adjusted accordingly. pNewBB is set to the lower part
	 * of the split BB.
	 * Returns true if the native address is that of an explicit or non explicit label, false otherwise. */

	/***************************************************************************/ /**
	 * \brief    Checks whether the given native address is a label (explicit or non explicit) or not. Returns false for
	 *                incomplete BBs.
	 *
	 *  So it returns true iff the address has already been decoded in some BB. If it was not
	 *  already a label (i.e. the first instruction of some BB), the BB is split so that it becomes a label.
	 *  Explicit labels are addresses that have already been tagged as being labels due to transfers of control
	 *  to that address, and are therefore the start of some BB.     Non explicit labels are those that belong
	 *  to basic blocks that have already been constructed (i.e. have previously been parsed) and now need to
	 *  be made explicit labels. In the case of non explicit labels, the basic block is split into two and types
	 *  and edges are adjusted accordingly. If \a pCurBB is the BB that gets split, it is changed to point to the
	 *  address of the new (lower) part of the split BB.
	 *  If there is an incomplete entry in the table for this address which overlaps with a completed address,
	 *  the completed BB is split and the BB for this address is completed.
	 *
	 * \param         uNativeAddr - native (source) address to check
	 * \param         pNewBB - See above
	 * \returns       True if \a uNativeAddr is a label, i.e. (now) the start of a BB
	 *                Note: pCurBB may be modified (as above)
	 ******************************************************************************/
	bool label(Address uNativeAddr, BasicBlock *& pNewBB);

	/***************************************************************************/ /**
	 * \brief        Return true if given address is the start of an incomplete basic block
	 *
	 * Checks whether the given native address is in the map. If not, returns false. If so, returns true if it is
	 * incomplete. Otherwise, returns false.
	 *
	 * \param       addr Address to look up
	 * \returns     True if uAddr starts an incomplete BB
	 ******************************************************************************/
	bool isIncomplete(Address addr) const;

	/***************************************************************************/ /**
	 * \brief Return true if the given address is the start of a basic block, complete or not
	 *
	 * Just checks to see if there exists a BB starting with this native address. If not, the address is NOT added
	 * to the map of labels to BBs.
	 * \note must ignore entries with a null pBB, since these are caused by
	 * calls to Label that failed, i.e. the instruction is not decoded yet.
	 *
	 * \param        uNativeAddr native address to look up
	 * \returns      True if uNativeAddr starts a BB
	 ******************************************************************************/
	bool existsBB(Address uNativeAddr) const;

	/***************************************************************************/ /**
	 * \brief   Sorts the BBs in a cfg by first address. Just makes it more convenient to read when BBs are
	 * iterated.
	 *
	 * Sorts the BBs in the CFG according to the low address of each BB.  Useful because it makes printouts easier,
	 * if they used iterators to traverse the list of BBs.
	 ******************************************************************************/
	void sortByAddress();

	/***************************************************************************/ /**
	 * \brief        Sorts the BBs in a cfg by their first DFT numbers.
	 ******************************************************************************/
	void sortByFirstDFT();

	/***************************************************************************/ /**
	 * \brief        Sorts the BBs in a cfg by their last DFT numbers.
	 ******************************************************************************/
	void sortByLastDFT();

	/***************************************************************************/ /**
	 * \brief Checks that all BBs are complete, and all out edges are valid. However, ADDRESSes that are
	 * interprocedural out edges are not checked or changed.
	 *
	 * Transforms the input machine-dependent cfg, which has ADDRESS labels for each out-edge, into a machine-
	 * independent cfg graph (i.e. a well-formed graph) which has references to basic blocks for each out-edge.
	 *
	 * \returns True if transformation was successful
	 ******************************************************************************/
	bool wellFormCfg() const;

	/***************************************************************************/ /**
	 * Given two basic blocks that belong to a well-formed graph, merges the second block onto the first one and
	 * returns the new block.  The in and out edges links are updated accordingly.
	 * Note that two basic blocks can only be merged if each has a unique out-edge and in-edge respectively, and
	 * these edges correspond to each other.
	 *
	 * \returns            true if the blocks are merged.
	 ******************************************************************************/
	bool mergeBBs(BasicBlock *pb1, BasicBlock *pb2);

	/***************************************************************************/ /**
	 * \brief   Compress the CFG. For now, it only removes BBs that are just branches
	 *
	 * Given a well-formed cfg, optimizations are performed on the graph to reduce the number of basic blocks
	 * and edges.
	 * Optimizations performed are: removal of branch chains (i.e. jumps to jumps), removal of redundant jumps (i.e.
	 *  jump to the next instruction), merge basic blocks where possible, and remove redundant basic blocks created
	 *  by the previous optimizations.
	 * \returns            Returns false if not successful.
	 ******************************************************************************/
	bool compressCfg();

	/***************************************************************************/ /**
	 * \brief        Given a well-formed cfg graph, a partial ordering is established between the nodes.
	 *
	 *   The ordering is based on the final visit to each node during a depth first traversal such that if node n1 was
	 * visited for the last time before node n2 was visited for the last time, n1 will be less than n2.
	 * The return value indicates if all nodes where ordered. This will not be the case for incomplete CFGs
	 * (e.g. switch table not completely recognised) or where there are nodes unreachable from the entry
	 * node.
	 * \returns            all nodes where ordered
	 ******************************************************************************/
	bool establishDFTOrder();

	/***************************************************************************/ /**
	 * \brief        Performs establishDFTOrder on the reverse (flip) of the graph, assumes: establishDFTOrder has
	 *                    already been called
	 *
	 * \returns            all nodes where ordered
	 ******************************************************************************/
	bool establishRevDFTOrder();

	/***************************************************************************/ /**
	 * \brief   Return an index for the given PBB
	 *
	 * Given a pointer to a basic block, return an index (e.g. 0 for the first basic block, 1 for the next, ... n-1
	 * for the last BB.
	 *
	 * \note Linear search: O(N) complexity
	 * \param pBB - BasicBlock to find
	 * \returns     Index, or -1 for unknown PBB
	 ******************************************************************************/
	int pbbToIndex(const BasicBlock *pBB);

	/***************************************************************************/ /**
	 * \brief   Reset all the traversed flags.
	 *
	 * Reset all the traversed flags.
	 * To make this a useful public function, we need access to the traversed flag with other public functions.
	 ******************************************************************************/
	void unTraverse();

	/***************************************************************************/ /**
	 * \brief Query the wellformed'ness status
	 * \returns WellFormed
	 ******************************************************************************/
	bool isWellFormed();

	/***************************************************************************/ /**
	 * \brief Return true if there is a BB at the address given whose first RTL is an orphan,
	 * i.e. GetAddress() returns 0.
	 ******************************************************************************/
	bool isOrphan(Address uAddr);

	/***************************************************************************/ /**
	 * \brief Amalgamate the RTLs for pb1 and pb2, and place the result into pb2
	 *
	 * This is called where a two-way branch is deleted, thereby joining a two-way BB with it's successor.
	 * This happens for example when transforming Intel floating point branches, and a branch on parity is deleted.
	 * The joined BB becomes the type of the successor.
	 *
	 * \note Assumes that fallthrough of *pb1 is *pb2
	 *
	 * \param   pb1 pointers to the BBs to join
	 * \param   pb2 pointers to the BBs to join
	 * \returns True if successful
	 ******************************************************************************/
	bool joinBB(BasicBlock *pb1, BasicBlock *pb2);

	/***************************************************************************/ /**
	 * \brief Completely remove a BB from the CFG.
	 ******************************************************************************/
	void removeBB(BasicBlock *bb);

	/***************************************************************************/ /**
	 * \brief Add a call to the set of calls within this procedure.
	 * \param call - a call instruction
	 ******************************************************************************/
	void addCall(CallStatement *call);

	/***************************************************************************/ /**
	 * \brief    Get the set of calls within this procedure.
	 * \returns  the set of calls within this procedure
	 ******************************************************************************/
	CallStatementSet& getCalls();

	/***************************************************************************/ /**
	 * \brief Replace all instances of \a search with \a replace in all BasicBlock's
	 * belonging to this Cfg. Can be type sensitive if reqd
	 *
	 * \param search a location to search for
	 * \param replace the expression with which to replace it
	 ******************************************************************************/
	void searchAndReplace(const Exp& search, const SharedExp& replace);

	bool searchAll(const Exp& search, std::list<SharedExp>& result);
	Exp *getReturnVal();

	/***************************************************************************/ /**
	 * \brief Structures the control flow graph
	 *******************************************************************************/
	void structure();

	/***************************************************************************/ /**
	 * \brief Remove Junction statements
	 *******************************************************************************/
	void removeJunctionStatements();

	/// return a bb given an address
	BasicBlock *bbForAddr(Address addr) { return m_mapBB[addr]; }

	/***************************************************************************/ /**
	* \brief Simplify all the expressions in the CFG
	******************************************************************************/
	void simplify();

	/**
	 * \brief Change the BB enclosing stmt to be CALL, not COMPCALL
	 */
	void undoComputedBB(Instruction *stmt);

private:

	/***************************************************************************/ /**
	 * Split the given basic block at the RTL associated with uNativeAddr. The first node's type becomes
	 * fall-through and ends at the RTL prior to that associated with uNativeAddr.  The second node's type becomes
	 * the type of the original basic block (pBB), and its out-edges are those of the original basic block.
	 * In edges of the new BB's descendants are changed.
	 * \pre assumes uNativeAddr is an address within the boundaries of the given basic block.
	 * \param   pBB -  pointer to the BB to be split
	 * \param   uNativeAddr - address of RTL to become the start of the new BB
	 * \param   pNewBB -  if non zero, it remains as the "bottom" part of the BB, and splitBB only modifies the top part
	 * to not overlap.
	 * \param   bDelRtls - if true, deletes the RTLs removed from the existing BB after the split point. Only used if
	 *                there is an overlap with existing instructions
	 * \returns Returns a pointer to the "bottom" (new) part of the split BB.
	 ******************************************************************************/
	BasicBlock *splitBB(BasicBlock *pBB, Address uNativeAddr, BasicBlock *pNewBB = nullptr, bool bDelRtls = false);

	/***************************************************************************/ /**
	 * \brief Complete the merge of two BBs by adjusting in and out edges.  If bDelete is true, delete pb1
	 *
	 * Completes the merge of pb1 and pb2 by adjusting out edges. No checks are made that the merge is valid
	 * (hence this is a private function) Deletes pb1 if bDelete is true
	 *
	 * \param pb1 pointers to the two BBs to merge
	 * \param pb2 pointers to the two BBs to merge
	 * \param bDelete if true, pb1 is deleted as well
	 *
	 ******************************************************************************/
	void completeMerge(BasicBlock *pb1, BasicBlock *pb2, bool bDelete = false);

	/***************************************************************************/ /**
	 * \brief        Check the entry BB pointer; if zero, emit error message
	 *                      and return true
	 * \returns            true if was null
	 ******************************************************************************/
	bool checkEntryBB();

public:

	/**
	 * Split the given BB at the RTL given, and turn it into the BranchStatement given. Sort out all the in and out
	 * edges.
	 */

	/*    pBB-> +----+    +----+ <-pBB
	 *   Change | A  | to | A  | where A and B could be empty. S is the string
	 *          |    |    |    | instruction (with will branch to itself and to the
	 *          +----+    +----+ start of the next instruction, i.e. the start of B,
	 *          | S  |      |       if B is non empty).
	 *          +----+      V
	 *          | B  |    +----+ <-skipBB
	 *          |    |    +-b1-+              b1 is just a branch for the skip part
	 *          +----+      |
	 *                      V
	 *                    +----+ <-rptBB
	 *                    | S' |              S' = S less the skip and repeat parts
	 *                    +-b2-+              b2 is a branch for the repeat part
	 *                      |
	 *                      V
	 *                    +----+ <-newBb
	 *                    | B  |
	 *                    |    |
	 *                    +----+
	 * S is an RTL with 6 statements representing one string instruction (so this function is highly specialised for the job
	 * of replacing the %SKIP and %RPT parts of string instructions)
	 */

	BasicBlock *splitForBranch(BasicBlock *pBB, RTL *rtl, BranchStatement *br1, BranchStatement *br2, BB_IT& it);

	/////////////////////////////////////////////////////////////////////////
	// Control flow analysis stuff, lifted from Doug Simon's honours thesis.
	/////////////////////////////////////////////////////////////////////////
	void setTimeStamps();

	/// Finds the common post dominator of the current immediate post dominator and its successor's immediate post dominator
	BasicBlock *commonPDom(BasicBlock *curImmPDom, BasicBlock *succImmPDom);

	/**
	 * Finds the immediate post dominator of each node in the graph PROC->cfg.
	 *
	 * Adapted version of the dominators algorithm by Hecht and Ullman;
	 * finds immediate post dominators only.
	 * \note graph should be reducible
	 */
	void findImmedPDom();

	/// Structures all conditional headers (i.e. nodes with more than one outedge)
	void structConds();

	/// \pre The graph for curProc has been built.
	/// \post Each node is tagged with the header of the most nested loop of which it is a member (possibly none).
	/// The header of each loop stores information on the latching node as well as the type of loop it heads.
	void structLoops();

	/// This routine is called after all the other structuring has been done. It detects conditionals that are in fact the
	/// head of a jump into/outof a loop or into a case body. Only forward jumps are considered as unstructured backward
	/// jumps will always be generated nicely.
	void checkConds();

	/// \pre  The loop induced by (head,latch) has already had all its member nodes tagged
	/// \post The type of loop has been deduced
	void determineLoopType(BasicBlock *header, bool *& loopNodes);

	/// \pre  The loop headed by header has been induced and all it's member nodes have been tagged
	/// \post The follow of the loop has been determined.
	void findLoopFollow(BasicBlock *header, bool *& loopNodes);

	/// \pre header has been detected as a loop header and has the details of the
	///        latching node
	/// \post the nodes within the loop have been tagged
	void tagNodesInLoop(BasicBlock *header, bool *& loopNodes);

	void removeUnneededLabels(ICodeGenerator *hll);
	void generateDotFile(QTextStream& of);

	/////////////////////////////////////////////////////////////////////////
	// Get the entry-point or exit BB
	/////////////////////////////////////////////////////////////////////////
	BasicBlock *getEntryBB() { return m_entryBB; }
	BasicBlock *getExitBB() { return m_exitBB; }

	/////////////////////////////////////////////////////////////////////////
	// Set the entry-point BB (and exit BB as well)
	/////////////////////////////////////////////////////////////////////////

	/***************************************************************************/ /**
	 * \brief       Set the entry and calculate exit BB pointers
	 * \note        Each cfg should have only one exit node now
	 * \param       bb pointer to the entry BB
	 ******************************************************************************/
	void setEntryBB(BasicBlock *bb);
	void setExitBB(BasicBlock *bb);

	BasicBlock *findRetNode();

	/***************************************************************************/ /**
	 * \brief Set an additional new out edge to a given value
	 *
	 * Append a new out-edge from the given BB to the other given BB
	 * Needed for example when converting a one-way BB to a two-way BB
	 *
	 * \note        Use BasicBlock::setOutEdge() for the common case where an existing out edge is merely changed
	 * \note        Use Cfg::addOutEdge for ordinary BB creation; this is for unusual cfg manipulation
	 * \note        side effect : Increments m_iNumOutEdges
	 *
	 * \param fromBB pointer to the BB getting the new out edge
	 * \param newOutEdge pointer to BB that will be the new successor
	 ******************************************************************************/
	void addNewOutEdge(BasicBlock *fromBB, BasicBlock *newOutEdge);

	/////////////////////////////////////////////////////////////////////////
	// print this cfg, mainly for debugging
	/////////////////////////////////////////////////////////////////////////
	void print(QTextStream& out, bool html = false);
	void printToLog();
	void dump();            // Dump to LOG_STREAM()
	void dumpImplicitMap(); // Dump the implicit map to LOG_STREAM()

	/**
	 * \brief Check for indirect jumps and calls. If any found, decode the extra code and return true
	 */
	bool decodeIndirectJmp(UserProc *proc);

	/////////////////////////////////////////////////////////////////////////
	// Implicit assignments
	/////////////////////////////////////////////////////////////////////////

	/// Find or create an implicit assign for x
	Instruction *findImplicitAssign(SharedExp x);

	/// Find the existing implicit assign for x (if any)
	Instruction *findTheImplicitAssign(const SharedExp& x);

	/// Find exiting implicit assign for parameter p
	Instruction *findImplicitParamAssign(Parameter *p);

	/// Remove an existing implicit assignment for x
	void removeImplicitAssign(SharedExp x);

	bool implicitsDone() const { return m_implicitsDone; }    ///<  True if implicits have been created
	void setImplicitsDone() { m_implicitsDone = true; } ///< Call when implicits have been created
	void findInterferences(ConnectionGraph& ig);
	void appendBBs(std::list<BasicBlock *>& worklist, std::set<BasicBlock *>& workset);
	void removeUsedGlobals(std::set<Global *>& unusedGlobals);
	void bbSearchAll(Exp *search, std::list<SharedExp>& result, bool ch);

	bool removeOrphanBBs();

protected:
	void addBB(BasicBlock *bb) { m_listBB.push_back(bb); }
};
