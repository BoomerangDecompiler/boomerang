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


#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/exp/Location.h"


#define TEST_SIMPLIFY(sourceExp, expectedResult) \
    {\
        SharedExp exp = (sourceExp); \
        QCOMPARE(QString(exp->simplifyArith()->toString()), QString((expectedResult)->toString())); \
    }


void ExpArithSimplifierTest::testSimplifyUnary()
{
    // a[eax - eax] is simplified by simplifyArith()
    TEST_SIMPLIFY(Unary::get(opAddrOf,
                             Binary::get(opMinus,
                                         Location::regOf(REG_PENT_EAX),
                                         Location::regOf(REG_PENT_EAX))),
                  Unary::get(opAddrOf, Const::get(0)));

    // -(eax - eax) is only simplified by simplify()
    TEST_SIMPLIFY(Unary::get(opNeg,
                             Binary::get(opMinus,
                                         Location::regOf(REG_PENT_EAX),
                                         Location::regOf(REG_PENT_EAX))),
                  Unary::get(opNeg,
                             Binary::get(opMinus,
                                         Location::regOf(REG_PENT_EAX),
                                         Location::regOf(REG_PENT_EAX))));
}


void ExpArithSimplifierTest::testSimplifyBinary()
{
    // 5 > 3 (only handled by simplify, not simpilfyArith)
    TEST_SIMPLIFY(Binary::get(opGtr, Const::get(5), Const::get(3)),
                  Binary::get(opGtr, Const::get(5), Const::get(3)));

    TEST_SIMPLIFY(Binary::get(opPlus, Const::get(5), Const::get(3)),
                  Const::get(8));

    TEST_SIMPLIFY(Binary::get(opPlus, Location::regOf(REG_PENT_ESP), Const::get(-4)),
                  Binary::get(opMinus, Location::regOf(REG_PENT_ESP), Const::get(4)));

    // Cancel duplicates
    TEST_SIMPLIFY(Binary::get(opMinus,
                              Location::regOf(REG_PENT_EAX),
                              Location::regOf(REG_PENT_EAX)),
                  Const::get(0));

    // positive const, negative reg
    TEST_SIMPLIFY(Binary::get(opMinus,
                              Const::get(5),
                              Location::regOf(REG_PENT_EAX)),
                  Binary::get(opMinus,
                              Const::get(5),
                              Location::regOf(REG_PENT_EAX)));

    // positives + negatives + int const
    TEST_SIMPLIFY(Binary::get(opMinus,
                              Location::regOf(REG_PENT_EAX),
                              Binary::get(opMinus,
                                          Location::regOf(REG_PENT_ECX),
                                          Binary::get(opPlus,
                                                      Binary::get(opMinus,
                                                                  Location::regOf(REG_PENT_EBX),
                                                                  Location::regOf(REG_PENT_EDX)),
                                                      Const::get(-5)))),
                  Binary::get(opMinus,
                              Binary::get(opMinus,
                                          Binary::get(opPlus,
                                                      Location::regOf(REG_PENT_EAX),
                                                      Location::regOf(REG_PENT_EBX)),
                                          Binary::get(opPlus,
                                                      Location::regOf(REG_PENT_ECX),
                                                      Location::regOf(REG_PENT_EDX))),
                              Const::get(5)));


    // pc + 108 + n - (pc + 92)
    TEST_SIMPLIFY(Binary::get(opMinus,
                              Binary::get(opPlus,
                                          Binary::get(opPlus,
                                                      Terminal::get(opPC),
                                                      Const::get(108)),
                                          Unary::get(opParam, Const::get("n"))),
                              Binary::get(opPlus,
                                          Terminal::get(opPC),
                                          Const::get(92))),
                  Binary::get(opPlus,
                              Unary::get(opParam, Const::get("n")),
                              Const::get(16)));

    // m[(r28 + -4) + 8]
    TEST_SIMPLIFY(Location::memOf(Binary::get(opPlus,
                                              Binary::get(opPlus,
                                                          Location::regOf(REG_PENT_ESP),
                                                          Const::get(-4)),
                                              Const::get(8))),
                  Location::memOf(Binary::get(opPlus,
                                              Location::regOf(REG_PENT_ESP),
                                              Const::get(4))));

    // r24 + m[(r28 - 4) - 4]
    TEST_SIMPLIFY(Binary::get(opPlus,
                              Location::regOf(REG_PENT_EAX),
                              Location::memOf(Binary::get(opMinus,
                                                          Binary::get(opMinus,
                                                                      Location::regOf(REG_PENT_ESP),
                                                                      Const::get(4)),
                                                          Const::get(4)))),
                  Binary::get(opPlus,
                              Location::regOf(REG_PENT_EAX),
                              Location::memOf(Binary::get(opMinus,
                                                          Location::regOf(REG_PENT_ESP),
                                                          Const::get(8)))));
}


QTEST_GUILESS_MAIN(ExpArithSimplifierTest)
