#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StmtConscriptSetter.h"

#include "boomerang/ssl/exp/Exp.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/BoolAssign.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/ssl/statements/ImpRefStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/visitor/expmodifier/ConscriptSetter.h"


StmtConscriptSetter::StmtConscriptSetter(int n, bool clear)
    : m_curConscript(n)
    , m_clear(clear)
{
}


bool StmtConscriptSetter::visit(const Assign *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getLeft()->acceptModifier(&sc);
    stmt->getRight()->acceptModifier(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(const PhiAssign *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getLeft()->acceptModifier(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(const ImplicitAssign *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getLeft()->acceptModifier(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(const CallStatement *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    for (const Statement *s : stmt->getArguments()) {
        s->accept(this);
    }

    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(const CaseStatement *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);
    const SwitchInfo *si = stmt->getSwitchInfo();

    if (si) {
        si->switchExp->acceptModifier(&sc);
        m_curConscript = sc.getLast();
    }

    return true;
}


bool StmtConscriptSetter::visit(const ReturnStatement *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    for (Statement *ret : *stmt) {
        ret->accept(this);
    }

    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(const BoolAssign *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getCondExpr()->acceptModifier(&sc);
    stmt->getLeft()->acceptModifier(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(const BranchStatement *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getCondExpr()->acceptModifier(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(const ImpRefStatement *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getAddressExp()->acceptModifier(&sc);
    m_curConscript = sc.getLast();
    return true;
}
