#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Ternary.h"

#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"


Ternary::Ternary(OPER op, SharedExp e1, SharedExp e2, SharedExp e3)
    : Binary(op, e1, e2)
{
    m_subExp3 = e3;
    assert(m_subExp1 && m_subExp2 && m_subExp3);
}


Ternary::Ternary(const Ternary &o)
    : Binary(o)
{
    m_subExp3 = o.m_subExp3->clone();
    assert(m_subExp1 && m_subExp2 && m_subExp3);
}


Ternary::~Ternary()
{
}


std::shared_ptr<Ternary> Ternary::get(OPER op, SharedExp e1, SharedExp e2, SharedExp e3)
{
    return std::make_shared<Ternary>(op, e1, e2, e3);
}


void Ternary::setSubExp3(SharedExp e)
{
    m_subExp3 = e;
    assert(m_subExp1 && m_subExp2 && m_subExp3);
}


SharedExp Ternary::getSubExp3()
{
    assert(m_subExp1 && m_subExp2 && m_subExp3);
    return m_subExp3;
}


SharedConstExp Ternary::getSubExp3() const
{
    assert(m_subExp1 && m_subExp2 && m_subExp3);
    return m_subExp3;
}


SharedExp &Ternary::refSubExp3()
{
    assert(m_subExp1 && m_subExp2 && m_subExp3);
    return m_subExp3;
}


SharedExp Ternary::clone() const
{
    assert(m_subExp1 && m_subExp2 && m_subExp3);
    return Ternary::get(m_oper, m_subExp1->clone(), m_subExp2->clone(), m_subExp3->clone());
}


bool Ternary::operator==(const Exp &o) const
{
    if (o.getOper() == opWild) {
        return true;
    }
    else if (o.getArity() != 3) {
        return false;
    }

    const Ternary &otherTern = static_cast<const Ternary &>(o);

    return m_oper == otherTern.m_oper && *m_subExp1 == *otherTern.getSubExp1() &&
           *m_subExp2 == *otherTern.getSubExp2() && *m_subExp3 == *otherTern.getSubExp3();
}


bool Ternary::operator<(const Exp &o) const
{
    if (m_oper != o.getOper()) {
        return m_oper < o.getOper();
    }

    const Ternary &otherTern = static_cast<const Ternary &>(o);

    if (*m_subExp1 != *otherTern.getSubExp1()) {
        return *m_subExp1 < *otherTern.getSubExp1();
    }
    else if (*m_subExp2 != *otherTern.getSubExp2()) {
        return *m_subExp2 < *otherTern.getSubExp2();
    }

    return *m_subExp3 < *otherTern.getSubExp3();
}


bool Ternary::equalNoSubscript(const Exp &o) const
{
    const Exp *other = &o;

    if (o.getOper() == opSubscript) {
        other = o.getSubExp1().get();
    }

    if (other->getOper() == opWild) {
        return true;
    }

    if (m_oper != other->getOper()) {
        return false;
    }

    if (!m_subExp1->equalNoSubscript(*other->getSubExp1())) {
        return false;
    }

    if (!m_subExp2->equalNoSubscript(*other->getSubExp2())) {
        return false;
    }

    return m_subExp3->equalNoSubscript(*other->getSubExp3());
}


void Ternary::doSearchChildren(const Exp &pattern, std::list<SharedExp *> &li, bool once)
{
    doSearch(pattern, m_subExp1, li, once);

    if (once && !li.empty()) {
        return;
    }

    doSearch(pattern, m_subExp2, li, once);

    if (once && !li.empty()) {
        return;
    }

    doSearch(pattern, m_subExp3, li, once);
}


bool Ternary::acceptVisitor(ExpVisitor *v)
{
    bool visitChildren = true;
    if (!v->preVisit(shared_from_base<Ternary>(), visitChildren)) {
        return false;
    }

    if (visitChildren) {
        if (!m_subExp1->acceptVisitor(v) || !m_subExp2->acceptVisitor(v) ||
            !m_subExp3->acceptVisitor(v)) {
            return false;
        }
    }

    return v->postVisit(shared_from_base<Ternary>());
}


SharedType Ternary::ascendType()
{
    switch (m_oper) {
    case opFsize: return FloatType::get(m_subExp2->access<Const>()->getInt());
    case opZfill:
    case opSgnEx: {
        const int toSize = m_subExp2->access<Const>()->getInt();
        return Type::newIntegerLikeType(toSize, m_oper == opZfill ? Sign::Unsigned : Sign::Signed);
    }

    default: break;
    }

    return VoidType::get();
}


bool Ternary::descendType(SharedType newType)
{
    switch (m_oper) {
    case opFsize: return m_subExp3->descendType(FloatType::get(access<Const, 1>()->getInt()));
    case opZfill:
    case opSgnEx: {
        const int fromSize = access<Const, 1>()->getInt();
        const Sign sign    = m_oper == opZfill ? Sign::Unsigned : Sign::Signed;
        return m_subExp3->descendType(Type::newIntegerLikeType(fromSize, sign));
    }
    case opTern: {
        bool thisChanged = false;
        thisChanged |= m_subExp2->descendType(newType);
        thisChanged |= m_subExp3->descendType(newType);
        return thisChanged;
    }

    default: return false;
    }
}


SharedExp Ternary::acceptPreModifier(ExpModifier *mod, bool &visitChildren)
{
    return mod->preModify(access<Ternary>(), visitChildren);
}


SharedExp Ternary::acceptChildModifier(ExpModifier *mod)
{
    m_subExp1 = m_subExp1->acceptModifier(mod);
    m_subExp2 = m_subExp2->acceptModifier(mod);
    m_subExp3 = m_subExp3->acceptModifier(mod);
    return shared_from_this();
}


SharedExp Ternary::acceptPostModifier(ExpModifier *mod)
{
    return mod->postModify(access<Ternary>());
}
