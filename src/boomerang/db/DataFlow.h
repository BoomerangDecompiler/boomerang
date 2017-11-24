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


#include "boomerang/db/exp/ExpHelp.h"
#include "boomerang/util/AssignSet.h"
#include "boomerang/util/LocationSet.h"

#include <vector>
#include <map>
#include <set>
#include <stack>


class Cfg;
class BasicBlock;
class Exp;
class RefExp;
class Statement;
class UserProc;
class PhiAssign;
class Type;
class QTextStream;
class LocationSet;

typedef std::set<SharedExp, lessExpStar> ExpSet;


/**
 * Dominator frontier code largely as per Appel 2002 ("Modern Compiler Implementation in Java")
 */
class DataFlow
{
public:
    DataFlow();
    DataFlow(const DataFlow& other) = delete;
    DataFlow(DataFlow&& other) = default;

    ~DataFlow();

    DataFlow& operator=(const DataFlow& other) = delete;
    DataFlow& operator=(DataFlow&& other) = default;

public:
    /// Place phi functions.
    /// \returns true if any change
    bool placePhiFunctions(UserProc *proc);

    /// \returns true if the expression \p e can be renamed
    bool canRename(SharedExp e, UserProc *proc);

    /// Essentially Algorithm 19.9 of Appel's "modern compiler implementation in Java" 2nd ed 2002
    void calculateDominators(Cfg *cfg);

    void setRenameLocalsParams(bool b) { renameLocalsAndParams = b; }

    /// Rename variables in basic block \p n.
    /// \returns true if any change made
    bool renameBlockVars(UserProc *proc, int n, bool clearStacks = false);

    void convertImplicits(Cfg *cfg);

    /**
     * Find the locations used by a live, dominating phi-function. Also removes dead phi-funcions.
     *
     * Helper function for UserProc::propagateStatements()
     * Works on basic block \p n; call from UserProc with n=0 (entry BB)
     *
     * If an SSA location is in usedByDomPhi it means it is used in a phi that dominates its assignment
     * However, it could turn out that the phi is dead, in which case we don't want to keep the associated entries in
     * usedByDomPhi. So we maintain the map defdByPhi which maps locations defined at a phi to the phi statements. Every
     * time we see a use of a location in defdByPhi, we remove that map entry. At the end of the procedure we therefore have
     * only dead phi statements in the map, so we can delete the associated entries in defdByPhi and also remove the dead
     * phi statements.
     *
     * We add to the set usedByDomPhi0 whenever we see a location referenced by a phi parameter. When we see a definition
     * for such a location, we remove it from the usedByDomPhi0 set (to save memory) and add it to the usedByDomPhi set.
     * For locations defined before they are used in a phi parameter, there will be no entry in usedByDomPhi, so we ignore
     * it. Remember that each location is defined only once, so that's the time to decide if it is dominated by a phi use or
     * not.
     */
    void findLiveAtDomPhi(int n, LocationSet& usedByDomPhi, LocationSet& usedByDomPhi0,
                          std::map<SharedExp, PhiAssign *, lessExpStar>& defdByPhi);

    // For testing:
    int pbbToNode(BasicBlock *bb) { return m_indices[bb]; }
    std::set<int>& getDF(size_t node) { return m_DF[node]; }
    BasicBlock *nodeToBB(size_t node) { return m_BBs[node]; }
    int getIdom(size_t node) { return m_idom[node]; }
    int getSemi(size_t node) { return m_semi[node]; }
    std::set<int>& getA_phi(SharedExp e) { return m_A_phi[e]; }

private:
    /// depth first search
    void dfs(int p, size_t n);

    /// Basically algorithm 19.10b of Appel 2002 (uses path compression for O(log N) amortised time per operation
    /// (overall O(N log N))
    int getAncestorWithLowestSemi(int v);

    void link(int p, int n);

    void computeDF(int n);

    /// Return true if n dominates w
    bool doesDominate(int n, int w);

    bool canRenameLocalsParams() const { return renameLocalsAndParams; }

    void setDominanceNums(int n, int& currNum); // Set the dominance statement number

    void clearA_phi() { m_A_phi.clear(); }

    // For debugging:
    void dumpStacks();
    void dumpDefsites();
    void dumpA_orig();

    // For debugging
    void dumpA_phi();

private:
    /* Dominance Frontier Data */

    /* These first two are not from Appel; they map PBBs to indices */
    std::vector<BasicBlock *> m_BBs;       ///< Pointers to BBs from indices
    std::map<BasicBlock *, int> m_indices; ///< Indices from pointers to BBs

    /*
     * Calculating the dominance frontier
     */
    // If there is a path from a to b in the cfg, then a is an ancestor of b
    // if dfnum[a] < denum[b]
    std::vector<int> m_dfnum;             ///< Number set in depth first search
    std::vector<int> m_semi;              ///< Semi dominators
    std::vector<int> m_ancestor;          ///< Defines the forest that becomes the spanning tree
    std::vector<int> m_idom;              ///< Immediate dominator
    std::vector<int> m_samedom;           ///< ? To do with deferring
    std::vector<int> m_vertex;            ///< ?
    std::vector<int> m_parent;            ///< Parent in the dominator tree?
    std::vector<int> m_best;              ///< Improves ancestorWithLowestSemi
    std::vector<std::set<int> > m_bucket; ///< Deferred calculation?
    int N;                                ///< Current node number in algorithm
    std::vector<std::set<int> > m_DF;     ///< The dominance frontiers

    /*
     * Inserting phi-functions
     */
    /// Array of sets of locations defined in BB n
    std::vector<ExpSet> m_A_orig;

    /// Array of sets of BBs needing phis
    std::map<SharedExp, std::set<int>, lessExpStar> m_A_phi;

    /// Map from expression to set of block numbers
    std::map<SharedExp, std::set<int>, lessExpStar> m_defsites;

    /// Set of block numbers defining all variables
    std::set<int> m_defallsites;

    /// A Boomerang requirement: Statements defining particular subscripted locations
    std::map<SharedExp, Statement *, lessExpStar> m_defStmts;

    /*
     * Renaming variables
     */
    /// The stack which remembers the last definition of an expression.
    /// A map from expression (Exp*) to a stack of (pointers to) Statements
    std::map<SharedExp, std::deque<Statement *>, lessExpStar> m_Stacks;

    // Initially false, meaning that locals and parameters are not renamed and hence not propagated.
    // When true, locals and parameters can be renamed if their address does not escape the local procedure.
    // See Mike's thesis for details.
    bool renameLocalsAndParams;
};
