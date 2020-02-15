#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Unary.h"

#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/CompoundType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"


Unary::Unary(OPER op, SharedExp e1)
    : Exp(op)
    , m_subExp1(e1)
{
    assert(m_subExp1);
}


Unary::Unary(const Unary &o)
    : Exp(o.m_oper)
{
    m_subExp1 = o.m_subExp1->clone();
    assert(m_subExp1);
}


Unary::~Unary()
{
}


SharedExp Unary::get(OPER op, SharedExp e1)
{
    return std::make_shared<Unary>(op, e1);
}


void Unary::setSubExp1(SharedExp e)
{
    m_subExp1 = e;
    assert(m_subExp1);
}


SharedExp Unary::getSubExp1()
{
    assert(m_subExp1);
    return m_subExp1;
}


SharedConstExp Unary::getSubExp1() const
{
    assert(m_subExp1);
    return m_subExp1;
}


SharedExp &Unary::refSubExp1()
{
    assert(m_subExp1);
    return m_subExp1;
}


SharedExp Unary::clone() const
{
    assert(m_subExp1);
    return std::make_shared<Unary>(m_oper, m_subExp1->clone());
}


bool Unary::operator==(const Exp &o) const
{
    if (o.getOper() == opWild) {
        return true;
    }

    if ((o.getOper() == opWildRegOf) && (m_oper == opRegOf)) {
        return true;
    }

    if ((o.getOper() == opWildMemOf) && (m_oper == opMemOf)) {
        return true;
    }

    if ((o.getOper() == opWildAddrOf) && (m_oper == opAddrOf)) {
        return true;
    }

    if (m_oper != o.getOper()) {
        return false;
    }

    return *m_subExp1 == *o.getSubExp1();
}


bool Unary::operator<(const Exp &o) const
{
    if (m_oper != static_cast<const Unary &>(o).m_oper) {
        return m_oper < static_cast<const Unary &>(o).m_oper;
    }

    return *m_subExp1 < *static_cast<const Unary &>(o).getSubExp1();
}


bool Unary::equalNoSubscript(const Exp &o) const
{
    const Exp *other = &o;

    if (o.getOper() == opSubscript) {
        other = o.getSubExp1().get();
    }

    if (other->getOper() == opWild) {
        return true;
    }

    if ((other->getOper() == opWildRegOf) && (m_oper == opRegOf)) {
        return true;
    }

    if ((other->getOper() == opWildMemOf) && (m_oper == opMemOf)) {
        return true;
    }

    if ((other->getOper() == opWildAddrOf) && (m_oper == opAddrOf)) {
        return true;
    }

    if (m_oper != other->getOper()) {
        return false;
    }

    return m_subExp1->equalNoSubscript(*other->getSubExp1());
}


void Unary::doSearchChildren(const Exp &pattern, std::list<SharedExp *> &li, bool once)
{
    doSearch(pattern, m_subExp1, li, once);
}


bool Unary::acceptVisitor(ExpVisitor *v)
{
    bool visitChildren = true;
    if (!v->preVisit(shared_from_base<Unary>(), visitChildren)) {
        return false;
    }

    if (visitChildren) {
        if (!m_subExp1->acceptVisitor(v)) {
            return false;
        }
    }

    return v->postVisit(shared_from_base<Unary>());
}


SharedType Unary::ascendType()
{
    SharedType ta = m_subExp1->ascendType();

    switch (m_oper) {
    case opMemOf:

        if (ta->resolvesToPointer()) {
            return ta->as<PointerType>()->getPointsTo();
        }
        else {
            return VoidType::get(); // NOT SURE! Really should be bottom
        }

        break;

    case opAddrOf: return PointerType::get(ta); break;

    default: break;
    }

    return VoidType::get();
}


bool Unary::descendType(SharedType newType)
{
    std::vector<SharedExp> matches;
    bool changed = false;

    switch (m_oper) {
    case opMemOf: {
        std::shared_ptr<Binary> as_bin = std::dynamic_pointer_cast<Binary>(access<Exp, 1>());

        // Check for m[x*K1 + K2]: array with base K2 and stride K1
        if (as_bin && (as_bin->getOper() == opPlus) &&
            (as_bin->getSubExp1()->getOper() == opMult) && as_bin->getSubExp2()->isIntConst() &&
            as_bin->getSubExp1()->getSubExp2()->isIntConst()) {
            SharedExp leftOfPlus = as_bin->getSubExp1();
            // We would expect the stride to be the same size as the base type
            size_t stride = leftOfPlus->access<Const, 2>()->getInt();

            if (stride * 8 != newType->getSize()) {
                LOG_VERBOSE("Type WARNING: apparent array reference at %1 has stride %2 bits, but "
                            "parent type %3 has size %4",
                            shared_from_this(), stride * 8, newType->getCtype(),
                            newType->getSize());
            }

            // The index is integer type
            SharedExp x = leftOfPlus->getSubExp1();
            changed |= x->descendType(IntegerType::get(newType->getSize(), Sign::Unknown));

            // K2 is of type <array of parentType>
            std::shared_ptr<Const> K2 = access<Const, 1, 2>();
            Prog *prog                = this->access<Location>()->getProc()->getProg();
            changed |= K2->descendType(prog->makeArrayType(K2->getAddr(), newType));
        }
        else {
            changed |= m_subExp1->descendType(PointerType::get(newType));
        }

        break;
    }

    case opAddrOf:
        if (newType->resolvesToPointer()) {
            changed |= m_subExp1->descendType(newType->as<PointerType>()->getPointsTo());
        }

        break;

    case opGlobal: {
        Prog *_prog   = access<Location>()->getProc()->getProg();
        QString name  = access<Const, 1>()->getStr();
        SharedType ty = _prog->getGlobalType(name);

        if (ty) {
            ty = ty->meetWith(newType, changed);

            if (changed) {
                _prog->setGlobalType(name, ty);
            }
        }

        break;
    }

    default: break;
    }

    return changed;
}


SharedExp Unary::acceptPreModifier(ExpModifier *mod, bool &visitChildren)
{
    return mod->preModify(access<Unary>(), visitChildren);
}


SharedExp Unary::acceptChildModifier(ExpModifier *mod)
{
    m_subExp1 = m_subExp1->acceptModifier(mod);
    return shared_from_this();
}


SharedExp Unary::acceptPostModifier(ExpModifier *mod)
{
    return mod->postModify(access<Unary>());
}
