#include "TypeVal.h"

#include "boomerang/db/Visitor.h"
#include "boomerang/core/Boomerang.h"
#include "boomerang/util/Util.h"


TypeVal::TypeVal(SharedType ty)
    : Terminal(opTypeVal)
    , val(ty)
{
}


TypeVal::~TypeVal()
{
    // delete val;
}


SharedExp TypeVal::clone() const
{
    return std::make_shared<TypeVal>(val->clone());
}


bool TypeVal::operator==(const Exp& o) const
{
    if (((TypeVal&)o).m_oper == opWild) {
        return true;
    }

    if (((TypeVal&)o).m_oper != opTypeVal) {
        return false;
    }

    return *val == *((TypeVal&)o).val;
}


bool TypeVal::operator<(const Exp& o) const
{
    if (opTypeVal < o.getOper()) {
        return true;
    }

    if (opTypeVal > o.getOper()) {
        return false;
    }

    return *val < *((TypeVal&)o).val;
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
    if (val) {
        os << "<" << val->getCtype() << ">";
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
    LOG_MSG("%1%2 %3", QString(ind, ' '), operToString(m_oper), val->getCtype());
}


