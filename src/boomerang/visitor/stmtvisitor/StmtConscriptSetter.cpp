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


#include "boomerang/visitor/expvisitor/ConscriptSetter.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/statements/BoolAssign.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/CaseStatement.h"
#include "boomerang/db/statements/ReturnStatement.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/ImpRefStatement.h"
#include "boomerang/db/exp/Exp.h"


StmtConscriptSetter::StmtConscriptSetter(int n, bool clear)
    : m_curConscript(n)
    , m_clear(clear)
{
}

bool StmtConscriptSetter::visit(const Assign *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getLeft()->accept(&sc);
    stmt->getRight()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(const PhiAssign *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getLeft()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(const ImplicitAssign *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getLeft()->accept(&sc);
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
    const SwitchInfo   *si = stmt->getSwitchInfo();

    if (si) {
        si->switchExp->accept(&sc);
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

    stmt->getCondExpr()->accept(&sc);
    stmt->getLeft()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(const BranchStatement *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getCondExpr()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(const ImpRefStatement *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getAddressExp()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}
