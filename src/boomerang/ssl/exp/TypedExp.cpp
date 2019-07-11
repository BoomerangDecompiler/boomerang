#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "TypedExp.h"

#include "boomerang/ssl/type/Type.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"


TypedExp::TypedExp(SharedExp e1)
    : Unary(opTypedExp, e1)
    , m_type(nullptr)
{
}


TypedExp::TypedExp(SharedType ty, SharedExp e1)
    : Unary(opTypedExp, e1)
    , m_type(ty)
{
}


TypedExp::TypedExp(const TypedExp &o)
    : Unary(o)
{
    m_type = o.m_type->clone();
}


std::shared_ptr<TypedExp> TypedExp::get(SharedExp exp)
{
    return std::make_shared<TypedExp>(exp);
}


std::shared_ptr<TypedExp> TypedExp::get(SharedType ty, SharedExp exp)
{
    return std::make_shared<TypedExp>(ty, exp);
}


SharedExp TypedExp::clone() const
{
    return std::make_shared<TypedExp>(m_type, m_subExp1->clone());
}


bool TypedExp::operator==(const Exp &o) const
{
    if (static_cast<const TypedExp &>(o).m_oper == opWild) {
        return true;
    }

    if (static_cast<const TypedExp &>(o).m_oper != opTypedExp) {
        return false;
    }

    // This is the strict type version
    if (*m_type != *static_cast<const TypedExp &>(o).m_type) {
        return false;
    }

    return *this->getSubExp1() == *o.getSubExp1();
}


bool TypedExp::operator<(const Exp &o) const // Type sensitive
{
    if (m_oper < o.getOper()) {
        return true;
    }

    if (m_oper > o.getOper()) {
        return false;
    }

    if (*m_type < *static_cast<const TypedExp &>(o).m_type) {
        return true;
    }

    if (*static_cast<const TypedExp &>(o).m_type < *m_type) {
        return false;
    }

    return *m_subExp1 < *o.getSubExp1();
}


bool TypedExp::equalNoSubscript(const Exp &o) const
{
    const Exp *other = &o;

    if (o.getOper() == opSubscript) {
        other = o.getSubExp1().get();
    }

    if (other->getOper() == opWild) {
        return true;
    }

    if (other->getOper() != opTypedExp) {
        return false;
    }

    // This is the strict type version
    if (*m_type != *static_cast<const TypedExp *>(other)->m_type) {
        return false;
    }

    return getSubExp1()->equalNoSubscript(*other->getSubExp1());
}


bool TypedExp::acceptVisitor(ExpVisitor *v)
{
    bool visitChildren = true;
    if (!v->preVisit(shared_from_base<TypedExp>(), visitChildren)) {
        return false;
    }

    if (visitChildren) {
        if (!getSubExp1()->acceptVisitor(v)) {
            return false;
        }
    }

    return v->postVisit(shared_from_base<TypedExp>());
}


SharedType TypedExp::ascendType()
{
    return m_type;
}


bool TypedExp::descendType(SharedType)
{
    return false;
}


SharedExp TypedExp::acceptPreModifier(ExpModifier *mod, bool &visitChildren)
{
    return mod->preModify(access<TypedExp>(), visitChildren);
}


SharedExp TypedExp::acceptPostModifier(ExpModifier *mod)
{
    return mod->postModify(access<TypedExp>());
}
