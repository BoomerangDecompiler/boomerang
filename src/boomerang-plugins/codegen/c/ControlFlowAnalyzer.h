#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include <unordered_map>
#include <vector>


class ProcCFG;
class IRFragment;


/// an enumerated type for the class of stucture determined for a node
enum class StructType : int8_t
{
    Invalid = -1,
    Loop,     // Header of a loop only
    Cond,     // Header of a conditional only (if-then-else or switch)
    LoopCond, // Header of a loop and a conditional
    Seq       // sequential statement (default)
};


/// an enumerated type for the class of unstructured conditional jumps
enum class UnstructType : int8_t
{
    Invalid = -1,
    Structured,
    JumpInOutLoop,
    JumpIntoCase
};


/// an enumerated type for the type of loop headers
enum class LoopType : int8_t
{
    Invalid = -1,
    PreTested,  ///< Header of a while loop
    PostTested, ///< Header of a do..while loop
    Endless     ///< Header of an endless loop
};


/// an enumerated type for the type of conditional headers
enum class CondType : int8_t
{
    Invalid = -1,
    IfThen,     ///< conditional with only a then clause
    IfThenElse, ///< conditional with a then and an else clause
    IfElse,     ///< conditional with only an else clause
    Case        ///< nway conditional header (case statement)
};


/// Depth-first traversal constants.
enum class TravType : uint8_t
{
    Untraversed, ///< Initial value
    DFS_LNum,    ///< DFS loop stamping pass
    DFS_RNum,    ///< DFS reverse loop stamping pass
    DFS_Case,    ///< DFS case head tagging traversal
    DFS_PDom,    ///< DFS post dominator ordering
};


enum class SBBType : uint8_t
{
    None,        ///< not structured
    PreTestLoop, ///< header of a loop
    PostTestLoop,
    EndlessLoop,
    JumpInOutLoop, ///< an unstructured jump in or out of a loop
    JumpIntoCase,  ///< an unstructured jump into a case statement
    IfGoto,        ///< unstructured conditional
    IfThen,        ///< conditional with then clause
    IfThenElse,    ///< conditional with then and else clauses
    IfElse,        ///< conditional with else clause only
    Case           ///< case statement (switch)
};


/// Holds all information about control Flow Structure.
struct BBStructInfo
{
    /// Control flow analysis stuff, lifted from Doug Simon's honours thesis.
    int m_postOrderIndex    = -1; ///< node's position within the ordering structure
    int m_revPostOrderIndex = -1; ///< position within ordering structure for the reverse graph

    int m_preOrderID     = 0; ///< (unique) id of the node during pre-order traversal
    int m_postOrderID    = 0; ///< (unique) id of the node during post-order traversal
    int m_revPreOrderID  = 0; ///< (unique) id of the node during reverse pre-order traversal
    int m_revPostOrderID = 0; ///< (unique) id of the node during reverse post-order traversal

    /* for traversal */
    TravType m_travType = TravType::Untraversed; ///< traversal flag for the numerous DFS's

    /* high level structuring */
    SBBType m_loopCondType = SBBType::None; ///< type of conditional to treat this loop header as
    SBBType m_structType   = SBBType::None; ///< structured type of this node

    /// the structuring class (Loop, Cond, etc)
    StructType m_structuringType = StructType::Seq;

    /// the restructured type of a conditional header
    UnstructType m_unstructuredType = UnstructType::Structured;

    /// the conditional type of a conditional header
    CondType m_conditionHeaderType = CondType::Invalid;
    LoopType m_loopHeaderType      = LoopType::Invalid; ///< the loop type of a loop header

    // analysis information
    const IRFragment *m_immPDom    = nullptr; ///< immediate post dominator
    const IRFragment *m_loopHead   = nullptr; ///< head of the most nested enclosing loop
    const IRFragment *m_caseHead   = nullptr; ///< head of the most nested enclosing case
    const IRFragment *m_condFollow = nullptr; ///< follow of a conditional header
    const IRFragment *m_loopFollow = nullptr; ///< follow of a loop header
    const IRFragment *m_latchNode  = nullptr; ///< latching node of a loop header
};


/**
 * Control flow analysis stuff, lifted from Doug Simon's honours thesis.
 * Analyzes the control flow of a CFG and tags loop constructs etc.
 */
class ControlFlowAnalyzer
{
public:
    ControlFlowAnalyzer();

public:
    /// Structures the control flow graph
    void structureCFG(ProcCFG *cfg);

    /// establish if \p source has a back edge to \p dest
    bool isBackEdge(const IRFragment *source, const IRFragment *dest) const;

public:
    inline bool isLatchNode(const IRFragment *bb) const
    {
        const IRFragment *loopHead = getLoopHead(bb);
        if (!loopHead) {
            return false;
        }

        return getLatchNode(loopHead) == bb;
    }

    inline const IRFragment *getLatchNode(const IRFragment *bb) const
    {
        return m_info[bb].m_latchNode;
    }

    inline const IRFragment *getLoopHead(const IRFragment *bb) const
    {
        return m_info[bb].m_loopHead;
    }

    inline const IRFragment *getLoopFollow(const IRFragment *bb) const
    {
        return m_info[bb].m_loopFollow;
    }

    inline const IRFragment *getCondFollow(const IRFragment *bb) const
    {
        return m_info[bb].m_condFollow;
    }

    inline const IRFragment *getCaseHead(const IRFragment *bb) const
    {
        return m_info[bb].m_caseHead;
    }

    TravType getTravType(const IRFragment *bb) const { return m_info[bb].m_travType; }
    StructType getStructType(const IRFragment *bb) const { return m_info[bb].m_structuringType; }
    CondType getCondType(const IRFragment *bb) const;
    UnstructType getUnstructType(const IRFragment *bb) const;
    LoopType getLoopType(const IRFragment *bb) const;

    void setTravType(const IRFragment *bb, TravType type) { m_info[bb].m_travType = type; }
    void setStructType(const IRFragment *bb, StructType s);

    bool isCaseOption(const IRFragment *bb) const;

private:
    void updateLoopStamps(const IRFragment *bb, int &time);
    void updateRevLoopStamps(const IRFragment *bb, int &time);
    void updateRevOrder(const IRFragment *bb);

    void setLoopHead(const IRFragment *bb, const IRFragment *head) { m_info[bb].m_loopHead = head; }
    void setLatchNode(const IRFragment *bb, const IRFragment *latch)
    {
        m_info[bb].m_latchNode = latch;
    }

    void setCaseHead(const IRFragment *bb, const IRFragment *head, const IRFragment *follow);

    void setUnstructType(const IRFragment *bb, UnstructType unstructType);
    void setLoopType(const IRFragment *bb, LoopType loopType);
    void setCondType(const IRFragment *bb, CondType condType);

    void setLoopFollow(const IRFragment *bb, const IRFragment *follow)
    {
        m_info[bb].m_loopFollow = follow;
    }

    void setCondFollow(const IRFragment *bb, const IRFragment *follow)
    {
        m_info[bb].m_condFollow = follow;
    }

    /// establish if this bb has any back edges leading FROM it
    bool hasBackEdge(const IRFragment *bb) const;

    /// \returns true if \p bb is an ancestor of \p other
    bool isAncestorOf(const IRFragment *bb, const IRFragment *other) const;
    bool isBBInLoop(const IRFragment *bb, const IRFragment *header, const IRFragment *latch) const;

    int getPostOrdering(const IRFragment *bb) const { return m_info[bb].m_postOrderIndex; }
    int getRevOrd(const IRFragment *bb) const { return m_info[bb].m_revPostOrderIndex; }

    const IRFragment *getImmPDom(const IRFragment *bb) const { return m_info[bb].m_immPDom; }

    void setImmPDom(const IRFragment *bb, const IRFragment *immPDom)
    {
        m_info[bb].m_immPDom = immPDom;
    }

    void unTraverse();

private:
    void setTimeStamps();

    /**
     * Finds the immediate post dominator of each node in the CFG.
     *
     * Adapted version of the dominators algorithm by Hecht and Ullman;
     * finds immediate post dominators only.
     * \note graph should be reducible
     */
    void updateImmedPDom();

    /// Structures all conditional headers (i.e. nodes with more than one outedge)
    void structConds();

    /// \pre The graph for curProc has been built.
    /// \post Each node is tagged with the header of the most nested loop of which it is a member
    /// (possibly none). The header of each loop stores information on the latching node as well as
    /// the type of loop it heads.
    void structLoops();

    /// This routine is called after all the other structuring has been done. It detects
    /// conditionals that are in fact the head of a jump into/outof a loop or into a case body. Only
    /// forward jumps are considered as unstructured backward jumps will always be generated nicely.
    void checkConds();

    /// Finds the common post dominator of the current immediate post dominator and its successor's
    /// immediate post dominator
    const IRFragment *findCommonPDom(const IRFragment *curImmPDom, const IRFragment *succImmPDom);

    /// \pre  The loop induced by (head,latch) has already had all its member nodes tagged
    /// \post The type of loop has been deduced
    void determineLoopType(const IRFragment *header, bool *&loopNodes);

    /// \pre  The loop headed by header has been induced and all it's member nodes have been tagged
    /// \post The follow of the loop has been determined.
    void findLoopFollow(const IRFragment *header, bool *&loopNodes);

    /// \pre header has been detected as a loop header and has the details of the
    ///        latching node
    /// \post the nodes within the loop have been tagged
    void tagNodesInLoop(const IRFragment *header, bool *&loopNodes);

    IRFragment *findEntryBB() const;
    IRFragment *findExitBB() const;

private:
    ProcCFG *m_cfg = nullptr;

    /// Post Ordering according to a DFS starting at the entry BB.
    std::vector<const IRFragment *> m_postOrdering;

    /// Post Ordering according to a DFS starting at the exit BB (usually the return BB).
    /// Note that this is not the reverse of m_postOrdering
    /// for functions containing calls to noreturn functions or infinite loops.
    std::vector<const IRFragment *> m_revPostOrdering;

private:
    /// mutable to allow using the map in const methods (might create entries).
    /// DO NOT change BBStructInfo in const methods!
    mutable std::unordered_map<const IRFragment *, BBStructInfo> m_info;
};
