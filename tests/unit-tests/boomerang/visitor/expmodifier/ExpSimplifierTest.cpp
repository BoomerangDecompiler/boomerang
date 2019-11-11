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


#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/exp/TypedExp.h"
#include "boomerang/ssl/type/IntegerType.h"


void ExpSimplifierTest::testSimplify()
{
    QFETCH(SharedExpWrapper, exp);
    QFETCH(SharedExpWrapper, expectedResult);

    SharedExp actualResult = exp->simplify();
    QString actual   = actualResult->toString();
    QString expected = expectedResult->toString();
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
                      Unary::get(opBitNot,
                                 Binary::get(opEquals,
                                             Location::regOf(REG_X86_EAX),
                                             Location::regOf(REG_X86_EDX))),
                      Binary::get(opNotEqual,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_EDX))
        );

        TEST_SIMPLIFY("UnaryNegConst",
                      Unary::get(opNeg, Const::get(0xFF)),
                      Const::get(0xFFFFFF01));

        TEST_SIMPLIFY("UnaryNotConst",
                      Unary::get(opBitNot, Const::get(0xFF)),
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
                      Unary::get(opNeg, Unary::get(opNeg, Location::regOf(REG_X86_EAX))),
                      Location::regOf(REG_X86_EAX));

        TEST_SIMPLIFY("UnaryDoubleNot",
                      Unary::get(opBitNot, Unary::get(opBitNot, Const::get(0x1000))),
                      Const::get(0x1000));

        TEST_SIMPLIFY("UnaryDeMorganBitAnd",
                      Unary::get(opBitNot, Binary::get(opBitAnd,
                                                       Location::regOf(REG_X86_EAX),
                                                       Location::regOf(REG_X86_EDX))),
                      Binary::get(opBitOr,
                                  Unary::get(opBitNot, Location::regOf(REG_X86_EAX)),
                                  Unary::get(opBitNot, Location::regOf(REG_X86_EDX))));

        TEST_SIMPLIFY("UnaryDeMorganBitAnd",
                      Unary::get(opBitNot, Binary::get(opBitOr,
                                                       Location::regOf(REG_X86_EAX),
                                                       Location::regOf(REG_X86_EDX))),
                      Binary::get(opBitAnd,
                                  Unary::get(opBitNot, Location::regOf(REG_X86_EAX)),
                                  Unary::get(opBitNot, Location::regOf(REG_X86_EDX))));

        TEST_SIMPLIFY("UnaryDeMorganLogOr",
                      Unary::get(opLNot, Binary::get(opOr,
                                                     Location::regOf(REG_X86_EAX),
                                                     Location::regOf(REG_X86_EDX))),
                      Binary::get(opAnd,
                                  Unary::get(opLNot, Location::regOf(REG_X86_EAX)),
                                  Unary::get(opLNot, Location::regOf(REG_X86_EDX))));

        TEST_SIMPLIFY("UnaryDeMorganLogAnd",
                      Unary::get(opLNot, Binary::get(opAnd,
                                                     Location::regOf(REG_X86_EAX),
                                                     Location::regOf(REG_X86_EDX))),
                      Binary::get(opOr,
                                  Unary::get(opLNot, Location::regOf(REG_X86_EAX)),
                                  Unary::get(opLNot, Location::regOf(REG_X86_EDX))));

        TEST_SIMPLIFY("UnaryDeMorganBitNotLogOr",
                      Unary::get(opBitNot, Binary::get(opOr,
                                                       Location::regOf(REG_X86_EAX),
                                                       Location::regOf(REG_X86_EDX))),
                      Binary::get(opAnd,
                                  Unary::get(opLNot, Location::regOf(REG_X86_EAX)),
                                  Unary::get(opLNot, Location::regOf(REG_X86_EDX))));
    }

    // Binary
    {
        TEST_SIMPLIFY("BinaryConstPlusConst",
                      Binary::get(opPlus, Const::get(100), Const::get(10)),
                      Const::get(110));

        TEST_SIMPLIFY("BinaryXxorX",
                      Binary::get(opBitXor,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_EAX)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryXandX",
                      Binary::get(opBitAnd,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_EAX)),
                      Location::regOf(REG_X86_EAX));

        TEST_SIMPLIFY("BinaryXorX",
                      Binary::get(opBitOr,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_EAX)),
                      Location::regOf(REG_X86_EAX));


        TEST_SIMPLIFY("BinaryXequalX",
                      Binary::get(opEquals,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_EAX)),
                      Terminal::get(opTrue));

        TEST_SIMPLIFY("BinaryXnotequalX",
                      Binary::get(opNotEqual,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_EAX)),
                      Terminal::get(opFalse));

        TEST_SIMPLIFY("BinaryCommutePlus",
                      Binary::get(opPlus,
                                  Const::get(100),
                                  Location::regOf(REG_X86_EAX)),
                      Binary::get(opPlus,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(100)));

        TEST_SIMPLIFY("BinaryCommuteMults",
                      Binary::get(opMults,
                                  Const::get(100),
                                  Location::regOf(REG_X86_EAX)),
                      Binary::get(opMults,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(100)));

        TEST_SIMPLIFY("BinaryCommuteMult",
                      Binary::get(opMult,
                                  Const::get(100),
                                  Location::regOf(REG_X86_EAX)),
                      Binary::get(opMult,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(100)));

        TEST_SIMPLIFY("BinaryCommuteGlobalAddr",
                      Binary::get(opPlus,
                                  Location::regOf(REG_X86_EAX),
                                  Unary::get(opAddrOf,
                                             RefExp::get(Location::global("test", nullptr), nullptr))),
                      Binary::get(opPlus,
                                  Unary::get(opAddrOf,
                                             RefExp::get(Location::global("test", nullptr), nullptr)),
                                  Location::regOf(REG_X86_EAX)));

        TEST_SIMPLIFY("BinaryCollapseConstPlus",
                      Binary::get(opPlus,
                                  Binary::get(opPlus,
                                              Location::regOf(REG_X86_EAX),
                                              Const::get(50)),
                                  Const::get(100)),
                      Binary::get(opPlus, Location::regOf(REG_X86_EAX), Const::get(150)));

        TEST_SIMPLIFY("BinaryCollapseConstMinus",
                      Binary::get(opPlus,
                                  Binary::get(opMinus,
                                              Location::regOf(REG_X86_EAX),
                                              Const::get(30)),
                                  Const::get(100)),
                      Binary::get(opPlus, Location::regOf(REG_X86_EAX), Const::get(70)));


        TEST_SIMPLIFY("BinaryLinearizeConstMinus",
                      Binary::get(opMinus,
                                  Binary::get(opMults,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_EDX)),
                                  Location::regOf(REG_X86_EAX)),
                      Binary::get(opMults,
                                  Location::regOf(REG_X86_EAX),
                                  Binary::get(opMinus,
                                              Location::regOf(REG_X86_EDX),
                                              Const::get(1))));

        TEST_SIMPLIFY("BinaryLinearizeConstPlus",
                      Binary::get(opPlus,
                                  Location::regOf(REG_X86_EAX),
                                  Binary::get(opMult,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_EDX))),
                      Binary::get(opMult,
                                  Location::regOf(REG_X86_EAX),
                                  Binary::get(opPlus,
                                              Location::regOf(REG_X86_EDX),
                                              Const::get(1))));

        TEST_SIMPLIFY("BinaryChangeAbsConst", // a + (-30) -> a - 30
                      Binary::get(opPlus,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(-30)),
                      Binary::get(opMinus,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(30)));

        TEST_SIMPLIFY("BinaryXplus0",
                      Binary::get(opPlus,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(0)),
                      Location::regOf(REG_X86_EAX));


        TEST_SIMPLIFY("BinaryFalseOrX",
                      Binary::get(opOr,
                                  Terminal::get(opFalse),
                                  Location::regOf(REG_X86_EAX)),
                      Location::regOf(REG_X86_EAX));

        TEST_SIMPLIFY("BinaryXandNull",
                      Binary::get(opBitAnd,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(0)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryXxorNull",
                      Binary::get(opBitXor,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(0)),
                      Location::regOf(REG_X86_EAX));

        TEST_SIMPLIFY("BinaryXandFalse",
                      Binary::get(opAnd,
                                  Location::regOf(REG_X86_EAX),
                                  Terminal::get(opFalse)),
                      Terminal::get(opFalse));

        TEST_SIMPLIFY("BinaryXmultNull",
                      Binary::get(opMult,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(0)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryXdivNull",
                      Binary::get(opDiv,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(0)),
                      Binary::get(opDiv,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(0)));

        TEST_SIMPLIFY("BinaryXmult1",
                      Binary::get(opMult,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(1)),
                      Location::regOf(REG_X86_EAX));

        TEST_SIMPLIFY("BinaryXdivMult",
                      Binary::get(opDiv,
                                  Binary::get(opMult,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX)),
                                  Location::regOf(REG_X86_ECX)),
                      Location::regOf(REG_X86_EAX));

        TEST_SIMPLIFY("BinaryXdiv1",
                      Binary::get(opDiv,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(1)),
                      Location::regOf(REG_X86_EAX));

        TEST_SIMPLIFY("BinaryXmod1",
                      Binary::get(opMod,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(1)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryAXmodX",
                      Binary::get(opMod,
                                  Binary::get(opMult,
                                              Location::regOf(REG_X86_EDX),
                                              Location::regOf(REG_X86_EAX)),
                                  Location::regOf(REG_X86_EAX)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryXmodX",
                      Binary::get(opMod,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_EAX)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryXandMinus1",
                      Binary::get(opBitAnd,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(-1)),
                      Location::regOf(REG_X86_EAX));

        TEST_SIMPLIFY("BinaryXandTrue",
                      Binary::get(opAnd,
                                  Binary::get(opGtr,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX)),
                                  Terminal::get(opTrue)),
                      Binary::get(opGtr,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_ECX)));

        TEST_SIMPLIFY("BinaryXorTrue",
                      Binary::get(opOr,
                                  Binary::get(opGtr,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX)),
                                  Terminal::get(opTrue)),
                      Terminal::get(opTrue));

        TEST_SIMPLIFY("BinaryConstPlusConst",
                      Binary::get(opPlus,
                                  Const::get(10),
                                  Const::get(3)),
                      Const::get(13));

        TEST_SIMPLIFY("BinaryConstMinusConst",
                      Binary::get(opMinus,
                                  Const::get(10),
                                  Const::get(3)),
                      Const::get(7));

        TEST_SIMPLIFY("BinaryConstMultsConst",
                      Binary::get(opMults,
                                  Const::get(10),
                                  Const::get(-3)),
                      Const::get(-30));

        TEST_SIMPLIFY("BinaryConstShlConst",
                      Binary::get(opShL,
                                  Const::get(0x100),
                                  Const::get(4)),
                      Const::get(0x1000));

        TEST_SIMPLIFY("BinaryConstShl32",
                      Binary::get(opShL,
                                  Const::get(0x100),
                                  Const::get(32)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstShrConst",
                      Binary::get(opShR,
                                  Const::get(0x100),
                                  Const::get(4)),
                      Const::get(0x010));

        TEST_SIMPLIFY("BinaryConstShr32",
                      Binary::get(opShR,
                                  Const::get(0x100),
                                  Const::get(32)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstAndConst",
                      Binary::get(opBitAnd,
                                  Const::get(3),
                                  Const::get(6)),
                      Const::get(2));

        TEST_SIMPLIFY("BinaryConstOrConst",
                      Binary::get(opBitOr,
                                  Const::get(3),
                                  Const::get(6)),
                      Const::get(7));

        TEST_SIMPLIFY("BinaryConstXorConst",
                      Binary::get(opBitXor,
                                  Const::get(3),
                                  Const::get(6)),
                      Const::get(5));

        TEST_SIMPLIFY("BinaryConstEqualsConst",
                      Binary::get(opEquals,
                                  Const::get(3),
                                  Const::get(6)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstNotEqualConst",
                      Binary::get(opNotEqual,
                                  Const::get(3),
                                  Const::get(6)),
                      Const::get(1));

        TEST_SIMPLIFY("BinaryConstLessConst",
                      Binary::get(opLess,
                                  Const::get(-3),
                                  Const::get(6)),
                      Const::get(1));

        TEST_SIMPLIFY("BinaryConstGtrConst",
                      Binary::get(opGtr,
                                  Const::get(-3),
                                  Const::get(6)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstLeqConst",
                      Binary::get(opLessEq,
                                  Const::get(-3),
                                  Const::get(6)),
                      Const::get(1));

        TEST_SIMPLIFY("BinaryConstGeqConst",
                      Binary::get(opGtrEq,
                                  Const::get(-3),
                                  Const::get(6)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstLessUnsConst",
                      Binary::get(opLessUns,
                                  Const::get(3),
                                  Const::get(6)),
                      Const::get(1));

        TEST_SIMPLIFY("BinaryConstGtrUnsConst",
                      Binary::get(opGtrUns,
                                  Const::get(3),
                                  Const::get(6)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstLeqUnsConst",
                      Binary::get(opLessEqUns,
                                  Const::get(3),
                                  Const::get(6)),
                      Const::get(1));

        TEST_SIMPLIFY("BinaryConstGeqUnsConst",
                      Binary::get(opGtrEqUns,
                                  Const::get(3),
                                  Const::get(6)),
                      Const::get(0));

        TEST_SIMPLIFY("BinaryConstSar0",
                      Binary::get(opShRA,
                                  Const::get(5),
                                  Const::get(0)),
                      Const::get(5));

        TEST_SIMPLIFY("BinaryConstSar32",
                      Binary::get(opShRA,
                                  Const::get(-1),
                                  Const::get(32)),
                      Const::get(-1));

        TEST_SIMPLIFY("BinaryConstSarConst",
                      Binary::get(opShRA,
                                  Const::get(-256),
                                  Const::get(5)),
                      Const::get(-8));

        TEST_SIMPLIFY("BinaryPosConstSarConst",
                      Binary::get(opShRA,
                                  Const::get(256),
                                  Const::get(5)),
                      Const::get(8));

        TEST_SIMPLIFY("BinaryMinus1SarConst",
                      Binary::get(opShRA,
                                  Const::get(-1),
                                  Const::get(1)),
                      Const::get(-1));

        TEST_SIMPLIFY("BinaryConstDivsConst",
                      Binary::get(opDivs,
                                  Const::get(-3),
                                  Const::get(2)),
                      Const::get(-1));

        TEST_SIMPLIFY("BinaryConstDivs0",
                      Binary::get(opDivs,
                                  Const::get(-10),
                                  Const::get(0)),
                      Binary::get(opDivs,
                                  Const::get(-10),
                                  Const::get(0)));

        TEST_SIMPLIFY("BinaryConstModsConst",
                      Binary::get(opMods,
                                  Const::get(-3),
                                  Const::get(2)),
                      Const::get(-1));

        TEST_SIMPLIFY("BinaryConstMods0",
                      Binary::get(opMods,
                                  Const::get(-10),
                                  Const::get(0)),
                      Binary::get(opMods,
                                  Const::get(-10),
                                  Const::get(0)));

        TEST_SIMPLIFY("BinaryConstDivConst",
                      Binary::get(opDiv,
                                  Const::get(3),
                                  Const::get(2)),
                      Const::get(1));

        TEST_SIMPLIFY("BinaryConstDiv0",
                      Binary::get(opDiv,
                                  Const::get(10),
                                  Const::get(0)),
                      Binary::get(opDiv,
                                  Const::get(10),
                                  Const::get(0)));

        TEST_SIMPLIFY("BinaryConstModConst",
                      Binary::get(opMod,
                                  Const::get(3),
                                  Const::get(2)),
                      Const::get(1));

        TEST_SIMPLIFY("BinaryConstMod0",
                      Binary::get(opMod,
                                  Const::get(10),
                                  Const::get(0)),
                      Binary::get(opMod,
                                  Const::get(10),
                                  Const::get(0)));

        TEST_SIMPLIFY("BinaryNegCmpPos",
                      Binary::get(opLess,
                                  Unary::get(opNeg, Location::regOf(REG_X86_EAX)),
                                  Location::regOf(REG_X86_ECX)),
                      Binary::get(opLess,
                                  Location::regOf(REG_X86_EAX),
                                  Unary::get(opNeg, Location::regOf(REG_X86_ECX))));

        TEST_SIMPLIFY("BinaryXplusYless0",
                      Binary::get(opLess,
                                  Binary::get(opPlus,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX)),
                                  Const::get(0)),
                      Binary::get(opLess,
                                  Location::regOf(REG_X86_EAX),
                                  Unary::get(opNeg, Location::regOf(REG_X86_ECX))));

        TEST_SIMPLIFY("BinaryXminusYequal0",
                      Binary::get(opEquals,
                                  Binary::get(opMinus,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX)),
                                  Const::get(0)),
                      Binary::get(opEquals,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_ECX)));

        TEST_SIMPLIFY("BinaryXplusNegConstEqual0",
                      Binary::get(opEquals,
                                  Binary::get(opPlus,
                                              Location::regOf(REG_X86_EAX),
                                              Const::get(-10)),
                                  Const::get(0)),
                      Binary::get(opEquals,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(10)));

        TEST_SIMPLIFY("BinaryUnsignedLess0",
                      Binary::get(opLessEqUns,
                                  Const::get(0),
                                  Const::get(REG_X86_EAX)),
                      Const::get(1));

        TEST_SIMPLIFY("BinaryUnsignedLessEqual0",
                      Binary::get(opLessUns,
                                  Const::get(0),
                                  Location::regOf(REG_X86_EAX)),
                      Binary::get(opNotEqual,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(0)));

        TEST_SIMPLIFY("BinaryDoubleEquality0",
                      Binary::get(opEquals,
                                  Binary::get(opEquals,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX)),
                                  Const::get(0)),
                      Binary::get(opNotEqual,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_ECX)));

        TEST_SIMPLIFY("BinaryDoubleEquality1",
                      Binary::get(opEquals,
                                  Binary::get(opEquals,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX)),
                                 Const::get(1)),
                      Binary::get(opEquals,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_ECX)));

        TEST_SIMPLIFY("BinaryDoubleEquality2",
                      Binary::get(opEquals,
                                  Binary::get(opEquals,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX)),
                                  Const::get(2)),
                      Terminal::get(opFalse));

        TEST_SIMPLIFY("BinaryDoubleNotEquality0",
                      Binary::get(opNotEqual,
                                  Binary::get(opEquals,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX)),
                                  Const::get(0)),
                      Binary::get(opEquals,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_ECX)));

        TEST_SIMPLIFY("BinaryDoubleNotEquality1",
                      Binary::get(opNotEqual,
                                  Binary::get(opEquals,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX)),
                                  Const::get(1)),
                      Binary::get(opNotEqual,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_ECX)));

        TEST_SIMPLIFY("BinaryDoubleNotEquality2",
                      Binary::get(opNotEqual,
                                  Binary::get(opEquals,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX)),
                                  Const::get(2)),
                      Terminal::get(opTrue));

        TEST_SIMPLIFY("Binary0minusXnotequal0",
                      Binary::get(opNotEqual,
                                  Binary::get(opMinus,
                                              Const::get(0),
                                              Location::regOf(REG_X86_EAX)),
                                  Const::get(0)),
                      Binary::get(opNotEqual,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(0)));

        TEST_SIMPLIFY("BinaryXcompareYequal0",
                      Binary::get(opEquals,
                                  Binary::get(opLess,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX)),
                                  Const::get(0)),
                      Binary::get(opGtrEq,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_ECX)));

        TEST_SIMPLIFY("BinaryEqualLessEqual",
                      Binary::get(opOr,
                                  Binary::get(opLessEq,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX)),
                                  Binary::get(opEquals,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX))),
                      Binary::get(opLessEq,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_ECX)));

        TEST_SIMPLIFY("BinaryGtrEqual",
                      Binary::get(opOr,
                                  Binary::get(opGtr,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX)),
                                  Binary::get(opEquals,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX))),
                      Binary::get(opGtrEq,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_ECX)));

        TEST_SIMPLIFY("BinaryAndRecurse",
                      Binary::get(opAnd,
                                  Binary::get(opOr,
                                              Binary::get(opGtr,
                                                          Location::regOf(REG_X86_EAX),
                                                          Location::regOf(REG_X86_ECX)),
                                              Binary::get(opEquals,
                                                          Location::regOf(REG_X86_EAX),
                                                          Location::regOf(REG_X86_ECX))),
                                  Binary::get(opGtrEq,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_ECX))),
                      Binary::get(opGtrEq,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_ECX)));

        TEST_SIMPLIFY("BinaryDoubleMultConst",
                      Binary::get(opMult,
                                  Binary::get(opMult,
                                              Location::regOf(REG_X86_EAX),
                                              Const::get(0x100)),
                                  Const::get(0x010)),
                      Binary::get(opMult,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(0x1000)));

        TEST_SIMPLIFY("BinaryFloatNullMinusX",
                      Binary::get(opFMinus,
                                  Const::get(0.0f),
                                  Location::regOf(REG_X86_ST0)),
                      Unary::get(opFNeg, Location::regOf(REG_X86_ST0)));

        TEST_SIMPLIFY("BinaryDistributeDivision",
                      Binary::get(opDiv,
                                  Binary::get(opPlus,
                                              Binary::get(opMult,
                                                          Location::regOf(REG_X86_EAX),
                                                          Const::get(0x100)),
                                              Binary::get(opMult,
                                                          Location::regOf(REG_X86_ECX),
                                                          Const::get(0x80))),
                                  Const::get(0x40)),
                      Binary::get(opPlus,
                                  Binary::get(opMult,
                                              Location::regOf(REG_X86_EAX),
                                              Const::get(4)),
                                  Binary::get(opMult,
                                              Location::regOf(REG_X86_ECX),
                                              Const::get(2))));

        TEST_SIMPLIFY("BinaryDistributeModLeft",
                      Binary::get(opMod,
                                  Binary::get(opPlus,
                                              Binary::get(opMult,
                                                          Location::regOf(REG_X86_EAX),
                                                          Const::get(0x100)),
                                              Binary::get(opMult,
                                                          Location::regOf(REG_X86_ECX),
                                                          Const::get(0x70))),
                                  Const::get(0x40)),
                      Binary::get(opMod,
                                  Binary::get(opMult,
                                              Location::regOf(REG_X86_ECX),
                                              Const::get(0x70)),
                                  Const::get(0x40)));

        TEST_SIMPLIFY("BinaryDistributeModRight",
                      Binary::get(opMod,
                                  Binary::get(opPlus,
                                              Binary::get(opMult,
                                                          Location::regOf(REG_X86_EAX),
                                                          Const::get(0x70)),
                                              Binary::get(opMult,
                                                          Location::regOf(REG_X86_ECX),
                                                          Const::get(0x60))),
                                  Const::get(0x30)),
                      Binary::get(opMod,
                                  Binary::get(opMult,
                                              Location::regOf(REG_X86_EAX),
                                              Const::get(0x70)),
                                  Const::get(0x30)));

        TEST_SIMPLIFY("BinaryDistributeModBoth",
                      Binary::get(opMod,
                                  Binary::get(opPlus,
                                              Binary::get(opMult,
                                                          Location::regOf(REG_X86_EAX),
                                                          Const::get(0x100)),
                                              Binary::get(opMult,
                                                          Location::regOf(REG_X86_ECX),
                                                          Const::get(0x80))),
                                  Const::get(0x40)),
                      Const::get(0));

        TEST_SIMPLIFY("BinarySimplifyOrNotEqual1",
                      Binary::get(opOr,
                                  Binary::get(opNotEqual,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_EDX)),
                                  Binary::get(opLess,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_EDX))),
                      Binary::get(opLess,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_EDX)));

        TEST_SIMPLIFY("BinarySimplifyOrNotEqual2",
                      Binary::get(opOr,
                                  Binary::get(opLess,
                                              Location::regOf(REG_X86_EAX),
                                              Location::regOf(REG_X86_EDX)),
                                  Binary::get(opNotEqual,
                                            Location::regOf(REG_X86_EAX),
                                            Location::regOf(REG_X86_EDX))),

                      Binary::get(opLess,
                                  Location::regOf(REG_X86_EAX),
                                  Location::regOf(REG_X86_EDX)));

        TEST_SIMPLIFY("BinaryComplexBitAnd",
                      Binary::get(opBitAnd,
                                  Binary::get(opMinus,
                                              Const::get(0),
                                              Binary::get(opLessEqUns,
                                                          Const::get(0),
                                                          Location::regOf(REG_X86_ECX))),
                                  Location::regOf(REG_X86_EAX)),
                      Location::regOf(REG_X86_EAX));
    }


    // Ternary
    {
        TEST_SIMPLIFY("TernToBool",
                      Ternary::get(opTern,
                                   Location::regOf(REG_X86_EAX),
                                   Const::get(1),
                                   Const::get(0)),
                      Binary::get(opNotEqual,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(0)));

        TEST_SIMPLIFY("TernToNotBool",
                      Ternary::get(opTern,
                                   Location::regOf(REG_X86_EAX),
                                   Const::get(0),
                                   Const::get(1)),
                      Binary::get(opEquals,
                                  Location::regOf(REG_X86_EAX),
                                  Const::get(0)));

        TEST_SIMPLIFY("TernConst0",
                      Ternary::get(opTern,
                                   Const::get(0),
                                   Location::regOf(REG_X86_EAX),
                                   Location::regOf(REG_X86_ECX)),
                      Location::regOf(REG_X86_ECX));

        TEST_SIMPLIFY("TernConst1",
                      Ternary::get(opTern,
                                   Const::get(1),
                                   Location::regOf(REG_X86_EAX),
                                   Location::regOf(REG_X86_ECX)),
                      Location::regOf(REG_X86_EAX));

        TEST_SIMPLIFY("TernConst2",
                      Ternary::get(opTern,
                                   Const::get(1),
                                   Location::regOf(REG_X86_EAX),
                                   Location::regOf(REG_X86_ECX)),
                      Location::regOf(REG_X86_EAX));

        TEST_SIMPLIFY("TernNoChoice",
                      Ternary::get(opTern,
                                   Binary::get(opEquals,
                                               Location::regOf(REG_X86_ECX),
                                               Const::get(0)),
                                   Location::regOf(REG_X86_EAX),
                                   Location::regOf(REG_X86_EAX)),
                      Location::regOf(REG_X86_EAX));


        TEST_SIMPLIFY("SgnExConst0To32",
                      Ternary::get(opSgnEx,
                                   Const::get(0),
                                   Const::get(32),
                                   Const::get(-10)),
                      Const::get(0));

        TEST_SIMPLIFY("SgnExConst8To32",
                      Ternary::get(opSgnEx,
                                   Const::get(8),
                                   Const::get(32),
                                   Const::get(-10)),
                      Const::get(-10));

        TEST_SIMPLIFY("SgnExConst8To32_16",
                      Ternary::get(opSgnEx,
                                   Const::get(8),
                                   Const::get(32),
                                   Const::get(0xFF7F)),
                      Const::get(0x7F));

        TEST_SIMPLIFY("SgnExConst16To32Pos",
                      Ternary::get(opSgnEx,
                                   Const::get(16),
                                   Const::get(32),
                                   Const::get(10)),
                      Const::get(10));

        TEST_SIMPLIFY("SgnExConst16To32Neg",
                      Ternary::get(opSgnEx,
                                   Const::get(16),
                                   Const::get(32),
                                   Const::get(0xFFFE)),
                      Const::get(-2));

        TEST_SIMPLIFY("SgnExConst32To16Neg",
                      Ternary::get(opSgnEx,
                                   Const::get(32),
                                   Const::get(16),
                                   Const::get(-1)),
                      Const::get(-1));

        TEST_SIMPLIFY("SgnExConst32To64Neg",
                      Ternary::get(opSgnEx,
                                   Const::get(32),
                                   Const::get(64),
                                   Const::get(-1)),
                      Const::get((uint64_t)-1));


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

        TEST_SIMPLIFY("AtConst",
                      Ternary::get(opAt,
                                   Const::get(5),
                                   Const::get(1),
                                   Const::get(2)),
                      Const::get(4));
    }

    // TypedExp
    {
        TEST_SIMPLIFY("TypedExp",
                      TypedExp::get(IntegerType::get(32, Sign::Signed), Location::memOf(Const::get(0x1000), nullptr)),
                      TypedExp::get(IntegerType::get(32, Sign::Signed), Location::memOf(Const::get(0x1000), nullptr)));

        TEST_SIMPLIFY("TypedExpRegOf",
                      TypedExp::get(IntegerType::get(32, Sign::Signed), Location::regOf(REG_X86_EAX)),
                      Location::regOf(REG_X86_EAX));
    }


    // Location
    {
        TEST_SIMPLIFY("LocMemOfAddrOf",
                      Location::memOf(Unary::get(opAddrOf, Const::get(0x10)), nullptr),
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
