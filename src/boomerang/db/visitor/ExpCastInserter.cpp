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
    SharedType ty             = e->ascendType();
    int        currSignedness = 0;
    bool       isInt          = ty->resolvesToInteger();

    if (isInt) {
        currSignedness = ty->as<IntegerType>()->getSignedness();
        currSignedness = (currSignedness >= 0) ? 1 : -1;
    }

    // if (!isInt || currSignedness != reqSignedness) { // }
    // Don't want to cast e.g. floats to integer
    if (isInt && (currSignedness != reqSignedness)) {
        std::shared_ptr<IntegerType> newtype;

        if (!isInt) {
            newtype = IntegerType::get(STD_SIZE, reqSignedness);
        }
        else {
            newtype = IntegerType::get(std::static_pointer_cast<IntegerType>(ty)->getSize(), reqSignedness); // Transfer size
        }

        newtype->setSigned(reqSignedness);
        return std::make_shared<TypedExp>(newtype, e);
    }

    return e;
}

SharedExp ExpCastInserter::preVisit(const std::shared_ptr< TypedExp >& e, bool& recur)
{
    recur = false;
    return e;
}



// Check the type of the address expression of memof to make sure it is compatible with the given memofType.
// memof may be changed internally to include a TypedExp, which will emit as a cast
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


SharedExp ExpCastInserter::postVisit(const std::shared_ptr<RefExp>& e)
{
    SharedExp base = e->getSubExp1();

    if (base->isMemOf()) {
        // Check to see if the address expression needs type annotation
        Statement *def = e->getDef();

        if (!def) {
            LOG_WARN("RefExp def is null");
            return e;
        }

        SharedType memofType = def->getTypeFor(base);
        checkMemofType(base, memofType);
    }

    return e;
}


SharedExp ExpCastInserter::postVisit(const std::shared_ptr<Binary>& e)
{
    OPER op = e->getOper();

    switch (op)
    {
    // This case needed for e.g. test/pentium/switch_gcc:
    case opLessUns:
    case opGtrUns:
    case opLessEqUns:
    case opGtrEqUns:
    case opShiftR:
        e->setSubExp1(checkSignedness(e->getSubExp1(), -1));

        if (op != opShiftR) { // The shift amount (second operand) is sign agnostic
            e->setSubExp2(checkSignedness(e->getSubExp2(), -1));
        }

        break;

    // This case needed for e.g. test/sparc/minmax2, if %g1 is declared as unsigned int
    case opLess:
    case opGtr:
    case opLessEq:
    case opGtrEq:
    case opShiftRA:
        e->setSubExp1(checkSignedness(e->getSubExp1(), +1));

        if (op != opShiftRA) {
            e->setSubExp2(checkSignedness(e->getSubExp2(), +1));
        }

        break;

    default:
        break;
    }

    return e;
}


SharedExp ExpCastInserter::postVisit(const std::shared_ptr<Const>& e)
{
    if (e->isIntConst()) {
        bool       naturallySigned = e->getInt() < 0;
        SharedType ty = e->getType();

        if (naturallySigned && ty->isInteger() && !ty->as<IntegerType>()->isSigned()) {
            return std::make_shared<TypedExp>(IntegerType::get(ty->as<IntegerType>()->getSize(), -1), e);
        }
    }

    return e;
}
