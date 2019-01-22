#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Binary.h"

#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/type/BooleanType.h"
#include "boomerang/ssl/type/CompoundType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"

#include <QRegularExpression>

#include <numeric>


Binary::Binary(OPER op, SharedExp e1, SharedExp e2)
    : Unary(op, e1)
    , m_subExp2(e2)
{
    assert(m_subExp1 && m_subExp2);
}


Binary::Binary(const Binary &o)
    : Unary(o)
{
    m_subExp2 = o.m_subExp2->clone();
    assert(m_subExp1 && m_subExp2);
}


Binary::~Binary()
{
}


std::shared_ptr<Binary> Binary::get(OPER op, SharedExp e1, SharedExp e2)
{
    return std::make_shared<Binary>(op, e1, e2);
}


void Binary::setSubExp2(SharedExp e)
{
    m_subExp2 = e;
    assert(m_subExp1 && m_subExp2);
}


SharedExp Binary::getSubExp2()
{
    assert(m_subExp1 && m_subExp2);
    return m_subExp2;
}


SharedExp &Binary::refSubExp2()
{
    assert(m_subExp1 && m_subExp2);
    return m_subExp2;
}


void Binary::commute()
{
    std::swap(m_subExp1, m_subExp2);
    assert(m_subExp1 && m_subExp2);
}


SharedExp Binary::clone() const
{
    assert(m_subExp1 && m_subExp2);
    return std::make_shared<Binary>(m_oper, m_subExp1->clone(), m_subExp2->clone());
}


bool Binary::operator==(const Exp &o) const
{
    assert(m_subExp1 && m_subExp2);

    if (o.getOper() == opWild) {
        return true;
    }

    if (nullptr == dynamic_cast<const Binary *>(&o)) {
        return false;
    }

    if (m_oper != static_cast<const Binary &>(o).m_oper) {
        return false;
    }

    if (!(*m_subExp1 == *static_cast<const Binary &>(o).getSubExp1())) {
        return false;
    }

    return *m_subExp2 == *static_cast<const Binary &>(o).getSubExp2();
}


bool Binary::operator<(const Exp &o) const
{
    assert(m_subExp1 && m_subExp2);

    if (m_oper < o.getOper()) {
        return true;
    }

    if (m_oper > o.getOper()) {
        return false;
    }

    if (*m_subExp1 < *static_cast<const Binary &>(o).getSubExp1()) {
        return true;
    }

    if (*static_cast<const Binary &>(o).getSubExp1() < *m_subExp1) {
        return false;
    }

    return *m_subExp2 < *static_cast<const Binary &>(o).getSubExp2();
}


bool Binary::equalNoSubscript(const Exp &o) const
{
    assert(m_subExp1 && m_subExp2);
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

    return m_subExp2->equalNoSubscript(*other->getSubExp2());
}


void Binary::doSearchChildren(const Exp &pattern, std::list<SharedExp *> &li, bool once)
{
    assert(m_subExp1 && m_subExp2);
    doSearch(pattern, m_subExp1, li, once);

    if (once && !li.empty()) {
        return;
    }

    doSearch(pattern, m_subExp2, li, once);
}


SharedConstExp Binary::getSubExp2() const
{
    assert(m_subExp1 && m_subExp2);
    return m_subExp2;
}


// Special operators for handling addition and subtraction in a data flow based type analysis
//                    ta=
//  tb=       alpha*     int      pi
//  beta*     bottom    void*    void*
//  int        void*     int      pi
//  pi         void*     pi       pi
SharedType sigmaSum(SharedType ta, SharedType tb)
{
    if (ta->resolvesToPointer()) {
        if (tb->resolvesToPointer()) {
            bool ch = false;
            return ta->createUnion(tb, ch);
        }

        return PointerType::get(VoidType::get());
    }

    if (ta->resolvesToInteger()) {
        if (tb->resolvesToPointer()) {
            return PointerType::get(VoidType::get());
        }

        return tb->clone();
    }

    if (tb->resolvesToPointer()) {
        return PointerType::get(VoidType::get());
    }

    return ta->clone();
}


//          ta=
// tb=      alpha*  int        pi
// beta*    int     bottom    int
// int      void*   int        pi
// pi       pi      int        pi
SharedType deltaDifference(SharedType ta, SharedType tb)
{
    if (ta->resolvesToPointer()) {
        if (tb->resolvesToPointer()) {
            return IntegerType::get(STD_SIZE, Sign::Unknown);
        }

        if (tb->resolvesToInteger()) {
            return PointerType::get(VoidType::get());
        }

        return tb->clone();
    }

    if (ta->resolvesToInteger()) {
        if (tb->resolvesToPointer()) {
            bool ch = false;
            return ta->createUnion(tb, ch);
        }

        return IntegerType::get(STD_SIZE, Sign::Unknown);
    }

    if (tb->resolvesToPointer()) {
        return IntegerType::get(STD_SIZE, Sign::Unknown);
    }

    return ta->clone();
}


SharedType Binary::ascendType()
{
    if (m_oper == opFlagCall) {
        return VoidType::get();
    }

    SharedType ta = m_subExp1->ascendType();
    SharedType tb = m_subExp2->ascendType();

    switch (m_oper) {
    case opPlus: return sigmaSum(ta, tb);

    // Do I need to check here for Array* promotion? I think checking in descendType is enough
    case opMinus: return deltaDifference(ta, tb);

    case opMult:
    case opDiv: return IntegerType::get(ta->getSize(), Sign::Unsigned);

    case opMults:
    case opDivs:
    case opShRA: return IntegerType::get(ta->getSize(), Sign::Signed);

    case opBitAnd:
    case opBitOr:
    case opBitXor:
    case opShR:
    case opShL: return IntegerType::get(ta->getSize(), Sign::Unknown);

    case opLess:
    case opGtr:
    case opLessEq:
    case opGtrEq:
    case opLessUns:
    case opGtrUns:
    case opLessEqUns:
    case opGtrEqUns: return BooleanType::get();

    case opFMinus:
    case opFPlus: return FloatType::get(ta->getSize());

    default:
        // Many more cases to implement
        return VoidType::get();
    }
}


//                    tc=
//  to=        beta*    int        pi
// alpha*    int        bottom    int
// int        void*    int        pi
// pi        pi        pi        pi
SharedType sigmaAddend(SharedType tc, SharedType to)
{
    if (tc->resolvesToPointer()) {
        if (to->resolvesToPointer()) {
            return IntegerType::get(STD_SIZE, Sign::Unknown);
        }

        if (to->resolvesToInteger()) {
            return PointerType::get(VoidType::get());
        }

        return to->clone();
    }

    if (tc->resolvesToInteger()) {
        if (to->resolvesToPointer()) {
            bool ch = false;
            return tc->createUnion(to, ch);
        }

        return to->clone();
    }

    if (to->resolvesToPointer()) {
        return IntegerType::get(STD_SIZE, Sign::Unknown);
    }

    return tc->clone();
}


//                    tc=
//  tb=        beta*    int        pi
// alpha*    bottom    void*    void*
// int        void*    int        pi
// pi        void*    int        pi
SharedType deltaMinuend(SharedType tc, SharedType tb)
{
    if (tc->resolvesToPointer()) {
        if (tb->resolvesToPointer()) {
            bool ch = false;
            return tc->createUnion(tb, ch);
        }

        return PointerType::get(VoidType::get());
    }

    if (tc->resolvesToInteger()) {
        if (tb->resolvesToPointer()) {
            return PointerType::get(VoidType::get());
        }

        return tc->clone();
    }

    if (tb->resolvesToPointer()) {
        return PointerType::get(VoidType::get());
    }

    return tc->clone();
}


//                    tc=
//  ta=        beta*    int        pi
// alpha*    int        void*    pi
// int        bottom    int        int
// pi        int        pi        pi
SharedType deltaSubtrahend(SharedType tc, SharedType ta)
{
    if (tc->resolvesToPointer()) {
        if (ta->resolvesToPointer()) {
            return IntegerType::get(STD_SIZE, Sign::Unknown);
        }

        if (ta->resolvesToInteger()) {
            bool ch = false;
            return tc->createUnion(ta, ch);
        }

        return IntegerType::get(STD_SIZE, Sign::Unknown);
    }

    if (tc->resolvesToInteger()) {
        if (ta->resolvesToPointer()) {
            return PointerType::get(VoidType::get());
        }
    }

    return ta->clone();
}


bool Binary::descendType(SharedType newType)
{
    if (m_oper == opFlagCall) {
        return false;
    }

    SharedType ta = m_subExp1->ascendType();
    SharedType tb = m_subExp2->ascendType();
    SharedType nt; // "New" type for certain operators
    bool changed = false;

    // The following is an idea of Mike's that is not yet implemented well. It is designed to handle
    // the situation where the only reference to a local is where its address is taken. In the
    // current implementation, it incorrectly triggers with every ordinary local reference, causing
    // esp to appear used in the final program
    switch (m_oper) {
    case opPlus: {
        ta = ta->meetWith(sigmaAddend(newType, tb), changed);
        tb = tb->meetWith(sigmaAddend(newType, ta), changed);
        changed |= m_subExp1->descendType(ta);
        changed |= m_subExp2->descendType(tb);
        break;
    }

    case opMinus:
        ta = ta->meetWith(deltaMinuend(newType, tb), changed);
        tb = tb->meetWith(deltaSubtrahend(newType, ta), changed);
        changed |= m_subExp1->descendType(ta);
        changed |= m_subExp2->descendType(tb);
        break;

    case opGtrUns:
    case opLessUns:
    case opGtrEqUns:
    case opLessEqUns:
        nt = IntegerType::get(ta->getSize(), Sign::Unsigned); // Used as unsigned
        ta = ta->meetWith(nt, changed);
        tb = tb->meetWith(nt, changed);
        changed |= m_subExp1->descendType(ta);
        changed |= m_subExp2->descendType(tb);
        break;

    case opGtr:
    case opLess:
    case opGtrEq:
    case opLessEq:
        nt = IntegerType::get(ta->getSize(), Sign::Signed); // Used as signed
        ta = ta->meetWith(nt, changed);
        tb = tb->meetWith(nt, changed);
        changed |= m_subExp1->descendType(ta);
        changed |= m_subExp2->descendType(tb);
        break;

    case opBitAnd:
    case opBitOr:
    case opBitXor:
    case opShR:
    case opShL:
    case opMults:
    case opDivs:
    case opShRA:
    case opMult:
    case opDiv: {
        Sign signedness;

        switch (m_oper) {
        case opBitAnd:
        case opBitOr:
        case opBitXor:
        case opShR:
        case opShL: signedness = Sign::Unknown; break;

        case opMults:
        case opDivs:
        case opShRA: signedness = Sign::Signed; break;

        case opMult:
        case opDiv: signedness = Sign::Unsigned; break;

        default: signedness = Sign::Unknown; break; // Unknown signedness
        }

        int parentSize = newType->getSize();
        ta             = ta->meetWith(IntegerType::get(parentSize, signedness), changed);
        changed |= m_subExp1->descendType(ta);

        if ((m_oper == opShL) || (m_oper == opShR) || (m_oper == opShRA)) {
            // These operators are not symmetric; doesn't force a signedness on the second operand
            // FIXME: should there be a gentle bias twowards unsigned? Generally, you can't shift by
            // negative amounts.
            signedness = Sign::Unknown;
        }

        tb = tb->meetWith(IntegerType::get(parentSize, signedness), changed);
        changed |= m_subExp2->descendType(tb);
        break;
    }

    default:
        // Many more cases to implement
        break;
    }

    return changed;
}


bool Binary::acceptVisitor(ExpVisitor *v)
{
    assert(m_subExp1 && m_subExp2);

    bool visitChildren = true;
    if (!v->preVisit(shared_from_base<Binary>(), visitChildren)) {
        return false;
    }

    if (visitChildren) {
        if (!m_subExp1->acceptVisitor(v) || !m_subExp2->acceptVisitor(v)) {
            return false;
        }
    }

    return v->postVisit(shared_from_base<Binary>());
}


SharedExp Binary::acceptPreModifier(ExpModifier *mod, bool &visitChildren)
{
    return mod->preModify(access<Binary>(), visitChildren);
}


SharedExp Binary::acceptChildModifier(ExpModifier *mod)
{
    m_subExp1 = m_subExp1->acceptModifier(mod);
    m_subExp2 = m_subExp2->acceptModifier(mod);
    return shared_from_this();
}


SharedExp Binary::acceptPostModifier(ExpModifier *mod)
{
    return mod->postModify(access<Binary>());
}
