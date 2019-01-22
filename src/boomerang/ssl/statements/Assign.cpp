#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Assign.h"

#include "boomerang/core/Project.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ifc/ICodeGenerator.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Unary.h"
#include "boomerang/ssl/type/Type.h"
#include "boomerang/util/LocationSet.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"
#include "boomerang/visitor/stmtexpvisitor/StmtExpVisitor.h"
#include "boomerang/visitor/stmtmodifier/StmtModifier.h"
#include "boomerang/visitor/stmtmodifier/StmtPartModifier.h"
#include "boomerang/visitor/stmtvisitor/StmtVisitor.h"


Assign::Assign(SharedExp lhs, SharedExp rhs, SharedExp guard)
    : Assignment(lhs)
    , m_rhs(rhs)
    , m_guard(guard)
{
    m_kind = StmtType::Assign;
}


Assign::Assign(SharedType ty, SharedExp lhs, SharedExp rhs, SharedExp guard)
    : Assignment(ty, lhs)
    , m_rhs(rhs)
    , m_guard(guard)
{
    m_kind = StmtType::Assign;
}


Assign::Assign(const Assign &other)
    : Assignment(m_lhs->clone())
{
    m_kind  = StmtType::Assign;
    m_rhs   = other.m_rhs->clone();
    m_type  = nullptr;
    m_guard = nullptr;

    if (other.m_type) {
        m_type = other.m_type->clone();
    }

    if (other.m_guard) {
        m_guard = other.m_guard->clone();
    }
}


Statement *Assign::clone() const
{
    Assign *asgn = new Assign(m_type == nullptr ? nullptr : m_type->clone(), m_lhs->clone(),
                              m_rhs->clone(), m_guard == nullptr ? nullptr : m_guard->clone());

    // Statement members
    asgn->m_bb     = m_bb;
    asgn->m_proc   = m_proc;
    asgn->m_number = m_number;
    return asgn;
}


bool Assign::accept(StmtVisitor *visitor) const
{
    return visitor->visit(this);
}


void Assign::simplify()
{
    m_lhs = m_lhs->simplifyArith();
    m_rhs = m_rhs->simplifyArith();

    if (m_guard) {
        m_guard = m_guard->simplifyArith();
    }

    // simplify the resultant expression
    m_lhs = m_lhs->simplify();
    m_rhs = m_rhs->simplify();

    if (m_guard) {
        m_guard = m_guard->simplify();
    }

    // Perhaps the guard can go away
    if (m_guard && (m_guard->isTrue() ||
                    (m_guard->isIntConst() && (m_guard->access<Const>()->getInt() == 1)))) {
        m_guard = nullptr; // No longer a guarded assignment
    }

    if (m_lhs->isMemOf()) {
        m_lhs->setSubExp1(m_lhs->getSubExp1()->simplifyArith());
    }
}


void Assign::simplifyAddr()
{
    m_lhs = m_lhs->simplifyAddr();
    m_rhs = m_rhs->simplifyAddr();
}


void Assign::printCompact(OStream &os) const
{
    os << "*" << m_type << "* ";

    if (m_guard) {
        os << m_guard << " => ";
    }

    if (m_lhs) {
        m_lhs->print(os);
    }

    os << " := ";

    if (m_rhs) {
        m_rhs->print(os);
    }
}


bool Assign::search(const Exp &pattern, SharedExp &result) const
{
    if (m_lhs->search(pattern, result)) {
        return true;
    }

    return m_rhs->search(pattern, result);
}


bool Assign::searchAll(const Exp &pattern, std::list<SharedExp> &result) const
{
    bool res;

    std::list<SharedExp> leftResult;
    res = m_lhs->searchAll(pattern, leftResult);
    // Ugh: searchAll clears the list!
    res |= m_rhs->searchAll(pattern, result);

    for (SharedExp exp : leftResult) {
        result.push_back(exp);
    }

    return res;
}


bool Assign::searchAndReplace(const Exp &pattern, SharedExp replace, bool /*cc*/)
{
    bool chl = false, chr = false, chg = false;

    m_lhs = m_lhs->searchReplaceAll(pattern, replace, chl);
    m_rhs = m_rhs->searchReplaceAll(pattern, replace, chr);

    if (m_guard) {
        m_guard = m_guard->searchReplaceAll(pattern, replace, chg);
    }

    return chl || chr || chg;
}


bool Assign::accept(StmtExpVisitor *v)
{
    bool visitChildren = true;
    bool ret           = v->visit(this, visitChildren);

    if (!visitChildren) {
        // The visitor has overridden this functionality.  This is needed for example in
        // UsedLocFinder, where the lhs of an assignment is not used (but if it's m[blah], then blah
        // is used)
        return ret;
    }

    if (ret && m_lhs) {
        ret = m_lhs->acceptVisitor(v->ev);
    }

    if (ret && m_rhs) {
        ret = m_rhs->acceptVisitor(v->ev);
    }

    return ret;
}


bool Assign::accept(StmtModifier *v)
{
    bool visitChildren;

    v->visit(this, visitChildren);

    if (v->m_mod) {
        v->m_mod->clearModified();

        if (visitChildren) {
            m_lhs = m_lhs->acceptModifier(v->m_mod);
        }

        if (visitChildren) {
            m_rhs = m_rhs->acceptModifier(v->m_mod);
        }

        if (v->m_mod->isModified()) {
            LOG_VERBOSE2("Assignment changed: now %1", this);
        }
    }

    return true;
}


bool Assign::accept(StmtPartModifier *v)
{
    bool visitChildren = true;
    v->visit(this, visitChildren);
    v->mod->clearModified();

    if (visitChildren && m_lhs->isMemOf()) {
        m_lhs->setSubExp1(m_lhs->getSubExp1()->acceptModifier(v->mod));
    }

    if (visitChildren) {
        m_rhs = m_rhs->acceptModifier(v->mod);
    }

    if (v->mod->isModified()) {
        LOG_VERBOSE2("Assignment changed: now %1", this);
    }

    return true;
}


Assign::Assign()
    : Assignment(nullptr)
    , m_rhs(nullptr)
    , m_guard(nullptr)
{
}
