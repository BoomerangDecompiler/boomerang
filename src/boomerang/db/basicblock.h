/*
 * Copyright (C) 1997-2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002-2006, Trent Waddington and Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       basicblock.h
 * \brief   Interface for the basic block class, which form nodes of the control flow graph
 ******************************************************************************/
#pragma once

#include "boomerang/include/managed.h" // For LocationSet etc
#include "boomerang/util/Address.h"

#include <QtCore/QString>

class Location;
class ICodeGenerator;
class BasicBlock;
class RTL;
class Function;
class UserProc;
struct SWITCH_INFO;

/*    *    *    *    *    *    *    *    *    *    *    *    *    *    *    *\
*                                                             *
*    e n u m s   u s e d   i n   C f g . h   a n d   h e r e     *
*                                                             *
\*    *    *    *    *    *    *    *    *    *    *    *    *    *    *    */

/// Depth-first traversal constants.
enum travType
{
    UNTRAVERSED, // Initial value
    DFS_TAG,     // Remove redundant nodes pass
    DFS_LNUM,    // DFS loop stamping pass
    DFS_RNUM,    // DFS reverse loop stamping pass
    DFS_CASE,    // DFS case head tagging traversal
    DFS_PDOM,    // DFS post dominator ordering
    DFS_CODEGEN  // Code generating pass
};

/// an enumerated type for the class of stucture determined for a node
enum structType
{
    Loop,     // Header of a loop only
    Cond,     // Header of a conditional only (if-then-else or switch)
    LoopCond, // Header of a loop and a conditional
    Seq       // sequential statement (default)
};

// an type for the class of unstructured conditional jumps
enum unstructType
{
    Structured, JumpInOutLoop, JumpIntoCase
};

/// an enumerated type for the type of conditional headers
enum CondType
{
    IfThen,     // conditional with only a then clause
    IfThenElse, // conditional with a then and an else clause
    IfElse,     // conditional with only an else clause
    Case        // nway conditional header (case statement)
};

// an enumerated type for the type of loop headers
enum LoopType
{
    PreTested,  // Header of a while loop
    PostTested, // Header of a repeat loop
    Endless     // Header of an endless loop
};

/*    *    *    *    *    *    *    *    *    *\
*                                     *
*    B a s i c B l o c k   e n u m s     *
*                                     *
\*    *    *    *    *    *    *    *    *    */

/// Kinds of basic block nodes
/// reordering these will break the save files - trent
enum class BBTYPE
{
    ONEWAY,   // unconditional branch
    TWOWAY,   // conditional branch
    NWAY,     // case branch
    CALL,     // procedure call
    RET,      // return
    FALL,     // fall-through node
    COMPJUMP, // computed jump
    COMPCALL, // computed call
    INVALID   // invalid instruction
};

enum SBBTYPE
{
    NONE,        // not structured
    PRETESTLOOP, // header of a loop
    POSTTESTLOOP,
    ENDLESSLOOP,
    JUMPINOUTLOOP, // an unstructured jump in or out of a loop
    JUMPINTOCASE,  // an unstructured jump into a case statement
    IFGOTO,        // unstructured conditional
    IFTHEN,        // conditional with then clause
    IFTHENELSE,    // conditional with then and else clauses
    IFELSE,        // conditional with else clause only
    CASE           // case statement (switch)
};

typedef std::list<BasicBlock *>::iterator         BB_IT;
typedef std::list<BasicBlock *>::const_iterator   BBC_IT;

/***************************************************************************/ /**
 * BasicBlock class.
 ******************************************************************************/
class BasicBlock
{
    /// Objects of class Cfg can access the internals of a BasicBlock object.
    friend class Cfg;

public:
    typedef std::vector<BasicBlock *>::iterator   iEdgeIterator;
    typedef std::list<RTL *>::iterator            rtlit;
    typedef std::list<RTL *>::reverse_iterator    rtlrit;
    typedef std::list<SharedExp>::iterator        elit;

protected:
    Function *m_parent;

    /* general basic block information */
    BBTYPE m_nodeType = BBTYPE::INVALID;      ///< type of basic block
    std::list<RTL *> *m_listOfRTLs = nullptr; ///< Ptr to list of RTLs
    int m_labelNum     = 0;                   ///< Nonzero if start of BB needs label
    bool m_labelNeeded = false;
    bool m_incomplete  = true;                ///< True if not yet complete
    bool m_jumpReqd    = false;               ///< True if jump required for "fall through"

    /* in-edges and out-edges */
    std::vector<BasicBlock *> m_inEdges;  ///< Vector of in-edges
    std::vector<BasicBlock *> m_outEdges; ///< Vector of out-edges
    size_t m_targetOutEdges;              ///< support resize() of vectors!

    /* for traversal */
    bool m_traversedMarker = false; ///< traversal marker

    /* Liveness */
    LocationSet m_liveIn;                  ///< Set of locations live at BB start

    /*
     * Depth first traversal of all bbs, numbering as we go and as we come back, forward and reverse passes.
     * Use Cfg::establishDFTOrder() and CFG::establishRevDFTOrder to create these values.
     */
    int m_DFTfirst = 0; ///< depth-first traversal first visit
    int m_DFTlast  = 0; ///< depth-first traversal last visit
    int m_DFTrevfirst;  ///< reverse depth-first traversal first visit
    int m_DFTrevlast;   ///< reverse depth-first traversal last visit

    /// Control flow analysis stuff, lifted from Doug Simon's honours thesis.
    int m_ord;                               ///< node's position within the ordering structure
    int m_revOrd;                            ///< position within ordering structure for the reverse graph
    int m_inEdgesVisited;                    ///< counts the number of in edges visited during a DFS
    int m_numForwardInEdges;                 ///< inedges to this node that aren't back edges
    int m_loopStamps[2], m_revLoopStamps[2]; ///< used for structuring analysis
    travType m_traversed;                    ///< traversal flag for the numerous DFS's
    bool m_emitHLLLabel;                     ///< emit a label for this node when generating HL code?
    QString m_labelStr;                      ///< the high level label for this node (if needed)
    int m_indentLevel;                       ///< the indentation level of this node in the final code

    /* high level structuring */
    SBBTYPE m_loopCondType = NONE; ///< type of conditional to treat this loop header as (if any)
    SBBTYPE m_structType   = NONE; ///< structured type of this node

    // analysis information
    BasicBlock *m_immPDom;         ///< immediate post dominator
    BasicBlock *m_loopHead;        ///< head of the most nested enclosing loop
    BasicBlock *m_caseHead;        ///< head of the most nested enclosing case
    BasicBlock *m_condFollow;      ///< follow of a conditional header
    BasicBlock *m_loopFollow;      ///< follow of a loop header
    BasicBlock *m_latchNode;       ///< latching node of a loop header

    // Structured type of the node
    structType m_structuringType;    ///< the structuring class (Loop, Cond , etc)
    unstructType m_unstructuredType; ///< the restructured type of a conditional header
    LoopType m_loopHeaderType;       ///< the loop type of a loop header
    CondType m_conditionHeaderType;  ///< the conditional type of a conditional header

    // true if processing for overlapped registers on statements in this BB
    // has been completed.
    bool m_overlappedRegProcessingDone = false;

public:
    BasicBlock(Function *parent);
    ~BasicBlock();
    BasicBlock(const BasicBlock& bb);

    /// \brief return enclosing function, null if none
    const Function *getParent() const { return m_parent; }
    Function *getParent()       { return m_parent; }

    /***************************************************************************/ /**
     * \brief   Return the type of the basic block.
     * \returns the type of the basic block
     ******************************************************************************/
    BBTYPE getType();

    QString& getLabelStr() { return m_labelStr; }
    void setLabelStr(const QString& s) { m_labelStr = s; }
    bool isLabelNeeded() { return m_labelNeeded; }
    void setLabelNeeded(bool b) { m_labelNeeded = b; }
    bool isCaseOption();

    /***************************************************************************/ /**
     * \returns true if this BB has been traversed
     ******************************************************************************/
    bool isTraversed();

    /***************************************************************************/ /**
     * \brief        Sets the traversed flag
     * \param        bTraversed true to set this BB to traversed
     ******************************************************************************/
    void setTraversed(bool bTraversed);

    /***************************************************************************/ /**
     * \brief Display the whole BB to the given stream
     *  Used for "-R" option, and handy for debugging
     * \param os - stream to output to
     * \param html - print in html mode
     ******************************************************************************/
    void print(QTextStream& os, bool html = false);
    void printToLog();

    /***************************************************************************/ /**
     * \brief       Print to a static string (for debugging)
     * \returns     Address of the static buffer
     ******************************************************************************/
    const char *prints(); // For debugging
    void dump();

    /***************************************************************************/ /**
     * \brief Update the type and number of out edges. Used for example where a COMPJUMP type is updated to an
     * NWAY when a switch idiom is discovered.
     * \param bbType - the new type
     * \param iNumOutEdges - new number of inedges
     ******************************************************************************/
    void updateType(BBTYPE bbType, uint32_t iNumOutEdges);

    /***************************************************************************/ /**
     * \brief Sets the "jump required" bit. This means that this BB is an orphan
     * (not generated from input code), and that the "fall through" out edge
     * (m_OutEdges[1]) needs to be implemented as a jump. The back end
     * needs to take heed of this bit
     ******************************************************************************/
    void setJumpReqd();


    /***************************************************************************/ /**
     * \brief    Returns the "jump required" bit. See \ref setJumpReqd for details
     * \returns True if a jump is required
     ******************************************************************************/
    bool isJumpReqd();

    /***************************************************************************/ /**
     * \brief Get the lowest real address associated with this BB.
     *  Note that although this is usually the address of the first RTL, it is not
     * always so. For example, if the BB contains just a delayed branch,and the delay
     * instruction for the branch does not affect the branch, so the delay instruction
     * is copied in front of the branch instruction. Its address will be
     * UpdateAddress()'ed to 0, since it is "not really there", so the low address
     * for this BB will be the address of the branch.
     * \returns the lowest real address associated with this BB
     ******************************************************************************/
    Address getLowAddr() const;

    /***************************************************************************/ /**
     * \brief  Get the highest address associated with this BB. This is
     *         always the address associated with the last RTL.
     * \returns the address
     ******************************************************************************/
    Address getHiAddr();

    /***************************************************************************/ /**
     *
     * \brief        Get pointer to the list of RTL*.
     * \returns     the pointer
     ******************************************************************************/
    std::list<RTL *> *getRTLs();

    const std::list<RTL *> *getRTLs() const;

    RTL *getRTLWithStatement(Instruction *stmt);

    /***************************************************************************/ /**
     * \brief Get a constant reference to the vector of in edges.
     * \returns a constant reference to the vector of in edges
     ******************************************************************************/
    std::vector<BasicBlock *>& getInEdges();

    size_t getNumInEdges() const { return m_inEdges.size(); }

    /***************************************************************************/ /**
     * \brief        Get a constant reference to the vector of out edges.
     * \returns            a constant reference to the vector of out edges
     ******************************************************************************/
    const std::vector<BasicBlock *>& getOutEdges();

    void clearOutEdges() { m_outEdges.clear(); } ///< called when noreturn call is found

    /***************************************************************************/ /**
     * \brief Change the given in-edge (0 is first) to the given value
     * Needed for example when duplicating BBs
     * \param i - index (0 based) of in-edge to change
     * \param newIn - pointer to BasicBlock that will be a new parent
     ******************************************************************************/
    void setInEdge(size_t i, BasicBlock *newIn);

    /***************************************************************************/ /**
     * \brief        Change the given out-edge (0 is first) to the given value
     * Needed for example when duplicating BBs
     * \note Cannot add an additional out-edge with this function; use addOutEdge for this rare case
     * \param i - index (0 based) of out-edge to change
     * \param newOutEdge - pointer to BB that will be the new successor
     ******************************************************************************/
    void setOutEdge(size_t i, BasicBlock *newOutEdge);

    /***************************************************************************/ /**
     * \brief        Returns the i-th out edge of this BB; counting starts at 0
     * \param i - index (0 based) of the desired out edge
     * \returns            the i-th out edge; 0 if there is no such out edge
     ******************************************************************************/
    BasicBlock *getOutEdge(size_t i);

    size_t getNumOutEdges() { return m_outEdges.size(); }


    /// \brief Get the index of my in-edges is BB pred
    /// Basically the "whichPred" function as per Briggs, Cooper, et al (and presumably "Cryton, Ferante, Rosen, Wegman, and
    /// Zadek").  Return -1 if not found
    int whichPred(BasicBlock *pred);

    /***************************************************************************/ /**
     * \brief Add the given in-edge
     * Needed for example when duplicating BBs
     * \param newInEdge -  pointer to BB that will be a new parent
     ******************************************************************************/
    void addInEdge(BasicBlock *newInEdge);
    void deleteEdge(BasicBlock *edge);

    /***************************************************************************/ /**
     * \brief Delete the in-edge from the given BB
     *  Needed for example when duplicating BBs
     * \param   it iterator to BB that will no longer be a parent
     * \note    Side effects: The iterator argument is incremented.
     * It should be used like this:
     * \code{.cpp}
     *     if (pred) deleteInEdge(it) else it++;
     * \endcode
     ******************************************************************************/
    void deleteInEdge(std::vector<BasicBlock *>::iterator& it);
    void deleteInEdge(BasicBlock *edge);

    /***************************************************************************/ /**
     * \brief Get the destination of the call, if this is a CALL BB with
     *  a fixed dest. Otherwise, return -1
     * \returns     Native destination of the call, or -1
     ******************************************************************************/
       Address getCallDest();
    Function *getCallDestProc();

    /***************************************************************************/ /**
     * \brief Traverse this node and recurse on its children in a depth first manner.
     * Records the times at which this node was first visited and last visited
     * \param first - the number of nodes that have been visited
     * \param last - the number of nodes that have been visited for the last time during this traversal
     * \returns the number of nodes (including this one) that were traversed from this node
     ******************************************************************************/
    unsigned getDFTOrder(int& first, int& last);

    /***************************************************************************/ /**
     * \brief Traverse this node and recurse on its parents in a reverse depth first manner.
     * Records the times at which this node was first visited and last visited
     * \param first - the number of nodes that have been visited
     * \param last - the number of nodes that have been visited for the last time during this traversal
     * \returns        the number of nodes (including this one) that were traversed from this node
     ******************************************************************************/
    unsigned getRevDFTOrder(int& first, int& last);

    /***************************************************************************/ /**
     * \brief Static comparison function that returns true if the first BB has an
     * address less than the second BB.
     * \param bb1 - first BB
     * \param bb2 - last BB
     * \returns bb1.address < bb2.address
     ******************************************************************************/
    static bool lessAddress(BasicBlock *bb1, BasicBlock *bb2);

    /***************************************************************************/ /**
     * \brief Static comparison function that returns true if the first BB has DFT
     * first order less than the second BB.
     * \param bb1 - first BB
     * \param bb2 - last BB
     * \returns bb1.first_DFS < bb2.first_DFS
     ******************************************************************************/
    static bool lessFirstDFT(BasicBlock *bb1, BasicBlock *bb2);

    /***************************************************************************/ /**
     * \brief Static comparison function that returns true if the first BB has DFT
     * first order less than the second BB.
     * \param bb1 - first BB
     * \param bb2 - last BB
     * \returns bb1.last_DFS < bb2.last_DFS
     ******************************************************************************/
    static bool lessLastDFT(BasicBlock *bb1, BasicBlock *bb2);

    bool isOverlappedRegProcessingDone() const { return m_overlappedRegProcessingDone; }
    void setOverlappedRegProcessingDone() { m_overlappedRegProcessingDone = true; }


    class LastStatementNotABranchError : public std::exception
    {
    public:
        Instruction *stmt;
        LastStatementNotABranchError(Instruction *_stmt)
            : stmt(_stmt) {}
    };

    class LastStatementNotAGotoError : public std::exception
    {
    public:
        Instruction *stmt;
        LastStatementNotAGotoError(Instruction *_stmt)
            : stmt(_stmt) {}
    };


    /// Get the condition
    /*
    * Structuring and code generation.
    *
    * This code is whole heartly based on AST by Doug Simon. Portions may be copyright to him and are available under a BSD
    * style license.
    *
    * Adapted for Boomerang by Trent Waddington, 20 June 2002.
    */
    SharedExp getCond() noexcept(false);

    /** set the condition */
    void setCond(SharedExp e) noexcept(false);

    /** Get the destiantion, if any */
    SharedExp getDest() noexcept(false);

    /** Check for branch if equal relation */
    bool isJmpZ(BasicBlock *dest);

    /** Get the loop body */
    BasicBlock *getLoopBody();

    /** Simplify all the expressions in this BB */
    void simplify();


    /***************************************************************************/ /**
     * \brief        given an address this method returns the corresponding
     *               out edge
     * \param        a the address
     * \returns      the outedge which corresponds to \a a or 0 if there was no such outedge
     ******************************************************************************/
    BasicBlock *getCorrectOutEdge(Address a);
    bool isPostCall();
    static void doAvail(InstructionSet& s, BasicBlock *inEdge);

    /**
     *  Get the destination proc
     *  \note this must be a call BB!
     */
    Function *getDestProc();

    /**
     * Get first/next statement this BB
     * Somewhat intricate because of the post call semantics; these funcs save a lot of duplicated, easily-bugged
     * code
     */
    Instruction *getFirstStmt(rtlit& rit, StatementList::iterator& sit);
    Instruction *getNextStmt(rtlit& rit, StatementList::iterator& sit);
    Instruction *getLastStmt(rtlrit& rit, StatementList::reverse_iterator& sit);
    Instruction *getFirstStmt();
    Instruction *getLastStmt();
    Instruction *getPrevStmt(rtlrit& rit, StatementList::reverse_iterator& sit);

    RTL *getLastRtl() { return m_listOfRTLs->back(); }
    void getStatements(StatementList& stmts) const;

    // Return the first statement number as a string.


    /**
     * Get the statement number for the first BB as a character array.
     * If not possible (e.g. because the BB has no statements), return
     * a unique string (e.g. bb8048c10)
     * \note Used in dotty file generation
     */
    char *getStmtNumber();

public:
    bool isBackEdge(size_t inEdge) const;

    void generateCode(ICodeGenerator *hll, int indLevel, BasicBlock *latch, std::list<BasicBlock *>& followSet,
                      std::list<BasicBlock *>& gotoSet, UserProc *proc);


    /// Prepend an assignment (usually a PhiAssign or ImplicitAssign)
    /// \a proc is the enclosing Proc
    void prependStmt(Instruction *s, UserProc *proc);

    // Liveness
    bool calcLiveness(ConnectionGraph& ig, UserProc *proc);

    /// Locations that are live at the end of this BB are the union of the locations that are live at the start of its
    /// successors
    /// liveout gets all the livenesses, and phiLocs gets a subset of these, which are due to phi statements at the top of
    /// successors
    void getLiveOut(LocationSet& live, LocationSet& phiLocs);

    /// Find indirect jumps and calls
    /// Find any BBs of type COMPJUMP or COMPCALL. If found, analyse, and if possible decode extra code and return true
    bool decodeIndirectJmp(UserProc *proc);

    /***************************************************************************/ /**
     * \brief    Called when a switch has been identified. Visits the destinations of the switch, adds out edges to the
     *                BB, etc
     * \note    Used to be called as soon as a switch statement is discovered, but this causes decoded but unanalysed
     *          BBs (statements not numbered, locations not SSA renamed etc) to appear in the CFG. This caused problems
     *          when there were nested switch statements. Now only called when re-decoding a switch statement
     * \param   proc - Pointer to the UserProc object for this code
     ******************************************************************************/
    void processSwitch(UserProc *proc);

    /**
     * Find the number of cases for this switch statement. Assumes that there is a compare and branch around the indirect
     * branch. Note: fails test/sparc/switchAnd_cc because of the and instruction, and the compare that is outside is not
     * the compare for the upper bound. Note that you CAN have an and and still a test for an upper bound. So this needs
     * tightening.
     * TMN: It also needs to check for and handle the double indirect case; where there is one array (of e.g. ubyte)
     * that is indexed by the actual switch value, then the value from that array is used as an index into the array of
     * code pointers.
     */
    int findNumCases();

    /// Change the BB enclosing stmt to be CALL, not COMPCALL
    bool undoComputedBB(Instruction *stmt);

    /***************************************************************************/ /**
     * \brief        Searches for all instances of "search" and adds them to "result"
     * in reverse nesting order. The search is optionally type sensitive.
     * \note out of date doc, unless type senistivity is a part of \a search_for ?
     * \param search_for - a location to search for
     * \param results - a list which will have any matching exprs
     *                 appended to it
     * \returns true if there were any matches
     ******************************************************************************/
    bool searchAll(const Exp& search_for, std::list<SharedExp>& results);

    /***************************************************************************/ /**
     * \brief Replace all instances of search with replace. Can be type sensitive if
     * reqd
     * \param search - ptr to an expression to search for
     * \param replace the expression with which to replace it
     * \returns true if replacement took place
     ******************************************************************************/
    bool searchAndReplace(const Exp& search, SharedExp replace);

    void generateCode_Loop(ICodeGenerator *hll, std::list<BasicBlock *>& gotoSet, int indLevel, UserProc *proc,
                           BasicBlock *latch, std::list<BasicBlock *>& followSet);

protected:
    void setLoopStamps(int& time, std::vector<BasicBlock *>& order);
    void setRevLoopStamps(int& time);
    void setRevOrder(std::vector<BasicBlock *>& order);

    void setLoopHead(BasicBlock *head) { m_loopHead = head; }
    BasicBlock *getLoopHead() { return m_loopHead; }
    void setLatchNode(BasicBlock *latch) { m_latchNode = latch; }
    bool isLatchNode() { return m_loopHead && m_loopHead->m_latchNode == this; }
    BasicBlock *getLatchNode() { return m_latchNode; }
    BasicBlock *getCaseHead() { return m_caseHead; }
    void setCaseHead(BasicBlock *head, BasicBlock *follow);

    structType getStructType() { return m_structuringType; }
    void setStructType(structType s);
    unstructType getUnstructType();
    void setUnstructType(unstructType us);
    LoopType getLoopType();
    void setLoopType(LoopType l);
    CondType getCondType();
    void setCondType(CondType l);

    void setLoopFollow(BasicBlock *other) { m_loopFollow = other; }
    BasicBlock *getLoopFollow() { return m_loopFollow; }
    void setCondFollow(BasicBlock *other) { m_condFollow = other; }
    BasicBlock *getCondFollow() { return m_condFollow; }

    /// establish if this bb has a back edge to the given destination
    bool hasBackEdgeTo(BasicBlock *dest);

    /// establish if this bb has any back edges leading FROM it
    bool hasBackEdge()
    {
        for (auto bb : m_outEdges) {
            if (hasBackEdgeTo(bb)) {
                return true;
            }
        }

        return false;
    }

    /// establish if this bb is an ancestor of another BB
    bool isAncestorOf(BasicBlock *other);
    bool inLoop(BasicBlock *header, BasicBlock *latch);

    bool isIn(const std::list<BasicBlock *>& set, BasicBlock *bb)
    {
        for (BasicBlock *it : set) {
            if (it == bb) {
                return true;
            }
        }

        return false;
    }

    char *indent(int indLevel, int extra = 0);

    /// Return true if every parent (i.e. forward in edge source) of this node has
    /// had its code generated
    bool allParentsGenerated();

    /// Emits a goto statement (at the correct indentation level) with the destination label for dest. Also places the label
    /// just before the destination code if it isn't already there.    If the goto is to the return block, it would be nice
    /// to
    /// emit a 'return' instead (but would have to duplicate the other code in that return BB).    Also, 'continue' and
    /// 'break'
    /// statements are used instead if possible
    void emitGotoAndLabel(ICodeGenerator *hll, int indLevel, BasicBlock *dest);

    /// Generates code for each non CTI (except procedure calls) statement within the block.
    void WriteBB(ICodeGenerator *hll, int indLevel);

    void addOutEdge(BasicBlock *bb) { m_outEdges.push_back(bb); }

    void addRTL(RTL *rtl)
    {
        if (m_listOfRTLs == nullptr) {
            m_listOfRTLs = new std::list<RTL *>;
        }

        m_listOfRTLs->push_back(rtl);
    }

    void addLiveIn(SharedExp e) { m_liveIn.insert(e); }

private:

    /***************************************************************************/ /**
     * \brief        Private constructor.
     * \param parent - Function this BasicBlock belongs to.
     * \param pRtls - rtl statements that will be contained in this BasicBlock
     * \param bbType - type of BasicBlock
     * \param iNumOutEdges - expected number of out edges from this BasicBlock
     ******************************************************************************/
    BasicBlock(Function *parent, std::list<RTL *> *pRtls, BBTYPE bbType, uint32_t iNumOutEdges);

    /***************************************************************************/ /**
     * \brief        Sets the RTLs for a basic block. This is the only place that
     * the RTLs for a block must be set as we need to add the back link for a call
     * instruction to its enclosing BB.
     * \param rtls - a list of RTLs
     *
     ******************************************************************************/
    void setRTLs(std::list<RTL *> *rtls);
};
