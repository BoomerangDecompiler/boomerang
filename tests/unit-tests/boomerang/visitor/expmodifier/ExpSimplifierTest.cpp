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
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/TypedExp.h"
#include "boomerang/type/type/IntegerType.h"


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

        TEST_SIMPLIFY("UnaryDoubleNeg",
                      Unary::get(opNeg, Unary::get(opNeg, Location::regOf(PENT_REG_EAX))),
                      Location::regOf(PENT_REG_EAX));

        TEST_SIMPLIFY("UnaryDoubleNot",
                      Unary::get(opNot, Unary::get(opNot, Const::get(0x1000))),
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

        TEST_SIMPLIFY("BinaryXorX",
                      Binary::get(opBitOr,
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

        TEST_SIMPLIFY("BinaryCommuteGlobalAddr",
                      Binary::get(opPlus,
                                  Location::regOf(PENT_REG_EAX),
                                  Unary::get(opAddrOf,
                                             RefExp::get(Location::global("test", nullptr), nullptr))),
                      Binary::get(opPlus,
                                  Unary::get(opAddrOf,
                                             RefExp::get(Location::global("test", nullptr), nullptr)),
                                  Location::regOf(PENT_REG_EAX)));

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

        TEST_SIMPLIFY("BinaryChangeAbsConst", // a + (-30) -> a - 30
                      Binary::get(opPlus,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(-30)),
                      Binary::get(opMinus,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(30)));

        TEST_SIMPLIFY("BinaryXplus0",
                      Binary::get(opPlus,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(0)),
                      Location::regOf(PENT_REG_EAX));


        TEST_SIMPLIFY("BinaryFalseOrX",
                      Binary::get(opOr,
                                  Terminal::get(opFalse),
                                  Location::regOf(PENT_REG_EAX)),
                      Location::regOf(PENT_REG_EAX));

        TEST_SIMPLIFY("BinaryXandNull",
                      Binary::get(opBitAnd,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(0)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryXandFalse",
                      Binary::get(opAnd,
                                  Location::regOf(PENT_REG_EAX),
                                  Terminal::get(opFalse)),
                      Terminal::get(opFalse));

        TEST_SIMPLIFY("BinaryXmultNull",
                      Binary::get(opMult,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(0)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryXmult1",
                      Binary::get(opMult,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(1)),
                      Location::regOf(PENT_REG_EAX));

        TEST_SIMPLIFY("BinaryXdivMult",
                      Binary::get(opDiv,
                                  Binary::get(opMult,
                                              Location::regOf(PENT_REG_EAX),
                                              Location::regOf(PENT_REG_ECX)),
                                  Location::regOf(PENT_REG_ECX)),
                      Location::regOf(PENT_REG_EAX));

        TEST_SIMPLIFY("BinaryXdiv1",
                      Binary::get(opDiv,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(1)),
                      Location::regOf(PENT_REG_EAX));

        TEST_SIMPLIFY("BinaryXmod1",
                      Binary::get(opMod,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(1)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryAXmodX",
                      Binary::get(opMod,
                                  Binary::get(opMult,
                                              Location::regOf(PENT_REG_EDX),
                                              Location::regOf(PENT_REG_EAX)),
                                  Location::regOf(PENT_REG_EAX)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryXmodX",
                      Binary::get(opMod,
                                  Location::regOf(PENT_REG_EAX),
                                  Location::regOf(PENT_REG_EAX)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstShrConst",
                      Binary::get(opShiftR,
                                  Const::get(0x100),
                                  Const::get(4)),
                      Const::get(0x010));

        TEST_SIMPLIFY("BinaryConstShr32",
                      Binary::get(opShiftR,
                                  Const::get(0x100),
                                  Const::get(32)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstShlConst",
                      Binary::get(opShiftL,
                                  Const::get(0x100),
                                  Const::get(4)),
                      Const::get(0x1000));

        TEST_SIMPLIFY("BinaryConstShl32",
                      Binary::get(opShiftL,
                                  Const::get(0x100),
                                  Const::get(32)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstSarConst",
                      Binary::get(opShiftRA,
                                  Const::get(-256),
                                  Const::get(5)),
                      Const::get(-8));

        TEST_SIMPLIFY("BinaryDoubleEquality",
                      Binary::get(opEquals,
                                  Binary::get(opEquals,
                                              Location::regOf(PENT_REG_EAX),
                                              Location::regOf(PENT_REG_ECX)),
                                 Const::get(1)),
                      Binary::get(opEquals,
                                  Location::regOf(PENT_REG_EAX),
                                  Location::regOf(PENT_REG_ECX)));

        TEST_SIMPLIFY("BinaryEqualLessEqual",
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

        TEST_SIMPLIFY("BinaryGtrEqual",
                      Binary::get(opOr,
                                  Binary::get(opGtr,
                                              Location::regOf(PENT_REG_EAX),
                                              Location::regOf(PENT_REG_ECX)),
                                  Binary::get(opEquals,
                                              Location::regOf(PENT_REG_EAX),
                                              Location::regOf(PENT_REG_ECX))),
                      Binary::get(opGtrEq,
                                  Location::regOf(PENT_REG_EAX),
                                  Location::regOf(PENT_REG_ECX)));


        TEST_SIMPLIFY("BinaryDoubleMultConst",
                      Binary::get(opMult,
                                  Binary::get(opMult,
                                              Location::regOf(PENT_REG_EAX),
                                              Const::get(0x100)),
                                  Const::get(0x010)),
                      Binary::get(opMult,
                                  Location::regOf(PENT_REG_EAX),
                                  Const::get(0x1000)));

        TEST_SIMPLIFY("BinaryFloatNullMinusX",
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


    // Ternary
    {
        TEST_SIMPLIFY("TernToBool",
                      Ternary::get(opTern,
                                   Location::regOf(PENT_REG_EAX),
                                   Const::get(1),
                                   Const::get(0)),
                      Location::regOf(PENT_REG_EAX)); // or %eax != 0

        TEST_SIMPLIFY("TernConst0",
                      Ternary::get(opTern,
                                   Const::get(0),
                                   Location::regOf(PENT_REG_EAX),
                                   Location::regOf(PENT_REG_ECX)),
                      Location::regOf(PENT_REG_ECX));

        TEST_SIMPLIFY("TernConst1",
                      Ternary::get(opTern,
                                   Const::get(1),
                                   Location::regOf(PENT_REG_EAX),
                                   Location::regOf(PENT_REG_ECX)),
                      Location::regOf(PENT_REG_EAX));

        TEST_SIMPLIFY("TernNoChoice",
                      Ternary::get(opTern,
                                   Binary::get(opEquals,
                                               Location::regOf(PENT_REG_ECX),
                                               Const::get(0)),
                                   Location::regOf(PENT_REG_EAX),
                                   Location::regOf(PENT_REG_EAX)),
                      Location::regOf(PENT_REG_EAX));

        TEST_SIMPLIFY("SgnExConst",
                      Ternary::get(opSgnEx,
                                   Const::get(8),
                                   Const::get(32),
                                   Const::get(-10)),
                      Const::get(-10));


        /// TODO What about zfill(8, 32, -10) ?
        TEST_SIMPLIFY("ZFillConst",
                      Ternary::get(opZfill,
                                   Const::get(8),
                                   Const::get(32),
                                   Const::get(10)),
                      Const::get(10));

        TEST_SIMPLIFY("FSizeFloatConst",
                      Ternary::get(opFsize,
                                   Const::get(32),
                                   Const::get(80),
                                   Const::get(5.0f)),
                      Const::get(5.0f));

        TEST_SIMPLIFY("TruncuConst",
                      Ternary::get(opTruncu,
                                   Const::get(32),
                                   Const::get(16),
                                   Const::get(0x12345678)),
                      Const::get(0x00005678));

        TEST_SIMPLIFY("TruncsConst",
                      Ternary::get(opTruncs,
                                   Const::get(32),
                                   Const::get(16),
                                   Const::get((int)0xF000F)),
                      Const::get(15));
    }

    // TypedExp
    {
        TEST_SIMPLIFY("TypedExp",
                      std::make_shared<TypedExp>(IntegerType::get(32, 1), Location::memOf(Const::get(0x1000), nullptr)),
                      std::make_shared<TypedExp>(IntegerType::get(32, 1), Location::memOf(Const::get(0x1000), nullptr)));

        TEST_SIMPLIFY("TypedExpRegOf",
                      std::make_shared<TypedExp>(IntegerType::get(32, 1), Location::regOf(PENT_REG_EAX)),
                      Location::regOf(PENT_REG_EAX));
    }


    // Location
    {
        TEST_SIMPLIFY("LocAddrOfMemOf",
                      Location::memOf(Unary::get(opAddrOf, Const::get(0x10)), nullptr),
                      Const::get(0x10));

        TEST_SIMPLIFY("LocMemOfAddrOf",
                      Unary::get(opAddrOf, Location::memOf(Const::get(0x10), nullptr)),
                      Const::get(0x10));
    }


    // RefExp
    {
        TEST_SIMPLIFY("RefFirstDF",
                      RefExp::get(Terminal::get(opDF), nullptr),
                      Const::get(0));
    }
}

QTEST_GUILESS_MAIN(ExpSimplifierTest)
