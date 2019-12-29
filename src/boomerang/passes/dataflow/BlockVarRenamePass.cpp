#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BlockVarRenamePass.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expmodifier/ExpSubscripter.h"
#include "boomerang/visitor/stmtmodifier/StmtSubscripter.h"


static const SharedExp defineAll = Terminal::get(opDefineAll); // An expression representing <all>

// There is an entry in stacks[defineAll] that represents the latest definition
// from a define-all source. It is needed for variables that don't have a definition as yet
// (i.e. stacks[x].empty() is true). As soon as a real definition to x appears,
// stacks[defineAll] does not apply for variable x. This is needed to get correct
// operation of the use collectors in calls.

// Care with the Stacks object (a map from expression to stack);
// using Stacks[q].empty() can needlessly insert an empty stack
#define STACKS_EMPTY(q) (stacks.find(q) == stacks.end() || stacks[q].empty())


BlockVarRenamePass::BlockVarRenamePass()
    : IPass("BlockVarRename", PassID::BlockVarRename)
{
}


bool BlockVarRenamePass::execute(UserProc *proc)
{
    /// The stack which remembers the last definition of an expression.
    IRFragment *entryFrag = proc->getCFG()->getEntryFragment();
    if (entryFrag == nullptr) {
        return false;
    }

    const FragIndex entryIdx = proc->getDataFlow()->fragToIdx(entryFrag);
    const bool changed       = renameBlockVars(proc, entryIdx);

    for (auto &[var, stack] : stacks) {
        Q_UNUSED(var);
        assert(stack.empty());
    }

    return changed;
}


bool BlockVarRenamePass::renameBlockVars(UserProc *proc, std::size_t n)
{
    if (proc->getCFG()->getNumFragments() == 0) {
        return false;
    }

    bool changed                   = false;
    const bool assumeABICompliance = proc->getProg()->getProject()->getSettings()->assumeABI;

    // For each statement S in block n
    IRFragment::RTLIterator rit;
    StatementList::iterator sit;
    IRFragment *frag = proc->getDataFlow()->idxToFrag(n);

    for (SharedStmt stmt = frag->getFirstStmt(rit, sit); stmt; stmt = frag->getNextStmt(rit, sit)) {
        if (!stmt->isPhi()) {
            continue;
        }

        // A phi statement may use a location defined in a childless call,
        // in which case its use collector needs updating
        std::shared_ptr<PhiAssign> phi = stmt->as<PhiAssign>();

        for (auto &pp : *phi) {
            SharedStmt def = pp->getDef();

            if (def && def->isCall()) {
                def->as<CallStatement>()->useBeforeDefine(phi->getLeft()->clone());
            }
        }
    }

    for (SharedStmt stmt = frag->getFirstStmt(rit, sit); stmt; stmt = frag->getNextStmt(rit, sit)) {
        changed |= subscriptUsedLocations(stmt);

        // MVE: Check for Call and Return Statements;
        // these have DefCollector objects that need to be updated
        // Do before the below, so CallStatements have not yet processed their defines
        if (stmt->isCall() || stmt->isReturn()) {
            DefCollector *col;

            if (stmt->isCall()) {
                col = stmt->as<CallStatement>()->getDefCollector();
            }
            else {
                col = stmt->as<ReturnStatement>()->getCollector();
            }

            col->updateDefs(stacks, proc);
        }

        pushDefinitions(stmt, assumeABICompliance);
    }

    // For each successor Y of block n
    for (IRFragment *yFrag : frag->getSuccessors()) {
        // For each phi-function in Y
        for (SharedStmt St = yFrag->getFirstStmt(rit, sit); St; St = yFrag->getNextStmt(rit, sit)) {
            if (!St->isPhi()) {
                continue;
            }

            std::shared_ptr<PhiAssign> pa = St->as<PhiAssign>();

            // Suppose the jth operand of the phi is 'a'
            // For now, just get the LHS
            const SharedExp a = pa->getLeft();

            // Only consider variables that can be renamed
            if (!proc->canRename(a)) {
                continue;
            }

            SharedStmt def = nullptr; // assume No reaching definition

            if (!STACKS_EMPTY(a)) {
                def = stacks[a].top();
            }

            // "Replace jth operand with a_i"
            pa->putAt(frag, def, a);
        }
    }

    // For each child X of n
    // Note: linear search!
    const std::size_t numFrags = proc->getCFG()->getNumFragments();

    for (FragIndex X = 0; X < numFrags; ++X) {
        const FragIndex idom = proc->getDataFlow()->getIdom(X);
        if (idom == n && X != n) { // if 'n' is immediate dominator of X
            renameBlockVars(proc, X);
        }
    }

    // NOTE: Because of the need to pop childless calls from the Stacks, it is important in my
    // algorithm to process the statments in the fragments *backwards*.
    // (It is not important in Appel's algorithm, since he always pushes a definition
    // for every variable defined on the Stacks).
    IRFragment::RTLRIterator rrit;
    StatementList::reverse_iterator srit;

    for (SharedStmt S = frag->getLastStmt(rrit, srit); S; S = frag->getPrevStmt(rrit, srit)) {
        popDefinitions(S, assumeABICompliance);
    }

    return changed;
}


void BlockVarRenamePass::subscriptVar(const SharedStmt &stmt, SharedExp var,
                                      const SharedStmt &varDef)
{
    ExpSubscripter es(var, varDef);
    StmtSubscripter ss(&es);

    stmt->accept(&ss);
}


bool BlockVarRenamePass::subscriptUsedLocations(SharedStmt stmt)
{
    bool changed   = false;
    UserProc *proc = stmt->getProc();

    // For each use of some variable x in stmt
    LocationSet locs;

    if (stmt->isPhi()) {
        const SharedExp phiLeft = stmt->as<PhiAssign>()->getLeft();

        if (phiLeft->isMemOf() || phiLeft->isRegOf()) {
            phiLeft->getSubExp1()->addUsedLocs(locs);
        }
    }
    else { // Not a phi assignment
        stmt->addUsedLocs(locs);
    }

    for (SharedExp location : locs) {
        // Don't rename memOfs that are not renamable according to the current policy
        if (!proc->canRename(location)) {
            continue;
        }

        SharedStmt def;

        if (location->isSubscript()) { // Already subscripted?
            // No renaming required, but redo the usage analysis, in case this is a new
            // return, and also because we may have just removed all call livenesses
            SharedExp base = location->getSubExp1();
            def            = location->access<RefExp>()->getDef();

            if (def && def->isCall()) {
                // Calls have UseCollectors for locations that are used before definition at
                // the call
                def->as<CallStatement>()->useBeforeDefine(base->clone());
                continue;
            }

            // Update use collector in the proc (for parameters)
            if (def == nullptr) {
                proc->markAsInitialParam(base->clone());
            }

            continue; // Don't re-rename the renamed variable
        }

        if (!STACKS_EMPTY(location)) {
            def = stacks[location].top();
        }
        else if (!STACKS_EMPTY(defineAll)) {
            def = stacks[defineAll].top();
        }
        else {
            // If the both stacks are empty, use a nullptr definition. This will be changed
            // into a pointer to an implicit definition at the start of type analysis, but
            // not until all the m[...] have stopped changing their expressions (complicates
            // implicit assignments considerably).
            def = nullptr;
            // Update the collector at the start of the UserProc
            proc->markAsInitialParam(location->clone());
        }


        if (def && def->isCall()) {
            // Calls have UseCollectors for locations that are used before definition
            // at the call
            def->as<CallStatement>()->useBeforeDefine(location->clone());
        }

        // Replace the use of x with x{def} in S
        changed = true;

        if (stmt->isPhi()) {
            SharedExp phiLeft = stmt->as<PhiAssign>()->getLeft();
            phiLeft->setSubExp1(phiLeft->getSubExp1()->expSubscriptVar(location, def));
        }
        else {
            subscriptVar(stmt, location, def);
        }
    }

    return changed;
}


void BlockVarRenamePass::pushDefinitions(SharedStmt stmt, bool assumeABICompliance)
{
    UserProc *proc = stmt->getProc();

    LocationSet defs;
    stmt->getDefinitions(defs, assumeABICompliance);

    for (SharedExp a : defs) {
        // Don't consider a if it cannot be renamed
        const bool suitable = proc->canRename(a);

        if (suitable) {
            // Push i onto Stacks[a]
            // Note: we clone a because otherwise it could be an expression
            // that gets deleted through various modifications.
            // This is necessary because we do several passes of this algorithm
            // to sort out the memory expressions.
            if (stacks.find(a) != stacks.end()) { // expression exists, no need for clone ?
                stacks[a].push(stmt);
            }
            else {
                stacks[a->clone()].push(stmt);
            }

            // Replace definition of 'a' with definition of a_i in S (we don't do this)
        }

        // FIXME: MVE: do we need this awful hack?
        if (a->isLocal()) {
            SharedConstExp a1 = stmt->getProc()->expFromSymbol(a->access<Const, 1>()->getStr());
            assert(a1);

            // Stacks already has a definition for a (as just the bare local)
            if (suitable) {
                stacks[a1->clone()].push(stmt);
            }
        }
    }

    // Special processing for define-alls (presently, only childless calls).
    // But note that only 'everythings' at the current memory level are defined!
    if (stmt->isCall() && stmt->as<CallStatement>()->isChildless() &&
        !proc->getProg()->getProject()->getSettings()->assumeABI) {
        // S is a childless call (and we're not assuming ABI compliance)
        stacks[defineAll]; // Ensure that there is an entry for defineAll

        for (auto &elem : stacks) {
            // if (dd->first->isMemDepth(memDepth))
            elem.second.push(stmt); // Add a definition for all vars
        }
    }
}


void BlockVarRenamePass::popDefinitions(SharedStmt stmt, bool assumeABICompliance)
{
    UserProc *proc = stmt->getProc();

    // For each definition of some variable a in S
    LocationSet defs;
    stmt->getDefinitions(defs, assumeABICompliance);

    for (const auto &def : defs) {
        if (!proc->canRename(def)) {
            continue;
        }

        auto stackIt = stacks.find(def);
        if (stackIt == stacks.end() || stackIt->second.empty()) {
            LOG_FATAL("Tried to pop '%1' from Stacks; does not exist", def);
        }

        stackIt->second.pop();
    }

    // Pop all defs due to childless calls
    if (stmt->isCall() && stmt->as<CallStatement>()->isChildless()) {
        for (auto &[var, lastDef] : stacks) {
            Q_UNUSED(var);

            if (!lastDef.empty() && (lastDef.top() == stmt)) {
                lastDef.pop();
            }
        }
    }
}
