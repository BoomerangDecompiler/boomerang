#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ArrayTypeTest.h"


#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/NamedType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/UnionType.h"
#include "boomerang/ssl/type/BooleanType.h"


void ArrayTypeTest::testConstruct()
{
    ArrayType a1(VoidType::get());
    QVERIFY(*a1.getBaseType() == *VoidType::get());
    QCOMPARE(a1.getLength(), ARRAY_UNBOUNDED);
    QCOMPARE(a1.getSize(), 0); // because VoidType size is 0

    ArrayType a2(VoidType::get(), 10);
    QVERIFY(*a2.getBaseType() == *VoidType::get());
    QCOMPARE(a2.getLength(), 10);
    QCOMPARE(a2.getSize(), 0); // because VoidType size is 0

    ArrayType a3(IntegerType::get(32), 20);
    QVERIFY(*a3.getBaseType() == *IntegerType::get(32));
    QCOMPARE(a3.getLength(), 20);
    QCOMPARE(a3.getSize(), 20 * 32);
}


void ArrayTypeTest::testEqual()
{
    QCOMPARE(ArrayType(VoidType::get())          == VoidType(),                              false);
    QCOMPARE(ArrayType(VoidType::get(), 30)      == VoidType(),                              false);

    QCOMPARE(ArrayType(VoidType::get())          == ArrayType(VoidType::get()),              true);
    QCOMPARE(ArrayType(IntegerType::get(32))     == ArrayType(IntegerType::get(32), 10),     false);
    QCOMPARE(ArrayType(IntegerType::get(32), 10) == ArrayType(IntegerType::get(16), 20),     false);
    QCOMPARE(ArrayType(FloatType::get(32), 5)    == ArrayType(FloatType::get(32), 5),        true);
}


void ArrayTypeTest::testLess()
{
    QCOMPARE(ArrayType(VoidType::get()) < PointerType(VoidType::get()), false);
    QCOMPARE(ArrayType(VoidType::get()) < NamedType("foo"),             true);

    QCOMPARE(ArrayType(VoidType::get()) < ArrayType(IntegerType::get(32)), true);
    QCOMPARE(ArrayType(IntegerType::get(32)) < ArrayType(IntegerType::get(32)), false);
    QCOMPARE(ArrayType(IntegerType::get(32), 10) < ArrayType(IntegerType::get(32), 20), true);
    QCOMPARE(ArrayType(IntegerType::get(32)) < ArrayType(IntegerType::get(32), 10), false);
}


void ArrayTypeTest::testIsCompatibleWith()
{
    QVERIFY(ArrayType(VoidType::get()).isCompatibleWith(*VoidType::get()));
    QVERIFY(ArrayType(IntegerType::get(32)).isCompatibleWith(*ArrayType::get(IntegerType::get(32))));
    QVERIFY(ArrayType(IntegerType::get(32)).isCompatibleWith(*UnionType::get({ IntegerType::get(32), FloatType::get(32) })));
    QVERIFY(!ArrayType(IntegerType::get(32)).isCompatibleWith(*FloatType::get(32)));
    QVERIFY(ArrayType(IntegerType::get(32)).isCompatibleWith(*IntegerType::get(32), false));
    QVERIFY(!ArrayType(IntegerType::get(32)).isCompatibleWith(*IntegerType::get(32), true));
}


void ArrayTypeTest::testGetSize()
{
    QCOMPARE(ArrayType(VoidType::get()).getSize(), 0);
    QCOMPARE(ArrayType(BooleanType::get()).getSize(), ARRAY_UNBOUNDED);
    QCOMPARE(ArrayType(IntegerType::get(2)).getSize(), ARRAY_UNBOUNDED * 2);
    QCOMPARE(ArrayType(IntegerType::get(32), 10).getSize(), 32 * 10);
}


void ArrayTypeTest::testGetCtype()
{
    QCOMPARE(ArrayType(VoidType::get()).getCtype(), "void[]");
    QCOMPARE(ArrayType(IntegerType::get(32, Sign::Unsigned)).getCtype(), "unsigned int[]");
    QCOMPARE(ArrayType(IntegerType::get(32, Sign::Unsigned), 10).getCtype(), "unsigned int[10]");
}


void ArrayTypeTest::testIsUnbounded()
{
    QVERIFY(ArrayType(VoidType::get()).isUnbounded());
    QVERIFY(ArrayType(IntegerType::get(32, Sign::Unsigned)).isUnbounded());
    QVERIFY(!ArrayType(IntegerType::get(32, Sign::Unsigned), 10).isUnbounded());
}

QTEST_GUILESS_MAIN(ArrayTypeTest)
