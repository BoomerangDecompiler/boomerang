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


#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Location.h"


#define TEST_SIMPLIFY(sourceExp, expectedResult) \
    {\
        SharedExp exp = (sourceExp); \
        QCOMPARE(QString(exp->simplifyAddr()->prints()), QString((expectedResult)->prints())); \
    }


void ExpAddrSimplifierTest::testSimplifyUnary()
{
    TEST_SIMPLIFY(Unary::get(opAddrOf, Location::memOf(Const::get(0x1000))),
                  Const::get(0x1000));

    TEST_SIMPLIFY(Unary::get(opAddrOf, Binary::get(opSize, Const::get(16), Location::memOf(Const::get(0x1000)))),
                  Const::get(0x1000));

    TEST_SIMPLIFY(Unary::get(opNeg, Const::get(0x1000)),
                  Unary::get(opNeg, Const::get(0x1000)));
}


void ExpAddrSimplifierTest::testSimplifyLocation()
{
    TEST_SIMPLIFY(Location::memOf(Unary::get(opAddrOf, Const::get(0x1000))),
                  Const::get(0x1000));

    TEST_SIMPLIFY(Location::memOf(Location::regOf(PENT_REG_EAX)),
                  Location::memOf(Location::regOf(PENT_REG_EAX)));
}


QTEST_GUILESS_MAIN(ExpAddrSimplifierTest)
