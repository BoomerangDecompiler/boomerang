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


#include "boomerang/ssl/exp/Exp.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/BoolAssign.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/visitor/expmodifier/ExpSubscripter.h"


StmtSubscripter::StmtSubscripter(ExpSubscripter *es)
    : StmtModifier(es)
{
}

void StmtSubscripter::visit(Assign *stmt, bool& visitChildren)
{
    SharedExp rhs = stmt->getRight();

    stmt->setRight(rhs->acceptModifier(m_mod));
    // Don't subscript the LHS of an assign, ever
    SharedExp lhs = stmt->getLeft();

    if (lhs->isMemOf() || lhs->isRegOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->acceptModifier(m_mod));
    }

    visitChildren = false;
}


void StmtSubscripter::visit(PhiAssign *stmt, bool& visitChildren)
{
    SharedExp lhs = stmt->getLeft();

    if (lhs->isMemOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->acceptModifier(m_mod));
    }

    visitChildren = false;
}


void StmtSubscripter::visit(ImplicitAssign *stmt, bool& visitChildren)
{
    SharedExp lhs = stmt->getLeft();

    if (lhs->isMemOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->acceptModifier(m_mod));
    }

    visitChildren = false;
}


void StmtSubscripter::visit(BoolAssign *stmt, bool& visitChildren)
{
    SharedExp lhs = stmt->getLeft();

    if (lhs->isMemOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->acceptModifier(m_mod));
    }

    SharedExp rhs = stmt->getCondExpr();
    stmt->setCondExpr(rhs->acceptModifier(m_mod));
    visitChildren = false;
}


void StmtSubscripter::visit(CallStatement *stmt, bool& visitChildren)
{
    SharedExp condExp = stmt->getDest();

    if (condExp) {
        stmt->setDest(condExp->acceptModifier(m_mod));
    }

    // Subscript the ordinary arguments
    const StatementList& arguments = stmt->getArguments();

    for (StatementList::const_iterator ss = arguments.begin(); ss != arguments.end(); ++ss) {
        (*ss)->accept(this);
    }

    // Returns are like the LHS of an assignment;
    // don't subscript them directly
    // (only if m[x], and then only subscript the x's)
    visitChildren = false; // Don't do the usual accept logic
}
