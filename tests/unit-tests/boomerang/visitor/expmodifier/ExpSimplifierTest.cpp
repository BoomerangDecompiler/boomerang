#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpSimplifierTest.h"


#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Ternary.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/Terminal.h"


/// HACK to work around limitations of QMetaType which does not allow templates
struct SharedExpWrapper
{
public:
    explicit SharedExpWrapper()               : exp(nullptr) {}
    explicit SharedExpWrapper(SharedExp _exp) : exp(_exp) {}

public:
    SharedExp operator->() { return exp; }
    SharedExp operator*()  { return exp; }
    operator SharedExp()  { return exp; }

public:
    SharedExp exp;
};

Q_DECLARE_METATYPE(SharedExpWrapper)


void ExpSimplifierTest::initTestCase()
{
    qRegisterMetaType<SharedExpWrapper>();
}


void ExpSimplifierTest::testSimplify()
{
    QFETCH(SharedExpWrapper, exp);
    QFETCH(SharedExpWrapper, expectedResult);

    SharedExp actualResult = exp->simplify();
    QString actual(actualResult->prints());
    QString expected(expectedResult->prints());

    QCOMPARE(actual, expected);
}

#define TEST_SIMPLIFY(name, exp, result) \
    QTest::newRow(name) << SharedExpWrapper(exp) << SharedExpWrapper(result)


void ExpSimplifierTest::testSimplify_data()
{
    QTest::addColumn<SharedExpWrapper>("exp");
    QTest::addColumn<SharedExpWrapper>("expectedResult");

    // Unary
    {
        TEST_SIMPLIFY("UnaryNotEqual",
                      Unary::get(opNot,
                                 Binary::get(opEquals,
                                             Location::regOf(PENT_REG_EAX),
                                             Location::regOf(PENT_REG_EDX))),
                      Binary::get(opNotEqual,
                                  Location::regOf(PENT_REG_EAX),
                                  Location::regOf(PENT_REG_EDX))
        );

        TEST_SIMPLIFY("UnaryNegConst",
                      Unary::get(opNeg, Const::get(0xFF)),
                      Const::get(0xFFFFFF01));

        TEST_SIMPLIFY("UnaryNotConst",
                      Unary::get(opNot, Const::get(0xFF)),
                      Const::get(0xFFFFFF00));

        TEST_SIMPLIFY("UnaryLNotConst",
                      Unary::get(opLNot, Const::get(0xFF)),
                      Const::get(0x00000000));

        TEST_SIMPLIFY("UnaryAddrOfMemOf",
                      Unary::get(opAddrOf, Unary::get(opMemOf, Const::get(0x1000))),
                      Const::get(0x1000));

        TEST_SIMPLIFY("UnaryMemOfAddrOf",
                      Unary::get(opMemOf, Unary::get(opAddrOf, Const::get(0x1000))),
                      Const::get(0x1000));
    }

    // Binary
    {
        TEST_SIMPLIFY("BinaryConstPlusConst",
                      Binary::get(opPlus, Const::get(100), Const::get(10)),
                      Const::get(110));

        TEST_SIMPLIFY("BinaryXxorX",
                      Binary::get(opBitXor,
                                  Location::regOf(PENT_REG_EAX),
                                  Location::regOf(PENT_REG_EAX)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryXandX",
                      Binary::get(opBitAnd,
                                  Location::regOf(PENT_REG_EAX),
                                  Location::regOf(PENT_REG_EAX)),
                      Location::regOf(PENT_REG_EAX));

        TEST_SIMPLIFY("BinaryXequalX",
                      Binary::get(opEquals,
                                  Location::regOf(PENT_REG_EAX),
                                  Location::regOf(PENT_REG_EAX)),
                      Terminal::get(opTrue));

        TEST_SIMPLIFY("BinaryXnotequalX",
                      Binary::get(opNotEqual,
                                  Location::regOf(PENT_REG_EAX),
                                  Location::regOf(PENT_REG_EAX)),
                      Terminal::get(opFalse));

        TEST_SIMPLIFY("BinaryCommutePlus",
                      Binary::get(opPlus,
                                  Const::get(100),
                                  Location::regOf(PENT_REG_EAX)),
                      Binary::get(opPlus,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(100)));

        TEST_SIMPLIFY("BinaryCommuteMults",
                      Binary::get(opMults,
                                  Const::get(100),
                                  Location::regOf(PENT_REG_EAX)),
                      Binary::get(opMults,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(100)));

        TEST_SIMPLIFY("BinaryCommuteMult",
                      Binary::get(opMult,
                                  Const::get(100),
                                  Location::regOf(PENT_REG_EAX)),
                      Binary::get(opMult,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(100)));

        TEST_SIMPLIFY("BinaryCollapseConstPlus",
                      Binary::get(opPlus,
                                  Binary::get(opPlus,
                                              Location::regOf(PENT_REG_EAX),
                                              Const::get(50)),
                                  Const::get(100)),
                      Binary::get(opPlus, Location::regOf(PENT_REG_EAX), Const::get(150)));

        TEST_SIMPLIFY("BinaryCollapseConstMinus",
                      Binary::get(opPlus,
                                  Binary::get(opMinus,
                                              Location::regOf(PENT_REG_EAX),
                                              Const::get(30)),
                                  Const::get(100)),
                      Binary::get(opPlus, Location::regOf(PENT_REG_EAX), Const::get(70)));

        TEST_SIMPLIFY("BinaryLinearizeConstPlus",
                      Binary::get(opPlus,
                                  Binary::get(opMult,
                                              Location::regOf(PENT_REG_EAX),
                                              Location::regOf(PENT_REG_EDX)),
                                  Location::regOf(PENT_REG_EAX)),
                      Binary::get(opMult,
                                  Location::regOf(PENT_REG_EAX),
                                  Binary::get(opPlus,
                                              Location::regOf(PENT_REG_EDX),
                                              Const::get(1))));

        TEST_SIMPLIFY("BinaryLinearizeConstMinus",
                      Binary::get(opMinus,
                                  Binary::get(opMults,
                                              Location::regOf(PENT_REG_EAX),
                                              Location::regOf(PENT_REG_EDX)),
                                  Location::regOf(PENT_REG_EAX)),
                      Binary::get(opMults,
                                  Location::regOf(PENT_REG_EAX),
                                  Binary::get(opMinus,
                                              Location::regOf(PENT_REG_EDX),
                                              Const::get(1))));

        TEST_SIMPLIFY("BinaryChangeAbsConst",
                      Binary::get(opPlus,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(-30)),
                      Binary::get(opMinus,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(30)));

        TEST_SIMPLIFY("BinaryConstRemoveNull",
                      Binary::get(opPlus,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(0)),
                      Location::regOf(PENT_REG_EAX));


        TEST_SIMPLIFY("BinaryConstRemoveFalse",
                      Binary::get(opOr,
                                  Location::regOf(PENT_REG_EAX),
                                  Terminal::get(opFalse)),
                      Location::regOf(PENT_REG_EAX));

        TEST_SIMPLIFY("BinaryConstRemoveAndNull",
                      Binary::get(opBitAnd,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(0)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstRemoveAndFalse",
                      Binary::get(opAnd,
                                  Location::regOf(PENT_REG_EAX),
                                  Terminal::get(opFalse)),
                      Terminal::get(opFalse));

        TEST_SIMPLIFY("BinaryConstRemoveMult0",
                      Binary::get(opMult,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(0)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstRemoveMult1",
                      Binary::get(opMult,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(1)),
                      Location::regOf(PENT_REG_EAX));

        TEST_SIMPLIFY("BinaryConstRemoveDiv1",
                      Binary::get(opDiv,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(1)),
                      Location::regOf(PENT_REG_EAX));

        TEST_SIMPLIFY("BinaryConstRemoveMod1",
                      Binary::get(opMod,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(1)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstRemoveAXmodX",
                      Binary::get(opMod,
                                  Binary::get(opMult,
                                              Location::regOf(PENT_REG_EDX),
                                              Location::regOf(PENT_REG_EAX)),
                                  Location::regOf(PENT_REG_EAX)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstRemoveXmodX",
                      Binary::get(opMod,
                                  Location::regOf(PENT_REG_EAX),
                                  Location::regOf(PENT_REG_EAX)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstCollapseShr",
                      Binary::get(opShiftR,
                                  Const::get(0x100),
                                  Const::get(4)),
                      Const::get(0x010));

        TEST_SIMPLIFY("BinaryConstCollapseShl",
                      Binary::get(opShiftL,
                                  Const::get(0x100),
                                  Const::get(4)),
                      Const::get(0x1000));

        TEST_SIMPLIFY("BinaryCollapseDoubleEquality",
                      Binary::get(opEquals,
                                  Binary::get(opEquals,
                                              Location::regOf(PENT_REG_EAX),
                                              Location::regOf(PENT_REG_ECX)),
                                 Const::get(1)),
                      Binary::get(opEquals,
                                  Location::regOf(PENT_REG_EAX),
                                  Location::regOf(PENT_REG_ECX)));

        TEST_SIMPLIFY("BinaryCollapseEqualLessEqual",
                      Binary::get(opOr,
                                  Binary::get(opLessEq,
                                              Location::regOf(PENT_REG_EAX),
                                              Location::regOf(PENT_REG_ECX)),
                                  Binary::get(opEquals,
                                              Location::regOf(PENT_REG_EAX),
                                              Location::regOf(PENT_REG_ECX))),
                      Binary::get(opLessEq,
                                  Location::regOf(PENT_REG_EAX),
                                  Location::regOf(PENT_REG_ECX)));

        TEST_SIMPLIFY("BinaryCollapseLessEqual",
                      Binary::get(opOr,
                                  Binary::get(opLess,
                                              Location::regOf(PENT_REG_EAX),
                                              Location::regOf(PENT_REG_ECX)),
                                  Binary::get(opEquals,
                                              Location::regOf(PENT_REG_EAX),
                                              Location::regOf(PENT_REG_ECX))),
                      Binary::get(opLessEq,
                                  Location::regOf(PENT_REG_EAX),
                                  Location::regOf(PENT_REG_ECX)));

        TEST_SIMPLIFY("BinaryCollapseBitAnd",
                      Binary::get(opBitAnd,
                                  Location::regOf(PENT_REG_EAX),
                                  Location::regOf(PENT_REG_EAX)),
                      Location::regOf(PENT_REG_EAX));

        TEST_SIMPLIFY("BinaryCollapseDoubleConstMult",
                      Binary::get(opMult,
                                  Binary::get(opMult,
                                              Location::regOf(PENT_REG_EAX),
                                              Const::get(0x100)),
                                  Const::get(0x010)),
                      Binary::get(opMult,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(0x1000)));

        TEST_SIMPLIFY("BinaryRemoveFloatNull",
                      Binary::get(opFMinus,
                                  Const::get(0.0f),
                                  Location::regOf(PENT_REG_ST0)),
                      Unary::get(opFNeg, Location::regOf(PENT_REG_ST0)));

        TEST_SIMPLIFY("BinaryDistributeDivision",
                      Binary::get(opDiv,
                                  Binary::get(opPlus,
                                              Binary::get(opMult,
                                                          Location::regOf(PENT_REG_EAX),
                                                          Const::get(0x100)),
                                              Binary::get(opMult,
                                                          Location::regOf(PENT_REG_ECX),
                                                          Const::get(0x80))),
                                  Const::get(0x40)),
                      Binary::get(opPlus,
                                  Binary::get(opMult,
                                              Location::regOf(PENT_REG_EAX),
                                              Const::get(4)),
                                  Binary::get(opMult,
                                              Location::regOf(PENT_REG_ECX),
                                              Const::get(2))));

        TEST_SIMPLIFY("BinaryDistributeMod",
                      Binary::get(opMod,
                                  Binary::get(opPlus,
                                              Binary::get(opMult,
                                                          Location::regOf(PENT_REG_EAX),
                                                          Const::get(0x100)),
                                              Binary::get(opMult,
                                                          Location::regOf(PENT_REG_ECX),
                                                          Const::get(0x80))),
                                  Const::get(0x40)),
                      Const::get(0));
    }
}

QTEST_GUILESS_MAIN(ExpSimplifierTest)
