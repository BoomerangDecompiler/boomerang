#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "RefExp.h"

#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"


RefExp::RefExp(SharedExp e, Statement *d)
    : Unary(opSubscript, e)
    , m_def(d)
{
    assert(e);
}


std::shared_ptr<RefExp> RefExp::get(SharedExp e, Statement *def)
{
    return std::make_shared<RefExp>(e, def);
}


SharedExp RefExp::clone() const
{
    return RefExp::get(m_subExp1->clone(), m_def);
}


bool RefExp::operator==(const Exp &o) const
{
    if (o.getOper() == opWild) {
        return true;
    }

    if (o.getOper() != opSubscript) {
        return false;
    }

    if (!(*m_subExp1 == *o.getSubExp1())) {
        return false;
    }

    // Allow a def of (Statement*)-1 as a wild card
    if (m_def == STMT_WILD) {
        return true;
    }

    assert(dynamic_cast<const RefExp *>(&o) != nullptr);

    const RefExp &otherRef = static_cast<const RefExp &>(o);

    // Allow a def of nullptr to match a def of an implicit assignment
    if (otherRef.m_def == STMT_WILD) {
        return true;
    }

    if (!m_def && otherRef.isImplicitDef()) {
        return true;
    }

    if (!otherRef.m_def && m_def && m_def->isImplicit()) {
        return true;
    }

    return m_def == otherRef.m_def;
}


bool RefExp::operator<(const Exp &o) const
{
    if (opSubscript < o.getOper()) {
        return true;
    }

    if (opSubscript > o.getOper()) {
        return false;
    }

    if (*m_subExp1 < *static_cast<const Unary &>(o).getSubExp1()) {
        return true;
    }

    if (*static_cast<const Unary &>(o).getSubExp1() < *m_subExp1) {
        return false;
    }

    // Allow a wildcard def to match any
    if (m_def == STMT_WILD) {
        return false; // Not less (equal)
    }

    if (static_cast<const RefExp &>(o).m_def == STMT_WILD) {
        return false;
    }

    return m_def < static_cast<const RefExp &>(o).m_def;
}


bool RefExp::equalNoSubscript(const Exp &o) const
{
    const Exp *other = &o;

    if (o.getOper() == opSubscript) {
        other = o.getSubExp1().get();
    }

    return m_subExp1->equalNoSubscript(*other);
}


bool RefExp::acceptVisitor(ExpVisitor *v)
{
    bool visitChildren = true;
    if (!v->preVisit(shared_from_base<RefExp>(), visitChildren)) {
        return false;
    }

    if (visitChildren) {
        if (!m_subExp1->acceptVisitor(v)) {
            return false;
        }
    }

    return v->postVisit(shared_from_base<RefExp>());
}


bool RefExp::isImplicitDef() const
{
    return m_def == nullptr || m_def->getKind() == StmtType::ImpAssign;
}


SharedExp RefExp::addSubscript(Statement *def)
{
    m_def = def;
    return shared_from_this();
}


void RefExp::setDef(Statement *def)
{
    m_def = def;
}


SharedType RefExp::ascendType()
{
    // Constants and subscripted locations are at the leaves
    // of the expression tree. Just return their stored types.
    if (m_def == nullptr) {
        LOG_WARN("Null reference in '%1'", shared_from_this());
        return VoidType::get();
    }

    return m_def->getTypeForExp(m_subExp1);
}


bool RefExp::descendType(SharedType newType)
{
    assert(getSubExp1());

    if (m_def == nullptr) {
        LOG_ERROR(
            "Cannot descendType of expression '%1' since it does not have a defining statement!",
            getSubExp1());
        return false;
    }

    bool thisChanged = false;
    newType          = m_def->meetWithFor(newType, m_subExp1, thisChanged);
    // In case subExp1 is a m[...]
    return m_subExp1->descendType(newType);
}


SharedExp RefExp::acceptPreModifier(ExpModifier *mod, bool &visitChildren)
{
    return mod->preModify(access<RefExp>(), visitChildren);
}


SharedExp RefExp::acceptPostModifier(ExpModifier *mod)
{
    return mod->postModify(access<RefExp>());
}
