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


// Common code for the left hand side of assignments
void StmtSsaXformer::commonLhs(Assignment *as)
{
    SharedExp lhs = as->getLeft();

    lhs = lhs->accept((ExpSsaXformer *)m_mod); // In case the LHS has say m[r28{0}+8] -> m[esp+8]
    QString sym = m_proc->lookupSymFromRefAny(RefExp::get(lhs, as));

    if (!sym.isNull()) {
        as->setLeft(Location::local(sym, m_proc));
    }
}


void StmtSsaXformer::visit(BoolAssign *s, bool& recur)
{
    commonLhs(s);
    SharedExp pCond = s->getCondExpr();
    pCond = pCond->accept((ExpSsaXformer *)m_mod);
    s->setCondExpr(pCond);
    recur = false; // TODO: verify recur setting
}


void StmtSsaXformer::visit(Assign *s, bool& recur)
{
    commonLhs(s);
    SharedExp rhs = s->getRight();
    rhs = rhs->accept(m_mod);
    s->setRight(rhs);
    recur = false; // TODO: verify recur setting
}


void StmtSsaXformer::visit(ImplicitAssign *s, bool& recur)
{
    commonLhs(s);
    recur = false; // TODO: verify recur setting
}


void StmtSsaXformer::visit(PhiAssign *s, bool& recur)
{
    commonLhs(s);

    UserProc *_proc = ((ExpSsaXformer *)m_mod)->getProc();

    for (auto& v : *s) {
        assert(v.second.e != nullptr);
        QString sym = _proc->lookupSymFromRefAny(RefExp::get(v.second.e, v.second.getDef()));

        if (!sym.isNull()) {
            v.second.e = Location::local(sym, _proc); // Some may be parameters, but hopefully it won't matter
        }
    }

    recur = false; // TODO: verify recur setting
}


void StmtSsaXformer::visit(CallStatement *s, bool& recur)
{
    SharedExp pDest = s->getDest();

    if (pDest) {
        pDest = pDest->accept((ExpSsaXformer *)m_mod);
        s->setDest(pDest);
    }

    const StatementList& arguments = s->getArguments();

    for (StatementList::const_iterator ss = arguments.begin(); ss != arguments.end(); ++ss) {
        (*ss)->accept(this);
    }

    // Note that defines have statements (assignments) within a statement (this call). The fromSSA logic, which needs
    // to subscript definitions on the left with the statement pointer, won't work if we just call the assignment's
    // fromSSA() function
    StatementList& defines = s->getDefines();

    for (StatementList::iterator ss = defines.begin(); ss != defines.end(); ++ss) {
        Assignment *as = ((Assignment *)*ss);
        // FIXME: use of fromSSAleft is deprecated
        SharedExp e = as->getLeft()->fromSSAleft(((ExpSsaXformer *)m_mod)->getProc(), s);
        // FIXME: this looks like a HACK that can go:
        Function *procDest = s->getDestProc();

        if (procDest && procDest->isLib() && e->isLocal()) {
            UserProc   *_proc = s->getProc(); // Enclosing proc
            SharedType lty    = _proc->getLocalType(e->access<Const, 1>()->getStr());
            SharedType ty     = as->getType();

            if (ty && lty && (*ty != *lty)) {
                LOG_MSG("Local %1 has type %2 that doesn't agree with type of define %3 of a library, why?",
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
    s->useColfromSSAForm(s);
    recur = false; // TODO: verify recur setting
}
