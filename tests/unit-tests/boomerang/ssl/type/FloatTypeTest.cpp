#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "FloatTypeTest.h"

#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/UnionType.h"


void FloatTypeTest::testConstruct()
{
    FloatType f1(0);
    QVERIFY(f1.getSize() == 0);

    FloatType f2(32);
    QVERIFY(f2.getSize() == 32);
}


void FloatTypeTest::testEquals()
{
    QVERIFY(FloatType() == FloatType());
    QVERIFY(FloatType(32) != FloatType(64));
    QVERIFY(FloatType(0) == FloatType(64));
    QVERIFY(FloatType(32) == FloatType(0));
    QVERIFY(FloatType(7) == FloatType(7));

    QVERIFY(FloatType(32) != IntegerType(32));
}


void FloatTypeTest::testLess()
{
    QCOMPARE(FloatType(32) < IntegerType(32), false);
    QCOMPARE(FloatType(32) < FloatType(32), false);
    QCOMPARE(FloatType(0) < FloatType(), true);
    QCOMPARE(FloatType(32) < FloatType(64), true);
    QCOMPARE(FloatType() < FloatType(0), false);
    QCOMPARE(FloatType() < *PointerType::get(VoidType::get()), true);
}


void FloatTypeTest::testGetCtype()
{
    QCOMPARE(FloatType().getCtype(true), "double");
    QCOMPARE(FloatType().getCtype(false), "double");
    QCOMPARE(FloatType(32).getCtype(true), "float");
    QCOMPARE(FloatType(32).getCtype(false), "float");
    QCOMPARE(FloatType(80).getCtype(true), "long double");
    QCOMPARE(FloatType(80).getCtype(false), "long double");
    QCOMPARE(FloatType(0).getCtype(true), "__float0");
    QCOMPARE(FloatType(0).getCtype(false), "__float0");
}


void FloatTypeTest::testIsCompatibleWith()
{
    QVERIFY(FloatType().isCompatibleWith(VoidType()));
    QVERIFY(FloatType().isCompatibleWith(FloatType(64)));
    QVERIFY(!FloatType().isCompatibleWith(FloatType(32)));
    QVERIFY(FloatType().isCompatibleWith(*UnionType::get({ FloatType::get(64), IntegerType::get(64) })));
    QVERIFY(!FloatType().isCompatibleWith(*UnionType::get({ IntegerType::get(64) })));
    QVERIFY(FloatType().isCompatibleWith(*ArrayType::get(FloatType::get())));
    QVERIFY(FloatType().isCompatibleWith(SizeType(64)));
    QVERIFY(!FloatType().isCompatibleWith(SizeType(32)));
    QVERIFY(!FloatType().isCompatibleWith(IntegerType(64)));
}


QTEST_GUILESS_MAIN(FloatTypeTest)
