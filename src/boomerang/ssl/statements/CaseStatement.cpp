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

#include "boomerang/ssl/exp/Exp.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"
#include "boomerang/visitor/stmtexpvisitor/StmtExpVisitor.h"
#include "boomerang/visitor/stmtmodifier/StmtModifier.h"
#include "boomerang/visitor/stmtmodifier/StmtPartModifier.h"
#include "boomerang/visitor/stmtvisitor/StmtVisitor.h"

#include <QTextStreamManipulator>


CaseStatement::CaseStatement()
    : m_switchInfo(nullptr)
{
    m_kind = StmtType::Case;
}


CaseStatement::CaseStatement(const CaseStatement &other)
    : GotoStatement(other)
    , m_switchInfo(new SwitchInfo(*other.m_switchInfo))
{
}


CaseStatement::~CaseStatement()
{
}


CaseStatement &CaseStatement::operator=(const CaseStatement &other)
{
    GotoStatement::operator=(other);

    m_switchInfo.reset(new SwitchInfo(*other.m_switchInfo));
    return *this;
}


SwitchInfo *CaseStatement::getSwitchInfo()
{
    return m_switchInfo.get();
}


const SwitchInfo *CaseStatement::getSwitchInfo() const
{
    return m_switchInfo.get();
}


void CaseStatement::setSwitchInfo(std::unique_ptr<SwitchInfo> psi)
{
    m_switchInfo = std::move(psi);
}


bool CaseStatement::searchAndReplace(const Exp &pattern, SharedExp replace, bool cc)
{
    bool ch  = GotoStatement::searchAndReplace(pattern, replace, cc);
    bool ch2 = false;

    if (m_switchInfo && m_switchInfo->switchExp) {
        m_switchInfo->switchExp = m_switchInfo->switchExp->searchReplaceAll(pattern, replace, ch2);
    }

    return ch | ch2;
}


bool CaseStatement::searchAll(const Exp &pattern, std::list<SharedExp> &result) const
{
    return GotoStatement::searchAll(pattern, result) ||
           (m_switchInfo && m_switchInfo->switchExp &&
            m_switchInfo->switchExp->searchAll(pattern, result));
}


void CaseStatement::print(OStream &os) const
{
    os << qSetFieldWidth(4) << m_number << qSetFieldWidth(0) << " ";
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
}


SharedStmt CaseStatement::clone() const
{
    std::shared_ptr<CaseStatement> ret(new CaseStatement());

    ret->m_dest       = m_dest ? m_dest->clone() : nullptr;
    ret->m_isComputed = m_isComputed;

    if (m_switchInfo) {
        ret->m_switchInfo.reset(new SwitchInfo);
        *ret->m_switchInfo           = *m_switchInfo;
        ret->m_switchInfo->switchExp = m_switchInfo->switchExp->clone();
    }

    // Statement members
    ret->m_bb     = m_bb;
    ret->m_proc   = m_proc;
    ret->m_number = m_number;

    return ret;
}


bool CaseStatement::accept(StmtVisitor *visitor) const
{
    return visitor->visit(this);
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
    bool ret           = v->visit(shared_from_this()->as<CaseStatement>(), visitChildren);

    if (!visitChildren) {
        return ret;
    }

    if (ret && m_dest) {
        ret = m_dest->acceptVisitor(v->ev);
    }

    if (ret && m_switchInfo && m_switchInfo->switchExp) {
        ret = m_switchInfo->switchExp->acceptVisitor(v->ev);
    }

    return ret;
}


bool CaseStatement::accept(StmtModifier *v)
{
    bool visitChildren;
    v->visit(shared_from_this()->as<CaseStatement>(), visitChildren);

    if (v->m_mod) {
        if (m_dest && visitChildren) {
            m_dest = m_dest->acceptModifier(v->m_mod);
        }

        if (m_switchInfo && m_switchInfo->switchExp && visitChildren) {
            m_switchInfo->switchExp = m_switchInfo->switchExp->acceptModifier(v->m_mod);
        }
    }

    return true;
}


bool CaseStatement::accept(StmtPartModifier *v)
{
    bool visitChildren;
    v->visit(shared_from_this()->as<CaseStatement>(), visitChildren);

    if (m_dest && visitChildren) {
        m_dest = m_dest->acceptModifier(v->mod);
    }

    if (m_switchInfo && m_switchInfo->switchExp && visitChildren) {
        m_switchInfo->switchExp = m_switchInfo->switchExp->acceptModifier(v->mod);
    }

    return true;
}
