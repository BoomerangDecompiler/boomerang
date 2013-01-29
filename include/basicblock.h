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

/***************************************************************************//**
 * \file       basicblock.h
 * \brief   Interface for the basic block class, which form nodes of the control flow graph
 ******************************************************************************/

/*
 * $Revision$    // 1.1.2.2
 *
 * 28 Jun 05 - Mike: Split off from cfg.h
 */

#ifndef __BASIC_BLOCK_H__
#define __BASIC_BLOCK_H__

#if defined(_MSC_VER)
#pragma warning(disable:4290)
#endif
#include "types.h"
#include "managed.h"            // For LocationSet etc

class Location;
class HLLCode;
class BasicBlock;
class RTL;
class Proc;
class UserProc;
struct SWITCH_INFO;                // Declared in include/statement.h

typedef BasicBlock* PBB;

/*    *    *    *    *    *    *    *    *    *    *    *    *    *    *    *\
*                                                             *
*    e n u m s   u s e d   i n   C f g . h   a n d   h e r e     *
*                                                             *
\*    *    *    *    *    *    *    *    *    *    *    *    *    *    *    */

// Depth-first traversal constants.
enum travType {
    UNTRAVERSED,   // Initial value
    DFS_TAG,       // Remove redundant nodes pass
    DFS_LNUM,       // DFS loop stamping pass
    DFS_RNUM,       // DFS reverse loop stamping pass
    DFS_CASE,       // DFS case head tagging traversal
    DFS_PDOM,       // DFS post dominator ordering
    DFS_CODEGEN       // Code generating pass
};

// an enumerated type for the class of stucture determined for a node
enum structType {
    Loop,       // Header of a loop only
    Cond,       // Header of a conditional only (if-then-else or switch)
    LoopCond,  // Header of a loop and a conditional
    Seq           // sequential statement (default)
};

// an type for the class of unstructured conditional jumps
enum unstructType {
    Structured,
    JumpInOutLoop,
    JumpIntoCase
};


// an enumerated type for the type of conditional headers
enum condType {
    IfThen,        // conditional with only a then clause
    IfThenElse, // conditional with a then and an else clause
    IfElse,        // conditional with only an else clause
    Case        // nway conditional header (case statement)
};

// an enumerated type for the type of loop headers
enum loopType {
    PreTested,       // Header of a while loop
    PostTested,       // Header of a repeat loop
    Endless           // Header of an endless loop
};

/*    *    *    *    *    *    *    *    *    *\
*                                     *
*    B a s i c B l o c k   e n u m s     *
*                                     *
\*    *    *    *    *    *    *    *    *    */

// Kinds of basic block nodes
// reordering these will break the save files - trent
enum BBTYPE {
    ONEWAY,                     // unconditional branch
    TWOWAY,                     // conditional branch
    NWAY,                     // case branch
    CALL,                     // procedure call
    RET,                     // return
    FALL,                     // fall-through node
    COMPJUMP,                 // computed jump
    COMPCALL,                 // computed call
    INVALID                     // invalid instruction
};

enum SBBTYPE {
    NONE,                     // not structured
    PRETESTLOOP,             // header of a loop
    POSTTESTLOOP,
    ENDLESSLOOP,
    JUMPINOUTLOOP,             // an unstructured jump in or out of a loop
    JUMPINTOCASE,             // an unstructured jump into a case statement
    IFGOTO,                     // unstructured conditional
    IFTHEN,                     // conditional with then clause
    IFTHENELSE,                 // conditional with then and else clauses
    IFELSE,                     // conditional with else clause only
    CASE                     // case statement (switch)
};

typedef std::list<PBB>::iterator BB_IT;

/***************************************************************************//**
 * BasicBlock class. <more comments>
 ******************************************************************************/
class BasicBlock {
        /*
         * Objects of class Cfg can access the internals of a BasicBlock object.
         */
        friend class Cfg;
public:
                        BasicBlock();
                        ~BasicBlock();
                        BasicBlock(const BasicBlock& bb);
        BBTYPE          getType();

        int             getLabel();
        std::string &   getLabelStr() { return m_labelStr; }
        void            setLabelStr(std::string &s) { m_labelStr = s; }
        bool            isLabelNeeded() { return m_labelneeded; }
        void            setLabelNeeded(bool b) { m_labelneeded = b; }
        bool            isCaseOption();
        bool            isTraversed();
        void            setTraversed(bool bTraversed);
        void            print(std::ostream& os, bool html = false);
        void            printToLog();
        char *          prints();                        // For debugging
        void            dump();
        void            updateType(BBTYPE bbType, int iNumOutEdges);
        void            setJumpReqd();
        bool            isJumpReqd();

        ADDRESS         getLowAddr();
        ADDRESS         getHiAddr();

        std::list<RTL*>* getRTLs();

        RTL *           getRTLWithStatement(Statement *stmt);

        std::vector<PBB>& getInEdges();

        int             getNumInEdges() { return m_iNumInEdges; }

        std::vector<PBB>& getOutEdges();
        void            setInEdge(int i, PBB newIn);
        void            setOutEdge(int i, PBB newInEdge);

        BasicBlock *    getOutEdge(unsigned int i);
        int             getNumOutEdges() { return m_iNumOutEdges; }
        int             whichPred(PBB pred);
        void            addInEdge(PBB newInEdge);
        void            deleteEdge(PBB edge);
        void            deleteInEdge(std::vector<PBB>::iterator& it);
        void            deleteInEdge(PBB edge);
        ADDRESS         getCallDest();
        Proc *          getCallDestProc();
        unsigned        DFTOrder(int& first, int& last);
        unsigned        RevDFTOrder(int& first, int& last);

static  bool            lessAddress(PBB bb1, PBB bb2);
static  bool            lessFirstDFT(PBB bb1, PBB bb2);
static  bool            lessLastDFT(PBB bb1, PBB bb2);

        class LastStatementNotABranchError : public std::exception {
        public:
            Statement *stmt;
            LastStatementNotABranchError(Statement *stmt) : stmt(stmt) { }
        };
        class LastStatementNotAGotoError : public std::exception {
        public:
            Statement *stmt;
            LastStatementNotAGotoError(Statement *stmt) : stmt(stmt) { }
        };

        Exp *           getCond() throw(LastStatementNotABranchError);
        void            setCond(Exp *e) throw(LastStatementNotABranchError);
        Exp*            getDest() throw(LastStatementNotAGotoError);
        bool            isJmpZ(PBB dest);
        BasicBlock *    getLoopBody();
        void            simplify();
        BasicBlock *    getCorrectOutEdge(ADDRESS a);

        /*
         * Depth first traversal of all bbs, numbering as we go and as we come back, forward and reverse passes.
         * Use Cfg::establishDFTOrder() and CFG::establishRevDFTOrder to create these values.
         */
        int             m_DFTfirst;         //!< depth-first traversal first visit
        int             m_DFTlast;          //!< depth-first traversal last visit
        int             m_DFTrevfirst;      //!< reverse depth-first traversal first visit
        int             m_DFTrevlast;       //!< reverse depth-first traversal last visit

private:
        /*
         * Constructor. Called by Cfg::NewBB.
         */
                        BasicBlock(std::list<RTL*>* pRtls, BBTYPE bbType, int iNumOutEdges);
        void            setRTLs(std::list<RTL*>* rtls);

public:
                        /* high level structuring */
        SBBTYPE         m_structType;       //!< structured type of this node
        SBBTYPE         m_loopCondType;     //!< type of conditional to treat this loop header as (if any)
        BasicBlock *    m_loopHead;         //!< head of the most nested enclosing loop
        BasicBlock *    m_caseHead;         //!< head of the most nested enclosing case
        BasicBlock *    m_condFollow;       //!< follow of a conditional header
        BasicBlock *    m_loopFollow;       //!< follow of a loop header
        BasicBlock *    m_latchNode;        //!< latch node of a loop header

protected:
                        /* general basic block information */
        BBTYPE          m_nodeType;         //!< type of basic block
        std::list<RTL*>* m_pRtls;           //!< Ptr to list of RTLs
        int             m_iLabelNum;        //!< Nonzero if start of BB needs label
        std::string     m_labelStr;         //!< string label of this bb.
        bool            m_labelneeded;
        bool            m_bIncomplete;      //!< True if not yet complete
        bool            m_bJumpReqd;        //!< True if jump required for "fall through"

                        /* in-edges and out-edges */
        std::vector<BasicBlock *> m_InEdges;//!< Vector of in-edges
        std::vector<BasicBlock *> m_OutEdges;//!< Vector of out-edges
        int             m_iNumInEdges;      //!< We need these two because GCC doesn't
        int             m_iNumOutEdges;     //!< support resize() of vectors!

                        /* for traversal */
        bool            m_iTraversed;       //!< traversal marker

                        /* Liveness */
        LocationSet     liveIn;             //!< Set of locations live at BB start

public:

        bool            isPostCall();
static  void             doAvail(StatementSet& s, PBB inEdge);
        Proc *          getDestProc();

        /**
         * Get first/next statement this BB
         * Somewhat intricate because of the post call semantics; these funcs save a lot of duplicated, easily-bugged
         * code
         */
        typedef std::list<RTL*>::iterator rtlit;
        typedef std::list<RTL*>::reverse_iterator rtlrit;
        typedef std::list<Exp*>::iterator elit;
        Statement *     getFirstStmt(rtlit& rit, StatementList::iterator& sit);
        Statement *     getNextStmt(rtlit& rit, StatementList::iterator& sit);
        Statement *     getLastStmt(rtlrit& rit, StatementList::reverse_iterator& sit);
        Statement *     getFirstStmt();
        Statement *     getLastStmt();
        Statement *     getPrevStmt(rtlrit& rit, StatementList::reverse_iterator& sit);
        RTL *           getLastRtl() { return m_pRtls->back(); }
        void            getStatements(StatementList &stmts);
        char *          getStmtNumber();
protected:
        /* Control flow analysis stuff, lifted from Doug Simon's honours thesis.
         */
        int             ord;                //!< node's position within the ordering structure
        int             revOrd;             //!< position within ordering structure for the reverse graph
        int             inEdgesVisited;     //!< counts the number of in edges visited during a DFS
        int             numForwardInEdges;  //!< inedges to this node that aren't back edges
        int             loopStamps[2], revLoopStamps[2]; //!< used for structuring analysis
        travType        traversed;          //!< traversal flag for the numerous DFS's
        bool            hllLabel;           //!< emit a label for this node when generating HL code?
        char *          labelStr;           //!< the high level label for this node (if needed)
        int             indentLevel;        //!< the indentation level of this node in the final code

        // analysis information
        BasicBlock *    immPDom;            //!< immediate post dominator
        BasicBlock *    loopHead;           //!< head of the most nested enclosing loop
        BasicBlock *    caseHead;           //!< head of the most nested enclosing case
        BasicBlock *    condFollow;         //!< follow of a conditional header
        BasicBlock *    loopFollow;         //!< follow of a loop header
        BasicBlock *    latchNode;          //!< latching node of a loop header

        // Structured type of the node
        structType      sType;              //!< the structuring class (Loop, Cond , etc)
        unstructType    usType;             //!< the restructured type of a conditional header
        loopType        lType;              //!< the loop type of a loop header
        condType        cType;              //!< the conditional type of a conditional header

        void            setLoopStamps(int &time, std::vector<PBB> &order);
        void            setRevLoopStamps(int &time);
        void            setRevOrder(std::vector<PBB> &order);
        void            setLoopHead(PBB head) { loopHead = head; }
        PBB             getLoopHead() { return loopHead; }
        void            setLatchNode(PBB latch) { latchNode = latch; }
        bool            isLatchNode() { return loopHead && loopHead->latchNode == this; }
        PBB             getLatchNode() { return latchNode; }
        PBB             getCaseHead() { return caseHead; }
        void            setCaseHead(PBB head, PBB follow);
        structType      getStructType() { return sType; }
        void            setStructType(structType s);
        unstructType    getUnstructType();
        void            setUnstructType(unstructType us);
        loopType        getLoopType();
        void            setLoopType(loopType l);
        condType        getCondType();
        void            setCondType(condType l);
        void            setLoopFollow(PBB other) { loopFollow = other; }
        PBB             getLoopFollow() { return loopFollow; }
        void            setCondFollow(PBB other) { condFollow = other; }
        PBB             getCondFollow() { return condFollow; }
        bool            hasBackEdgeTo(BasicBlock *dest);
                        //! establish if this bb has any back edges leading FROM it
        bool            hasBackEdge() {
                            for (auto bb : m_OutEdges)
                                if (hasBackEdgeTo(bb))
                                    return true;
                            return false;
                        }
public:
        bool            isBackEdge(int inEdge);
protected:

        bool            isAncestorOf(BasicBlock *other);
        bool            inLoop(BasicBlock *header, BasicBlock *latch);
        bool            isIn(std::list<PBB> &set, PBB bb) {
                            for (std::list<PBB>::iterator it = set.begin(); it != set.end(); it++)
                                if (*it == bb) return true;
                            return false;
                        }

        char *          indent(int indLevel, int extra = 0);
        bool            allParentsGenerated();
        void            emitGotoAndLabel(HLLCode *hll, int indLevel, PBB dest);
        void            WriteBB(HLLCode *hll, int indLevel);

public:
        void            generateCode(HLLCode *hll, int indLevel, PBB latch, std::list<PBB> &followSet,
                                     std::list<PBB> &gotoSet, UserProc* proc);

        void            prependStmt(Statement* s, UserProc* proc);

        // Liveness
        bool            calcLiveness(ConnectionGraph& ig, UserProc* proc);
        void            getLiveOut(LocationSet& live, LocationSet& phiLocs);

        bool            decodeIndirectJmp(UserProc* proc);
        void            processSwitch(UserProc* proc);
        int             findNumCases();
        bool            undoComputedBB(Statement* stmt);

                        // true if processing for overlapped registers on statements in this BB
                        // has been completed.
        bool            overlappedRegProcessingDone;

protected:
        friend class XMLProgParser;
        void            addOutEdge(PBB bb) { m_OutEdges.push_back(bb); }
        void            addRTL(RTL *rtl) {
                            if (m_pRtls == NULL)
                                m_pRtls = new std::list<RTL*>;
                            m_pRtls->push_back(rtl);
                        }
        void            addLiveIn(Exp *e) { liveIn.insert(e); }

};        // class BasicBlock

#endif        // #define __BASIC_BLOCK_H__
