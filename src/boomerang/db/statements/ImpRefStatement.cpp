#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ImpRefStatement.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/Visitor.h"
#include "boomerang/type/Type.h"
#include "boomerang/util/Log.h"


ImpRefStatement::ImpRefStatement(SharedType ty, SharedExp a)
    : TypingStatement(ty)
    , m_addressExp(a)
{
    m_kind = STMT_IMPREF;
}


void ImpRefStatement::print(QTextStream& os, bool html) const
{
    os << "     *";     // No statement number

    if (html) {
        os << "</td><td>";
        os << "<a name=\"stmt" << m_number << "\">";
    }

    os << m_type << "* IMP REF " << m_addressExp;

    if (html) {
        os << "</a></td>";
    }
}


void ImpRefStatement::meetWith(SharedType ty, bool& ch)
{
    m_type = m_type->meetWith(ty, ch);
}


Statement *ImpRefStatement::clone() const
{
    return new ImpRefStatement(m_type->clone(), m_addressExp->clone());
}


bool ImpRefStatement::accept(StmtVisitor *visitor)
{
    return visitor->visit(this);
}


bool ImpRefStatement::accept(StmtExpVisitor *v)
{
    bool override;
    bool ret = v->visit(this, override);

    if (override) {
        return ret;
    }

    if (ret) {
        ret = m_addressExp->accept(v->ev);
    }

    return ret;
}


bool ImpRefStatement::accept(StmtModifier *v)
{
    bool recur;

    v->visit(this, recur);
    v->m_mod->clearMod();

    if (recur) {
        m_addressExp = m_addressExp->accept(v->m_mod);
    }

    if (v->m_mod->isMod()) {
        LOG_VERBOSE("ImplicitRef changed: now %1", this);
    }

    return true;
}


bool ImpRefStatement::accept(StmtPartModifier *v)
{
    bool recur;

    v->visit(this, recur);
    v->mod->clearMod();

    if (recur) {
        m_addressExp = m_addressExp->accept(v->mod);
    }

    if (v->mod->isMod()) {
        LOG_VERBOSE("ImplicitRef changed: now %1", this);
    }

    return true;
}


bool ImpRefStatement::search(const Exp& search, SharedExp& result) const
{
    result = nullptr;
    return m_addressExp->search(search, result);
}


bool ImpRefStatement::searchAll(const Exp& search, std::list<SharedExp, std::allocator<SharedExp> >& result) const
{
    return m_addressExp->searchAll(search, result);
}


bool ImpRefStatement::searchAndReplace(const Exp& search, SharedExp replace, bool /*cc*/)
{
    bool change;

    m_addressExp = m_addressExp->searchReplaceAll(search, replace, change);
    return change;
}


void ImpRefStatement::simplify()
{
    m_addressExp = m_addressExp->simplify();
}
