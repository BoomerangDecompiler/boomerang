#include "TypedExp.h"

#include "boomerang/db/Visitor.h"
#include "boomerang/core/Boomerang.h"


TypedExp::TypedExp()
    : Unary(opTypedExp)
    , type(nullptr)
{
}


TypedExp::TypedExp(SharedExp e1)
    : Unary(opTypedExp, e1)
    , type(nullptr)
{
}


TypedExp::TypedExp(SharedType ty, SharedExp e1)
    : Unary(opTypedExp, e1)
    , type(ty)
{
}


TypedExp::TypedExp(TypedExp& o)
    : Unary(opTypedExp)
{
    subExp1 = o.subExp1->clone();
    type    = o.type->clone();
}


SharedExp TypedExp::clone() const
{
    return std::make_shared<TypedExp>(type, subExp1->clone());
}


bool TypedExp::operator==(const Exp& o) const
{
    if (((TypedExp&)o).m_oper == opWild) {
        return true;
    }

    if (((TypedExp&)o).m_oper != opTypedExp) {
        return false;
    }

    // This is the strict type version
    if (*type != *((TypedExp&)o).type) {
        return false;
    }

    return *((Unary *)this)->getSubExp1() == *((Unary&)o).getSubExp1();
}


bool TypedExp::operator<<(const Exp& o) const   // Type insensitive
{
    if (m_oper < o.getOper()) {
        return true;
    }

    if (m_oper > o.getOper()) {
        return false;
    }

    return *subExp1 << *((Unary&)o).getSubExp1();
}


bool TypedExp::operator<(const Exp& o) const   // Type sensitive
{
    if (m_oper < o.getOper()) {
        return true;
    }

    if (m_oper > o.getOper()) {
        return false;
    }

    if (*type < *((TypedExp&)o).type) {
        return true;
    }

    if (*((TypedExp&)o).type < *type) {
        return false;
    }

    return *subExp1 < *((Unary&)o).getSubExp1();
}


bool TypedExp::operator*=(const Exp& o) const
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
    if (*type != *((TypedExp *)other)->type) {
        return false;
    }

    return *((Unary *)this)->getSubExp1() *= *other->getSubExp1();
}


void TypedExp::print(QTextStream& os, bool html) const
{
    os << " ";
    type->starPrint(os);
    SharedConstExp p1 = this->getSubExp1();
    p1->print(os, html);
}


void TypedExp::appendDotFile(QTextStream& of)
{
    of << "e_" << HostAddress(this) << " [shape=record,label=\"{";
    of << "opTypedExp\\n" << HostAddress(this) << " | ";
    // Just display the C type for now
    of << type->getCtype() << " | <p1>";
    of << " }\"];\n";
    subExp1->appendDotFile(of);
    of << "e_" << HostAddress(this) << ":p1->e_" << HostAddress(subExp1.get()) << ";\n";
}


SharedExp TypedExp::polySimplify(bool& bMod)
{
    SharedExp res = shared_from_this();

    if (subExp1->getOper() == opRegOf) {
        // type cast on a reg of.. hmm.. let's remove this
        res  = res->getSubExp1();
        bMod = true;
        return res;
    }

    subExp1 = subExp1->simplify();
    return res;
}


bool TypedExp::accept(ExpVisitor *v)
{
    bool override, ret = v->visit(shared_from_base<TypedExp>(), override);

    if (override) {
        return ret;
    }

    if (ret) {
        ret = subExp1->accept(v);
    }

    return ret;
}


SharedExp TypedExp::accept(ExpModifier *v)
{
    bool recur;
    auto ret          = v->preVisit(shared_from_base<TypedExp>(), recur);
    auto typedexp_ret = std::dynamic_pointer_cast<TypedExp>(ret);

    if (recur) {
        subExp1 = subExp1->accept(v);
    }

    assert(typedexp_ret);
    return v->postVisit(typedexp_ret);
}


void TypedExp::printx(int ind) const
{
    LOG_MSG("%1%2 %3", QString(ind, ' '), operToString(m_oper), type->getCtype());
    child(subExp1, ind);
}
