#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ImplicitAssign.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/exp/Exp.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"
#include "boomerang/visitor/stmtvisitor/StmtVisitor.h"
#include "boomerang/visitor/stmtexpvisitor/StmtExpVisitor.h"
#include "boomerang/visitor/stmtmodifier/StmtModifier.h"
#include "boomerang/visitor/stmtmodifier/StmtPartModifier.h"
#include "boomerang/type/type/Type.h"
#include "boomerang/util/Log.h"


ImplicitAssign::ImplicitAssign(SharedExp _lhs)
    : Assignment(_lhs)
{
    m_kind = StmtType::ImpAssign;
}


ImplicitAssign::ImplicitAssign(SharedType ty, SharedExp _lhs)
    : Assignment(ty, _lhs)
{
    m_kind = StmtType::ImpAssign;
}


ImplicitAssign::ImplicitAssign(const ImplicitAssign& other)
    : Assignment(other.m_type ? other.m_type->clone() : nullptr, other.m_lhs->clone())
{
    m_kind = StmtType::ImpAssign;
}


Statement *ImplicitAssign::clone() const
{
    return new ImplicitAssign(m_type, m_lhs);
}


bool ImplicitAssign::accept(StmtVisitor *visitor) const
{
    return visitor->visit(this);
}


void ImplicitAssign::printCompact(QTextStream& os, bool html) const
{
    os << "*" << m_type << "* ";

    if (m_lhs) {
        m_lhs->print(os, html);
    }

    os << " := -";
}


bool ImplicitAssign::search(const Exp& pattern, SharedExp& result) const
{
    return m_lhs->search(pattern, result);
}


bool ImplicitAssign::searchAll(const Exp& pattern, std::list<SharedExp>& result) const
{
    return m_lhs->searchAll(pattern, result);
}


bool ImplicitAssign::searchAndReplace(const Exp& pattern, SharedExp replace, bool cc)
{
    Q_UNUSED(cc);
    bool change;
    m_lhs = m_lhs->searchReplaceAll(pattern, replace, change);
    return change;
}


bool ImplicitAssign::accept(StmtExpVisitor *v)
{
    bool visitChildren = true;
    bool ret = v->visit(this, visitChildren);

    if (!visitChildren) {
        return ret;
    }

    if (ret && m_lhs) {
        ret = m_lhs->acceptVisitor(v->ev);
    }

    return ret;
}


bool ImplicitAssign::accept(StmtModifier *v)
{
    bool visitChildren = true;
    v->visit(this, visitChildren);

    if (v->m_mod) {
        v->m_mod->clearModified();

        if (visitChildren) {
            m_lhs = m_lhs->acceptModifier(v->m_mod);
        }

        if (v->m_mod->isModified()) {
            LOG_VERBOSE("ImplicitAssign changed: now %1", this);
        }
    }

    return true;
}


bool ImplicitAssign::accept(StmtPartModifier *v)
{
    bool visitChildren;
    v->visit(this, visitChildren);
    v->mod->clearModified();

    if (visitChildren && m_lhs->isMemOf()) {
        m_lhs->setSubExp1(m_lhs->getSubExp1()->acceptModifier(v->mod));
    }

    if (v->mod->isModified()) {
        LOG_VERBOSE("ImplicitAssign changed: now %1", this);
    }

    return true;
}
