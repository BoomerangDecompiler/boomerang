#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BooleanTypeTest.h"


#include "boomerang/ssl/type/BooleanType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/FuncType.h"
#include "boomerang/ssl/type/CharType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/UnionType.h"


void BooleanTypeTest::testConstruct()
{
    BooleanType b1;
    QVERIFY(b1.getSize() == 1);
}


void BooleanTypeTest::testEquals()
{
    QCOMPARE(BooleanType() == BooleanType(), true);
    QCOMPARE(BooleanType() == IntegerType(1), false);
    QCOMPARE(BooleanType() == SizeType(1), false);
}


void BooleanTypeTest::testLess()
{
    QCOMPARE(BooleanType() < FuncType(), false);
    QCOMPARE(BooleanType() < BooleanType(), false);
    QCOMPARE(BooleanType() < CharType(), true);
}


void BooleanTypeTest::testGetCtype()
{
    QCOMPARE(BooleanType().getCtype(true), "bool");
    QCOMPARE(BooleanType().getCtype(false), "bool");
}


void BooleanTypeTest::testIsCompatibleWith()
{
    QCOMPARE(BooleanType().isCompatibleWith(VoidType()), true);
    QCOMPARE(BooleanType().isCompatibleWith(BooleanType()), true);
    QCOMPARE(BooleanType().isCompatibleWith(SizeType(8)), false);
    QCOMPARE(BooleanType().isCompatibleWith(SizeType(1)), true);
    QCOMPARE(BooleanType().isCompatibleWith(*UnionType::get({ BooleanType::get() })), true);
    QCOMPARE(BooleanType().isCompatibleWith(*UnionType::get({ FloatType::get(32) })), false);
}


QTEST_GUILESS_MAIN(BooleanTypeTest)
