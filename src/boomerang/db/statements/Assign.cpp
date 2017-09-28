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


#include "boomerang/core/Boomerang.h"
#include "boomerang/util/Log.h"

#include "boomerang/db/Visitor.h"

#include "boomerang/codegen/ICodeGenerator.h"


Assign::Assign(SharedExp lhs, SharedExp r, SharedExp guard)
    : Assignment(lhs)
    , m_rhs(r)
    , m_guard(guard)
{
    m_kind = STMT_ASSIGN;
}


Assign::Assign(SharedType ty, SharedExp lhs, SharedExp r, SharedExp guard)
    : Assignment(ty, lhs)
    , m_rhs(r)
    , m_guard(guard)
{
    m_kind = STMT_ASSIGN;
}


Assign::Assign(Assign& o)
    : Assignment(m_lhs->clone())
{
    m_kind  = STMT_ASSIGN;
    m_rhs   = o.m_rhs->clone();
    m_type  = nullptr;
    m_guard = nullptr;

    if (o.m_type) {
        m_type = o.m_type->clone();
    }

    if (o.m_guard) {
        m_guard = o.m_guard->clone();
    }
}


Statement *Assign::clone() const
{
    Assign *a = new Assign(m_type == nullptr ? nullptr : m_type->clone(),
                           m_lhs->clone(), m_rhs->clone(),
                           m_guard == nullptr ? nullptr : m_guard->clone());

    // Statement members
    a->m_parent = m_parent;
    a->m_proc   = m_proc;
    a->m_number = m_number;
    return a;
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


bool Assign::search(const Exp& search, SharedExp& result) const
{
    if (m_lhs->search(search, result)) {
        return true;
    }

    return m_rhs->search(search, result);
}


bool Assign::searchAll(const Exp& search, std::list<SharedExp>& result) const
{
    bool res;

    std::list<SharedExp>           leftResult;
    std::list<SharedExp>::iterator it;
    res = m_lhs->searchAll(search, leftResult);
    // Ugh: searchAll clears the list!
    res |= m_rhs->searchAll(search, result);

    for (it = leftResult.begin(); it != leftResult.end(); it++) {
        result.push_back(*it);
    }

    return res;
}


bool Assign::searchAndReplace(const Exp& search, SharedExp replace, bool /*cc*/)
{
    bool chl, chr, chg = false;

    m_lhs = m_lhs->searchReplaceAll(search, replace, chl);
    m_rhs = m_rhs->searchReplaceAll(search, replace, chr);

    if (m_guard) {
        m_guard = m_guard->searchReplaceAll(search, replace, chg);
    }

    return chl | chr | chg;
}


void Assign::generateCode(ICodeGenerator *gen, BasicBlock *pbb)
{
    Q_UNUSED(pbb);
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


void Assign::genConstraints(LocationSet& cons)
{
    Assignment::genConstraints(cons);     // Gen constraint for the LHS
    SharedExp con = m_rhs->genConstraints(Unary::get(opTypeOf, RefExp::get(m_lhs->clone(), this)));

    if (con) {
        cons.insert(con);
    }
}


bool Assign::accept(StmtExpVisitor *v)
{
    bool override;
    bool ret = v->visit(this, override);

    if (override) {
        // The visitor has overridden this functionality.  This is needed for example in UsedLocFinder, where the
        // lhs of
        // an assignment is not used (but if it's m[blah], then blah is used)
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
    bool recur;

    v->visit(this, recur);
    v->m_mod->clearMod();

    if (recur) {
        m_lhs = m_lhs->accept(v->m_mod);
    }

    if (recur) {
        m_rhs = m_rhs->accept(v->m_mod);
    }

    if (v->m_mod->isMod()) {
        LOG_VERBOSE("Assignment changed: now %1", this);
    }

    return true;
}


bool Assign::accept(StmtPartModifier *v)
{
    bool recur;

    v->visit(this, recur);
    v->mod->clearMod();

    if (recur && m_lhs->isMemOf()) {
        m_lhs->setSubExp1(m_lhs->getSubExp1()->accept(v->mod));
    }

    if (recur) {
        m_rhs = m_rhs->accept(v->mod);
    }

    if (v->mod->isMod()) {
        LOG_VERBOSE("Assignment changed: now %1", this);
    }

    return true;
}


void Assign::dfaTypeAnalysis(bool& ch)
{
    SharedType tr = m_rhs->ascendType();

    m_type = m_type->meetWith(tr, ch, true); // Note: bHighestPtr is set true, since the lhs could have a greater type
    // (more possibilities) than the rhs. Example: pEmployee = pManager.
    m_rhs->descendType(m_type, ch, this);    // This will effect rhs = rhs MEET lhs
    Assignment::dfaTypeAnalysis(ch);         // Handle the LHS wrt m[] operands
}


Assign::Assign()
    : Assignment(nullptr)
    , m_rhs(nullptr)
    , m_guard(nullptr)
{
}
