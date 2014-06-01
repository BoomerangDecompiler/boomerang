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

#include "types.h"
#include "managed.h" // For LocationSet etc

#include <QtCore/QString>

class Location;
class HLLCode;
class BasicBlock;
class RTL;
class Function;
class UserProc;
struct SWITCH_INFO; // Declared in include/statement.h

/*    *    *    *    *    *    *    *    *    *    *    *    *    *    *    *\
*                                                             *
*    e n u m s   u s e d   i n   C f g . h   a n d   h e r e     *
*                                                             *
\*    *    *    *    *    *    *    *    *    *    *    *    *    *    *    */

// Depth-first traversal constants.
enum travType {
    UNTRAVERSED, // Initial value
    DFS_TAG,     // Remove redundant nodes pass
    DFS_LNUM,    // DFS loop stamping pass
    DFS_RNUM,    // DFS reverse loop stamping pass
    DFS_CASE,    // DFS case head tagging traversal
    DFS_PDOM,    // DFS post dominator ordering
    DFS_CODEGEN  // Code generating pass
};

// an enumerated type for the class of stucture determined for a node
enum structType {
    Loop,     // Header of a loop only
    Cond,     // Header of a conditional only (if-then-else or switch)
    LoopCond, // Header of a loop and a conditional
    Seq       // sequential statement (default)
};

// an type for the class of unstructured conditional jumps
enum unstructType { Structured, JumpInOutLoop, JumpIntoCase };

// an enumerated type for the type of conditional headers
enum CondType {
    IfThen,     // conditional with only a then clause
    IfThenElse, // conditional with a then and an else clause
    IfElse,     // conditional with only an else clause
    Case        // nway conditional header (case statement)
};

// an enumerated type for the type of loop headers
enum LoopType {
    PreTested,  // Header of a while loop
    PostTested, // Header of a repeat loop
    Endless     // Header of an endless loop
};

/*    *    *    *    *    *    *    *    *    *\
*                                     *
*    B a s i c B l o c k   e n u m s     *
*                                     *
\*    *    *    *    *    *    *    *    *    */

// Kinds of basic block nodes
// reordering these will break the save files - trent
enum BBTYPE {
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

enum SBBTYPE {
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

typedef std::list<BasicBlock *>::iterator BB_IT;
typedef std::list<BasicBlock *>::const_iterator BBC_IT;

/***************************************************************************/ /**
  * BasicBlock class. <more comments>
  ******************************************************************************/
class BasicBlock {
    /*
     * Objects of class Cfg can access the internals of a BasicBlock object.
     */
    friend class Cfg;

  public:
    typedef std::vector<BasicBlock *>::iterator iEdgeIterator;
    typedef std::list<RTL *>::iterator rtlit;
    typedef std::list<RTL *>::reverse_iterator rtlrit;
    typedef std::list<Exp *>::iterator elit;

  protected:
    Function *Parent;
    /* general basic block information */
    BBTYPE NodeType = INVALID;         //!< type of basic block
    std::list<RTL *> *ListOfRTLs = nullptr; //!< Ptr to list of RTLs
    int LabelNum = 0;                 //!< Nonzero if start of BB needs label
    bool LabelNeeded = false;
    bool Incomplete = true; //!< True if not yet complete
    bool JumpReqd = false;  //!< True if jump required for "fall through"

    /* in-edges and out-edges */
    std::vector<BasicBlock *> InEdges;  //!< Vector of in-edges
    std::vector<BasicBlock *> OutEdges; //!< Vector of out-edges
    size_t TargetOutEdges;             //!< support resize() of vectors!

    /* for traversal */
    bool TraversedMarker = false; //!< traversal marker

    /* Liveness */
    LocationSet LiveIn;                  //!< Set of locations live at BB start
    /*
     * Depth first traversal of all bbs, numbering as we go and as we come back, forward and reverse passes.
     * Use Cfg::establishDFTOrder() and CFG::establishRevDFTOrder to create these values.
     */
    int DFTfirst = 0; //!< depth-first traversal first visit
    int DFTlast = 0;  //!< depth-first traversal last visit
    int DFTrevfirst;  //!< reverse depth-first traversal first visit
    int DFTrevlast;   //!< reverse depth-first traversal last visit

                                         /* Control flow analysis stuff, lifted from Doug Simon's honours thesis.
                                          */
    int Ord;                             //!< node's position within the ordering structure
    int RevOrd;                          //!< position within ordering structure for the reverse graph
    int InEdgesVisited;                  //!< counts the number of in edges visited during a DFS
    int NumForwardInEdges;               //!< inedges to this node that aren't back edges
    int LoopStamps[2], RevLoopStamps[2]; //!< used for structuring analysis
    travType Traversed;                  //!< traversal flag for the numerous DFS's
    bool HllLabel;                       //!< emit a label for this node when generating HL code?
    QString LabelStr;                    //!< the high level label for this node (if needed)
    int IndentLevel;                     //!< the indentation level of this node in the final code

    /* high level structuring */
    SBBTYPE LoopCondType = NONE;        //!< type of conditional to treat this loop header as (if any)
    SBBTYPE StructType = NONE;          //!< structured type of this node
    // analysis information
    BasicBlock *ImmPDom;    //!< immediate post dominator
    BasicBlock *LoopHead;   //!< head of the most nested enclosing loop
    BasicBlock *CaseHead;   //!< head of the most nested enclosing case
    BasicBlock *CondFollow; //!< follow of a conditional header
    BasicBlock *LoopFollow; //!< follow of a loop header
    BasicBlock *LatchNode;  //!< latching node of a loop header

    // Structured type of the node
    structType StructuringType;    //!< the structuring class (Loop, Cond , etc)
    unstructType UnstructuredType; //!< the restructured type of a conditional header
    LoopType LoopHeaderType;      //!< the loop type of a loop header
    CondType ConditionHeaderType;      //!< the conditional type of a conditional header

  public:
    BasicBlock();
    ~BasicBlock();
    BasicBlock(const BasicBlock &bb);
    /// \brief return enclosing function, null if none
    const Function *getParent() const { return Parent; }
          Function *getParent()       { return Parent; }
    BBTYPE getType();

    int getLabel();
    QString &getLabelStr() { return LabelStr; }
    void setLabelStr(const QString &s) { LabelStr = s; }
    bool isLabelNeeded() { return LabelNeeded; }
    void setLabelNeeded(bool b) { LabelNeeded = b; }
    bool isCaseOption();
    bool isTraversed();
    void setTraversed(bool bTraversed);
    void print(std::ostream &os, bool html = false);
    void printToLog();
    char *prints(); // For debugging
    void dump();
    void updateType(BBTYPE bbType, uint32_t iNumOutEdges);
    void setJumpReqd();
    bool isJumpReqd();

    ADDRESS getLowAddr();
    ADDRESS getHiAddr();

    std::list<RTL *> *getRTLs();
    const std::list<RTL *> *getRTLs() const;

    RTL *getRTLWithStatement(Statement *stmt);

    std::vector<BasicBlock *> &getInEdges();

    size_t getNumInEdges() const { return InEdges.size(); }

    std::vector<BasicBlock *> &getOutEdges();
    void setInEdge(size_t i, BasicBlock *newIn);
    void setOutEdge(size_t i, BasicBlock *newInEdge);
    BasicBlock *getOutEdge(unsigned int i);
    size_t getNumOutEdges() { return OutEdges.size(); }
    int whichPred(BasicBlock *pred);
    void addInEdge(BasicBlock *newInEdge);
    void deleteEdge(BasicBlock *edge);
    void deleteInEdge(std::vector<BasicBlock *>::iterator &it);
    void deleteInEdge(BasicBlock *edge);
    ADDRESS getCallDest();
    Function *getCallDestProc();
    unsigned DFTOrder(int &first, int &last);
    unsigned RevDFTOrder(int &first, int &last);

    static bool lessAddress(BasicBlock *bb1, BasicBlock *bb2);
    static bool lessFirstDFT(BasicBlock *bb1, BasicBlock *bb2);
    static bool lessLastDFT(BasicBlock *bb1, BasicBlock *bb2);

    class LastStatementNotABranchError : public std::exception {
      public:
        Statement *stmt;
        LastStatementNotABranchError(Statement *_stmt) : stmt(_stmt) {}
    };
    class LastStatementNotAGotoError : public std::exception {
      public:
        Statement *stmt;
        LastStatementNotAGotoError(Statement *_stmt) : stmt(_stmt) {}
    };

    Exp *getCond() throw(LastStatementNotABranchError);
    void setCond(Exp *e) throw(LastStatementNotABranchError);
    Exp *getDest() throw(LastStatementNotAGotoError);
    bool isJmpZ(BasicBlock *dest);
    BasicBlock *getLoopBody();
    void simplify();
    BasicBlock *getCorrectOutEdge(ADDRESS a);
    bool isPostCall();
    static void doAvail(StatementSet &s, BasicBlock *inEdge);
    Function *getDestProc();

    /**
     * Get first/next statement this BB
     * Somewhat intricate because of the post call semantics; these funcs save a lot of duplicated, easily-bugged
     * code
     */
    Statement *getFirstStmt(rtlit &rit, StatementList::iterator &sit);
    Statement *getNextStmt(rtlit &rit, StatementList::iterator &sit);
    Statement *getLastStmt(rtlrit &rit, StatementList::reverse_iterator &sit);
    Statement *getFirstStmt();
    Statement *getLastStmt();
    Statement *getPrevStmt(rtlrit &rit, StatementList::reverse_iterator &sit);
    RTL *getLastRtl() { return ListOfRTLs->back(); }
    void getStatements(StatementList &stmts) const;
    char *getStmtNumber();

  public:
    bool isBackEdge(size_t inEdge) const;

    void generateCode(HLLCode *hll, int indLevel, BasicBlock *latch, std::list<BasicBlock *> &followSet,
                      std::list<BasicBlock *> &gotoSet, UserProc *proc);

    void prependStmt(Statement *s, UserProc *proc);

    // Liveness
    bool calcLiveness(ConnectionGraph &ig, UserProc *proc);
    void getLiveOut(LocationSet &live, LocationSet &phiLocs);

    bool decodeIndirectJmp(UserProc *proc);
    void processSwitch(UserProc *proc);
    int findNumCases();
    bool undoComputedBB(Statement *stmt);
    bool searchAll(const Exp &search_for, std::list<Exp *> &results);
    bool searchAndReplace(const Exp &search, Exp *replace);
    // true if processing for overlapped registers on statements in this BB
    // has been completed.
    bool overlappedRegProcessingDone;

    void generateCode_Loop(HLLCode *hll, std::list<BasicBlock *> &gotoSet, int indLevel, UserProc *proc,
                           BasicBlock *latch, std::list<BasicBlock *> &followSet);

  protected:
    void setLoopStamps(int &time, std::vector<BasicBlock *> &order);
    void setRevLoopStamps(int &time);
    void setRevOrder(std::vector<BasicBlock *> &order);
    void setLoopHead(BasicBlock *head) { LoopHead = head; }
    BasicBlock *getLoopHead() { return LoopHead; }
    void setLatchNode(BasicBlock *latch) { LatchNode = latch; }
    bool isLatchNode() { return LoopHead && LoopHead->LatchNode == this; }
    BasicBlock *getLatchNode() { return LatchNode; }
    BasicBlock *getCaseHead() { return CaseHead; }
    void setCaseHead(BasicBlock *head, BasicBlock *follow);
    structType getStructType() { return StructuringType; }
    void setStructType(structType s);
    unstructType getUnstructType();
    void setUnstructType(unstructType us);
    LoopType getLoopType();
    void setLoopType(LoopType l);
    CondType getCondType();
    void setCondType(CondType l);
    void setLoopFollow(BasicBlock *other) { LoopFollow = other; }
    BasicBlock *getLoopFollow() { return LoopFollow; }
    void setCondFollow(BasicBlock *other) { CondFollow = other; }
    BasicBlock *getCondFollow() { return CondFollow; }
    bool hasBackEdgeTo(BasicBlock *dest);
    //! establish if this bb has any back edges leading FROM it
    bool hasBackEdge() {
        for (auto bb : OutEdges)
            if (hasBackEdgeTo(bb))
                return true;
        return false;
    }
    friend class XMLProgParser;
    bool isAncestorOf(BasicBlock *other);
    bool inLoop(BasicBlock *header, BasicBlock *latch);
    bool isIn(const std::list<BasicBlock *> &set, BasicBlock *bb) {
        for (BasicBlock *it : set)
            if (it == bb)
                return true;
        return false;
    }

    char *indent(int indLevel, int extra = 0);
    bool allParentsGenerated();
    void emitGotoAndLabel(HLLCode *hll, int indLevel, BasicBlock *dest);
    void WriteBB(HLLCode *hll, int indLevel);
    void addOutEdge(BasicBlock *bb) { OutEdges.push_back(bb); }
    void addRTL(RTL *rtl) {
        if (ListOfRTLs == nullptr)
            ListOfRTLs = new std::list<RTL *>;
        ListOfRTLs->push_back(rtl);
    }
    void addLiveIn(Exp *e) { LiveIn.insert(e); }

  private:
    /*
     * Constructor. Called by Cfg::NewBB.
     */
    BasicBlock(Function *parent,std::list<RTL *> *pRtls, BBTYPE bbType, uint32_t iNumOutEdges);
    void setRTLs(std::list<RTL *> *rtls);

}; // class BasicBlock
