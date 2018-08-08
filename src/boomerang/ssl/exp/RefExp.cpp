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
    return RefExp::get(subExp1->clone(), m_def);
}


bool RefExp::operator==(const Exp& o) const
{
    if (o.getOper() == opWild) {
        return true;
    }

    if (o.getOper() != opSubscript) {
        return false;
    }

    if (!(*subExp1 == *o.getSubExp1())) {
        return false;
    }

    // Allow a def of (Statement*)-1 as a wild card
    if (m_def == STMT_WILD) {
        return true;
    }

    assert(dynamic_cast<const RefExp *>(&o) != nullptr);

    const RefExp& otherRef = static_cast<const RefExp &>(o);

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


bool RefExp::operator<(const Exp& o) const
{
    if (opSubscript < o.getOper()) {
        return true;
    }

    if (opSubscript > o.getOper()) {
        return false;
    }

    if (*subExp1 < *static_cast<const Unary &>(o).getSubExp1()) {
        return true;
    }

    if (*static_cast<const Unary &>(o).getSubExp1() < *subExp1) {
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


bool RefExp::operator*=(const Exp& o) const
{
    const Exp *other = &o;

    if (o.getOper() == opSubscript) {
        other = o.getSubExp1().get();
    }

    return *subExp1 *= *other;
}



bool RefExp::acceptVisitor(ExpVisitor *v)
{
    bool visitChildren = true;
    if (!v->preVisit(shared_from_base<RefExp>(), visitChildren)) {
        return false;
    }

    if (visitChildren) {
        if (!subExp1->acceptVisitor(v)) {
            return false;
        }
    }

    return v->postVisit(shared_from_base<RefExp>());
}


void RefExp::printx(int ind) const
{
    LOG_VERBOSE("%1%2", QString(ind, ' '), operToString(m_oper));
    LOG_VERBOSE("{");

    if (m_def == nullptr) {
        LOG_VERBOSE("nullptr");
    }
    else {
        LOG_VERBOSE("%1=%2", HostAddress(m_def).toString(), m_def->getNumber());
    }

    LOG_VERBOSE("}");
    printChild(subExp1, ind);
}


bool RefExp::isImplicitDef() const
{
    return m_def == nullptr || m_def->getKind() == StmtType::ImpAssign;
}


void RefExp::print(OStream& os, bool html) const
{
    if (subExp1) {
        subExp1->print(os, html);
    }
    else {
        os << "<nullptr>";
    }

    if (html) {
        os << "<sub>";
    }
    else {
        os << "{";
    }

    if (m_def == STMT_WILD) {
        os << "WILD";
    }
    else if (m_def) {
        if (html) {
            os << "<a href=\"#stmt" << m_def->getNumber() << "\">";
        }

        os << m_def->getNumber();

        if (html) {
            os << "</a>";
        }
    }
    else {
        os << "-"; // So you can tell the difference with {0}
    }

    if (html) {
        os << "</sub>";
    }
    else {
        os << "}";
    }
}


SharedExp RefExp::addSubscript(Statement* _def)
{
    m_def = _def;
    return shared_from_this();
}


void RefExp::setDef(Statement* _def)
{
//         assert(_def != nullptr);
    m_def = _def;
}


SharedType RefExp::ascendType()
{
    // Constants and subscripted locations are at the leaves
    // of the expression tree. Just return their stored types.
    if (m_def == nullptr) {
        LOG_WARN("Null reference in '%1'", this->prints());
        return VoidType::get();
    }

    return m_def->getTypeFor(subExp1);
}


void RefExp::descendType(SharedType parentType, bool& changed, Statement *s)
{
    assert(getSubExp1());

    if (m_def == nullptr) {
        LOG_ERROR("Cannot descendType of expression '%1' since it does not have a defining statement!", getSubExp1());
        changed = false;
        return;
    }

    SharedType newType = m_def->meetWithFor(parentType, subExp1, changed);
    // In case subExp1 is a m[...]
    subExp1->descendType(newType, changed, s);
}


SharedExp RefExp::acceptPreModifier(ExpModifier *mod, bool& visitChildren)
{
    return mod->preModify(access<RefExp>(), visitChildren);
}


SharedExp RefExp::acceptPostModifier(ExpModifier* mod)
{
    return mod->postModify(access<RefExp>());
}
