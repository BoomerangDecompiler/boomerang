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


#include "boomerang/db/visitor/ConscriptSetter.h"
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

bool StmtConscriptSetter::visit(Assign *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getLeft()->accept(&sc);
    stmt->getRight()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(PhiAssign *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getLeft()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(ImplicitAssign *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getLeft()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(CallStatement *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    for (Statement *s : stmt->getArguments()) {
        s->accept(this);
    }

    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(CaseStatement *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);
    SwitchInfo   *si = stmt->getSwitchInfo();

    if (si) {
        si->switchExp->accept(&sc);
        m_curConscript = sc.getLast();
    }

    return true;
}


bool StmtConscriptSetter::visit(ReturnStatement *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    for (Statement *ret : *stmt) {
        ret->accept(this);
    }

    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(BoolAssign *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getCondExpr()->accept(&sc);
    stmt->getLeft()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(BranchStatement *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getCondExpr()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(ImpRefStatement *stmt)
{
    ConscriptSetter sc(m_curConscript, m_clear);

    stmt->getAddressExp()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}
