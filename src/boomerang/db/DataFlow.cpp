#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DataFlow.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expmodifier/ExpSSAXformer.h"
#include "boomerang/visitor/expmodifier/ImplicitConverter.h"

#include <cstring>
#include <sstream>


DataFlow::DataFlow(UserProc *proc)
    : m_proc(proc)
    , renameLocalsAndParams(false)
{
}


DataFlow::~DataFlow()
{
}


void DataFlow::dfs(BBIndex myIdx, BBIndex parentIdx)
{
    assert(myIdx != BBINDEX_INVALID);
    if (m_dfnum[myIdx] >= 0) {
        // already visited
        return;
    }

    m_dfnum[myIdx]  = N;
    m_vertex[N]     = myIdx;
    m_parent[myIdx] = parentIdx;

    N++;

    // Recurse to successors
    BasicBlock *bb = m_BBs[myIdx];

    for (BasicBlock *succ : bb->getSuccessors()) {
        dfs(m_indices[succ], myIdx);
    }
}


bool DataFlow::calculateDominators()
{
    ProcCFG *cfg            = m_proc->getCFG();
    BasicBlock *entryBB     = cfg->getEntryBB();
    const std::size_t numBB = cfg->getNumBBs();

    if (!entryBB || numBB == 0) {
        return false; // nothing to do
    }

    allocateData();
    recalcSpanningTree();

    assert(N >= 1);

    // Process BBs in reverse pre-traversal order (i.e. return blocks first)
    for (std::size_t i = N - 1; i >= 1; i--) {
        BBIndex n = m_vertex[i];
        BBIndex p = m_parent[n];
        BBIndex s = p;

        /* These lines calculate the semi-dominator of n, based on the Semidominator Theorem */
        // for each predecessor v of n
        for (BasicBlock *pred : m_BBs[n]->getPredecessors()) {
            if (m_indices.find(pred) == m_indices.end()) {
                LOG_ERROR("BB not in indices: ", pred->toString());
                return false;
            }

            BBIndex v     = m_indices[pred];
            BBIndex sdash = v;

            if (m_dfnum[v] > m_dfnum[n]) {
                sdash = m_semi[getAncestorWithLowestSemi(v)];
            }

            if (m_dfnum[sdash] < m_dfnum[s]) {
                s = sdash;
            }
        }

        m_semi[n] = s;
        /* Calculation of n's dominator is deferred until the path from s to n has been linked
         * intothe forest */
        m_bucket[s].insert(n);
        link(p, n);

        // for each v in bucket[p]
        for (BBIndex v : m_bucket[p]) {
            /* Now that the path from p to v has been linked into the spanning forest,
             * these lines calculate the dominator of v, based on the first clause of the Dominator
             * Theorem,# or else defer the calculation until y's dominator is known. */
            const BBIndex y = getAncestorWithLowestSemi(v);

            if (m_semi[y] == m_semi[v]) {
                m_idom[v] = p; // Success!
            }
            else {
                m_samedom[v] = y; // Defer
            }
        }

        m_bucket[p].clear();
    }

    for (std::size_t i = 1; i < N - 1; i++) {
        // Now all the deferred dominator calculations, based on the second clause of the Dominator
        // Theorem, are performed.
        BBIndex n = m_vertex[i];

        if (m_samedom[n] != BBINDEX_INVALID) {
            m_idom[n] = m_idom[m_samedom[n]]; // Deferred success!
        }
    }

    const BBIndex entryIndex = pbbToNode(entryBB);
    assert(entryIndex != BBINDEX_INVALID);

    // the entry BB is always executed.
    m_idom[entryIndex] = entryIndex;
    m_semi[entryIndex] = entryIndex;

    computeDF(entryIndex); // Finally, compute the dominance frontiers
    return true;
}


BBIndex DataFlow::getAncestorWithLowestSemi(BBIndex v)
{
    assert(v != BBINDEX_INVALID);

    const BBIndex a = m_ancestor[v];
    if (m_ancestor[a] != BBINDEX_INVALID) {
        BBIndex b     = getAncestorWithLowestSemi(a);
        m_ancestor[v] = m_ancestor[a];

        if (m_dfnum[m_semi[b]] < m_dfnum[m_semi[m_best[v]]]) {
            m_best[v] = b;
        }
    }

    return m_best[v];
}


void DataFlow::link(BBIndex p, BBIndex n)
{
    assert(n != BBINDEX_INVALID);

    m_ancestor[n] = p;
    m_best[n]     = n;
}


bool DataFlow::doesDominate(BBIndex n, BBIndex w)
{
    assert(n != BBINDEX_INVALID);
    assert(w != BBINDEX_INVALID);

    while (m_idom[w] != w) {
        if (m_idom[w] == n) {
            return true;
        }

        w = m_idom[w]; // Move up the dominator tree
        assert(w != BBINDEX_INVALID);
    }

    return false;
}


void DataFlow::computeDF(BBIndex n)
{
    assert(n != BBINDEX_INVALID);

    std::set<BBIndex> S;
    // This loop computes DF_local[n]
    // for each node y in succ(n)
    BasicBlock *bb = m_BBs[n];

    for (BasicBlock *b : bb->getSuccessors()) {
        BBIndex y = m_indices[b];

        if (m_idom[y] != n) {
            S.insert(y);
        }
    }

    // for each child c of n in the dominator tree
    // Note: this is a linear search!
    const size_t sz = m_idom.size(); // ? Was ancestor.size()

    for (BBIndex c = 0; c < sz; ++c) {
        if (m_idom[c] != n) {
            continue;
        }
        else if (c != n) { // do not calculate DF for entry BB again
            computeDF(c);
        }

        /* This loop computes DF_up[c] */
        // for each element w of DF[c]
        std::set<BBIndex> &s = m_DF[c];

        for (BBIndex w : s) {
            if (n == w || !doesDominate(n, w)) {
                S.insert(w);
            }
        }
    }

    m_DF[n] = S;
}


bool DataFlow::canRename(SharedConstExp exp) const
{
    if (exp->isSubscript()) {
        exp = exp->getSubExp1(); // Look inside refs
    }

    if (exp->isRegOf() ||    // Always rename registers
        exp->isTemp() ||     // Always rename temps (always want to propagate away)
        exp->isFlags() ||    // Always rename flags
        exp->isMainFlag() || // Always rename individual flags like %CF
        exp->isLocal()) {    // Rename hard locals in the post fromSSA pass
        return true;
    }

    if (!exp->isMemOf()) {
        return false; // Can't rename %pc or other junk
    }

    // I used to check here if there was a symbol for the memory expression, and if so allow it to
    // be renamed. However, even named locals and parameters could have their addresses escape the
    // local function, so we need another test anyway. So locals and parameters should not be
    // renamed (and hence propagated) until escape analysis is done (and hence renaleLocalsAndParams
    // is set) Besides,  before we have types and references, it is not easy to find a type for the
    // location, so we can't tell if e.g. m[esp{-}+12] is evnp or a separate local. It certainly
    // needs to have the local/parameter pattern
    if (!m_proc->isLocalOrParamPattern(exp)) {
        return false;
    }

    // e is a local or parameter; allow it to be propagated iff we've done escape analysis and the
    // address has not
    return renameLocalsAndParams;
}


bool DataFlow::placePhiFunctions()
{
    // First free some memory no longer needed
    m_dfnum.resize(0);
    m_semi.resize(0);
    m_ancestor.resize(0);
    m_samedom.resize(0);
    m_vertex.resize(0);
    m_parent.resize(0);
    m_best.resize(0);
    m_bucket.resize(0);
    m_defsites.clear();
    m_defallsites.clear();

    for (ExSet &exps : m_definedAt) {
        for (auto iter = exps.begin(); iter != exps.end();) {
            if (m_A_phi.find(*iter) == m_A_phi.end()) {
                iter = exps.erase(iter);
            }
            else {
                ++iter;
            }
        }
    }

    m_definedAt.clear(); // and A_orig,
    m_defStmts.clear();  // and the map from variable to defining Stmt


    // Set the sizes of needed vectors
    const std::size_t numIndices = m_indices.size();
    const std::size_t numBB      = m_proc->getCFG()->getNumBBs();
    assert(numIndices == numBB);
    Q_UNUSED(numIndices);

    m_definedAt.resize(numBB);

    const bool assumeABICompliance = m_proc->getProg()->getProject()->getSettings()->assumeABI;

    // We need to create m_definedAt[n] for all n
    // Recreate each call because propagation and other changes make old data invalid
    for (std::size_t n = 0; n < numBB; n++) {
        BasicBlock::RTLIterator rit;
        StatementList::iterator sit;
        BasicBlock *bb = m_BBs[n];

        for (Statement *stmt = bb->getFirstStmt(rit, sit); stmt; stmt = bb->getNextStmt(rit, sit)) {
            LocationSet locationSet;
            stmt->getDefinitions(locationSet, assumeABICompliance);

            // If this is a childless call
            if (stmt->isCall() && static_cast<const CallStatement *>(stmt)->isChildless()) {
                // then this block defines every variable
                m_defallsites.insert(n);
            }

            for (const SharedExp &exp : locationSet) {
                if (canRename(exp)) {
                    m_definedAt[n].insert(exp->clone());
                    m_defStmts[exp] = stmt;
                }
            }
        }
    }

    for (std::size_t n = 0; n < numBB; n++) {
        for (const SharedExp &a : m_definedAt[n]) {
            m_defsites[a].insert(n);
        }
    }

    bool change = false;
    // For each variable a (in defsites, i.e. defined anywhere)
    for (auto &val : m_defsites) {
        SharedExp a = val.first;

        // Those variables that are defined everywhere (i.e. in defallsites)
        // need to be defined at every defsite, too
        for (BBIndex da : m_defallsites) {
            m_defsites[a].insert(da);
        }

        std::set<BBIndex> W = m_defsites[a];

        while (!W.empty()) {
            // Pop first node from W
            const BBIndex n = *W.begin();
            W.erase(W.begin());

            for (BBIndex y : m_DF[n]) {
                // phi function already created for y?
                if (m_A_phi[a].find(y) != m_A_phi[a].end()) {
                    continue;
                }

                // Insert trivial phi function for a at top of block y: a := phi()
                change = true;
                m_BBs[y]->addPhi(a->clone());

                // A_phi[a] <- A_phi[a] U {y}
                m_A_phi[a].insert(y);

                // if a !elementof A_orig[y]
                if (!m_definedAt[y].contains(a)) {
                    // W <- W U {y}
                    W.insert(y);
                }
            }
        }
    }

    return change;
}


void DataFlow::convertImplicits()
{
    ProcCFG *cfg = m_proc->getCFG();

    // Convert statements in A_phi from m[...]{-} to m[...]{0}
    std::map<SharedExp, std::set<BBIndex>, lessExpStar> A_phi_copy = m_A_phi; // Object copy
    ImplicitConverter ic(cfg);
    m_A_phi.clear();

    for (auto &[exp, set] : A_phi_copy) {
        SharedExp e = exp->clone()->acceptModifier(&ic);
        m_A_phi[e]  = set; // Copy the set (doesn't have to be deep)
    }

    std::map<SharedExp, std::set<BBIndex>, lessExpStar> defsites_copy = m_defsites; // Object copy
    m_defsites.clear();

    for (auto &[exp, set] : defsites_copy) {
        SharedExp e   = exp->clone()->acceptModifier(&ic);
        m_defsites[e] = set; // Copy the set (doesn't have to be deep)
    }

    std::vector<ExSet> definedAtCopy = m_definedAt;
    m_definedAt.clear();

    for (ExSet &se : definedAtCopy) {
        ExSet se_new;

        for (const SharedExp &ee : se) {
            SharedExp e = ee->clone()->acceptModifier(&ic);
            se_new.insert(e);
        }

        // Copy the set (doesn't have to be a deep copy)
        m_definedAt.insert(m_definedAt.end(), se_new);
    }
}


bool DataFlow::findLiveAtDomPhi(LocationSet &usedByDomPhi, LocationSet &usedByDomPhi0,
                                std::map<SharedExp, PhiAssign *, lessExpStar> &defdByPhi)
{
    const BasicBlock *entryBB = m_proc->getCFG()->getEntryBB();
    if (!entryBB) {
        return false;
    }

    const BBIndex entryIndex = pbbToNode(entryBB);
    assert(entryIndex != BBINDEX_INVALID);
    findLiveAtDomPhi(entryIndex, usedByDomPhi, usedByDomPhi0, defdByPhi);
    return true;
}


void DataFlow::findLiveAtDomPhi(BBIndex n, LocationSet &usedByDomPhi, LocationSet &usedByDomPhi0,
                                std::map<SharedExp, PhiAssign *, lessExpStar> &defdByPhi)
{
    assert(n != BBINDEX_INVALID);

    if (m_BBs.empty()) {
        return;
    }

    // For each statement this BB
    BasicBlock::RTLIterator rit;
    StatementList::iterator sit;
    BasicBlock *bb                 = m_BBs[n];
    const bool assumeABICompliance = m_proc->getProg()->getProject()->getSettings()->assumeABI;

    for (Statement *S = bb->getFirstStmt(rit, sit); S; S = bb->getNextStmt(rit, sit)) {
        if (S->isPhi()) {
            // For each phi parameter, insert an entry into usedByDomPhi0
            PhiAssign *pa = static_cast<PhiAssign *>(S);

            for (RefExp &exp : *pa) {
                if (exp.getSubExp1()) {
                    auto re = RefExp::get(exp.getSubExp1(), exp.getDef());
                    usedByDomPhi0.insert(re);
                }
            }

            // Insert an entry into the defdByPhi map
            auto wrappedLhs       = RefExp::get(pa->getLeft(), pa);
            defdByPhi[wrappedLhs] = pa;
            // Fall through to the below, because phi uses are also legitimate uses
        }

        LocationSet ls;
        S->addUsedLocs(ls);

        // Consider uses of this statement
        for (const SharedExp &it : ls) {
            // Remove this entry from the map, since it is not unused
            defdByPhi.erase(it);
        }

        // Now process any definitions
        ls.clear();
        S->getDefinitions(ls, assumeABICompliance);

        for (const SharedExp &it : ls) {
            auto wrappedDef(RefExp::get(it, S));

            // If this definition is in the usedByDomPhi0 set,
            // then it is in fact dominated by a phi use, so move it to
            // the final usedByDomPhi set
            if (usedByDomPhi0.contains(wrappedDef)) {
                usedByDomPhi0.remove(wrappedDef);
                usedByDomPhi.insert(RefExp::get(it, S));
            }
        }
    }

    // Visit each child in the dominator graph
    // Note: this is a linear search!
    // Note also that usedByDomPhi0 may have some irrelevant entries, but this will do no harm, and
    // attempting to erase the irrelevant ones would probably cost more than leaving them alone
    const size_t sz = m_idom.size();

    for (BBIndex c = 0; c < sz; ++c) {
        if (m_idom[c] != n || n == c) {
            continue;
        }

        // Recurse to the child
        findLiveAtDomPhi(c, usedByDomPhi, usedByDomPhi0, defdByPhi);
    }
}


void DataFlow::allocateData()
{
    ProcCFG *cfg             = m_proc->getCFG();
    const std::size_t numBBs = cfg->getNumBBs();

    m_BBs.assign(numBBs, nullptr);
    m_indices.clear();

    m_dfnum.assign(numBBs, -1);
    m_semi.assign(numBBs, BBINDEX_INVALID);
    m_ancestor.assign(numBBs, BBINDEX_INVALID);
    m_idom.assign(numBBs, BBINDEX_INVALID);
    m_samedom.assign(numBBs, BBINDEX_INVALID);
    m_vertex.assign(numBBs, BBINDEX_INVALID);
    m_parent.assign(numBBs, BBINDEX_INVALID);
    m_best.assign(numBBs, BBINDEX_INVALID);
    m_bucket.resize(numBBs);
    m_definedAt.resize(numBBs);
    m_DF.resize(numBBs);

    m_A_phi.clear();
    m_defsites.clear();
    m_defallsites.clear();
    m_defStmts.clear();

    // Set up the BBs and indices vectors. Do this here
    // because sometimes a BB can be unreachable
    // (so relying on in-edges doesn't work)
    std::size_t i = 0;
    for (BasicBlock *bb : *cfg) {
        m_BBs[i++] = bb;
    }

    for (std::size_t j = 0; j < numBBs; j++) {
        m_indices[m_BBs[j]] = j;
    }
}


void DataFlow::recalcSpanningTree()
{
    BasicBlock *entryBB = m_proc->getEntryBB();
    assert(entryBB);
    const BBIndex entryIndex = pbbToNode(entryBB);
    assert(entryIndex != BBINDEX_INVALID);

    N = 0;
    dfs(entryIndex, BBINDEX_INVALID);
}
