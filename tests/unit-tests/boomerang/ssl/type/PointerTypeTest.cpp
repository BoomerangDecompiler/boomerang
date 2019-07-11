#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PointerTypeTest.h"

#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/CharType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/ssl/type/UnionType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/FuncType.h"


void PointerTypeTest::testConstruct()
{
    PointerType p1(VoidType::get());
    QVERIFY(p1.getPointsTo()->isVoid());
}


void PointerTypeTest::testEquals()
{
    QCOMPARE(PointerType(IntegerType::get(32, Sign::Unsigned)) == PointerType(IntegerType::get(32, Sign::Unsigned)), true);
    QCOMPARE(PointerType(IntegerType::get(32, Sign::Unsigned)) == PointerType(VoidType::get()), false);
    QCOMPARE(PointerType(IntegerType::get(32, Sign::Unsigned)) == IntegerType(32, Sign::Signed), false);

    QCOMPARE(PointerType(PointerType::get(VoidType::get())) == PointerType(PointerType::get(VoidType::get())), true);
    QCOMPARE(PointerType(PointerType::get(VoidType::get())) == PointerType(VoidType::get()), false);
    QCOMPARE(PointerType(VoidType::get()) == PointerType(PointerType::get(VoidType::get())), false);
}


void PointerTypeTest::testLess()
{
    QCOMPARE(PointerType(VoidType::get()) < FloatType(32), false);
    QCOMPARE(PointerType(VoidType::get()) < ArrayType(VoidType::get(), 32), true);

    QCOMPARE(PointerType(VoidType::get()) < PointerType(PointerType::get(VoidType::get())), true);
    QCOMPARE(PointerType(CharType::get()) < PointerType(VoidType::get()), false);
}


void PointerTypeTest::testGetCtype()
{
    QCOMPARE(PointerType(VoidType::get()).getCtype(false), "void *");
    QCOMPARE(PointerType(VoidType::get()).getCtype(true),  "void *");
    QCOMPARE(PointerType(IntegerType::get(32, Sign::Unknown)).getCtype(false), "/*signed?*/int *");
    QCOMPARE(PointerType(IntegerType::get(32, Sign::Unknown)).getCtype(true),  "int *");
    QCOMPARE(PointerType(PointerType::get(VoidType::get())).getCtype(false), "void **");
    QCOMPARE(PointerType(FuncType::get()).getCtype(false), "void (void) *"); // FIXME Should be void (*)(void)
}


void PointerTypeTest::testPointsTo()
{
    QVERIFY(*PointerType::get(VoidType::get())->getPointsTo() == *VoidType::get());
    QVERIFY(*PointerType::get(VoidType::get())->getFinalPointsTo() == *VoidType::get());

    QVERIFY(*PointerType::get(PointerType::get(VoidType::get()))->getPointsTo() == *PointerType::get(VoidType::get()));
    QVERIFY(*PointerType::get(PointerType::get(VoidType::get()))->getFinalPointsTo() == *VoidType::get());
}


void PointerTypeTest::testIsVoidPointer()
{
    QCOMPARE(PointerType::get(VoidType::get())->isVoidPointer(), true);
    QCOMPARE(PointerType::get(PointerType::get(VoidType::get()))->isVoidPointer(), false);
    QCOMPARE(PointerType::get(IntegerType::get(32, Sign::Unsigned))->isVoidPointer(), false);
}


void PointerTypeTest::testGetPointerDepth()
{
    QCOMPARE(PointerType::get(VoidType::get())->getPointerDepth(), 1);
    QCOMPARE(PointerType::get(PointerType::get(VoidType::get()))->getPointerDepth(), 2);
    QCOMPARE(PointerType::get(PointerType::get(FloatType::get(32)))->getPointerDepth(), 2);
}


void PointerTypeTest::testIsCompatibleWith()
{
    QVERIFY(PointerType::get(VoidType::get())->isCompatibleWith(*VoidType::get()));
    QVERIFY(PointerType::get(VoidType::get())->isCompatibleWith(*SizeType::get(STD_SIZE)));
    QVERIFY(!PointerType::get(VoidType::get())->isCompatibleWith(*SizeType::get(64)));
    QVERIFY(!PointerType::get(VoidType::get())->isCompatibleWith(*FloatType::get(32)));
    QVERIFY(!PointerType::get(VoidType::get())->isCompatibleWith(*UnionType::get({ FloatType::get(32) })));
    QVERIFY(PointerType::get(VoidType::get())->isCompatibleWith(*PointerType::get(VoidType::get())));
}



QTEST_GUILESS_MAIN(PointerTypeTest)
