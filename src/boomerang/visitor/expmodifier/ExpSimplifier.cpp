#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpSimplifier.h"


#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/Ternary.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/TypedExp.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/util/Log.h"


SharedExp ExpSimplifier::postModify(const std::shared_ptr<Unary>& exp)
{
    bool& changed = m_modified;
    SharedExp res = exp;
    if (exp->getOper() == opNot || exp->getOper() == opLNot) {
        switch (exp->getSubExp1()->getOper())
        {
        case opEquals:
            changed = true;
            exp->getSubExp1()->setOper(opNotEqual);
            return exp->getSubExp1();

        case opNotEqual:
            changed = true;
            exp->getSubExp1()->setOper(opEquals);
            return exp->getSubExp1();

        case opLess:
            res = res->getSubExp1();
            res->setOper(opGtrEq);
            changed = true;
            return res;

        case opLessEq:
            res = res->getSubExp1();
            res->setOper(opGtr);
            changed = true;
            return res;

        case opGtr:
            res = res->getSubExp1();
            res->setOper(opLessEq);
            changed = true;
            return res;

        case opGtrEq:
            res = res->getSubExp1();
            res->setOper(opLess);
            changed = true;
            return res;

        case opLessUns:
            res = res->getSubExp1();
            res->setOper(opGtrEqUns);
            changed = true;
            return res;

        case opLessEqUns:
            res = res->getSubExp1();
            res->setOper(opGtrUns);
            changed = true;
            return res;

        case opGtrUns:
            res = res->getSubExp1();
            res->setOper(opLessEqUns);
            changed = true;
            return res;

        case opGtrEqUns:
            res = res->getSubExp1();
            res->setOper(opLessUns);
            changed = true;
            return res;

        default:
            break;
        }
    }

    switch (exp->getOper())
    {
    case opNeg:
    case opNot:
    case opLNot:
    case opSize:
        {
            OPER subOP = exp->getSubExp1()->getOper();

            if (subOP == opIntConst) {
                // -k, ~k, or !k
                OPER op2 = exp->getOper();
                res = res->getSubExp1();
                int k = std::static_pointer_cast<Const>(res)->getInt();

                switch (op2)
                {
                case opNeg:
                    k = -k;
                    break;

                case opNot:
                    k = ~k;
                    break;

                case opLNot:
                    k = !k;
                    break;

                case opSize: /* No change required */
                    break;

                default:
                    break;
                }

                std::static_pointer_cast<Const>(res)->setInt(k);
                changed = true;
            }
            else if (exp->getOper() == subOP) {
                res  = res->getSubExp1();
                res  = res->getSubExp1();
                changed = true;
                break;
            }
        }
        break;

    case opAddrOf:
        // check for a[m[x]], becomes x
        if (exp->getSubExp1()->getOper() == opMemOf) {
            res  = res->getSubExp1();
            res  = res->getSubExp1();
            changed = true;
            return res;
        }
        break;

    default:
        break;
    }

    return res;
}


SharedExp ExpSimplifier::postModify(const std::shared_ptr<Binary>& exp)
{
    bool& changed = m_modified;
    SharedExp res = exp->shared_from_this();

    OPER opSub1 = exp->getSubExp1()->getOper();
    OPER opSub2 = exp->getSubExp2()->getOper();

    if ((opSub1 == opIntConst) && (opSub2 == opIntConst)) {
        // k1 op k2, where k1 and k2 are integer constants
        int  k1     = std::static_pointer_cast<Const>(exp->getSubExp1())->getInt();
        int  k2     = std::static_pointer_cast<Const>(exp->getSubExp2())->getInt();
        bool change = true;

        switch (exp->getOper())
        {
        case opPlus:
            k1 = k1 + k2;
            break;

        case opMinus:
            k1 = k1 - k2;
            break;

        case opDiv:
            k1 = static_cast<int>(static_cast<unsigned>(k1) / static_cast<unsigned>(k2));
            break;

        case opDivs:
            k1 = k1 / k2;
            break;

        case opMod:
            k1 = static_cast<int>(static_cast<unsigned>(k1) % static_cast<unsigned>(k2));
            break;

        case opMods:
            k1 = k1 % k2;
            break;

        case opMult:
            k1 = static_cast<int>(static_cast<unsigned>(k1) * static_cast<unsigned>(k2));
            break;

        case opMults:
            k1 = k1 * k2;
            break;

        case opShiftL:

            if (k2 >= 32) {
                k1 = 0;
            }
            else {
                k1 = k1 << k2;
            }

            break;

        case opShiftR:
            k1 = k1 >> k2;
            break;

        case opShiftRA:
            k1 = (k1 >> k2) | (((1 << k2) - 1) << (32 - k2));
            break;

        case opBitOr:
            k1 = k1 | k2;
            break;

        case opBitAnd:
            k1 = k1 & k2;
            break;

        case opBitXor:
            k1 = k1 ^ k2;
            break;

        case opEquals:
            k1 = (k1 == k2);
            break;

        case opNotEqual:
            k1 = (k1 != k2);
            break;

        case opLess:
            k1 = (k1 < k2);
            break;

        case opGtr:
            k1 = (k1 > k2);
            break;

        case opLessEq:
            k1 = (k1 <= k2);
            break;

        case opGtrEq:
            k1 = (k1 >= k2);
            break;

        case opLessUns:
            k1 = static_cast<unsigned>(k1) < static_cast<unsigned>(k2);
            break;

        case opGtrUns:
            k1 = static_cast<unsigned>(k1) > static_cast<unsigned>(k2);
            break;

        case opLessEqUns:
            k1 = static_cast<unsigned>(k1) <= static_cast<unsigned>(k2);
            break;

        case opGtrEqUns:
            k1 = static_cast<unsigned>(k1) >= static_cast<unsigned>(k2);
            break;

        default:
            change = false;
        }

        if (change) {
            changed = true;
            return Const::get(k1);
        }
    }

    if (((exp->getOper() == opBitXor) || (exp->getOper() == opMinus)) && (*exp->getSubExp1() == *exp->getSubExp2())) {
        // x ^ x or x - x: result is zero
        res  = Const::get(0);
        changed = true;
        return res;
    }

    if (((exp->getOper() == opBitOr) || (exp->getOper() == opBitAnd)) && (*exp->getSubExp1() == *exp->getSubExp2())) {
        // x | x or x & x: result is x
        res  = exp->getSubExp1();
        changed = true;
        return res;
    }

    if ((exp->getOper() == opEquals) && (*exp->getSubExp1() == *exp->getSubExp2())) {
        // x == x: result is true
        // delete this;
        res  = std::make_shared<Terminal>(opTrue);
        changed = true;
        return res;
    }

    // Might want to commute to put an integer constant on the RHS
    // Later simplifications can rely on this (ADD other ops as necessary)
    if ((opSub1 == opIntConst) && ((exp->getOper() == opPlus) || (exp->getOper() == opMult) || (exp->getOper() == opMults) || (exp->getOper() == opBitOr) || (exp->getOper() == opBitAnd))) {
        exp->commute();
        // Swap opSub1 and opSub2 as well
        std::swap(opSub1, opSub2);
        // This is not counted as a modification
    }

    // Similarly for boolean constants
    if (exp->getSubExp1()->isBoolConst() && !exp->getSubExp2()->isBoolConst() && ((exp->getOper() == opAnd) || (exp->getOper() == opOr))) {
        exp->commute();
        // Swap opSub1 and opSub2 as well
        std::swap(opSub1, opSub2);
        // This is not counted as a modification
    }

    // Similarly for adding stuff to the addresses of globals
    if (exp->getSubExp2()->isAddrOf() && exp->getSubExp2()->getSubExp1()->isSubscript() && exp->getSubExp2()->getSubExp1()->getSubExp1()->isGlobal() && (exp->getOper() == opPlus)) {
        exp->commute();
        // Swap opSub1 and opSub2 as well
        std::swap(opSub1, opSub2);
        // This is not counted as a modification
    }

    // check for (x + a) + b where a and b are constants, becomes x + a+b
    if ((exp->getOper() == opPlus) && (opSub1 == opPlus) && (opSub2 == opIntConst) && (exp->getSubExp1()->getSubExp2()->getOper() == opIntConst)) {
        int n = std::static_pointer_cast<Const>(exp->getSubExp2())->getInt();
        res = res->getSubExp1();
        std::shared_ptr<Const> c_subexp(std::static_pointer_cast<Const>(res->getSubExp2()));
        c_subexp->setInt(c_subexp->getInt() + n);
        changed = true;
        return res;
    }

    // check for (x - a) + b where a and b are constants, becomes x + -a+b
    if ((exp->getOper() == opPlus) && (opSub1 == opMinus) && (opSub2 == opIntConst) && (exp->getSubExp1()->getSubExp2()->getOper() == opIntConst)) {
        int n = std::static_pointer_cast<Const>(exp->getSubExp2())->getInt();
        res = res->getSubExp1();
        res->setOper(opPlus);
        std::shared_ptr<Const> c_subexp(std::static_pointer_cast<Const>(res->getSubExp2()));
        c_subexp->setInt(-c_subexp->getInt() + n);
        changed = true;
        return res;
    }

    // check for (x * k) - x, becomes x * (k-1)
    // same with +
    if (((exp->getOper() == opMinus) || (exp->getOper() == opPlus)) && ((opSub1 == opMults) || (opSub1 == opMult)) && (*exp->getSubExp2() == *exp->getSubExp1()->getSubExp1())) {
        res = res->getSubExp1();
        res->setSubExp2(Binary::get(exp->getOper(), res->getSubExp2(), Const::get(1)));
        changed = true;
        return res;
    }

    // check for x + (x * k), becomes x * (k+1)
    if ((exp->getOper() == opPlus) && ((opSub2 == opMults) || (opSub2 == opMult)) && (*exp->getSubExp1() == *exp->getSubExp2()->getSubExp1())) {
        res = res->getSubExp2();
        res->setSubExp2(Binary::get(opPlus, res->getSubExp2(), Const::get(1)));
        changed = true;
        return res;
    }

    // Turn a + -K into a - K (K is int const > 0)
    // Also a - -K into a + K (K is int const > 0)
    // Does not count as a change
    if (((exp->getOper() == opPlus) || (exp->getOper() == opMinus)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() < 0)) {
        std::static_pointer_cast<Const>(exp->getSubExp2())->setInt(-std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt());
        exp->setOper(exp->getOper() == opPlus ? opMinus : opPlus);
    }

    // Check for exp + 0  or  exp - 0  or  exp | 0
    if (((exp->getOper() == opPlus) || (exp->getOper() == opMinus) || (exp->getOper() == opBitOr)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() == 0)) {
        res  = res->getSubExp1();
        changed = true;
        return res;
    }

    // Check for exp or false
    if ((exp->getOper() == opOr) && exp->getSubExp2()->isFalse()) {
        res  = res->getSubExp1();
        changed = true;
        return res;
    }

    // Check for SharedExp 0  or exp & 0
    if (((exp->getOper() == opMult) || (exp->getOper() == opMults) || (exp->getOper() == opBitAnd)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() == 0)) {
        // delete res;
        res  = Const::get(0);
        changed = true;
        return res;
    }

    // Check for exp and false
    if ((exp->getOper() == opAnd) && exp->getSubExp2()->isFalse()) {
        // delete res;
        res  = Terminal::get(opFalse);
        changed = true;
        return res;
    }

    // Check for SharedExp 1
    if (((exp->getOper() == opMult) || (exp->getOper() == opMults)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() == 1)) {
        res  = res->getSubExp1();
        changed = true;
        return res;
    }

    // Check for SharedExp x / x
    if (((exp->getOper() == opDiv) || (exp->getOper() == opDivs)) && ((opSub1 == opMult) || (opSub1 == opMults)) && (*exp->getSubExp2() == *exp->getSubExp1()->getSubExp2())) {
        res  = res->getSubExp1();
        res  = res->getSubExp1();
        changed = true;
        return res;
    }

    // Check for exp / 1, becomes exp
    if (((exp->getOper() == opDiv) || (exp->getOper() == opDivs)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() == 1)) {
        res  = res->getSubExp1();
        changed = true;
        return res;
    }

    // Check for exp % 1, becomes 0
    if (((exp->getOper() == opMod) || (exp->getOper() == opMods)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() == 1)) {
        res  = Const::get(0);
        changed = true;
        return res;
    }

    // Check for SharedExp x % x, becomes 0
    if (((exp->getOper() == opMod) || (exp->getOper() == opMods)) && ((opSub1 == opMult) || (opSub1 == opMults)) &&
        (*exp->getSubExp2() == *exp->getSubExp1()->getSubExp2())) {
        res  = Const::get(0);
        changed = true;
        return res;
    }

    // Check for exp AND -1 (bitwise AND)
    if ((exp->getOper() == opBitAnd) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() == -1)) {
        res  = res->getSubExp1();
        changed = true;
        return res;
    }

    // Check for exp AND TRUE (logical AND)
    if ((exp->getOper() == opAnd) &&
        // Is the below really needed?
        ((((opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() != 0))) || exp->getSubExp2()->isTrue())) {
        res  = res->getSubExp1();
        changed = true;
        return res;
    }

    // Check for exp OR TRUE (logical OR)
    if ((exp->getOper() == opOr) && ((((opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() != 0))) || exp->getSubExp2()->isTrue())) {
        // delete res;
        res  = Terminal::get(opTrue);
        changed = true;
        return res;
    }

    // Check for [exp] << k where k is a positive integer const

    if ((exp->getOper() == opShiftL) && (opSub2 == opIntConst)) {
        int k = std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt();

        if ((k >= 0) && (k < 32)) {
            res->setOper(opMult);
            std::static_pointer_cast<Const>(exp->getSubExp2())->setInt(1 << k);
            changed = true;
            return res;
        }
    }

    if ((exp->getOper() == opShiftR) && (opSub2 == opIntConst)) {
        int k = std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt();

        if ((k >= 0) && (k < 32)) {
            res->setOper(opDiv);
            std::static_pointer_cast<Const>(exp->getSubExp2())->setInt(1 << k);
            changed = true;
            return res;
        }
    }

    /*
     *  // Check for -x compare y, becomes x compare -y
     *  // doesn't count as a change
     *  if (    isComparison() &&
     *                  opSub1 == opNeg) {
     *          SharedExp e = exp->getSubExp1();
     *          exp->getSubExp1() = e->getSubExp1()->clone();
     *          ;//delete e;
     *          exp->getSubExp2() = Unary::get(opNeg, exp->getSubExp2());
     *  }
     *
     *  // Check for (x + y) compare 0, becomes x compare -y
     *  if (    isComparison() &&
     *                  opSub2 == opIntConst && ((Const*)exp->getSubExp2())->getInt() == 0 &&
     *                  opSub1 == opPlus) {
     *          ;//delete exp->getSubExp2();
     *          Binary *b = (Binary*)exp->getSubExp1();
     *          exp->getSubExp2() = b->exp->getSubExp2();
     *          b->exp->getSubExp2() = 0;
     *          exp->getSubExp1() = b->exp->getSubExp1();
     *          b->exp->getSubExp1() = 0;
     *          ;//delete b;
     *          exp->getSubExp2() = Unary::get(opNeg, exp->getSubExp2());
     *          changed = true;
     *          return res;
     *  }
     */

    // Check for (x == y) == 1, becomes x == y
    if ((exp->getOper() == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() == 1) && (opSub1 == opEquals)) {
        exp->setSubExp1(exp->access<Exp, 1, 1>());
        exp->setSubExp2(exp->access<Exp, 1, 2>());
        changed    = true;
        return res;
    }

    // Check for x + -y == 0, becomes x == y
    if ((exp->getOper() == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() == 0) && (opSub1 == opPlus) && (exp->getSubExp1()->getSubExp2()->getOper() == opIntConst)) {
        auto b = std::static_pointer_cast<Binary>(exp->getSubExp1());
        int  n = std::static_pointer_cast<Const>(b->getSubExp2())->getInt();

        if (n < 0) {
            exp->refSubExp2() = b->getSubExp2();
            std::static_pointer_cast<Const>(exp->getSubExp2())->setInt(-std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt());
            exp->refSubExp1() = b->getSubExp1();
            changed    = true;
            return res;
        }
    }

    // Check for (x == y) == 0, becomes x != y
    if ((exp->getOper() == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() == 0) && (opSub1 == opEquals)) {
        auto b = std::static_pointer_cast<Binary>(exp->getSubExp1());
        exp->refSubExp1() = b->getSubExp1();
        exp->refSubExp2() = b->getSubExp2();
        changed    = true;
        res->setOper(opNotEqual);
        return res;
    }

    // Check for (x == y) != 1, becomes x != y
    if ((exp->getOper() == opNotEqual) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() == 1) && (opSub1 == opEquals)) {
        auto b = std::static_pointer_cast<Binary>(exp->getSubExp1());
        exp->refSubExp2() = b->getSubExp2();
        exp->refSubExp1() = b->getSubExp1();
        changed    = true;
        res->setOper(opNotEqual);
        return res;
    }

    // Check for (x == y) != 0, becomes x == y
    if ((exp->getOper() == opNotEqual) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() == 0) && (opSub1 == opEquals)) {
        res  = res->getSubExp1();
        changed = true;
        return res;
    }

    // Check for (0 - x) != 0, becomes x != 0
    if ((exp->getOper() == opNotEqual) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() == 0) && (opSub1 == opMinus) &&
        exp->getSubExp1()->getSubExp1()->isIntConst() && (std::static_pointer_cast<const Const>(exp->getSubExp1()->getSubExp1())->getInt() == 0)) {
        res  = Binary::get(opNotEqual, exp->getSubExp1()->getSubExp2()->clone(), exp->getSubExp2()->clone());
        changed = true;
        return res;
    }

    // Check for (x > y) == 0, becomes x <= y
    if ((exp->getOper() == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() == 0) && (opSub1 == opGtr)) {
        auto b = std::static_pointer_cast<Binary>(exp->getSubExp1());
        exp->refSubExp2() = b->getSubExp2();
        exp->refSubExp1() = b->getSubExp1();
        changed    = true;
        res->setOper(opLessEq);
        return res;
    }

    // Check for (x >u y) == 0, becomes x <=u y
    if ((exp->getOper() == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt() == 0) && (opSub1 == opGtrUns)) {
        auto b = std::static_pointer_cast<Binary>(exp->getSubExp1());
        exp->refSubExp2() = b->getSubExp2();
        exp->refSubExp1() = b->getSubExp1();
        changed    = true;
        res->setOper(opLessEqUns);
        return res;
    }

    auto b1 = std::dynamic_pointer_cast<Binary>(exp->getSubExp1());
    auto b2 = std::dynamic_pointer_cast<Binary>(exp->getSubExp2());

    // Check for (x <= y) || (x == y), becomes x <= y
    if ((exp->getOper() == opOr) && (opSub2 == opEquals) &&
        ((opSub1 == opGtrEq) || (opSub1 == opLessEq) || (opSub1 == opGtrEqUns) || (opSub1 == opLessEqUns)) &&
        (((*b1->getSubExp1() == *b2->getSubExp1()) && (*b1->getSubExp2() == *b2->getSubExp2())) ||
         ((*b1->getSubExp1() == *b2->getSubExp2()) && (*b1->getSubExp2() == *b2->getSubExp1())))) {
        res  = res->getSubExp1();
        changed = true;
        return res;
    }

    // For (a || b) or (a && b) recurse on a and b
    if ((exp->getOper() == opOr) || (exp->getOper() == opAnd)) {
        exp->refSubExp1() = exp->getSubExp1()->acceptModifier(this);
        exp->refSubExp2() = exp->getSubExp2()->acceptModifier(this);
        return res;
    }

    // check for (x & x), becomes x
    if ((exp->getOper() == opBitAnd) && (*exp->getSubExp1() == *exp->getSubExp2())) {
        res  = res->getSubExp1();
        changed = true;
        return res;
    }

    // check for a + a*n, becomes a*(n+1) where n is an int
    if ((exp->getOper() == opPlus) && (opSub2 == opMult) && (*exp->getSubExp1() == *exp->getSubExp2()->getSubExp1()) &&
        (exp->getSubExp2()->getSubExp2()->getOper() == opIntConst)) {
        res = res->getSubExp2();
        res->access<Const, 2>()->setInt(res->access<Const, 2>()->getInt() + 1);
        changed = true;
        return res;
    }

    // check for a*n*m, becomes a*(n*m) where n and m are ints
    if ((exp->getOper() == opMult) && (opSub1 == opMult) && (opSub2 == opIntConst) && (exp->getSubExp1()->getSubExp2()->getOper() == opIntConst)) {
        int m = std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt();
        res = res->getSubExp1();
        res->access<Const, 2>()->setInt(res->access<Const, 2>()->getInt() * m);
        changed = true;
        return res;
    }

    // check for !(a == b) becomes a != b
    if ((exp->getOper() == opLNot) && (opSub1 == opEquals)) {
        res = res->getSubExp1();
        res->setOper(opNotEqual);
        changed = true;
        return res;
    }

    // check for !(a != b) becomes a == b
    if ((exp->getOper() == opLNot) && (opSub1 == opNotEqual)) {
        res = res->getSubExp1();
        res->setOper(opEquals);
        changed = true;
        return res;
    }

    // FIXME: suspect this was only needed for ADHOC TA
    // check for exp + n where exp is a pointer to a compound type
    // becomes &m[exp].m + r where m is the member at offset n and r is n - the offset to member m
    SharedConstType ty = nullptr; // Type of exp->getSubExp1()

    if (exp->getSubExp1()->isSubscript()) {
        const Statement *def = std::static_pointer_cast<RefExp>(exp->getSubExp1())->getDef();

        if (def) {
            ty = def->getTypeFor(exp->getSubExp1()->getSubExp1());
        }
    }

    if ((exp->getOper() == opPlus) && ty && ty->resolvesToPointer() && ty->as<PointerType>()->getPointsTo()->resolvesToCompound() && (opSub2 == opIntConst)) {
        unsigned n = std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt();
        std::shared_ptr<CompoundType> c = ty->as<PointerType>()->getPointsTo()->as<CompoundType>();
        res = Exp::convertFromOffsetToCompound(exp->getSubExp1(), c, n);

        if (res) {
            LOG_VERBOSE("(trans1) replacing %1 with %2", exp->shared_from_this(), res);
            changed = true;
            return res;
        }
    }

    if ((exp->getOper() == opFMinus) && (exp->getSubExp1()->getOper() == opFltConst) && (std::static_pointer_cast<const Const>(exp->getSubExp1())->getFlt() == 0.0)) {
        res  = Unary::get(opFNeg, exp->getSubExp2());
        changed = true;
        return res;
    }

    if (((exp->getOper() == opPlus) || (exp->getOper() == opMinus)) && ((exp->getSubExp1()->getOper() == opMults) || (exp->getSubExp1()->getOper() == opMult)) &&
        (exp->getSubExp2()->getOper() == opIntConst) && (exp->getSubExp1()->getSubExp2()->getOper() == opIntConst)) {
        int n1 = std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt();
        int n2 = exp->getSubExp1()->access<Const, 2>()->getInt();

        if (n1 == n2) {
            res = Binary::get(exp->getSubExp1()->getOper(), Binary::get(exp->getOper(), exp->getSubExp1()->getSubExp1()->clone(), Const::get(1)),
                              Const::get(n1));
            changed = true;
            return res;
        }
    }

    if (((exp->getOper() == opPlus) || (exp->getOper() == opMinus)) && (exp->getSubExp1()->getOper() == opPlus) && (exp->getSubExp2()->getOper() == opIntConst) &&
        ((exp->getSubExp1()->getSubExp2()->getOper() == opMults) || (exp->getSubExp1()->getSubExp2()->getOper() == opMult)) &&
        (exp->getSubExp1()->access<Exp, 2, 2>()->getOper() == opIntConst)) {
        int n1 = std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt();
        int n2 = exp->getSubExp1()->access<Const, 2, 2>()->getInt();

        if (n1 == n2) {
            res = Binary::get(opPlus, exp->getSubExp1()->getSubExp1(),
                              Binary::get(exp->getSubExp1()->getSubExp2()->getOper(),
                                          Binary::get(exp->getOper(), exp->getSubExp1()->access<Exp, 2, 1>()->clone(), Const::get(1)),
                                          Const::get(n1)));
            changed = true;
            return res;
        }
    }

    // check for ((x * a) + (y * b)) / c where a, b and c are all integers and a and b divide evenly by c
    // becomes: (x * a/c) + (y * b/c)
    if ((exp->getOper() == opDiv) && (exp->getSubExp1()->getOper() == opPlus) && (exp->getSubExp2()->getOper() == opIntConst) &&
        (exp->getSubExp1()->getSubExp1()->getOper() == opMult) && (exp->getSubExp1()->getSubExp2()->getOper() == opMult) &&
        (exp->getSubExp1()->access<Exp, 1, 2>()->getOper() == opIntConst) &&
        (exp->getSubExp1()->access<Exp, 2, 2>()->getOper() == opIntConst)) {
        int a = exp->getSubExp1()->access<Const, 1, 2>()->getInt();
        int b = exp->getSubExp1()->access<Const, 2, 2>()->getInt();
        int c = std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt();

        if (((a % c) == 0) && ((b % c) == 0)) {
            res = Binary::get(opPlus, Binary::get(opMult, exp->getSubExp1()->getSubExp1()->getSubExp1(), Const::get(a / c)),
                              Binary::get(opMult, exp->getSubExp1()->access<Exp, 2, 1>(), Const::get(b / c)));
            changed = true;
            return res;
        }
    }

    // check for ((x * a) + (y * b)) % c where a, b and c are all integers
    // becomes: (y * b) % c if a divides evenly by c
    // becomes: (x * a) % c if b divides evenly by c
    // becomes: 0            if both a and b divide evenly by c
    if ((exp->getOper() == opMod) && (exp->getSubExp1()->getOper() == opPlus) && (exp->getSubExp2()->getOper() == opIntConst) &&
        (exp->getSubExp1()->getSubExp1()->getOper() == opMult) && (exp->getSubExp1()->getSubExp2()->getOper() == opMult) &&
        (exp->getSubExp1()->getSubExp1()->getSubExp2()->getOper() == opIntConst) &&
        (exp->getSubExp1()->getSubExp2()->getSubExp2()->getOper() == opIntConst)) {
        int a = exp->getSubExp1()->access<Const, 1, 2>()->getInt();
        int b = exp->getSubExp1()->access<Const, 2, 2>()->getInt();
        int c = std::static_pointer_cast<const Const>(exp->getSubExp2())->getInt();

        if (((a % c) == 0) && ((b % c) == 0)) {
            res  = Const::get(0);
            changed = true;
            return res;
        }

        if ((a % c) == 0) {
            res  = Binary::get(opMod, exp->getSubExp1()->getSubExp2()->clone(), Const::get(c));
            changed = true;
            return res;
        }

        if ((b % c) == 0) {
            res  = Binary::get(opMod, exp->getSubExp1()->getSubExp1()->clone(), Const::get(c));
            changed = true;
            return res;
        }
    }

    // Check for 0 - (0 <u exp1) & exp2 => exp2
    if ((exp->getOper() == opBitAnd) && (opSub1 == opMinus)) {
        SharedExp leftOfMinus = exp->getSubExp1()->getSubExp1();

        if (leftOfMinus->isIntConst() && (std::static_pointer_cast<const Const>(leftOfMinus)->getInt() == 0)) {
            SharedExp rightOfMinus = exp->getSubExp1()->getSubExp2();

            if (rightOfMinus->getOper() == opLessUns) {
                SharedExp leftOfLess = rightOfMinus->getSubExp1();

                if (leftOfLess->isIntConst() && (std::static_pointer_cast<const Const>(leftOfLess)->getInt() == 0)) {
                    res  = exp->getSubExp2();
                    changed = true;
                    return res;
                }
            }
        }
    }

    // Replace opSize(n, loc) with loc and set the type if needed
    if ((exp->getOper() == opSize) && exp->getSubExp2()->isLocation()) {
        res  = res->getSubExp2();
        changed = true;
        return res;
    }

    return res;
}


SharedExp ExpSimplifier::postModify(const std::shared_ptr<Ternary>& exp)
{
    bool& changed = m_modified;

    // p ? 1 : 0 -> p
    if (exp->getOper() == opTern &&
        exp->getSubExp2()->isIntConst() &&
        exp->getSubExp3()->isIntConst()) {
            const int val2 = exp->access<Const, 2>()->getInt();
            const int val3 = exp->access<Const, 3>()->getInt();

            if (val2 == 1 && val3 == 0) {
                changed = true;
                return exp->getSubExp1();
            }
            else if (val2 == 0 && val3 == 1) {
                changed = true;
                return Unary::get(opLNot, exp->getSubExp1());
            }
    }

    // Const ? x : y
    if (exp->getOper() == opTern &&
        exp->getSubExp1()->isIntConst()) {
            const int val = exp->access<Const, 1>()->getInt();
            if (val != 1 && val != 0) {
                LOG_VERBOSE("Treating constant value %1 as true in Ternary '%2'", val, exp);
            }

            changed = true;
            return (val != 0) ? exp->getSubExp2() : exp->getSubExp3();
    }

    /// sign-extend constant value
    if ((exp->getOper() == opSgnEx || exp->getOper() == opZfill) &&
        exp->getSubExp3()->isIntConst()) {
            changed = true;
            return exp->getSubExp3();
    }

    if (exp->getOper() == opFsize &&
        exp->getSubExp3()->getOper() == opItof &&
        *exp->getSubExp1() == *exp->access<Exp, 3, 2>() &&
        *exp->getSubExp2() == *exp->access<Exp, 3, 1>()) {
            changed = true;
            return exp->getSubExp3();
    }

    if (exp->getOper() == opFsize && exp->getSubExp3()->isFltConst()) {
        changed = true;
        return exp->getSubExp3();
    }


    if (exp->getOper() == opItof &&
        exp->getSubExp2()->isIntConst() &&
        exp->getSubExp3()->isIntConst() &&
        exp->access<Const, 2>()->getInt() == 32) {
            changed = true;
            unsigned int n = exp->access<Const, 3>()->getInt();
            return Const::get(*reinterpret_cast<float *>(&n));
    }

    if (exp->getOper() == opFsize &&
        exp->getSubExp3()->getOper() == opMemOf &&
        exp->getSubExp3()->getSubExp1()->isIntConst()) {
            assert(exp->getSubExp3()->isLocation());
            Address  u  = exp->access<Const, 3, 1>()->getAddr();
            UserProc *p = exp->access<Location, 3>()->getProc();

            if (p) {
                Prog   *prog = p->getProg();
                double d;
                const bool ok =  prog->getFloatConstant(u, d, exp->access<Const, 1>()->getInt());

                if (ok) {
                    changed    = true;
                    LOG_VERBOSE("Replacing %1 with %2 in %3", exp->getSubExp3(), d, exp->shared_from_this());
                    return Const::get(d);
                }
            }
    }

    if (exp->getOper() == opTruncu && exp->getSubExp3()->isIntConst()) {
        int          from = exp->access<Const, 1>()->getInt();
        int          to   = exp->access<Const, 2>()->getInt();
        unsigned int val  = exp->access<Const, 3>()->getInt();

        if (from == 32) {
            if (to == 16) {
                changed = true;
                return Const::get(Address(val & 0xffff));
            }

            if (to == 8) {
                changed = true;
                return  Const::get(Address(val & 0xff));
            }
        }
    }

    if (exp->getOper() == opTruncs && exp->getSubExp3()->isIntConst()) {
        int from = exp->access<Const, 1>()->getInt();
        int to   = exp->access<Const, 2>()->getInt();
        int val  = exp->access<Const, 3>()->getInt();

        if (from == 32) {
            if (to == 16) {
                changed = true;
                return Const::get(val & 0xffff);
            }

            if (to == 8) {
                changed = true;
                return  Const::get(val & 0xff);
            }
        }
    }

    return exp;
}


SharedExp ExpSimplifier::preModify(const std::shared_ptr<TypedExp>& exp, bool& visitChildren)
{
    visitChildren = true;

    if (exp->getSubExp1()->isRegOf()) {
        // type cast on a reg of.. hmm.. let's remove this
        m_modified = true;
        return exp->getSubExp1();
    }

    return exp;
}


SharedExp ExpSimplifier::postModify(const std::shared_ptr<Location>& exp)
{
    bool& changed = m_modified;

    if (exp->isMemOf() && exp->getSubExp1()->isAddrOf()) {
        changed = true;
        return exp->getSubExp1()->getSubExp1();
    }

    // check for m[a[loc.x]] becomes loc.x
    if (exp->isMemOf() && exp->getSubExp1()->isAddrOf() &&
        exp->getSubExp1()->getSubExp1()->isMemberOf()) {
            changed = true;
            return exp->getSubExp1()->getSubExp1();
    }

    return exp;
}


SharedExp ExpSimplifier::postModify(const std::shared_ptr<RefExp>& exp)
{
    if (m_modified) {
        return exp;
    }

    bool& changed = m_modified;

    /*
     * This is a nasty hack.  We assume that %DF{0} is 0.  This happens
     * when string instructions are used without first clearing the direction flag.
     * By convention, the direction flag is assumed to be clear on entry to a
     * procedure.
     */
    if (exp->getSubExp1()->getOper() == opDF && exp->getDef() == nullptr) {
        changed = true;
        return Const::get(int(0));
    }

    // another hack, this time for aliasing
    // FIXME: do we really want this now? Pentium specific,
    // and only handles ax/eax (not al or ah)
    if (exp->getSubExp1()->isRegN(PENT_REG_AX) && exp->getDef() && exp->getDef()->isAssign() &&
        static_cast<const Assign *>(exp->getDef())->getLeft()->isRegN(PENT_REG_EAX)) {
            changed = true;
            return std::make_shared<TypedExp>(IntegerType::get(16),
                RefExp::get(Location::regOf(PENT_REG_EAX), exp->getDef()));
    }

    // Was code here for bypassing phi statements that are now redundant
    return exp;
}
