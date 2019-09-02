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

void StmtSubscripter::visit(const std::shared_ptr<Assign> &stmt, bool &visitChildren)
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


void StmtSubscripter::visit(const std::shared_ptr<PhiAssign> &stmt, bool &visitChildren)
{
    SharedExp lhs = stmt->getLeft();

    if (lhs->isMemOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->acceptModifier(m_mod));
    }

    visitChildren = false;
}


void StmtSubscripter::visit(const std::shared_ptr<ImplicitAssign> &stmt, bool &visitChildren)
{
    SharedExp lhs = stmt->getLeft();

    if (lhs->isMemOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->acceptModifier(m_mod));
    }

    visitChildren = false;
}


void StmtSubscripter::visit(const std::shared_ptr<BoolAssign> &stmt, bool &visitChildren)
{
    SharedExp lhs = stmt->getLeft();

    if (lhs->isMemOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->acceptModifier(m_mod));
    }

    SharedExp rhs = stmt->getCondExpr();
    stmt->setCondExpr(rhs->acceptModifier(m_mod));
    visitChildren = false;
}


void StmtSubscripter::visit(const std::shared_ptr<CallStatement> &stmt, bool &visitChildren)
{
    if (stmt->getDest()) {
        stmt->setDest(stmt->getDest()->acceptModifier(m_mod));
    }

    // Subscript the ordinary arguments
    for (SharedStmt arg : stmt->getArguments()) {
        arg->accept(this);
    }

    // Returns are like the LHS of an assignment;
    // don't subscript them directly
    // (only if m[x], and then only subscript the x's)
    visitChildren = false; // Don't do the usual accept logic
}
