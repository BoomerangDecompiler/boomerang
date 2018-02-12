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


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/visitor/ExpSSAXformer.h"
#include "boomerang/db/visitor/ImplicitConverter.h"
#include "boomerang/util/Log.h"

#include <sstream>
#include <cstring>


DataFlow::DataFlow(UserProc *proc)
    : m_proc(proc)
    , renameLocalsAndParams(false)
{
}


DataFlow::~DataFlow()
{
}


void DataFlow::dfs(int myIdx, int parentIdx)
{
    if (m_dfnum[myIdx] == -1) {
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
}


void DataFlow::calculateDominators()
{
    Cfg *cfg = m_proc->getCFG();
    BasicBlock *entryBB = cfg->getEntryBB();
    const int numBB     = cfg->getNumBBs();

    if (!entryBB || numBB == 0) {
        return; // nothing to do
    }

    N = 0;
    allocateData();

    dfs(0, -1);
    assert(N >= 1);

    for (int i = N - 1; i >= 1; i--) {
        int n = m_vertex[i];
        int p = m_parent[n];
        int s = p;

        /* These lines calculate the semi-dominator of n, based on the Semidominator Theorem */
        // for each predecessor v of n
        for (BasicBlock *pred : m_BBs[n]->getPredecessors()) {
            if (m_indices.find(pred) == m_indices.end()) {
                QTextStream q_cerr(stderr);

                q_cerr << "BB not in indices: ";
                pred->print(q_cerr);
                assert(false);
            }

            int v     = m_indices[pred];
            int sdash = v;

            if (m_dfnum[v] > m_dfnum[n]) {
                sdash = m_semi[getAncestorWithLowestSemi(v)];
            }

            if (m_dfnum[sdash] < m_dfnum[s]) {
                s = sdash;
            }
        }

        m_semi[n] = s;
        /* Calculation of n's dominator is deferred until the path from s to n has been linked into the forest */
        m_bucket[s].insert(n);
        link(p, n);

        // for each v in bucket[p]
        for (int v : m_bucket[p]) {
            /* Now that the path from p to v has been linked into the spanning forest,
             * these lines calculate the dominator of v, based on the first clause of the Dominator Theorem,#
             * or else defer the calculation until y's dominator is known. */
            int y = getAncestorWithLowestSemi(v);

            if (m_semi[y] == m_semi[v]) {
                m_idom[v] = p;             // Success!
            }
            else {
                m_samedom[v] = y;             // Defer
            }
        }

        m_bucket[p].clear();
    }

    for (int i = 1; i < N - 1; i++) {
        /* Now all the deferred dominator calculations, based on the second clause of the Dominator Theorem, are
         *              performed. */
        int n = m_vertex[i];

        if (m_samedom[n] != -1) {
            m_idom[n] = m_idom[m_samedom[n]];          // Deferred success!
        }
    }

    computeDF(0); // Finally, compute the dominance frontiers
}


int DataFlow::getAncestorWithLowestSemi(int v)
{
    int a = m_ancestor[v];

    if (m_ancestor[a] != -1) {
        int b = getAncestorWithLowestSemi(a);
        m_ancestor[v] = m_ancestor[a];

        if (m_dfnum[m_semi[b]] < m_dfnum[m_semi[m_best[v]]]) {
            m_best[v] = b;
        }
    }

    return m_best[v];
}


void DataFlow::link(int p, int n)
{
    m_ancestor[n] = p;
    m_best[n]     = n;
}


bool DataFlow::doesDominate(int n, int w)
{
    while (m_idom[w] != -1) {
        if (m_idom[w] == n) {
            return true;
        }

        w = m_idom[w]; // Move up the dominator tree
    }

    return false;
}


void DataFlow::computeDF(int n)
{
    std::set<int> S;
    /* This loop computes DF_local[n] */
    // for each node y in succ(n)
    BasicBlock *bb = m_BBs[n];

    for (BasicBlock *b : bb->getSuccessors()) {
        int y = m_indices[b];

        if (m_idom[y] != n) {
            S.insert(y);
        }
    }

    // for each child c of n in the dominator tree
    // Note: this is a linear search!
    int sz = m_idom.size(); // ? Was ancestor.size()

    for (int c = 0; c < sz; ++c) {
        if (m_idom[c] != n) {
            continue;
        }

        computeDF(c);

        /* This loop computes DF_up[c] */
        // for each element w of DF[c]
        std::set<int>&          s = m_DF[c];
        std::set<int>::iterator ww;

        for (int w : s) {
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

    if (exp->isRegOf()    ||  // Always rename registers
        exp->isTemp()     ||  // Always rename temps (always want to propagate away)
        exp->isFlags()    ||  // Always rename flags
        exp->isMainFlag() ||  // Always rename individual flags like %CF
        exp->isLocal()) {     // Rename hard locals in the post fromSSA pass
            return true;
    }

    if (!exp->isMemOf()) {
        return false; // Can't rename %pc or other junk
    }

    // I used to check here if there was a symbol for the memory expression, and if so allow it to be renamed. However,
    // even named locals and parameters could have their addresses escape the local function, so we need another test
    // anyway. So locals and parameters should not be renamed (and hence propagated) until escape analysis is done (and
    // hence renaleLocalsAndParams is set)
    // Besides,  before we have types and references, it is not easy to find a type for the location, so we can't tell
    // if e.g. m[esp{-}+12] is evnp or a separate local.
    // It certainly needs to have the local/parameter pattern
    if (!m_proc->isLocalOrParamPattern(exp)) {
        return false;
    }

    // e is a local or parameter; allow it to be propagated iff we've done escape analysis and the address has not
    return renameLocalsAndParams && !m_proc->isAddressEscapedVar(exp); // escaped
}


void DataFlow::dumpA_phi()
{
    LOG_MSG("A_phi:");

    for (auto zz = m_A_phi.begin(); zz != m_A_phi.end(); ++zz) {
        std::set<int>& si = zz->second;

        LOG_MSG("  %1 ->", zz->first);

        for (std::set<int>::iterator qq = si.begin(); qq != si.end(); ++qq) {
            LOG_MSG("    %2", *qq);
        }
    }

    LOG_MSG("end A_phi");
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

    for (ExpSet& exps : m_definedAt) {
        for (auto iter = exps.begin(); iter != exps.end(); ) {
            if (m_A_phi.find(*iter) == m_A_phi.end()) {
                iter = exps.erase(iter);
            }
            else {
                ++iter;
            }
        }
    }

    m_definedAt.clear();   // and A_orig,
    m_defStmts.clear();    // and the map from variable to defining Stmt


    // Set the sizes of needed vectors
    const int numIndices = m_indices.size();
    const int numBB      = m_proc->getCFG()->getNumBBs();
    assert(numIndices == numBB);
    Q_UNUSED(numIndices);

    m_definedAt.resize(numBB);

    // We need to create m_definedAt[n] for all n
    // Recreate each call because propagation and other changes make old data invalid
    for (int n = 0; n < numBB; n++) {
        BasicBlock::RTLIterator rit;
        StatementList::iterator sit;
        BasicBlock              *bb = m_BBs[n];

        for (Statement *stmt = bb->getFirstStmt(rit, sit); stmt; stmt = bb->getNextStmt(rit, sit)) {
            LocationSet locationSet;
            stmt->getDefinitions(locationSet);

            if (stmt->isCall() && static_cast<const CallStatement *>(stmt)->isChildless()) { // If this is a childless call
                m_defallsites.insert(n);                              // then this block defines every variable
            }

            for (const SharedExp& exp : locationSet) {
                if (canRename(exp)) {
                    m_definedAt[n].insert(exp->clone());
                    m_defStmts[exp] = stmt;
                }
            }
        }
    }

    for (int n = 0; n < numBB; n++) {
        for (const SharedExp& a : m_definedAt[n]) {
            m_defsites[a].insert(n);
        }
    }

    bool change = false;
    // For each variable a (in defsites, i.e. defined anywhere)
    for (auto& val : m_defsites) {
        SharedExp a = val.first;

        // Those variables that are defined everywhere (i.e. in defallsites)
        // need to be defined at every defsite, too
        for (int da : m_defallsites) {
            m_defsites[a].insert(da);
        }

        std::set<int> W = m_defsites[a];

        while (!W.empty()) {
            // Pop first node from W
            int n = *W.begin();
            W.erase(W.begin());

            for (int y : m_DF[n]) {
                // phi function already created for y?
                if (m_A_phi[a].find(y) != m_A_phi[a].end()) {
                    continue;
                }

                // Insert trivial phi function for a at top of block y: a := phi()
                change = true;
                m_BBs[y]->prependStmt(new PhiAssign(a->clone()), m_proc);

                // A_phi[a] <- A_phi[a] U {y}
                m_A_phi[a].insert(y);

                // if a !elementof A_orig[y]
                if (m_definedAt[y].find(a) == m_definedAt[y].end()) {
                    // W <- W U {y}
                    W.insert(y);
                }
            }
        }
    }

    return change;
}


static SharedExp defineAll = Terminal::get(opDefineAll); // An expression representing <all>

// There is an entry in stacks[defineAll] that represents the latest definition
// from a define-all source. It is needed for variables that don't have a definition as yet
// (i.e. stacks[x].empty() is true). As soon as a real definition to x appears,
// stacks[defineAll] does not apply for variable x. This is needed to get correct
// operation of the use collectors in calls.

// Care with the Stacks object (a map from expression to stack); using Stacks[q].empty() can needlessly insert an empty
// stack
#define STACKS_EMPTY(q)    (m_Stacks.find(q) == m_Stacks.end() || m_Stacks[q].empty())

// Subscript dataflow variables
bool DataFlow::renameBlockVars(int n, bool clearStacks /* = false */)
{
    bool changed = false;

    // Need to clear the Stacks of old, renamed locations like m[esp-4] (these will be deleted, and will cause compare
    // failures in the Stacks, so it can't be correctly ordered and hence balanced etc, and will lead to segfaults)
    if (clearStacks) {
        m_Stacks.clear();
    }

    // For each statement S in block n
    BasicBlock::RTLIterator rit;
    StatementList::iterator sit;
    BasicBlock              *bb = m_BBs[n];

    for (Statement *S = bb->getFirstStmt(rit, sit); S; S = bb->getNextStmt(rit, sit)) {
        // if S is not a phi function (per Appel)
        /* if (!S->isPhi()) */
        {
            // For each use of some variable x in S (not just assignments)
            LocationSet locs;

            if (S->isPhi()) {
                PhiAssign *pa     = static_cast<PhiAssign *>(S);
                SharedExp phiLeft = pa->getLeft();

                if (phiLeft->isMemOf() || phiLeft->isRegOf()) {
                    phiLeft->getSubExp1()->addUsedLocs(locs);
                }

                // A phi statement may use a location defined in a childless call, in which case its use collector
                // needs updating
                for (auto& pp : *pa) {
                    Statement *def = pp.getDef();

                    if (def && def->isCall()) {
                        static_cast<CallStatement *>(def)->useBeforeDefine(phiLeft->clone());
                    }
                }
            }
            else {   // Not a phi assignment
                S->addUsedLocs(locs);
            }

            for (SharedExp location : locs) {
                // Don't rename memOfs that are not renamable according to the current policy
                if (!canRename(location)) {
                    continue;
                }

                Statement *def = nullptr;

                if (location->isSubscript()) { // Already subscripted?
                    // No renaming required, but redo the usage analysis, in case this is a new return, and also because
                    // we may have just removed all call livenesses
                    // Update use information in calls, and in the proc (for parameters)
                    SharedExp base = location->getSubExp1();
                    def = std::static_pointer_cast<RefExp>(location)->getDef();

                    if (def && def->isCall()) {
                        // Calls have UseCollectors for locations that are used before definition at the call
                        static_cast<CallStatement *>(def)->useBeforeDefine(base->clone());
                        continue;
                    }

                    // Update use collector in the proc (for parameters)
                    if (def == nullptr) {
                        m_proc->useBeforeDefine(base->clone());
                    }

                    continue; // Don't re-rename the renamed variable
                }

                if (!STACKS_EMPTY(location)) {
                    def = m_Stacks[location].back();
                }
                else if (!STACKS_EMPTY(defineAll)) {
                    def = m_Stacks[defineAll].back();
                }
                else {
                    // If the both stacks are empty, use a nullptr definition. This will be changed into a pointer
                    // to an implicit definition at the start of type analysis, but not until all the m[...]
                    // have stopped changing their expressions (complicates implicit assignments considerably).
                    def = nullptr;
                    // Update the collector at the start of the UserProc
                    m_proc->useBeforeDefine(location->clone());
                }


                if (def && def->isCall()) {
                    // Calls have UseCollectors for locations that are used before definition at the call
                    static_cast<CallStatement *>(def)->useBeforeDefine(location->clone());
                }

                // Replace the use of x with x{def} in S
                changed = true;

                if (S->isPhi()) {
                    SharedExp phiLeft = static_cast<PhiAssign *>(S)->getLeft();
                    phiLeft->setSubExp1(phiLeft->getSubExp1()->expSubscriptVar(location, def));
                }
                else {
                    S->subscriptVar(location, def);
                }
            }
        }

        // MVE: Check for Call and Return Statements; these have DefCollector objects that need to be updated
        // Do before the below, so CallStatements have not yet processed their defines
        if (S->isCall() || S->isReturn()) {
            DefCollector *col;

            if (S->isCall()) {
                col = static_cast<CallStatement *>(S)->getDefCollector();
            }
            else {
                col = static_cast<ReturnStatement *>(S)->getCollector();
            }

            col->updateDefs(m_Stacks, m_proc);
        }

        // For each definition of some variable a in S
        LocationSet defs;
        S->getDefinitions(defs);

        for (SharedExp a : defs) {
            // Don't consider a if it cannot be renamed
            const bool suitable = canRename(a);

            if (suitable) {
                // Push i onto Stacks[a]
                // Note: we clone a because otherwise it could be an expression
                // that gets deleted through various modifications.
                // This is necessary because we do several passes of this algorithm
                // to sort out the memory expressions.
                if (m_Stacks.find(a) != m_Stacks.end()) { // expression exists, no need for clone ?
                    m_Stacks[a].push_back(S);
                }
                else {
                    m_Stacks[a->clone()].push_back(S);
                }

                // Replace definition of 'a' with definition of a_i in S (we don't do this)
            }

            // FIXME: MVE: do we need this awful hack?
            if (a->getOper() == opLocal) {
                SharedConstExp a1 = S->getProc()->expFromSymbol(std::static_pointer_cast<Const>(a->getSubExp1())->getStr());
                assert(a1);

                // Stacks already has a definition for a (as just the bare local)
                if (suitable) {
                    m_Stacks[a1->clone()].push_back(S);
                }
            }
        }

        // Special processing for define-alls (presently, only childless calls).
        // But note that only 'everythings' at the current memory level are defined!
        if (S->isCall() && static_cast<const CallStatement *>(S)->isChildless() && !SETTING(assumeABI)) {
            // S is a childless call (and we're not assuming ABI compliance)
            m_Stacks[defineAll];          // Ensure that there is an entry for defineAll

            for (auto& elem : m_Stacks) {
                // if (dd->first->isMemDepth(memDepth))
                elem.second.push_back(S); // Add a definition for all vars
            }
        }
    }

    // For each successor Y of block n
    for (BasicBlock *Ybb : bb->getSuccessors()) {
        // For each phi-function in Y
        for (Statement *St = Ybb->getFirstStmt(rit, sit); St; St = Ybb->getNextStmt(rit, sit)) {
            PhiAssign *pa = dynamic_cast<PhiAssign *>(St);

            // if S is not a phi function, then quit the loop (no more phi's)
            // Wrong: do not quit the loop: there's an optimisation that turns a PhiAssign into an ordinary Assign.
            // So continue, not break.
            if (!pa) {
                continue;
            }

            // Suppose the jth operand of the phi is 'a'
            // For now, just get the LHS
            SharedExp a = pa->getLeft();

            // Only consider variables that can be renamed
            if (!canRename(a)) {
                continue;
            }

            Statement *def = nullptr; // assume No reaching definition

            if (!STACKS_EMPTY(a)) {
                def = m_Stacks[a].back();
            }

            // "Replace jth operand with a_i"
            pa->putAt(bb, def, a);
        }
    }

    // For each child X of n
    // Note: linear search!
    const int numBB = m_proc->getCFG()->getNumBBs();

    for (int X = 0; X < numBB; X++) {
        if (m_idom[X] == n) { // if 'n' is immediate dominator of X
            renameBlockVars(X);
        }
    }

    // For each statement S in block n
    // NOTE: Because of the need to pop childless calls from the Stacks, it is important in my algorithm to process the
    // statments in the BB *backwards*. (It is not important in Appel's algorithm, since he always pushes a definition
    // for every variable defined on the Stacks).
    BasicBlock::RTLRIterator              rrit;
    StatementList::reverse_iterator srit;

    for (Statement *S = bb->getLastStmt(rrit, srit); S; S = bb->getPrevStmt(rrit, srit)) {
        // For each definition of some variable a in S
        LocationSet defs;
        S->getDefinitions(defs);

        for (auto dd = defs.begin(); dd != defs.end(); ++dd) {
            if (!canRename(*dd)) {
                continue;
            }

            // if ((*dd)->getMemDepth() == memDepth)
            auto ss = m_Stacks.find(*dd);

            if (ss == m_Stacks.end()) {
                LOG_FATAL("Tried to pop %1 from Stacks; does not exist", (*dd)->toString());
            }

            ss->second.pop_back();
        }

        // Pop all defs due to childless calls
        if (S->isCall() && static_cast<const CallStatement *>(S)->isChildless()) {
            for (auto sss = m_Stacks.begin(); sss != m_Stacks.end(); ++sss) {
                if (!sss->second.empty() && (sss->second.back() == S)) {
                    sss->second.pop_back();
                }
            }
        }
    }

    return changed;
}


void DataFlow::dumpStacks()
{
    LOG_MSG("Stacks: %1 entries", m_Stacks.size());

    for (auto zz = m_Stacks.begin(); zz != m_Stacks.end(); zz++) {
        std::deque<Statement *> tt = zz->second; // Copy the stack!

        LOG_MSG("  Var %1 [", zz->first);

        while (!tt.empty()) {
            LOG_MSG("    %1", tt.back()->getNumber());
            tt.pop_back();
        }

        LOG_MSG("  ]");
    }
}


void DataFlow::dumpDefsites()
{
    std::map<SharedExp, std::set<int>, lessExpStar>::iterator dd;

    for (dd = m_defsites.begin(); dd != m_defsites.end(); ++dd) {
        std::set<int>& si = dd->second;

        LOG_MSG("%1", dd->first);

        for (std::set<int>::iterator ii = si.begin(); ii != si.end(); ++ii) {
            LOG_MSG("  ", *ii);
        }
    }
}


void DataFlow::dumpA_orig()
{
    int n = m_definedAt.size();

    for (int i = 0; i < n; ++i) {
        LOG_MSG("%1", i);

        for (const SharedExp& ee : m_definedAt[i]) {
            LOG_MSG("  %1 ", ee);
        }
    }
}


void DataFlow::convertImplicits()
{
    Cfg *cfg = m_proc->getCFG();

    // Convert statements in A_phi from m[...]{-} to m[...]{0}
    std::map<SharedExp, std::set<int>, lessExpStar> A_phi_copy = m_A_phi; // Object copy
    ImplicitConverter ic(cfg);
    m_A_phi.clear();

    for (std::pair<SharedExp, std::set<int> > it : A_phi_copy) {
        SharedExp e = it.first->clone();
        e          = e->accept(&ic);
        m_A_phi[e] = it.second;       // Copy the set (doesn't have to be deep)
    }

    std::map<SharedExp, std::set<int>, lessExpStar> defsites_copy = m_defsites; // Object copy
    m_defsites.clear();

    for (std::pair<SharedExp, std::set<int> > dd : defsites_copy) {
        SharedExp e = dd.first->clone();
        e             = e->accept(&ic);
        m_defsites[e] = dd.second;       // Copy the set (doesn't have to be deep)
    }

    std::vector<ExpSet> A_orig_copy = m_definedAt;
    m_definedAt.clear();

    for (ExpSet& se : A_orig_copy) {
        ExpSet se_new;

        for (const SharedExp& ee : se) {
            SharedExp e = ee->clone();
            e = e->accept(&ic);
            se_new.insert(e);
        }

        m_definedAt.insert(m_definedAt.end(), se_new);       // Copy the set (doesn't have to be a deep copy)
    }
}


void DataFlow::findLiveAtDomPhi(LocationSet &usedByDomPhi, LocationSet &usedByDomPhi0,
                                std::map<SharedExp, PhiAssign *, lessExpStar> &defdByPhi)
{
    return findLiveAtDomPhi(0, usedByDomPhi, usedByDomPhi0, defdByPhi);
}


void DataFlow::findLiveAtDomPhi(int n, LocationSet& usedByDomPhi, LocationSet& usedByDomPhi0,
                                std::map<SharedExp, PhiAssign *, lessExpStar>& defdByPhi)
{
    // For each statement this BB
    BasicBlock::RTLIterator       rit;
    StatementList::iterator sit;
    BasicBlock              *bb = m_BBs[n];

    for (Statement *S = bb->getFirstStmt(rit, sit); S; S = bb->getNextStmt(rit, sit)) {
        if (S->isPhi()) {
            // For each phi parameter, insert an entry into usedByDomPhi0
            PhiAssign           *pa = static_cast<PhiAssign *>(S);

            for (RefExp& exp : *pa) {
                if (exp.getSubExp1()) {
                    auto re = RefExp::get(exp.getSubExp1(), exp.getDef());
                    usedByDomPhi0.insert(re);
                }
            }

            // Insert an entry into the defdByPhi map
            auto wrappedLhs = RefExp::get(pa->getLeft(), pa);
            defdByPhi[wrappedLhs] = pa;
            // Fall through to the below, because phi uses are also legitimate uses
        }

        LocationSet ls;
        S->addUsedLocs(ls);

        // Consider uses of this statement
        for (const SharedExp& it : ls) {
            // Remove this entry from the map, since it is not unused
            defdByPhi.erase(it);
        }

        // Now process any definitions
        ls.clear();
        S->getDefinitions(ls);

        for (const SharedExp& it : ls) {
            auto wrappedDef(RefExp::get(it, S));

            // If this definition is in the usedByDomPhi0 set, then it is in fact dominated by a phi use, so move it to
            // the final usedByDomPhi set
            if (usedByDomPhi0.find(wrappedDef) != usedByDomPhi0.end()) {
                usedByDomPhi0.remove(wrappedDef);
                usedByDomPhi.insert(RefExp::get(it, S));
            }
        }
    }

    // Visit each child in the dominator graph
    // Note: this is a linear search!
    // Note also that usedByDomPhi0 may have some irrelevant entries, but this will do no harm, and attempting to erase
    // the irrelevant ones would probably cost more than leaving them alone
    const size_t sz = m_idom.size();

    for (size_t c = 0; c < sz; ++c) {
        if (m_idom[c] != n) {
            continue;
        }

        // Recurse to the child
        findLiveAtDomPhi(c, usedByDomPhi, usedByDomPhi0, defdByPhi);
    }
}


void DataFlow::setDominanceNums(int n, int& currNum)
{
#if USE_DOMINANCE_NUMS
    BasicBlock::rtlit       rit;
    StatementList::iterator sit;
    BasicBlock              *bb = m_BBs[n];
    Instruction             *S;

    for (S = bb->getFirstStmt(rit, sit); S; S = bb->getNextStmt(rit, sit)) {
        S->setDomNumber(currNum++);
    }

    int sz = m_idom.size();

    for (int c = 0; c < sz; ++c) {
        if (m_idom[c] != n) {
            continue;
        }

        // Recurse to the child
        setDominanceNums(c, currNum);
    }
#else
    Q_UNUSED(n);
    Q_UNUSED(currNum);
#endif
}


void DataFlow::allocateData()
{
    Cfg *cfg = m_proc->getCFG();
    const int numBBs = cfg->getNumBBs();

    m_BBs.assign(numBBs, nullptr);
    m_indices.clear();

    m_dfnum.assign(numBBs, -1);
    m_semi.assign(numBBs, -1);
    m_ancestor.assign(numBBs, -1);
    m_idom.assign(numBBs, -1);
    m_samedom.assign(numBBs, -1);
    m_vertex.assign(numBBs, -1);
    m_parent.assign(numBBs, -1);
    m_best.assign(numBBs, -1);
    m_bucket.resize(numBBs);
    m_definedAt.resize(numBBs);
    m_DF.resize(numBBs);

    m_A_phi.clear();
    m_defsites.clear();
    m_defallsites.clear();
    m_defStmts.clear();
    m_Stacks.clear();


    // Set up the BBs and indices vectors. Do this here
    // because sometimes a BB can be unreachable
    // (so relying on in-edges doesn't work)
    int i = 0;
    for (BasicBlock *bb : *cfg) {
        m_BBs[i++] = bb;
    }

    for (int j = 0; j < numBBs; j++) {
        m_indices[m_BBs[j]] = j;
    }
}
