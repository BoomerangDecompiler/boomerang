#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CharTypeTest.h"

#include "boomerang/ssl/type/CharType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/UnionType.h"
#include "boomerang/ssl/type/BooleanType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/ArrayType.h"


void CharTypeTest::testConstruct()
{
    CharType c1;
    QVERIFY(c1.getSize() == 8);
}


void CharTypeTest::testEquals()
{
    QCOMPARE(CharType() == CharType(), true);
    QCOMPARE(CharType() == VoidType(), false);
    QCOMPARE(CharType() == SizeType(8), false);
}


void CharTypeTest::testLess()
{
    QCOMPARE(CharType() < CharType(), false);
    QCOMPARE(CharType() < BooleanType(), false);
    QCOMPARE(CharType() < IntegerType(8), true);
}


void CharTypeTest::testGetCtype()
{
    QCOMPARE(CharType().getCtype(true), "char");
    QCOMPARE(CharType().getCtype(false), "char");
}


void CharTypeTest::testIsCompatibleWith()
{
    QVERIFY(CharType().isCompatibleWith(VoidType()));
    QVERIFY(CharType().isCompatibleWith(CharType()));
    QVERIFY(CharType().isCompatibleWith(IntegerType(4)));
    QVERIFY(CharType().isCompatibleWith(IntegerType(8)));
    QVERIFY(CharType().isCompatibleWith(IntegerType(32)));
    QVERIFY(CharType().isCompatibleWith(SizeType(8)));
    QVERIFY(!CharType().isCompatibleWith(SizeType(32)));
    QVERIFY(CharType().isCompatibleWith(UnionType({ CharType::get() })));
    QVERIFY(CharType().isCompatibleWith(UnionType({ CharType::get(), SizeType::get(32) })));
    QVERIFY(!CharType().isCompatibleWith(UnionType({ SizeType::get(32) })));
    QVERIFY(CharType().isCompatibleWith(ArrayType(CharType::get())));
    QVERIFY(!CharType().isCompatibleWith(ArrayType(SizeType::get(32))));
}


QTEST_GUILESS_MAIN(CharTypeTest)
