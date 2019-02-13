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
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/util/StatementList.h"
#include "boomerang/util/log/Log.h"


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
        sig->getLibraryDefines(defines); // Set the locations defined
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
        const StatementList
            &modifieds = static_cast<UserProc *>(callee)->getRetStmt()->getModifieds();

        for (Statement *mm : modifieds) {
            Assignment *as = static_cast<Assignment *>(mm);
            SharedExp loc  = as->getLeft();

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
        for (SharedExp loc : *callStmt->getUseCollector()) {
            if (proc->filterReturns(loc)) {
                continue; // Filtered out
            }

            if (!newDefines.existsOnLeft(loc)) {
                ImplicitAssign *as = new ImplicitAssign(loc->clone());
                as->setProc(proc);
                as->setBB(callStmt->getBB());
                newDefines.append(as);
            }
        }
    }

    for (Statement *stmt : newDefines) {
        // Make sure the LHS is still in the return or collector
        Assignment *as = static_cast<Assignment *>(stmt);
        SharedExp lhs  = as->getLeft();

        if (callStmt->getCalleeReturn()) {
            if (!callStmt->getCalleeReturn()->definesLoc(lhs)) {
                delete stmt;
                continue; // Not in callee returns
            }
        }
        else if (!callStmt->getUseCollector()->exists(lhs)) {
            delete stmt;
            continue; // Not in collector: delete it (don't copy it)
        }

        if (proc->filterReturns(lhs)) {
            delete stmt;
            continue; // Filtered out: delete it
        }

        callStmt->getDefines().append(stmt);
    }

    callStmt->getDefines().sort([sig] (Statement *left, Statement *right) {
        return sig->returnCompare(*static_cast<Assignment *>(left),
                                  *static_cast<Assignment *>(right));
    });

    return true;
}
