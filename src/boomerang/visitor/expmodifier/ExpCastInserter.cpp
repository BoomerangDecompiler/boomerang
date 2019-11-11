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

#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/TypedExp.h"
#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/util/log/Log.h"


static SharedExp checkSignedness(SharedExp e, Sign reqSignedness)
{
    SharedConstType ty  = e->ascendType();
    const bool isInt    = ty->resolvesToInteger();
    Sign currSignedness = Sign::Unknown;

    if (isInt) {
        currSignedness = ty->as<IntegerType>()->isMaybeSigned() ? Sign::Signed : Sign::Unsigned;
    }

    // Don't want to cast e.g. floats to integer
    if (isInt && (currSignedness != reqSignedness)) {
        // Transfer size
        std::shared_ptr<IntegerType> newtype = IntegerType::get(
            ty->as<const IntegerType>()->getSize(), reqSignedness);

        newtype->setSignedness(reqSignedness);
        return TypedExp::get(newtype, e);
    }

    return e;
}

SharedExp ExpCastInserter::preModify(const std::shared_ptr<TypedExp> &exp, bool &visitChildren)
{
    visitChildren = false;
    return exp;
}


void ExpCastInserter::checkMemofType(const SharedExp &memof, SharedType memofType)
{
    SharedExp addr = memof->getSubExp1();

    if (addr->isSubscript()) {
        SharedExp addrBase      = addr->getSubExp1();
        SharedType actType      = addr->access<RefExp>()->getDef()->getTypeForExp(addrBase);
        SharedType expectedType = PointerType::get(memofType);

        if (!actType->isCompatibleWith(*expectedType)) {
            memof->setSubExp1(TypedExp::get(expectedType, addrBase));
        }
    }
}


SharedExp ExpCastInserter::postModify(const std::shared_ptr<RefExp> &exp)
{
    SharedExp base = exp->getSubExp1();

    if (base->isMemOf()) {
        // Check to see if the address expression needs type annotation
        SharedStmt def = exp->getDef();

        if (!def) {
            LOG_WARN("RefExp def is null");
            return exp;
        }

        SharedType memofType = def->getTypeForExp(base);
        checkMemofType(base, memofType);
    }

    return exp;
}


SharedExp ExpCastInserter::postModify(const std::shared_ptr<Binary> &exp)
{
    OPER op = exp->getOper();

    switch (op) {
    // This case needed for e.g. test/x86/switch_gcc:
    case opLessUns:
    case opGtrUns:
    case opLessEqUns:
    case opGtrEqUns:
    case opShR:
        exp->setSubExp1(checkSignedness(exp->getSubExp1(), Sign::Unsigned));

        if (op != opShR) { // The shift amount (second operand) is sign agnostic
            exp->setSubExp2(checkSignedness(exp->getSubExp2(), Sign::Unsigned));
        }

        break;

    // This case needed for e.g. test/sparc/minmax2, if %g1 is declared as unsigned int
    case opLess:
    case opGtr:
    case opLessEq:
    case opGtrEq:
    case opShRA:
        exp->setSubExp1(checkSignedness(exp->getSubExp1(), Sign::Signed));

        if (op != opShRA) {
            exp->setSubExp2(checkSignedness(exp->getSubExp2(), Sign::Signed));
        }

        break;

    default: break;
    }

    return exp;
}


SharedExp ExpCastInserter::postModify(const std::shared_ptr<Const> &exp)
{
    if (exp->isIntConst()) {
        const bool naturallySigned = exp->getInt() < 0;
        SharedType ty              = exp->getType();

        if (naturallySigned && ty->isInteger() && ty->as<IntegerType>()->isUnsigned()) {
            return TypedExp::get(IntegerType::get(ty->as<IntegerType>()->getSize(), Sign::Unsigned),
                                 exp);
        }
    }

    return exp;
}
