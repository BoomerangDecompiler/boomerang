#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StmtSubscripter.h"


#include "boomerang/db/exp/Exp.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/statements/BoolAssign.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/visitor/ExpSubscripter.h"


StmtSubscripter::StmtSubscripter(ExpSubscripter *es)
    : StmtModifier(es)
{
}

void StmtSubscripter::visit(Assign *s, bool& recur)
{
    SharedExp rhs = s->getRight();

    s->setRight(rhs->accept(m_mod));
    // Don't subscript the LHS of an assign, ever
    SharedExp lhs = s->getLeft();

    if (lhs->isMemOf() || lhs->isRegOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->accept(m_mod));
    }

    recur = false;
}


void StmtSubscripter::visit(PhiAssign *s, bool& recur)
{
    SharedExp lhs = s->getLeft();

    if (lhs->isMemOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->accept(m_mod));
    }

    recur = false;
}


void StmtSubscripter::visit(ImplicitAssign *s, bool& recur)
{
    SharedExp lhs = s->getLeft();

    if (lhs->isMemOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->accept(m_mod));
    }

    recur = false;
}


void StmtSubscripter::visit(BoolAssign *s, bool& recur)
{
    SharedExp lhs = s->getLeft();

    if (lhs->isMemOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->accept(m_mod));
    }

    SharedExp rhs = s->getCondExpr();
    s->setCondExpr(rhs->accept(m_mod));
    recur = false;
}


void StmtSubscripter::visit(CallStatement *s, bool& recur)
{
    SharedExp pDest = s->getDest();

    if (pDest) {
        s->setDest(pDest->accept(m_mod));
    }

    // Subscript the ordinary arguments
    const StatementList& arguments = s->getArguments();

    for (StatementList::const_iterator ss = arguments.begin(); ss != arguments.end(); ++ss) {
        (*ss)->accept(this);
    }

    // Returns are like the LHS of an assignment;
    // don't subscript them directly
    // (only if m[x], and then only subscript the x's)
    recur = false; // Don't do the usual accept logic
}
