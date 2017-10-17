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


#include "boomerang/db/Visitor.h"


CaseStatement::CaseStatement()
    : pSwitchInfo(nullptr)
{
    m_kind = STMT_CASE;
}


CaseStatement::~CaseStatement()
{
    if (pSwitchInfo) {
        // delete pSwitchInfo;
    }
}


SWITCH_INFO *CaseStatement::getSwitchInfo()
{
    return pSwitchInfo;
}


void CaseStatement::setSwitchInfo(SWITCH_INFO *psi)
{
    pSwitchInfo = psi;
}


bool CaseStatement::searchAndReplace(const Exp& search, SharedExp replace, bool cc)
{
    bool ch  = GotoStatement::searchAndReplace(search, replace, cc);
    bool ch2 = false;

    if (pSwitchInfo && pSwitchInfo->pSwitchVar) {
        pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->searchReplaceAll(search, replace, ch2);
    }

    return ch | ch2;
}


bool CaseStatement::searchAll(const Exp& search, std::list<SharedExp>& result) const
{
    return GotoStatement::searchAll(search, result) ||
           (pSwitchInfo && pSwitchInfo->pSwitchVar && pSwitchInfo->pSwitchVar->searchAll(search, result));
}


void CaseStatement::print(QTextStream& os, bool html) const
{
    os << qSetFieldWidth(4) << m_number << qSetFieldWidth(0) << " ";

    if (html) {
        os << "</td><td>";
        os << "<a name=\"stmt" << m_number << "\">";
    }

    if (pSwitchInfo == nullptr) {
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
        os << "SWITCH(" << pSwitchInfo->pSwitchVar << ")\n";
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

    if (pSwitchInfo) {
        ret->pSwitchInfo             = new SWITCH_INFO;
        *ret->pSwitchInfo            = *pSwitchInfo;
        ret->pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->clone();
    }

    // Statement members
    ret->m_parent = m_parent;
    ret->m_proc   = m_proc;
    ret->m_number = m_number;
    return ret;
}


bool CaseStatement::accept(StmtVisitor *visitor)
{
    return visitor->visit(this);
}


void CaseStatement::generateCode(ICodeGenerator *, BasicBlock *)
{
    // don't generate any code for switches, they will be handled by the bb
}


bool CaseStatement::usesExp(const Exp& e) const
{
    // Before a switch statement is recognised, pDest is non null
    if (m_dest) {
        return *m_dest == e;
    }

    // After a switch statement is recognised, pDest is null, and pSwitchInfo->pSwitchVar takes over
    if (pSwitchInfo->pSwitchVar) {
        return *pSwitchInfo->pSwitchVar == e;
    }

    return false;
}


void CaseStatement::simplify()
{
    if (m_dest) {
        m_dest = m_dest->simplify();
    }
    else if (pSwitchInfo && pSwitchInfo->pSwitchVar) {
        pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->simplify();
    }
}


bool CaseStatement::accept(StmtExpVisitor *v)
{
    bool override;
    bool ret = v->visit(this, override);

    if (override) {
        return ret;
    }

    if (ret && m_dest) {
        ret = m_dest->accept(v->ev);
    }

    if (ret && pSwitchInfo && pSwitchInfo->pSwitchVar) {
        ret = pSwitchInfo->pSwitchVar->accept(v->ev);
    }

    return ret;
}


bool CaseStatement::accept(StmtModifier *v)
{
    bool recur;

    v->visit(this, recur);

    if (m_dest && recur) {
        m_dest = m_dest->accept(v->m_mod);
    }

    if (pSwitchInfo && pSwitchInfo->pSwitchVar && recur) {
        pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->accept(v->m_mod);
    }

    return true;
}


bool CaseStatement::accept(StmtPartModifier *v)
{
    bool recur;

    v->visit(this, recur);

    if (m_dest && recur) {
        m_dest = m_dest->accept(v->mod);
    }

    if (pSwitchInfo && pSwitchInfo->pSwitchVar && recur) {
        pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->accept(v->mod);
    }

    return true;
}
