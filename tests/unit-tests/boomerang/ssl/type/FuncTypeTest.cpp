#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "FuncTypeTest.h"


#include "boomerang/ssl/type/FuncType.h"
#include "boomerang/ssl/type/BooleanType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/exp/Location.h"


void FuncTypeTest::testConstruct()
{
    std::shared_ptr<Signature> sig = Signature::instantiate(Machine::X86, CallConv::C, "foo");

    QVERIFY(FuncType().getSignature() == nullptr);
    QVERIFY(*FuncType(sig).getSignature() == *sig);
}


void FuncTypeTest::testEquals()
{
    std::shared_ptr<Signature> sig1 = Signature::instantiate(Machine::X86, CallConv::C, "foo1");
    std::shared_ptr<Signature> sig2 = Signature::instantiate(Machine::X86, CallConv::C, "foo2");

    QCOMPARE(FuncType() == VoidType(), false);
    QCOMPARE(FuncType() == FuncType(), true);
    QCOMPARE(FuncType(sig1) == FuncType(sig1), true);
    QCOMPARE(FuncType(sig1) == FuncType(sig2), false);
}


void FuncTypeTest::testLess()
{
    QCOMPARE(FuncType() < VoidType(), false);
    QCOMPARE(FuncType() < FuncType(), false);
    QCOMPARE(FuncType() < BooleanType(), true);

    std::shared_ptr<Signature> sig1 = Signature::instantiate(Machine::X86, CallConv::C, "foo1");
    std::shared_ptr<Signature> sig2 = Signature::instantiate(Machine::X86, CallConv::C, "foo2");

    QCOMPARE(FuncType(sig1) < FuncType(), false);
    QCOMPARE(FuncType() < FuncType(sig1), true);
    QCOMPARE(FuncType(sig1) < FuncType(sig2), true);
    QCOMPARE(FuncType(sig1) < FuncType(sig1), false);

    sig1->addParameter(Location::regOf(REG_PENT_EDX), IntegerType::get(32, Sign::Unsigned));
    sig2->addParameter(Location::regOf(REG_PENT_EBX), IntegerType::get(32, Sign::Unsigned));

    QCOMPARE(FuncType(sig1) < FuncType(sig2), true);
    QCOMPARE(FuncType(sig2) < FuncType(sig1), false);
}


void FuncTypeTest::testGetCtype()
{
    std::shared_ptr<Signature> sig1 = Signature::instantiate(Machine::X86, CallConv::C, "foo1");
    std::shared_ptr<Signature> sig2 = Signature::instantiate(Machine::X86, CallConv::C, "foo2");

    QCOMPARE(FuncType().getCtype(), "void (void)");
    QCOMPARE(FuncType(sig1).getCtype(), "void * ()");

    sig1->addParameter(Location::regOf(REG_PENT_EDX), IntegerType::get(32, Sign::Unsigned));
    QCOMPARE(FuncType(sig1).getCtype(), "void * (unsigned int)");

    sig1->addReturn(IntegerType::get(32, Sign::Signed));
    QCOMPARE(FuncType(sig1).getCtype(), "void * (unsigned int)"); // because void * is first return
}


void FuncTypeTest::testGetReturnAndParam()
{
    QString ret, param;

    std::shared_ptr<Signature> sig1 = Signature::instantiate(Machine::X86, CallConv::C, "foo1");

    FuncType().getReturnAndParam(ret, param);
    QCOMPARE(ret, "void");
    QCOMPARE(param, "(void)");

    FuncType(sig1).getReturnAndParam(ret, param);
    QCOMPARE(ret, "void *");
    QCOMPARE(param, " ()");

    sig1->addParameter(Location::regOf(REG_PENT_EDX), IntegerType::get(32, Sign::Unsigned));
    FuncType(sig1).getReturnAndParam(ret, param);
    QCOMPARE(ret, "void *");
    QCOMPARE(param, " (unsigned int)");

    sig1->addParameter(Location::regOf(REG_PENT_EDX), IntegerType::get(32, Sign::Signed));
    FuncType(sig1).getReturnAndParam(ret, param);
    QCOMPARE(ret, "void *");
    QCOMPARE(param, " (unsigned int, int)");
}


void FuncTypeTest::testIsCompatibleWith()
{
    std::shared_ptr<Signature> sig1 = Signature::instantiate(Machine::X86, CallConv::C, "foo1");
    std::shared_ptr<Signature> sig2 = Signature::instantiate(Machine::X86, CallConv::C, "foo2");

    QCOMPARE(FuncType::get(sig1)->isCompatibleWith(*VoidType::get()), true);
    QCOMPARE(FuncType::get(sig1)->isCompatibleWith(*FuncType::get(sig1)), true);
    QCOMPARE(FuncType::get(sig1)->isCompatibleWith(*FuncType::get(sig2)), false);
    QCOMPARE(FuncType::get(sig1)->isCompatibleWith(*SizeType::get(32)), true);
    QCOMPARE(FuncType::get(sig1)->isCompatibleWith(*SizeType::get(64)), false);
}


QTEST_GUILESS_MAIN(FuncTypeTest)
