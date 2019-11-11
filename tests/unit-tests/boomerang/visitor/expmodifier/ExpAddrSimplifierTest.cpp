#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpAddrSimplifierTest.h"


#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"


#define TEST_SIMPLIFY(sourceExp, expectedResult) \
    {\
        SharedExp exp = (sourceExp); \
        QCOMPARE(QString(exp->simplifyAddr()->toString()), QString((expectedResult)->toString())); \
    }


void ExpAddrSimplifierTest::testSimplifyUnary()
{
    TEST_SIMPLIFY(Unary::get(opAddrOf, Location::memOf(Const::get(0x1000))),
                  Const::get(0x1000));

    TEST_SIMPLIFY(Unary::get(opNeg, Const::get(0x1000)),
                  Unary::get(opNeg, Const::get(0x1000)));
}


void ExpAddrSimplifierTest::testSimplifyLocation()
{
    TEST_SIMPLIFY(Location::memOf(Unary::get(opAddrOf, Const::get(0x1000))),
                  Const::get(0x1000));

    TEST_SIMPLIFY(Location::memOf(Location::regOf(REG_X86_EAX)),
                  Location::memOf(Location::regOf(REG_X86_EAX)));
}


QTEST_GUILESS_MAIN(ExpAddrSimplifierTest)
