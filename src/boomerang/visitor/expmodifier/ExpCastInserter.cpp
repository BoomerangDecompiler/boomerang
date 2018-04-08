#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpCastInserter.h"


#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/TypedExp.h"
#include "boomerang/db/statements/Statement.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/util/Log.h"


static SharedExp checkSignedness(SharedExp e, int reqSignedness)
{
    SharedConstType ty        = e->ascendType();
    const bool isInt          = ty->resolvesToInteger();
    int        currSignedness = 0;

    if (isInt) {
        currSignedness = ty->as<IntegerType>()->getSignedness();
        currSignedness = (currSignedness >= 0) ? 1 : -1;
    }

    // Don't want to cast e.g. floats to integer
    if (isInt && (currSignedness != reqSignedness)) {
        // Transfer size
        std::shared_ptr<IntegerType> newtype =
            IntegerType::get(std::static_pointer_cast<const IntegerType>(ty)->getSize(), reqSignedness);

        newtype->setSigned(reqSignedness);
        return std::make_shared<TypedExp>(newtype, e);
    }

    return e;
}

SharedExp ExpCastInserter::preModify(const std::shared_ptr<TypedExp>& exp, bool& visitChildren)
{
    visitChildren = false;
    return exp;
}


void ExpCastInserter::checkMemofType(const SharedExp& memof, SharedType memofType)
{
    SharedExp addr = memof->getSubExp1();

    if (addr->isSubscript()) {
        SharedExp  addrBase     = addr->getSubExp1();
        SharedType actType      = addr->access<RefExp>()->getDef()->getTypeFor(addrBase);
        SharedType expectedType = PointerType::get(memofType);

        if (!actType->isCompatibleWith(*expectedType)) {
            memof->setSubExp1(std::make_shared<TypedExp>(expectedType, addrBase));
        }
    }
}


SharedExp ExpCastInserter::postModify(const std::shared_ptr<RefExp>& exp)
{
    SharedExp base = exp->getSubExp1();

    if (base->isMemOf()) {
        // Check to see if the address expression needs type annotation
        Statement *def = exp->getDef();

        if (!def) {
            LOG_WARN("RefExp def is null");
            return exp;
        }

        SharedType memofType = def->getTypeFor(base);
        checkMemofType(base, memofType);
    }

    return exp;
}


SharedExp ExpCastInserter::postModify(const std::shared_ptr<Binary>& exp)
{
    OPER op = exp->getOper();

    switch (op)
    {
    // This case needed for e.g. test/pentium/switch_gcc:
    case opLessUns:
    case opGtrUns:
    case opLessEqUns:
    case opGtrEqUns:
    case opShiftR:
        exp->setSubExp1(checkSignedness(exp->getSubExp1(), -1));

        if (op != opShiftR) { // The shift amount (second operand) is sign agnostic
            exp->setSubExp2(checkSignedness(exp->getSubExp2(), -1));
        }

        break;

    // This case needed for e.g. test/sparc/minmax2, if %g1 is declared as unsigned int
    case opLess:
    case opGtr:
    case opLessEq:
    case opGtrEq:
    case opShiftRA:
        exp->setSubExp1(checkSignedness(exp->getSubExp1(), +1));

        if (op != opShiftRA) {
            exp->setSubExp2(checkSignedness(exp->getSubExp2(), +1));
        }

        break;

    default:
        break;
    }

    return exp;
}


SharedExp ExpCastInserter::postModify(const std::shared_ptr<Const>& exp)
{
    if (exp->isIntConst()) {
        bool       naturallySigned = exp->getInt() < 0;
        SharedType ty = exp->getType();

        if (naturallySigned && ty->isInteger() && !ty->as<IntegerType>()->isSigned()) {
            return std::make_shared<TypedExp>(IntegerType::get(ty->as<IntegerType>()->getSize(), -1), exp);
        }
    }

    return exp;
}
