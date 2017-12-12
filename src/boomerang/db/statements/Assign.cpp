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


#include "boomerang/codegen/ICodeGenerator.h"
#include "boomerang/core/Boomerang.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Unary.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/visitor/ExpVisitor.h"
#include "boomerang/db/visitor/ExpModifier.h"
#include "boomerang/db/visitor/StmtExpVisitor.h"
#include "boomerang/db/visitor/StmtVisitor.h"
#include "boomerang/db/visitor/StmtModifier.h"
#include "boomerang/db/visitor/StmtPartModifier.h"
#include "boomerang/type/type/Type.h"
#include "boomerang/util/LocationSet.h"
#include "boomerang/util/Log.h"


Assign::Assign(SharedExp lhs, SharedExp r, SharedExp guard)
    : Assignment(lhs)
    , m_rhs(r)
    , m_guard(guard)
{
    m_kind = StmtType::Assign;
}


Assign::Assign(SharedType ty, SharedExp lhs, SharedExp r, SharedExp guard)
    : Assignment(ty, lhs)
    , m_rhs(r)
    , m_guard(guard)
{
    m_kind = StmtType::Assign;
}


Assign::Assign(const Assign& other)
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
    Assign *asgn = new Assign(m_type == nullptr ? nullptr : m_type->clone(),
                           m_lhs->clone(), m_rhs->clone(),
                           m_guard == nullptr ? nullptr : m_guard->clone());

    // Statement members
    asgn->m_parent = m_parent;
    asgn->m_proc   = m_proc;
    asgn->m_number = m_number;
    return asgn;
}


bool Assign::accept(StmtVisitor *visitor)
{
    return visitor->visit(this);
}


void Assign::simplify()
{
    // simplify arithmetic of assignment
    OPER leftop = m_lhs->getOper();

    if (SETTING(noBranchSimplify)) {
        if ((leftop == opZF) || (leftop == opCF) || (leftop == opOF) || (leftop == opNF)) {
            return;
        }
    }

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
    if (m_guard && (m_guard->isTrue() || (m_guard->isIntConst() && (m_guard->access<Const>()->getInt() == 1)))) {
        m_guard = nullptr;     // No longer a guarded assignment
    }

    if (m_lhs->getOper() == opMemOf) {
        m_lhs->setSubExp1(m_lhs->getSubExp1()->simplifyArith());
    }
}


void Assign::fixSuccessor()
{
    m_lhs = m_lhs->fixSuccessor();
    m_rhs = m_rhs->fixSuccessor();
}


void Assign::simplifyAddr()
{
    m_lhs = m_lhs->simplifyAddr();
    m_rhs = m_rhs->simplifyAddr();
}


void Assign::printCompact(QTextStream& os, bool html) const
{
    os << "*" << m_type << "* ";

    if (m_guard) {
        os << m_guard << " => ";
    }

    if (m_lhs) {
        m_lhs->print(os, html);
    }

    os << " := ";

    if (m_rhs) {
        m_rhs->print(os, html);
    }
}


bool Assign::search(const Exp& pattern, SharedExp& result) const
{
    if (m_lhs->search(pattern, result)) {
        return true;
    }

    return m_rhs->search(pattern, result);
}


bool Assign::searchAll(const Exp& pattern, std::list<SharedExp>& result) const
{
    bool res;

    std::list<SharedExp>           leftResult;
    std::list<SharedExp>::iterator it;
    res = m_lhs->searchAll(pattern, leftResult);
    // Ugh: searchAll clears the list!
    res |= m_rhs->searchAll(pattern, result);

    for (it = leftResult.begin(); it != leftResult.end(); it++) {
        result.push_back(*it);
    }

    return res;
}


bool Assign::searchAndReplace(const Exp& pattern, SharedExp replace, bool /*cc*/)
{
    bool chl, chr, chg = false;

    m_lhs = m_lhs->searchReplaceAll(pattern, replace, chl);
    m_rhs = m_rhs->searchReplaceAll(pattern, replace, chr);

    if (m_guard) {
        m_guard = m_guard->searchReplaceAll(pattern, replace, chg);
    }

    return chl | chr | chg;
}


void Assign::generateCode(ICodeGenerator *gen, const BasicBlock *)
{
    gen->addAssignmentStatement(this);
}


int Assign::getMemDepth() const
{
    return std::max(m_lhs->getMemDepth(), m_rhs->getMemDepth());
}


bool Assign::usesExp(const Exp& e) const
{
    SharedExp where = nullptr;

    return(m_rhs->search(e, where) ||
           ((m_lhs->isMemOf() || m_lhs->isRegOf()) && m_lhs->getSubExp1()->search(e, where)));
}


bool Assign::accept(StmtExpVisitor *v)
{
    bool visitChildren = true;
    bool ret = v->visit(this, visitChildren);

    if (!visitChildren) {
        // The visitor has overridden this functionality.  This is needed for example in UsedLocFinder, where the
        // lhs of an assignment is not used (but if it's m[blah], then blah is used)
        return ret;
    }

    if (ret && m_lhs) {
        ret = m_lhs->accept(v->ev);
    }

    if (ret && m_rhs) {
        ret = m_rhs->accept(v->ev);
    }

    return ret;
}


bool Assign::accept(StmtModifier *v)
{
    bool visitChildren;

    v->visit(this, visitChildren);

    if (v->m_mod) {
        v->m_mod->clearMod();

        if (visitChildren) {
            m_lhs = m_lhs->accept(v->m_mod);
        }

        if (visitChildren) {
            m_rhs = m_rhs->accept(v->m_mod);
        }

        if (v->m_mod->isMod()) {
            LOG_VERBOSE("Assignment changed: now %1", this);
        }
    }

    return true;
}


bool Assign::accept(StmtPartModifier *v)
{
    bool visitChildren = true;
    v->visit(this, visitChildren);
    v->mod->clearMod();

    if (visitChildren && m_lhs->isMemOf()) {
        m_lhs->setSubExp1(m_lhs->getSubExp1()->accept(v->mod));
    }

    if (visitChildren) {
        m_rhs = m_rhs->accept(v->mod);
    }

    if (v->mod->isMod()) {
        LOG_VERBOSE("Assignment changed: now %1", this);
    }

    return true;
}


Assign::Assign()
    : Assignment(nullptr)
    , m_rhs(nullptr)
    , m_guard(nullptr)
{
}
