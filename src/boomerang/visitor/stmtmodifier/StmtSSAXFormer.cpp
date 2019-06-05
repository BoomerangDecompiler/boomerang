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
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/BoolAssign.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/ssl/type/Type.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expmodifier/ExpSSAXformer.h"


StmtSSAXformer::StmtSSAXformer(ExpSSAXformer *esx, UserProc *proc)
    : StmtModifier(esx)
    , m_proc(proc)
{
}


void StmtSSAXformer::handleCommonLHS(Assignment *as)
{
    SharedExp lhs = as->getLeft();

    lhs = lhs->acceptModifier(
        static_cast<ExpSSAXformer *>(m_mod)); // In case the LHS has say m[r28{0}+8] -> m[esp+8]

    const QString sym = m_proc->lookupSymFromRefAny(RefExp::get(lhs, as));
    if (!sym.isEmpty()) {
        as->setLeft(Location::local(sym, m_proc));
    }
}


void StmtSSAXformer::visit(BoolAssign *stmt, bool &visitChildren)
{
    handleCommonLHS(stmt);
    SharedExp condExp = stmt->getCondExpr();
    condExp           = condExp->acceptModifier(static_cast<ExpSSAXformer *>(m_mod));
    stmt->setCondExpr(condExp);
    visitChildren = false; // TODO: verify recur setting
}


void StmtSSAXformer::visit(Assign *stmt, bool &visitChildren)
{
    handleCommonLHS(stmt);
    SharedExp rhs = stmt->getRight();
    rhs           = rhs->acceptModifier(m_mod);
    stmt->setRight(rhs);
    visitChildren = false; // TODO: verify recur setting
}


void StmtSSAXformer::visit(ImplicitAssign *stmt, bool &visitChildren)
{
    handleCommonLHS(stmt);
    visitChildren = false; // TODO: verify recur setting
}


void StmtSSAXformer::visit(PhiAssign *stmt, bool &visitChildren)
{
    handleCommonLHS(stmt);

    UserProc *_proc = static_cast<ExpSSAXformer *>(m_mod)->getProc();

    for (const std::shared_ptr<RefExp> &v : *stmt) {
        assert(v->getSubExp1() != nullptr);
        QString sym = _proc->lookupSymFromRefAny(RefExp::get(v->getSubExp1(), v->getDef()));

        if (!sym.isEmpty()) {
            // Some may be parameters, but hopefully it won't matter
            v->refSubExp1() = Location::local(sym, _proc);
        }
    }

    visitChildren = false; // TODO: verify recur setting
}


void StmtSSAXformer::visit(CallStatement *stmt, bool &visitChildren)
{
    SharedExp callDest = stmt->getDest();

    if (callDest) {
        stmt->setDest(callDest->acceptModifier(static_cast<ExpSSAXformer *>(m_mod)));
    }

    const StatementList &arguments = stmt->getArguments();

    for (Statement *s : arguments) {
        s->accept(this);
    }

    // Note that defines have statements (assignments) within a statement (this call).
    // The fromSSA logic, which needs to subscript definitions on the left
    // with the statement pointer, won't work if we just call the assignment's
    // fromSSA() function
    StatementList &defines = stmt->getDefines();

    for (Statement *define : defines) {
        assert(define->isAssignment());
        Assignment *as = static_cast<Assignment *>(define);
        // FIXME: use of fromSSAleft is deprecated
        SharedExp e = as->getLeft()->fromSSAleft(static_cast<ExpSSAXformer *>(m_mod)->getProc(),
                                                 stmt);

        // FIXME: this looks like a HACK that can go:
        Function *procDest = stmt->getDestProc();

        if (procDest && procDest->isLib() && e->isLocal()) {
            UserProc *_proc     = stmt->getProc(); // Enclosing proc
            SharedConstType lty = _proc->getLocalType(e->access<Const, 1>()->getStr());
            SharedType ty       = as->getType();

            if (ty && lty && (*ty != *lty)) {
                LOG_WARN("Forcing type of local '%1' from '%2' to '%3' due to library constraints",
                         e, lty->getCtype(), ty->getCtype());
                _proc->setLocalType(e->access<Const, 1>()->getStr(), ty);
            }
        }

        as->setLeft(e);
    }

    // Need modifications of the use collector; needed when say %eax is renamed to local5,
    // otherwise local5 is removed from the results of the call
    stmt->useColfromSSAForm(stmt);
    visitChildren = false; // TODO: verify recur setting
}
