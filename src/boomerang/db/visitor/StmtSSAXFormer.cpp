#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StmtSSAXFormer.h"


#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/statements/BoolAssign.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/visitor/ExpSSAXformer.h"
#include "boomerang/util/Log.h"


StmtSsaXformer::StmtSsaXformer(ExpSsaXformer* esx, UserProc* p)
    : StmtModifier(esx)
    , m_proc(p)
{
}


void StmtSsaXformer::commonLhs(Assignment *as)
{
    SharedExp lhs = as->getLeft();

    lhs = lhs->accept(static_cast<ExpSsaXformer *>(m_mod)); // In case the LHS has say m[r28{0}+8] -> m[esp+8]
    QString sym = m_proc->lookupSymFromRefAny(RefExp::get(lhs, as));

    if (!sym.isNull()) {
        as->setLeft(Location::local(sym, m_proc));
    }
}


void StmtSsaXformer::visit(BoolAssign *stmt, bool& visitChildren)
{
    commonLhs(stmt);
    SharedExp condExp = stmt->getCondExpr();
    condExp = condExp->accept(static_cast<ExpSsaXformer *>(m_mod));
    stmt->setCondExpr(condExp);
    visitChildren = false; // TODO: verify recur setting
}


void StmtSsaXformer::visit(Assign *stmt, bool& visitChildren)
{
    commonLhs(stmt);
    SharedExp rhs = stmt->getRight();
    rhs = rhs->accept(m_mod);
    stmt->setRight(rhs);
    visitChildren = false; // TODO: verify recur setting
}


void StmtSsaXformer::visit(ImplicitAssign *stmt, bool& visitChildren)
{
    commonLhs(stmt);
    visitChildren = false; // TODO: verify recur setting
}


void StmtSsaXformer::visit(PhiAssign *stmt, bool& visitChildren)
{
    commonLhs(stmt);

    UserProc *_proc = static_cast<ExpSsaXformer *>(m_mod)->getProc();

    for (RefExp& v : *stmt) {
        assert(v.getSubExp1() != nullptr);
        QString sym = _proc->lookupSymFromRefAny(RefExp::get(v.getSubExp1(), v.getDef()));

        if (!sym.isNull()) {
            v.getSubExp1() = Location::local(sym, _proc); // Some may be parameters, but hopefully it won't matter
        }
    }

    visitChildren = false; // TODO: verify recur setting
}


void StmtSsaXformer::visit(CallStatement *stmt, bool& visitChildren)
{
    SharedExp callDest = stmt->getDest();

    if (callDest) {
        stmt->setDest(callDest->accept(static_cast<ExpSsaXformer*>(m_mod)));
    }

    const StatementList& arguments = stmt->getArguments();

    for (Statement *s : arguments) {
        s->accept(this);
    }

    // Note that defines have statements (assignments) within a statement (this call).
    // The fromSSA logic, which needs to subscript definitions on the left
    // with the statement pointer, won't work if we just call the assignment's
    // fromSSA() function
    StatementList& defines = stmt->getDefines();

    for (StatementList::iterator ss = defines.begin(); ss != defines.end(); ++ss) {
        assert((*ss)->isAssignment());
        Assignment *as = static_cast<Assignment *>(*ss);
        // FIXME: use of fromSSAleft is deprecated
        SharedExp e = as->getLeft()->fromSSAleft(static_cast<ExpSsaXformer *>(m_mod)->getProc(), stmt);

        // FIXME: this looks like a HACK that can go:
        Function *procDest = stmt->getDestProc();

        if (procDest && procDest->isLib() && e->isLocal()) {
            UserProc   *_proc = stmt->getProc(); // Enclosing proc
            SharedType lty    = _proc->getLocalType(e->access<Const, 1>()->getStr());
            SharedType ty     = as->getType();

            if (ty && lty && (*ty != *lty)) {
                LOG_WARN("Forcing type of local '%1' from '%2' to '%3' due to library constraints",
                        e, lty->getCtype(), ty->getCtype());
                _proc->setLocalType(e->access<Const, 1>()->getStr(), ty);
            }
        }

        as->setLeft(e);
    }

    // Don't think we'll need this anyway:
    // defCol.fromSSAForm(ig);

    // However, need modifications of the use collector; needed when say eax is renamed to local5, otherwise
    // local5 is removed from the results of the call
    stmt->useColfromSSAForm(stmt);
    visitChildren = false; // TODO: verify recur setting
}
