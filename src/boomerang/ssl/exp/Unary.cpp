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
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"
#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/util/Log.h"


Unary::Unary(OPER _op, SharedExp e)
    : Exp(_op)
    , subExp1(e)
{
    assert(subExp1);
}


Unary::Unary(const Unary& o)
    : Exp(o.m_oper)
{
    subExp1 = o.subExp1->clone();
    assert(subExp1);
}


Unary::~Unary()
{
}


void Unary::setSubExp1(SharedExp e)
{
    subExp1 = e;
    assert(subExp1);
}


SharedExp Unary::getSubExp1()
{
    assert(subExp1);
    return subExp1;
}


SharedConstExp Unary::getSubExp1() const
{
    assert(subExp1);
    return subExp1;
}


SharedExp& Unary::refSubExp1()
{
    assert(subExp1);
    return subExp1;
}


SharedExp Unary::clone() const
{
    assert(subExp1);
    return std::make_shared<Unary>(m_oper, subExp1->clone());
}


bool Unary::operator==(const Exp& o) const
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

    return *subExp1 == *o.getSubExp1();
}


bool Unary::operator<(const Exp& o) const
{
    if (m_oper != static_cast<const Unary &>(o).m_oper) {
        return m_oper < static_cast<const Unary &>(o).m_oper;
    }

    return *subExp1 < *static_cast<const Unary &>(o).getSubExp1();
}


bool Unary::operator*=(const Exp& o) const
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

    return *subExp1 *= *other->getSubExp1();
}


void Unary::print(QTextStream& os, bool html) const
{
    SharedConstExp p1 = this->getSubExp1();

    switch (m_oper)
    {
    //    //    //    //    //    //    //
    //    x[ subexpression ]    //
    //    //    //    //    //    //    //
    case opRegOf:

        // Make a special case for the very common case of r[intConst]
        if (p1->isIntConst()) {
            os << "r" << std::static_pointer_cast<const Const>(p1)->getInt();
#ifdef DUMP_TYPES
            os << "T(" << std::static_pointer_cast<const Const>(p1)->getType() << ")";
#endif
            break;
        }
        else if (p1->isTemp()) {
            // Just print the temp {   // balance }s
            p1->print(os, html);
            break;
        }
        else {
            os << "r["; // e.g. r[r2]
            // Use print, not printr, because this is effectively the top level again (because the [] act as
            // parentheses)
            p1->print(os, html);
        }

        os << "]";
        break;

    case opMemOf:
    case opAddrOf:
    case opVar:
    case opTypeOf:
    case opKindOf:

        switch (m_oper)
        {
        case opMemOf:
            os << "m[";
            break;

        case opAddrOf:
            os << "a[";
            break;

        case opVar:
            os << "v[";
            break;

        case opTypeOf:
            os << "T[";
            break;

        case opKindOf:
            os << "K[";
            break;

        default:
            break; // Suppress compiler warning
        }

        if (m_oper == opVar) {
            std::static_pointer_cast<const Const>(p1)->printNoQuotes(os);
        }
        // Use print, not printr, because this is effectively the top level again (because the [] act as
        // parentheses)
        else {
            p1->print(os, html);
        }

        os << "]";
#ifdef DUMP_TYPES
        os << "T(" << std::static_pointer_cast<const Const>(p1)->getType() << ")";
#endif
        break;

    //    //    //    //    //    //    //
    //      Unary operators    //
    //    //    //    //    //    //    //

    case opNot:
    case opLNot:
    case opNeg:
    case opFNeg:

        if (m_oper == opNot) {
            os << "~";
        }
        else if (m_oper == opLNot) {
            os << "L~";
        }
        else if (m_oper == opFNeg) {
            os << "~f ";
        }
        else {
            os << "-";
        }

        p1->printr(os, html);
        return;

    case opSignExt:
        p1->printr(os, html);
        os << "!"; // Operator after expression
        return;

    //    //    //    //    //    //    //    //
    //    Function-like operators //
    //    //    //    //    //    //    //    //

    case opSQRTs:
    case opSQRTd:
    case opSQRTq:
    case opSqrt:
    case opSin:
    case opCos:
    case opTan:
    case opArcTan:
    case opLog2:
    case opLog10:
    case opLoge:
    case opPow:
    case opMachFtr:
    case opSuccessor:

        switch (m_oper)
        {
        case opSQRTs:
            os << "SQRTs(";
            break;

        case opSQRTd:
            os << "SQRTd(";
            break;

        case opSQRTq:
            os << "SQRTq(";
            break;

        case opSqrt:
            os << "sqrt(";
            break;

        case opSin:
            os << "sin(";
            break;

        case opCos:
            os << "cos(";
            break;

        case opTan:
            os << "tan(";
            break;

        case opArcTan:
            os << "arctan(";
            break;

        case opLog2:
            os << "log2(";
            break;

        case opLog10:
            os << "log10(";
            break;

        case opLoge:
            os << "loge(";
            break;

        case opExecute:
            os << "execute(";
            break;

        case opMachFtr:
            os << "machine(";
            break;

        case opSuccessor:
            os << "succ(";
            break;

        default:
            break; // For warning
        }

        p1->printr(os, html);
        os << ")";
        return;

    //    Misc    //
    case opSgnEx: // Different because the operator appears last
        p1->printr(os, html);
        os << "! ";
        return;

    case opTemp:

        if (p1->getOper() == opWildStrConst) {
            assert(p1->isTerminal());
            os << "t[";
            std::static_pointer_cast<const Terminal>(p1)->print(os);
            os << "]";
            return;
        }

    // fallthrough

    // Temp: just print the string, no quotes
    case opGlobal:
    case opLocal:
    case opParam:
        // Print a more concise form than param["foo"] (just foo)
        std::static_pointer_cast<const Const>(p1)->printNoQuotes(os);
        return;

    case opInitValueOf:
        p1->printr(os, html);
        os << "'";
        return;

    case opPhi:
        os << "phi(";
        p1->print(os, html);
        os << ")";
        return;

    case opFtrunc:
        os << "ftrunc(";
        p1->print(os, html);
        os << ")";
        return;

    case opFabs:
        os << "fabs(";
        p1->print(os, html);
        os << ")";
        return;

    default:
        LOG_FATAL("Invalid operator %1", operToString(m_oper));
    }
}


void Unary::doSearchChildren(const Exp& pattern, std::list<SharedExp *>& li, bool once)
{
    if (m_oper != opInitValueOf) { // don't search child
        doSearch(pattern, subExp1, li, once);
    }
}


void Unary::printx(int ind) const
{
    LOG_MSG("%1%2", QString(ind, ' '), operToString(m_oper));
    printChild(subExp1, ind);
}


bool Unary::acceptVisitor(ExpVisitor *v)
{
    bool visitChildren = true;
    if (!v->preVisit(shared_from_base<Unary>(), visitChildren)) {
        return false;
    }

    if (visitChildren) {
        if (!subExp1->acceptVisitor(v)) {
            return false;
        }
    }

    return v->postVisit(shared_from_base<Unary>());
}


SharedType Unary::ascendType()
{
    SharedType ta = subExp1->ascendType();

    switch (m_oper)
    {
    case opMemOf:

        if (ta->resolvesToPointer()) {
            return ta->as<PointerType>()->getPointsTo();
        }
        else {
            return VoidType::get(); // NOT SURE! Really should be bottom
        }

        break;

    case opAddrOf:
        return PointerType::get(ta);

        break;

    default:
        break;
    }

    return VoidType::get();
}



// match m[l1{} + K] pattern
bool match_l1_K(SharedExp in, std::vector<SharedExp>& matches)
{
    if (!in->isMemOf()) {
        return false;
    }

    auto as_bin = std::dynamic_pointer_cast<Binary>(in->getSubExp1());

    if (!as_bin || (as_bin->getOper() != opPlus)) {
        return false;
    }

    if (!as_bin->access<Exp, 2>()->isIntConst()) {
        return false;
    }

    if (!as_bin->access<Exp, 1>()->isSubscript()) {
        return false;
    }

    auto refexp = std::static_pointer_cast<RefExp>(as_bin->getSubExp1());

    if (!refexp->getSubExp1()->isLocation()) {
        return false;
    }

    matches.push_back(refexp);
    matches.push_back(as_bin->getSubExp2());
    return true;
}


void Unary::descendType(SharedType parentType, bool& changed, Statement *s)
{
    UserProc *owner_proc = s->getProc();
    auto     sig         = owner_proc != nullptr ? owner_proc->getSignature() : nullptr;
    Prog     *prog       = owner_proc->getProg();

    std::vector<SharedExp> matches;

    switch (m_oper)
    {
    case opMemOf:
        {
            auto as_bin = std::dynamic_pointer_cast<Binary>(subExp1);

            // Check for m[x*K1 + K2]: array with base K2 and stride K1
            if (as_bin && (as_bin->getOper() == opPlus) && (as_bin->getSubExp1()->getOper() == opMult) &&
                as_bin->getSubExp2()->isIntConst() &&
                as_bin->getSubExp1()->getSubExp2()->isIntConst()) {
                SharedExp leftOfPlus = as_bin->getSubExp1();
                // We would expect the stride to be the same size as the base type
                size_t stride = leftOfPlus->access<Const, 2>()->getInt();

                if (stride * 8 != parentType->getSize()) {
                    LOG_WARN("Type WARNING: apparent array reference at %1 has stride %2 bits, but parent type %3 has size %4",
                             shared_from_this(), stride * 8, parentType->getCtype(), parentType->getSize());
                }

                // The index is integer type
                SharedExp x = leftOfPlus->getSubExp1();
                x->descendType(IntegerType::get(parentType->getSize(), Sign::Unknown), changed, s);
                // K2 is of type <array of parentType>
                auto    constK2 = subExp1->access<Const, 2>();
                Address intK2   = Address(constK2->getInt());         // TODO: use getAddr ?
                constK2->descendType(prog->makeArrayType(intK2, parentType), changed, s);
            }
            else if (match_l1_K(shared_from_this(), matches)) {
                // m[l1 + K]
                auto       l1     = std::static_pointer_cast<Location>(matches[0]->access<Location, 1>());
                SharedType l1Type = l1->ascendType();
                int        K      = matches[1]->access<Const>()->getInt();

                if (l1Type->resolvesToPointer()) {
                    // This is a struct reference m[ptr + K]; ptr points to the struct and K is an offset into it
                    // First find out if we already have struct information
                    SharedType st(l1Type->as<PointerType>()->getPointsTo());

                    if (st->resolvesToCompound()) {
                        auto ct = st->as<CompoundType>();

                        if (ct->isGeneric()) {
                            ct->updateGenericMember(K, parentType, changed);
                        }
                        else {
                            // would like to force a simplify here; I guess it will happen soon enough
                        }
                    }
                    else {
                        // Need to create a generic stuct with a least one member at offset K
                        auto ct = CompoundType::get(true);
                        ct->updateGenericMember(K, parentType, changed);
                    }
                }
                else {
                    // K must be the pointer, so this is a global array
                    // FIXME: finish this case
                }

                // FIXME: many other cases
            }
            else {
                subExp1->descendType(PointerType::get(parentType), changed, s);
            }

            break;
        }

    case opAddrOf:

        if (parentType->resolvesToPointer()) {
            subExp1->descendType(parentType->as<PointerType>()->getPointsTo(), changed, s);
        }

        break;

    case opGlobal:
        {
            Prog       *_prog = s->getProc()->getProg();
            QString    name   = subExp1->access<Const>()->getStr();
            SharedType ty     = _prog->getGlobalType(name);

            if (ty) {
                ty = ty->meetWith(parentType, changed);

                if (changed) {
                    _prog->setGlobalType(name, ty);
                }
            }

            break;
        }

    default:
        break;
    }
}


SharedExp Unary::acceptPreModifier(ExpModifier *mod, bool& visitChildren)
{
    return mod->preModify(access<Unary>(), visitChildren);
}


SharedExp Unary::acceptChildModifier(ExpModifier *mod)
{
    subExp1 = subExp1->acceptModifier(mod);
    return shared_from_this();
}


SharedExp Unary::acceptPostModifier(ExpModifier *mod)
{
    return mod->postModify(access<Unary>());
}
