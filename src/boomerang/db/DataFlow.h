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


#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/util/LocationSet.h"

#include <map>
#include <unordered_map>


class IRFragment;
class PhiAssign;

typedef std::size_t FragIndex;
static constexpr const FragIndex INDEX_INVALID = FragIndex(-1);


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

    // for testing
public:
    /// \note can only be called after \ref calculateDominators()
    const IRFragment *getSemiDominator(const IRFragment *frag) const
    {
        return idxToFrag(getSemi(fragToIdx(frag)));
    }

    /// \note can only be called after \ref calculateDominators()
    const IRFragment *getDominator(const IRFragment *frag) const
    {
        return idxToFrag(getIdom(fragToIdx(frag)));
    }

    /// \note can only be called after \ref calculateDominators()
    std::set<const IRFragment *> getDominanceFrontier(const IRFragment *frag) const
    {
        std::set<const IRFragment *> ret;
        for (FragIndex idx : m_DF.at(fragToIdx(frag))) {
            ret.insert(idxToFrag(idx));
        }

        return ret;
    }

public:
    const IRFragment *idxToFrag(FragIndex node) const { return m_frags.at(node); }
    IRFragment *idxToFrag(FragIndex node) { return m_frags.at(node); }

    FragIndex fragToIdx(const IRFragment *frag) const
    {
        return m_indices.at(const_cast<IRFragment *>(frag));
    }

    std::set<FragIndex> &getDF(FragIndex node) { return m_DF[node]; }
    FragIndex getIdom(FragIndex node) const { return m_idom[node]; }
    FragIndex getSemi(FragIndex node) const { return m_semi[node]; }
    std::set<FragIndex> &getA_phi(SharedExp e) { return m_A_phi[e]; }

private:
    void recalcSpanningTree();

    /// depth first search
    /// \param myIdx index of the current fragment
    /// \param parentIdx index of the parent of the current fragment
    void dfs(FragIndex myIdx, FragIndex parentIdx);

    /// Basically algorithm 19.10b of Appel 2002 (uses path compression for O(log N) amortised time
    /// per operation (overall O(N log N))
    FragIndex getAncestorWithLowestSemi(FragIndex v);

    void link(FragIndex p, FragIndex n);

    void computeDF(FragIndex n);

    /// \return true if \p n dominates \p w.
    bool doesDominate(FragIndex n, FragIndex w);

    bool canRenameLocalsParams() const { return renameLocalsAndParams; }

    void clearA_phi() { m_A_phi.clear(); }

private:
    void allocateData();

    bool isAncestorOf(FragIndex n, FragIndex parent) const;

private:
    UserProc *m_proc = nullptr;

    /* Dominance Frontier Data */

    // These first two are not from Appel; they map fragments to indices and back
    std::vector<IRFragment *> m_frags;                     ///< Maps index -> IRFragment
    std::unordered_map<IRFragment *, FragIndex> m_indices; ///< Maps IRFragment -> index

    /// Calculating the dominance frontier

    /// Order number of fragment n during a depth first search.
    /// If there is a path from a to b in the ProcCFG, then a is an ancestor of b
    /// if dfnum[a] < dfnum[b]. If fragment a has not yet been visited, m_dfnum[a] will be -1.
    std::vector<int> m_dfnum;

    std::vector<FragIndex>
        m_ancestor;                ///< Immediate unique ancestor in the depth first spanning tree
    std::vector<FragIndex> m_semi; ///< Semi-dominator of n
    std::vector<FragIndex> m_idom; ///< Immediate dominator

    std::vector<FragIndex> m_samedom;          ///< ? To do with deferring
    std::vector<FragIndex> m_vertex;           ///< ?
    std::vector<FragIndex> m_parent;           ///< Parent in the dominator tree?
    std::vector<FragIndex> m_best;             ///< Improves ancestorWithLowestSemi
    std::vector<std::set<FragIndex>> m_bucket; ///< Deferred calculation?
    std::vector<std::set<FragIndex>> m_DF;     ///< Dominance frontier for every node n
    std::size_t N = 0;                         ///< Current node number in algorithm

    /*
     * Inserting phi-functions
     */
    /// Array of sets of locations defined in fragment n
    std::vector<ExSet> m_definedAt; // was: m_A_orig

    /// For a given expression e, stores the fragments needing a phi for e
    std::map<SharedExp, std::set<FragIndex>, lessExpStar> m_A_phi;

    /// For a given expression e, stores the fragments where e is defined
    std::map<SharedExp, std::set<FragIndex>, lessExpStar> m_defsites;

    /// Set of block numbers defining all variables
    std::set<FragIndex> m_defallsites;

    /// A Boomerang requirement: Statements defining particular subscripted locations
    std::map<SharedExp, SharedStmt, lessExpStar> m_defStmts;

    /**
     * Initially false, meaning that locals and parameters are not renamed and hence not propagated.
     * When true, locals and parameters can be renamed if their address does not escape the local
     * procedure. See Mike's thesis for details.
     */
    bool renameLocalsAndParams;
};
