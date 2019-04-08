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


#include "boomerang/util/LocationSet.h"

#include <map>
#include <unordered_map>


class BasicBlock;
class PhiAssign;


typedef std::size_t BBIndex;
static constexpr const BBIndex BBINDEX_INVALID = ((BBIndex)-1);


/**
 * Dominator frontier code largely as per Appel 2002
 * ("Modern Compiler Implementation in Java")
 */
class BOOMERANG_API DataFlow
{
    using ExSet = ExpSet<Exp>;

public:
    DataFlow(UserProc *proc);
    DataFlow(const DataFlow &other) = delete;
    DataFlow(DataFlow &&other)      = default;

    ~DataFlow();

    DataFlow &operator=(const DataFlow &other) = delete;
    DataFlow &operator=(DataFlow &&other) = default;

public:
    /**
     * Calculate dominators for every node n using Lengauer-Tarjan with path compression.
     * Essentially Algorithm 19.9 of Appel's
     * "Modern compiler implementation in Java" 2nd ed 2002
     */
    bool calculateDominators();

    /// Place phi functions.
    /// \returns true if any change
    bool placePhiFunctions();

    /// \returns true if the expression \p e can be renamed
    bool canRename(SharedConstExp e) const;

    void setRenameLocalsParams(bool b) { renameLocalsAndParams = b; }

    void convertImplicits();

    /**
     * Find the locations in the CFG used by a live, dominating phi-function; also removes dead
     * phi-funcions. Helper function for StatementPropagationPass.
     *
     * If an SSA location is in \p usedByDomPhi it means it is used in a phi that dominates its
     * assignment However, it could turn out that the phi is dead, in which case we don't want to
     * keep the associated entries in \p usedByDomPhi. So we maintain the map \p defdByPhi which
     * maps locations defined at a phi to the phi statements. Every time we see a use of a location
     * in \p defdByPhi, we remove that map entry. At the end of the procedure we therefore have only
     * dead phi statements in the map, so we can delete the associated entries in \p defdByPhi and
     * also remove the dead phi statements.
     *
     * We add to the set \p usedByDomPhi0 whenever we see a location referenced by a phi parameter.
     * When we see a definition for such a location, we remove it from the usedByDomPhi0 set (to
     * save memory) and add it to the usedByDomPhi set. For locations defined before they are used
     * in a phi parameter, there will be no entry in usedByDomPhi, so we ignore it. Remember that
     * each location is defined only once, so that's the time to decide if it is dominated by a phi
     * use or not.
     */
    void findLiveAtDomPhi(LocationSet &usedByDomPhi, LocationSet &usedByDomPhi0,
                          std::map<SharedExp, PhiAssign *, lessExpStar> &defdByPhi);

    // for testing
public:
    /// \note can only be called after \ref calculateDominators()
    const BasicBlock *getSemiDominator(const BasicBlock *bb) const
    {
        return nodeToBB(getSemi(pbbToNode(bb)));
    }

    /// \note can only be called after \ref calculateDominators()
    const BasicBlock *getDominator(const BasicBlock *bb) const
    {
        return nodeToBB(getIdom(pbbToNode(bb)));
    }

    /// \note can only be called after \ref calculateDominators()
    std::set<const BasicBlock *> getDominanceFrontier(const BasicBlock *bb) const
    {
        std::set<const BasicBlock *> ret;
        for (int idx : m_DF.at(pbbToNode(bb))) {
            ret.insert(nodeToBB(idx));
        }

        return ret;
    }

public:
    const BasicBlock *nodeToBB(BBIndex node) const { return m_BBs.at(node); }
    BasicBlock *nodeToBB(BBIndex node) { return m_BBs.at(node); }

    BBIndex pbbToNode(const BasicBlock *bb) const
    {
        return m_indices.at(const_cast<BasicBlock *>(bb));
    }

    std::set<BBIndex> &getDF(int node) { return m_DF[node]; }
    BBIndex getIdom(BBIndex node) const { return m_idom[node]; }
    BBIndex getSemi(BBIndex node) const { return m_semi[node]; }
    std::set<BBIndex> &getA_phi(SharedExp e) { return m_A_phi[e]; }

private:
    void recalcSpanningTree();

    /// depth first search
    /// \param myIdx index of the current BB
    /// \param parentIdx index of the parent of the current BB
    void dfs(BBIndex myIdx, BBIndex parentIdx);

    /// Basically algorithm 19.10b of Appel 2002 (uses path compression for O(log N) amortised time
    /// per operation (overall O(N log N))
    BBIndex getAncestorWithLowestSemi(BBIndex v);

    void link(BBIndex p, BBIndex n);

    void computeDF(BBIndex n);

    /// \return true if \p n dominates \p w.
    bool doesDominate(BBIndex n, BBIndex w);

    bool canRenameLocalsParams() const { return renameLocalsAndParams; }

    void clearA_phi() { m_A_phi.clear(); }

private:
    void allocateData();

    void findLiveAtDomPhi(BBIndex n, LocationSet &usedByDomPhi, LocationSet &usedByDomPhi0,
                          std::map<SharedExp, PhiAssign *, lessExpStar> &defdByPhi);

private:
    UserProc *m_proc = nullptr;

    /* Dominance Frontier Data */

    /* These first two are not from Appel; they map PBBs to indices */
    std::vector<BasicBlock *> m_BBs;                     ///< Maps index -> BasicBlock
    std::unordered_map<BasicBlock *, BBIndex> m_indices; ///< Maps BasicBlock -> index

    /// Calculating the dominance frontier

    /// Order number of BB n during a depth first search.
    /// If there is a path from a to b in the ProcCFG, then a is an ancestor of b
    /// if dfnum[a] < dfnum[b]. If BB a has not yet been visited, m_dfnum[a] will be -1.
    std::vector<int> m_dfnum;

    std::vector<BBIndex>
        m_ancestor;              /// Immediate (unique) ancestor of the depth first spanning tree
    std::vector<BBIndex> m_semi; /// Semi dominator of n
    std::vector<BBIndex> m_idom; /// Immediate dominator

    std::vector<BBIndex> m_samedom;          ///< ? To do with deferring
    std::vector<BBIndex> m_vertex;           ///< ?
    std::vector<BBIndex> m_parent;           ///< Parent in the dominator tree?
    std::vector<BBIndex> m_best;             ///< Improves ancestorWithLowestSemi
    std::vector<std::set<BBIndex>> m_bucket; ///< Deferred calculation?
    std::vector<std::set<BBIndex>> m_DF;     ///< Dominance frontier for every node n
    std::size_t N = 0;                       ///< Current node number in algorithm

    /*
     * Inserting phi-functions
     */
    /// Array of sets of locations defined in BB n
    std::vector<ExSet> m_definedAt; // was: m_A_orig

    /// For a given expression e, stores the BBs needing a phi for e
    std::map<SharedExp, std::set<BBIndex>, lessExpStar> m_A_phi;

    /// For a given expression e, stores the BBs where e is defined
    std::map<SharedExp, std::set<BBIndex>, lessExpStar> m_defsites;

    /// Set of block numbers defining all variables
    std::set<BBIndex> m_defallsites;

    /// A Boomerang requirement: Statements defining particular subscripted locations
    std::map<SharedExp, Statement *, lessExpStar> m_defStmts;

    /**
     * Initially false, meaning that locals and parameters are not renamed and hence not propagated.
     * When true, locals and parameters can be renamed if their address does not escape the local
     * procedure. See Mike's thesis for details.
     */
    bool renameLocalsAndParams;
};
