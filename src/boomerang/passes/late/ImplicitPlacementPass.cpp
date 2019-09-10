#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ImplicitPlacementPass.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/util/StatementList.h"
#include "boomerang/visitor/expmodifier/ImplicitConverter.h"
#include "boomerang/visitor/stmtmodifier/StmtImplicitConverter.h"


ImplicitPlacementPass::ImplicitPlacementPass()
    : IPass("ImplicitPlacement", PassID::ImplicitPlacement)
{
}


bool ImplicitPlacementPass::execute(UserProc *proc)
{
    StatementList stmts;
    proc->getStatements(stmts);
    ImplicitConverter ic(proc->getCFG());
    StmtImplicitConverter sm(&ic, proc->getCFG());

    for (SharedStmt stmt : stmts) {
        stmt->accept(&sm);
    }

    proc->getCFG()->setImplicitsDone();
    proc->getDataFlow()->convertImplicits(); // Some maps have m[...]{-} need to be m[...]{0} now
    makeSymbolsImplicit(proc);

    return true;
}


bool ImplicitPlacementPass::makeSymbolsImplicit(UserProc *proc)
{
    // Copy the whole map; necessary because the keys (Exps) change
    UserProc::SymbolMap sm2 = proc->getSymbolMap();
    proc->getSymbolMap().clear();
    ImplicitConverter ic(proc->getCFG());

    for (const auto &[first, second] : sm2) {
        SharedExp impFrom = std::const_pointer_cast<Exp>(first)->acceptModifier(&ic);
        proc->mapSymbolTo(impFrom, second);
    }

    return true;
}
