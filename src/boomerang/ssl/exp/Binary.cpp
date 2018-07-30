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


int tlstrchr(const QString& str, char ch)
{
    static QMap<QChar, QChar> braces { {
                                           '[', ']'
                                       }, {
                                           '{', '}'
                                       }, {
                                           '(', ')'
                                       }
    };
    int i = 0, e = str.length();

    for ( ; i < e; ++i) {
        if (str[i].toLatin1() == ch) {
            return i;
        }

        if (braces.contains(str[i])) {
            QChar end_brace = braces[str[i]];
            ++i; // from next char

            for ( ; i < e; ++i) {
                if (str[i] == end_brace) {
                    break;
                }
            }
        }
    }

    if (i == e) {
        return -1;
    }

    return i;
}


Binary::Binary(OPER op, SharedExp e1, SharedExp e2)
    : Unary(op, e1)
    , subExp2(e2)
{
    assert(subExp1 && subExp2);
}


Binary::Binary(const Binary& o)
    : Unary(o)
{
    subExp2 = o.subExp2->clone();
    assert(subExp1 && subExp2);
}


Binary::~Binary()
{
}


void Binary::setSubExp2(SharedExp e)
{
    subExp2 = e;
    assert(subExp1 && subExp2);
}


SharedExp Binary::getSubExp2()
{
    assert(subExp1 && subExp2);
    return subExp2;
}


SharedExp& Binary::refSubExp2()
{
    assert(subExp1 && subExp2);
    return subExp2;
}


void Binary::commute()
{
    std::swap(subExp1, subExp2);
    assert(subExp1 && subExp2);
}


SharedExp Binary::clone() const
{
    assert(subExp1 && subExp2);
    return std::make_shared<Binary>(m_oper, subExp1->clone(), subExp2->clone());
}


bool Binary::operator==(const Exp& o) const
{
    assert(subExp1 && subExp2);

    if (o.getOper() == opWild) {
        return true;
    }

    if (nullptr == dynamic_cast<const Binary *>(&o)) {
        return false;
    }

    if (m_oper != static_cast<const Binary &>(o).m_oper) {
        return false;
    }

    if (!(*subExp1 == *static_cast<const Binary &>(o).getSubExp1())) {
        return false;
    }

    return *subExp2 == *static_cast<const Binary &>(o).getSubExp2();
}


bool Binary::operator<(const Exp& o) const
{
    assert(subExp1 && subExp2);

    if (m_oper < o.getOper()) {
        return true;
    }

    if (m_oper > o.getOper()) {
        return false;
    }

    if (*subExp1 < *static_cast<const Binary &>(o).getSubExp1()) {
        return true;
    }

    if (*static_cast<const Binary &>(o).getSubExp1() < *subExp1) {
        return false;
    }

    return *subExp2 < *static_cast<const Binary &>(o).getSubExp2();
}


bool Binary::operator*=(const Exp& o) const
{
    assert(subExp1 && subExp2);
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

    if (!(*subExp1 *= *other->getSubExp1())) {
        return false;
    }

    return *subExp2 *= *other->getSubExp2();
}


void Binary::printr(QTextStream& os, bool html) const
{
    assert(subExp1 && subExp2);

    // The "r" is for recursive: the idea is that we don't want parentheses at the outer level, but a subexpression
    // (recursed from a higher level), we want the parens (at least for standard infix operators)
    switch (m_oper)
    {
    case opSize:
    case opList:     // Otherwise, you get (a, (b, (c, d)))
        // There may be others
        // These are the noparen cases
        print(os, html);
        return;

    default:
        break;
    }

    // Normal case: we want the parens
    os << "(";
    this->print(os, html);
    os << ")";
}


void Binary::print(QTextStream& os, bool html) const
{
    assert(subExp1 && subExp2);
    SharedConstExp p1 = getSubExp1();
    SharedConstExp p2 = getSubExp2();

    // Special cases
    switch (m_oper)
    {
    case opSize:
        // This can still be seen after decoding and before type analysis after m[...]
        // *size* is printed after the expression, even though it comes from the first subexpression
        p2->printr(os, html);
        os << "*";
        p1->printr(os, html);
        os << "*";
        return;

    case opFlagCall:
        // The name of the flag function (e.g. ADDFLAGS) should be enough
        std::static_pointer_cast<const Const>(p1)->printNoQuotes(os);
        os << "( ";
        p2->printr(os, html);
        os << " )";
        return;

    case opExpTable:
    case opNameTable:

        if (m_oper == opExpTable) {
            os << "exptable(";
        }
        else {
            os << "nametable(";
        }

        os << p1 << ", " << p2 << ")";
        return;

    case opList:
        // Because "," is the lowest precedence operator, we don't need printr here.
        // Also, same as UQBT, so easier to test
        p1->print(os, html);

        if (!p2->isNil()) {
            os << ", ";
        }

        p2->print(os, html);
        return;

    case opMemberAccess:
        p1->print(os, html);
        os << ".";
        std::static_pointer_cast<const Const>(p2)->printNoQuotes(os);
        return;

    case opArrayIndex:
        p1->print(os, html);
        os << "[";
        p2->print(os, html);
        os << "]";
        return;

    default:
        break;
    }

    // Ordinary infix operators. Emit parens around the binary
    if (p1 == nullptr) {
        os << "<nullptr>";
    }
    else {
        p1->printr(os, html);
    }

    switch (m_oper)
    {
    case opPlus:    os << " + ";    break;
    case opMinus:   os << " - ";    break;
    case opMult:    os << " * ";    break;
    case opMults:   os << " *! ";   break;
    case opDiv:     os << " / ";    break;
    case opDivs:    os << " /! ";   break;
    case opMod:     os << " % ";    break;
    case opMods:    os << " %! ";   break;
    case opFPlus:   os << " +f ";   break;
    case opFMinus:  os << " -f ";   break;
    case opFMult:   os << " *f ";   break;
    case opFDiv:    os << " /f ";   break;
    case opPow:     os << " pow ";  break;     // Raising to power
    case opAnd:     os << " and ";  break;
    case opOr:      os << " or ";   break;
    case opBitAnd:  os << " & ";    break;
    case opBitOr:   os << " | ";    break;
    case opBitXor:  os << " ^ ";    break;
    case opEquals:  os << " = ";    break;
    case opNotEqual:os << " ~= ";   break;
    case opLess:    os << (html ? " &lt; "  : " < ");  break;
    case opGtr:     os << (html ? " &gt; "  : " > ");  break;
    case opLessEq:  os << (html ? " &lt;= " : " <= "); break;
    case opGtrEq:   os << (html ? " &gt;= " : " >= "); break;
    case opLessUns: os << (html ? " &lt;u " : " <u "); break;
    case opGtrUns:  os << (html ? " &gt;u " : " >u "); break;
    case opLessEqUns:os<< (html ? " &lt;u " : " <=u "); break;
    case opGtrEqUns:os << (html ? " &gt;=u " : " >=u "); break;
    case opUpper:   os << " GT "; break;
    case opLower:   os << " LT "; break;
    case opShiftL:  os << (html ? " &lt;&lt; " : " << "); break;
    case opShiftR:  os << (html ? " &gt;&gt; " : " >> "); break;
    case opShiftRA: os << (html ? " &gt;&gt;A " : " >>A "); break;
    case opRotateL: os << " rl "; break;
    case opRotateR: os << " rr "; break;
    case opRotateLC:os << " rlc "; break;
    case opRotateRC:os << " rrc "; break;
    default:
        LOG_FATAL("Invalid operator %1", operToString(m_oper));
    }

    if (p2 == nullptr) {
        os << "<nullptr>";
    }
    else {
        p2->printr(os, html);
    }
}


void Binary::doSearchChildren(const Exp& pattern, std::list<SharedExp *>& li, bool once)
{
    assert(subExp1 && subExp2);
    doSearch(pattern, subExp1, li, once);

    if (once && !li.empty()) {
        return;
    }

    doSearch(pattern, subExp2, li, once);
}


SharedConstExp Binary::getSubExp2() const
{
    assert(subExp1 && subExp2);
    return subExp2;
}


void Binary::printx(int ind) const
{
    assert(subExp1 && subExp2);
    LOG_MSG("%1%2", QString(ind, ' '), operToString(m_oper));
    printChild(subExp1, ind);
    printChild(subExp2, ind);
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

    SharedType ta = subExp1->ascendType();
    SharedType tb = subExp2->ascendType();

    switch (m_oper)
    {
    case opPlus:
        return sigmaSum(ta, tb);

    // Do I need to check here for Array* promotion? I think checking in descendType is enough
    case opMinus:
        return deltaDifference(ta, tb);

    case opMult:
    case opDiv:
        return IntegerType::get(ta->getSize(), Sign::Unsigned);

    case opMults:
    case opDivs:
    case opShiftRA:
        return IntegerType::get(ta->getSize(), Sign::Signed);

    case opBitAnd:
    case opBitOr:
    case opBitXor:
    case opShiftR:
    case opShiftL:
        return IntegerType::get(ta->getSize(), Sign::Unknown);

    case opLess:
    case opGtr:
    case opLessEq:
    case opGtrEq:
    case opLessUns:
    case opGtrUns:
    case opLessEqUns:
    case opGtrEqUns:
        return BooleanType::get();

    case opFMinus:
    case opFPlus:
        return FloatType::get(ta->getSize());

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

void Binary::descendType(SharedType parentType, bool& changed, Statement *s)
{
    if (m_oper == opFlagCall) {
        return;
    }

    SharedType ta = subExp1->ascendType();
    SharedType tb = subExp2->ascendType();
    SharedType nt; // "New" type for certain operators
    // The following is an idea of Mike's that is not yet implemented well. It is designed to handle the situation
    // where the only reference to a local is where its address is taken. In the current implementation, it incorrectly
    // triggers with every ordinary local reference, causing esp to appear used in the final program

    switch (m_oper)
    {
    case opPlus:
        ta = ta->meetWith(sigmaAddend(parentType, tb), changed);
        subExp1->descendType(ta, changed, s);
        tb = tb->meetWith(sigmaAddend(parentType, ta), changed);
        subExp2->descendType(tb, changed, s);
        break;

    case opMinus:
        ta = ta->meetWith(deltaMinuend(parentType, tb), changed);
        subExp1->descendType(ta, changed, s);
        tb = tb->meetWith(deltaSubtrahend(parentType, ta), changed);
        subExp2->descendType(tb, changed, s);
        break;

    case opGtrUns:
    case opLessUns:
    case opGtrEqUns:
    case opLessEqUns:
        nt = IntegerType::get(ta->getSize(), Sign::Unsigned); // Used as unsigned
        ta = ta->meetWith(nt, changed);
        tb = tb->meetWith(nt, changed);
        subExp1->descendType(ta, changed, s);
        subExp2->descendType(tb, changed, s);
        break;

    case opGtr:
    case opLess:
    case opGtrEq:
    case opLessEq:
        nt = IntegerType::get(ta->getSize(), Sign::Signed); // Used as signed
        ta = ta->meetWith(nt, changed);
        tb = tb->meetWith(nt, changed);
        subExp1->descendType(ta, changed, s);
        subExp2->descendType(tb, changed, s);
        break;

    case opBitAnd:
    case opBitOr:
    case opBitXor:
    case opShiftR:
    case opShiftL:
    case opMults:
    case opDivs:
    case opShiftRA:
    case opMult:
    case opDiv:
        {
            Sign signedness;

            switch (m_oper)
            {
            case opBitAnd:
            case opBitOr:
            case opBitXor:
            case opShiftR:
            case opShiftL:
                signedness = Sign::Unknown;
                break;

            case opMults:
            case opDivs:
            case opShiftRA:
                signedness = Sign::Signed;
                break;

            case opMult:
            case opDiv:
                signedness = Sign::Unsigned;
                break;

            default:
                signedness = Sign::Unknown;
                break; // Unknown signedness
            }

            int parentSize = parentType->getSize();
            ta = ta->meetWith(IntegerType::get(parentSize, signedness), changed);
            subExp1->descendType(ta, changed, s);

            if ((m_oper == opShiftL) || (m_oper == opShiftR) || (m_oper == opShiftRA)) {
                // These operators are not symmetric; doesn't force a signedness on the second operand
                // FIXME: should there be a gentle bias twowards unsigned? Generally, you can't shift by negative
                // amounts.
                signedness = Sign::Unknown;
            }

            tb = tb->meetWith(IntegerType::get(parentSize, signedness), changed);
            subExp2->descendType(tb, changed, s);
            break;
        }

    default:
        // Many more cases to implement
        break;
    }
}


bool Binary::acceptVisitor(ExpVisitor *v)
{
    assert(subExp1 && subExp2);

    bool visitChildren = true;
    if (!v->preVisit(shared_from_base<Binary>(), visitChildren)) {
        return false;
    }

    if (visitChildren) {
        if (!subExp1->acceptVisitor(v) || !subExp2->acceptVisitor(v)) {
            return false;
        }
    }

    return v->postVisit(shared_from_base<Binary>());
}


SharedExp Binary::acceptPreModifier(ExpModifier *mod, bool& visitChildren)
{
    return mod->preModify(access<Binary>(), visitChildren);
}


SharedExp Binary::acceptChildModifier(ExpModifier* mod)
{
    subExp1 = subExp1->acceptModifier(mod);
    subExp2 = subExp2->acceptModifier(mod);
    return shared_from_this();
}


SharedExp Binary::acceptPostModifier(ExpModifier *mod)
{
    return mod->postModify(access<Binary>());
}
