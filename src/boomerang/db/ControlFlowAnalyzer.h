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
#include <unordered_set>
#include <vector>

class Cfg;
class BasicBlock;


/**
 * Control flow analysis stuff, lifted from Doug Simon's honours thesis.
 * Analyzes the control flow of a CFG and tags loop constructs etc.
 */
class ControlFlowAnalyzer
{
public:
    ControlFlowAnalyzer(Cfg *cfg);

public:
    /// Structures the control flow graph
    void structureCFG();

private:
    void setTimeStamps();

    /**
     * Finds the immediate post dominator of each node in the graph PROC->cfg.
     *
     * Adapted version of the dominators algorithm by Hecht and Ullman;
     * finds immediate post dominators only.
     * \note graph should be reducible
     */
    void updateImmedPDom();

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

    /// Finds the common post dominator of the current immediate post dominator and its successor's immediate post dominator
    BasicBlock *commonPDom(BasicBlock *curImmPDom, BasicBlock *succImmPDom);

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

private:
    Cfg *m_cfg = nullptr;
    bool m_structured = false; ///< true if the CFG is structured

    std::vector<BasicBlock *> m_ordering;    ///< Ordering of BBs for control flow structuring
    std::vector<BasicBlock *> m_revOrdering; ///< Ordering of BBs for control flow structuring
};
