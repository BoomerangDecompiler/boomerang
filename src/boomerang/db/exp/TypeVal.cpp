#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "TypeVal.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/util/Util.h"
#include "boomerang/type/type/Type.h"
#include "boomerang/db/visitor/ExpVisitor.h"
#include "boomerang/db/visitor/ExpModifier.h"


TypeVal::TypeVal(SharedType ty)
    : Terminal(opTypeVal)
    , m_type(ty)
{
}


TypeVal::~TypeVal()
{
    // delete val;
}


SharedExp TypeVal::clone() const
{
    return std::make_shared<TypeVal>(m_type->clone());
}


bool TypeVal::operator==(const Exp& o) const
{
    if (((const TypeVal&)o).m_oper == opWild) {
        return true;
    }

    if (((const TypeVal&)o).m_oper != opTypeVal) {
        return false;
    }

    return *m_type == *((const TypeVal&)o).m_type;
}


bool TypeVal::operator<(const Exp& o) const
{
    if (opTypeVal < o.getOper()) {
        return true;
    }

    if (opTypeVal > o.getOper()) {
        return false;
    }

    return *m_type < *((const TypeVal&)o).m_type;
}


bool TypeVal::operator*=(const Exp& o) const
{
    const Exp *other = &o;

    if (o.getOper() == opSubscript) {
        other = o.getSubExp1().get();
    }

    return *this == *other;
}


void TypeVal::print(QTextStream& os, bool) const
{
    if (m_type) {
        os << "<" << m_type->getCtype() << ">";
    }
    else {
        os << "<nullptr>";
    }
}


bool TypeVal::accept(ExpVisitor *v)
{
    return v->visit(shared_from_base<TypeVal>());
}


SharedExp TypeVal::accept(ExpModifier *v)
{
    auto ret         = v->preVisit(shared_from_base<TypeVal>());
    auto typeval_ret = std::dynamic_pointer_cast<TypeVal>(ret);

    assert(typeval_ret);
    return v->postVisit(typeval_ret);
}


void TypeVal::printx(int ind) const
{
    LOG_MSG("%1%2 %3", QString(ind, ' '), operToString(m_oper), m_type->getCtype());
}
