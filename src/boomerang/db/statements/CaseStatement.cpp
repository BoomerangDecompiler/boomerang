#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CaseStatement.h"


#include "boomerang/db/visitor/ExpVisitor.h"
#include "boomerang/db/visitor/StmtVisitor.h"
#include "boomerang/db/visitor/StmtExpVisitor.h"
#include "boomerang/db/visitor/StmtModifier.h"
#include "boomerang/db/visitor/StmtPartModifier.h"


CaseStatement::CaseStatement()
    : m_switchInfo(nullptr)
{
    m_kind = StmtType::Case;
}


CaseStatement::~CaseStatement()
{
}


SwitchInfo *CaseStatement::getSwitchInfo()
{
    return m_switchInfo;
}


const SwitchInfo *CaseStatement::getSwitchInfo() const
{
    return m_switchInfo;
}


void CaseStatement::setSwitchInfo(SwitchInfo *psi)
{
    m_switchInfo = psi;
}


bool CaseStatement::searchAndReplace(const Exp& pattern, SharedExp replace, bool cc)
{
    bool ch  = GotoStatement::searchAndReplace(pattern, replace, cc);
    bool ch2 = false;

    if (m_switchInfo && m_switchInfo->switchExp) {
        m_switchInfo->switchExp = m_switchInfo->switchExp->searchReplaceAll(pattern, replace, ch2);
    }

    return ch | ch2;
}


bool CaseStatement::searchAll(const Exp& pattern, std::list<SharedExp>& result) const
{
    return GotoStatement::searchAll(pattern, result) ||
           (m_switchInfo && m_switchInfo->switchExp && m_switchInfo->switchExp->searchAll(pattern, result));
}


void CaseStatement::print(QTextStream& os, bool html) const
{
    os << qSetFieldWidth(4) << m_number << qSetFieldWidth(0) << " ";

    if (html) {
        os << "</td><td>";
        os << "<a name=\"stmt" << m_number << "\">";
    }

    if (m_switchInfo == nullptr) {
        os << "CASE [";

        if (m_dest == nullptr) {
            os << "*no dest*";
        }
        else {
            os << m_dest;
        }

        os << "]";
    }
    else {
        os << "SWITCH(" << m_switchInfo->switchExp << ")\n";
    }

    if (html) {
        os << "</a></td>";
    }
}


Statement *CaseStatement::clone() const
{
    CaseStatement *ret = new CaseStatement();

    ret->m_dest       = m_dest ? m_dest->clone() : nullptr;
    ret->m_isComputed = m_isComputed;

    if (m_switchInfo) {
        ret->m_switchInfo             = new SwitchInfo;
        *ret->m_switchInfo            = *m_switchInfo;
        ret->m_switchInfo->switchExp = m_switchInfo->switchExp->clone();
    }

    // Statement members
    ret->m_bb = m_bb;
    ret->m_proc   = m_proc;
    ret->m_number = m_number;
    return ret;
}


bool CaseStatement::accept(StmtVisitor *visitor)
{
    return visitor->visit(this);
}


void CaseStatement::generateCode(ICodeGenerator *, const BasicBlock *)
{
    // don't generate any code for switches, they will be handled by the bb
}


bool CaseStatement::usesExp(const Exp& e) const
{
    // Before a switch statement is recognised, m_dest is non null
    if (m_dest) {
        return *m_dest == e;
    }

    // After a switch statement is recognised, m_dest is null, and m_switchInfo->m_switchVar takes over
    if (m_switchInfo->switchExp) {
        return *m_switchInfo->switchExp == e;
    }

    return false;
}


void CaseStatement::simplify()
{
    if (m_dest) {
        m_dest = m_dest->simplify();
    }
    else if (m_switchInfo && m_switchInfo->switchExp) {
        m_switchInfo->switchExp = m_switchInfo->switchExp->simplify();
    }
}


bool CaseStatement::accept(StmtExpVisitor *v)
{
    bool visitChildren = true;
    bool ret = v->visit(this, visitChildren);

    if (!visitChildren) {
        return ret;
    }

    if (ret && m_dest) {
        ret = m_dest->accept(v->ev);
    }

    if (ret && m_switchInfo && m_switchInfo->switchExp) {
        ret = m_switchInfo->switchExp->accept(v->ev);
    }

    return ret;
}


bool CaseStatement::accept(StmtModifier *v)
{
    bool visitChildren;
    v->visit(this, visitChildren);

    if (v->m_mod) {
        if (m_dest && visitChildren) {
            m_dest = m_dest->accept(v->m_mod);
        }

        if (m_switchInfo && m_switchInfo->switchExp && visitChildren) {
            m_switchInfo->switchExp = m_switchInfo->switchExp->accept(v->m_mod);
        }
    }

    return true;
}


bool CaseStatement::accept(StmtPartModifier *v)
{
    bool visitChildren;
    v->visit(this, visitChildren);

    if (m_dest && visitChildren) {
        m_dest = m_dest->accept(v->mod);
    }

    if (m_switchInfo && m_switchInfo->switchExp && visitChildren) {
        m_switchInfo->switchExp = m_switchInfo->switchExp->accept(v->mod);
    }

    return true;
}
