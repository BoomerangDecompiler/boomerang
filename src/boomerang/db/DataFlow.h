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


/***************************************************************************/ /**
 * \file  dataflow.h
 * \brief Interface for SSA based data flow analysis
 ******************************************************************************/

#include "boomerang/db/exp/ExpHelp.h"      // For lessExpStar, etc
#include "boomerang/db/Managed.h" // For LocationSet

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

/// Dominator frontier code largely as per Appel 2002 ("Modern Compiler Implementation in Java")
class DataFlow
{
public:
    DataFlow();

    ~DataFlow();

    /// depth first search
    void dfs(int p, size_t n);

    /// Essentially Algorithm 19.9 of Appel's "modern compiler implementation in Java" 2nd ed 2002
    void dominators(Cfg *cfg);

    /// Basically algorithm 19.10b of Appel 2002 (uses path compression for O(log N) amortised time per operation
    /// (overall O(N log N))
    int getAncestorWithLowestSemi(int v);
    void link(int p, int n);
    void computeDF(int n);

    // Place phi functions. Return true if any change
    bool placePhiFunctions(UserProc *proc);

    // Rename variables in basicblock n. Return true if any change made
    bool renameBlockVars(UserProc *proc, int n, bool clearStacks = false);

    /// Return true if n dominates w
    bool doesDominate(int n, int w);

    void setRenameLocalsParams(bool b) { renameLocalsAndParams = b; }
    bool canRenameLocalsParams() const { return renameLocalsAndParams; }
    bool canRename(SharedExp e, UserProc *proc);
    void convertImplicits(Cfg *cfg);

    /**
     * Find the locations used by a live, dominating phi-function. Also removes dead phi-funcion.
     *
     * Helper function for UserProc::propagateStatements()
     * Works on basic block n; call from UserProc with n=0 (entry BB)
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

    void setDominanceNums(int n, int& currNum); // Set the dominance statement number

    void clearA_phi() { m_A_phi.clear(); }

    // For testing:
    int pbbToNode(BasicBlock *bb) { return m_indices[bb]; }
    std::set<int>& getDF(size_t node) { return m_DF[node]; }
    BasicBlock *nodeToBB(size_t node) { return m_BBs[node]; }
    int getIdom(size_t node) { return m_idom[node]; }
    int getSemi(size_t node) { return m_semi[node]; }
    std::set<int>& getA_phi(SharedExp e) { return m_A_phi[e]; }

    // For debugging:
    void dumpStacks();
    void dumpDefsites();
    void dumpA_orig();

    // For debugging
    void dumpA_phi();

private:
    /******************** Dominance Frontier Data *******************/

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

/*    *    *    *    *    *    *\
*                         *
*    C o l l e c t o r s  *
*                         *
\*    *    *    *    *    *    */

/**
 * This class collects all definitions that reach the statement
 * that contains this collector.
 */
class DefCollector
{
    /*
     * True if initialised. When not initialised, callees should not subscript parameters inserted into the
     * associated CallStatement
     */
    bool m_initialised;
    AssignSet m_defs; ///< The set of definitions.

public:
    DefCollector()
        : m_initialised(false) {}

    /**
     * Clone the given Collector into this one
     */
    void makeCloneOf(const DefCollector& other);

    /*
     * Return true if initialised
     */
    bool isInitialised() const { return m_initialised; }

    /*
     * Clear the location set
     */
    void clear()
    {
        m_defs.clear();
        m_initialised = false;
    }

    /*
     * Insert a new member (make sure none exists yet)
     */
    void insert(Assign *a);


    /// Print the collected locations to stream os
    void print(QTextStream& os, bool html = false) const;

    /*
     * Print to string (for debugging)
     */
    char *prints() const;
    void dump() const;
    Assign *dumpAddrOfFourth();

    /*
     * begin() and end() so we can iterate through the locations
     */
    typedef AssignSet::const_iterator   const_iterator;
    typedef AssignSet::iterator         iterator;

    iterator begin() { return m_defs.begin(); }
    iterator end() { return m_defs.end(); }

    const_iterator begin() const { return m_defs.begin(); }
    const_iterator end() const { return m_defs.end(); }

    bool existsOnLeft(SharedExp e) const { return m_defs.definesLoc(e); }

    /*
     * Update the definitions with the current set of reaching definitions
     * proc is the enclosing procedure
     */
    void updateDefs(std::map<SharedExp, std::deque<Statement *>, lessExpStar>& Stacks, UserProc *proc);

    /**
     * Find the definition for a location.
     * Find the definition for e that reaches this Collector.
     * If none reaches here, return nullptr
     */
    SharedExp findDefFor(SharedExp e) const;

    /**
     * Search and replace all occurrences
     */
    void searchReplaceAll(const Exp& from, SharedExp to, bool& change);
};


/**
 * UseCollector class. This class collects all uses (live variables)
 * that will be defined by the statement that contains this collector
 * (or the UserProc that contains it).
 *
 * Typically the entries are not subscripted, like parameters or locations on the LHS of assignments
 */
class UseCollector
{
    /// True if initialised. When not initialised, callees should not subscript parameters inserted into the
    /// associated CallStatement
    bool m_initialised;

    /// The set of locations. Use lessExpStar to compare properly
    LocationSet m_locs;

public:
    UseCollector()
        : m_initialised(false) {}

    /**
     * clone the given Collector into this one
     */
    void makeCloneOf(UseCollector& other);

    /// \returns true if initialised
    bool isInitialised() const { return m_initialised; }

    /*
     * Clear the location set
     */
    void clear()
    {
        m_locs.clear();
        m_initialised = false;
    }

    /// Insert a new member
    void insert(SharedExp e) { m_locs.insert(e); }

    /*
     * Print the collected locations to stream os
     */
    void print(QTextStream& os, bool html = false) const;

    /// Print to string (for debugging)
    char *prints() const;
    void dump() const;

    /*
     * begin() and end() so we can iterate through the locations
     */
    typedef LocationSet::iterator         iterator;
    typedef LocationSet::const_iterator   const_iterator;

    iterator begin() { return m_locs.begin(); }
    iterator end()   { return m_locs.end(); }
    const_iterator begin() const { return m_locs.begin(); }
    const_iterator end() const { return m_locs.end(); }

    bool exists(SharedExp e) { return m_locs.exists(e); } // True if e is in the collection
    LocationSet& getLocSet() { return m_locs; }

public:
    /// Add a new use from Statement u
    void updateLocs(Statement *u);

    /// Remove the given location
    void remove(SharedExp loc)
    {
        m_locs.remove(loc);
    }

    /// Remove the current location
    void remove(iterator it)
    {
        m_locs.remove(it);
    }

    /// Translate out of SSA form
    /// Called from CallStatement::fromSSAform. The UserProc is needed for the symbol map
    void fromSSAform(UserProc *proc, Statement *def);
    bool operator==(UseCollector& other);
};
