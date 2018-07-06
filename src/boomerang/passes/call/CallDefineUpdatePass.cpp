#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CallDefineUpdatePass.h"


#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/util/StatementList.h"
#include "boomerang/util/Log.h"


CallDefineUpdatePass::CallDefineUpdatePass()
    : IPass("CallDefineUpdate", PassID::CallDefineUpdate)
{
}


bool CallDefineUpdatePass::execute(UserProc *proc)
{
    StatementList stmts;
    proc->getStatements(stmts);

    bool changed = false;

    for (Statement *s : stmts) {
        if (!s->isCall()) {
            continue;
        }

        assert(dynamic_cast<CallStatement *>(s) != nullptr);
        changed |= updateCallDefines(proc, static_cast<CallStatement *>(s));
    }

    return changed;
}


bool CallDefineUpdatePass::updateCallDefines(UserProc *proc, CallStatement *callStmt)
{
    assert(callStmt->getProc() == proc);
    Function *callee = callStmt->getDestProc();

    std::shared_ptr<Signature> sig = callee ? callee->getSignature() : proc->getSignature();

    if (callee && callee->isLib()) {
        StatementList defines;
        sig->getLibraryDefines(defines);     // Set the locations defined
        callStmt->setDefines(defines);
        return true;
    }
    else if (proc->getProg()->getProject()->getSettings()->assumeABI) {
        // Risky: just assume the ABI caller save registers are defined
        Signature::getABIDefines(proc->getProg()->getMachine(), callStmt->getDefines());
        return true;
    }

    // Move the defines to a temporary list. We must make sure that all defines
    // that are not inserted into m_defines again are deleted.
    StatementList newDefines(callStmt->getDefines());
    callStmt->getDefines().clear();

    if (callee && callStmt->getCalleeReturn()) {
        assert(!callee->isLib());
        const StatementList& modifieds = static_cast<UserProc *>(callee)->getModifieds();

        for (Statement *mm : modifieds) {
            Assignment *as = static_cast<Assignment *>(mm);
            SharedExp  loc = as->getLeft();

            if (proc->filterReturns(loc)) {
                continue;
            }

            SharedType ty = as->getType();

            if (!newDefines.existsOnLeft(loc)) {
                newDefines.append(new ImplicitAssign(ty, loc));
            }
        }
    }
    else {
        // Ensure that everything in the UseCollector has an entry in oldDefines
        LocationSet::iterator ll;

        for (ll = callStmt->getUseCollector()->begin(); ll != callStmt->getUseCollector()->end(); ++ll) {
            SharedExp loc = *ll;

            if (proc->filterReturns(loc)) {
                continue;     // Filtered out
            }

            if (!newDefines.existsOnLeft(loc)) {
                ImplicitAssign *as = new ImplicitAssign(loc->clone());
                as->setProc(proc);
                as->setBB(callStmt->getBB());
                newDefines.append(as);
            }
        }
    }

    for (StatementList::reverse_iterator it = newDefines.rbegin(); it != newDefines.rend(); ++it) {
        // Make sure the LHS is still in the return or collector
        Assignment *as = static_cast<Assignment *>(*it);
        SharedExp  lhs = as->getLeft();

        if (callStmt->getCalleeReturn()) {
            if (!callStmt->getCalleeReturn()->definesLoc(lhs)) {
                delete *it;
                continue;     // Not in callee returns
            }
        }
        else if (!callStmt->getUseCollector()->exists(lhs)) {
            delete *it;
            continue;     // Not in collector: delete it (don't copy it)
        }

        if (proc->filterReturns(lhs)) {
            delete *it;
            continue;     // Filtered out: delete it
        }

        // Insert as, in order, into the existing set of definitions
        bool inserted = false;

        for (StatementList::iterator nn = callStmt->getDefines().begin(); nn != callStmt->getDefines().end(); ++nn) {
            if (sig->returnCompare(*as, *static_cast<Assignment *>(*nn))) {     // If the new assignment is less than the current one
                nn       = callStmt->getDefines().insert(nn, as);               // then insert before this position
                inserted = true;
                break;
            }
        }

        if (!inserted) {
            callStmt->getDefines().append(as);     // In case larger than all existing elements
        }
    }

    return true;
}
