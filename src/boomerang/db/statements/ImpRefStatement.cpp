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
#include "boomerang/db/exp/Exp.h"
#include "boomerang/db/visitor/ExpVisitor.h"
#include "boomerang/db/visitor/ExpModifier.h"
#include "boomerang/db/visitor/StmtVisitor.h"
#include "boomerang/db/visitor/StmtExpVisitor.h"
#include "boomerang/db/visitor/StmtModifier.h"
#include "boomerang/db/visitor/StmtPartModifier.h"
#include "boomerang/type/type/Type.h"
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
    bool dontVisitChildren;
    bool ret = v->visit(this, dontVisitChildren);

    if (dontVisitChildren) {
        return ret;
    }

    if (ret) {
        ret = m_addressExp->accept(v->ev);
    }

    return ret;
}


bool ImpRefStatement::accept(StmtModifier *v)
{
    bool visitChildren;
    v->visit(this, visitChildren);
    v->m_mod->clearMod();

    if (visitChildren) {
        m_addressExp = m_addressExp->accept(v->m_mod);
    }

    if (v->m_mod->isMod()) {
        LOG_VERBOSE("ImplicitRef changed: now %1", this);
    }

    return true;
}


bool ImpRefStatement::accept(StmtPartModifier *v)
{
    bool visitChildren;

    v->visit(this, visitChildren);
    v->mod->clearMod();

    if (visitChildren) {
        m_addressExp = m_addressExp->accept(v->mod);
    }

    if (v->mod->isMod()) {
        LOG_VERBOSE("ImplicitRef changed: now %1", this);
    }

    return true;
}


bool ImpRefStatement::search(const Exp& pattern, SharedExp& result) const
{
    result = nullptr;
    return m_addressExp->search(pattern, result);
}


bool ImpRefStatement::searchAll(const Exp& pattern, std::list<SharedExp, std::allocator<SharedExp> >& result) const
{
    return m_addressExp->searchAll(pattern, result);
}


bool ImpRefStatement::searchAndReplace(const Exp& pattern, SharedExp replace, bool /*cc*/)
{
    bool change;

    m_addressExp = m_addressExp->searchReplaceAll(pattern, replace, change);
    return change;
}


void ImpRefStatement::simplify()
{
    m_addressExp = m_addressExp->simplify();
}
