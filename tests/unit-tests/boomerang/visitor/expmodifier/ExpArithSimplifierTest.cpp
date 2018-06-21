#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpArithSimplifierTest.h"


#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/Location.h"


#define TEST_SIMPLIFY(sourceExp, expectedResult) \
    {\
        SharedExp exp = (sourceExp); \
        QCOMPARE(QString(exp->simplifyArith()->prints()), QString((expectedResult)->prints())); \
    }


void ExpArithSimplifierTest::testSimplifyUnary()
{
}


void ExpArithSimplifierTest::testSimplifyBinary()
{
    // afp + 108 + n - (afp + 92)
    TEST_SIMPLIFY(Binary::get(opMinus,
                              Binary::get(opPlus,
                                          Binary::get(opPlus,
                                                      Terminal::get(opAFP),
                                                      Const::get(108)),
                                          Unary::get(opVar, Const::get("n"))),
                              Binary::get(opPlus,
                                          Terminal::get(opAFP),
                                          Const::get(92))),
                  Binary::get(opPlus,
                              Unary::get(opVar, Const::get("n")),
                              Const::get(16)));

    // m[(r28 + -4) + 8]
    TEST_SIMPLIFY(Location::memOf(Binary::get(opPlus,
                                              Binary::get(opPlus,
                                                          Location::regOf(PENT_REG_ESP),
                                                          Const::get(-4)),
                                              Const::get(8))),
                  Location::memOf(Binary::get(opPlus,
                                              Location::regOf(PENT_REG_ESP),
                                              Const::get(4))));

    // r24 + m[(r28 - 4) - 4]
    TEST_SIMPLIFY(Binary::get(opPlus,
                              Location::regOf(PENT_REG_EAX),
                              Location::memOf(Binary::get(opMinus,
                                                          Binary::get(opMinus,
                                                                      Location::regOf(PENT_REG_ESP),
                                                                      Const::get(4)),
                                                          Const::get(4)))),
                  Binary::get(opPlus,
                              Location::regOf(PENT_REG_EAX),
                              Location::memOf(Binary::get(opMinus,
                                                          Location::regOf(PENT_REG_ESP),
                                                          Const::get(8)))));
}


QTEST_GUILESS_MAIN(ExpArithSimplifierTest)
