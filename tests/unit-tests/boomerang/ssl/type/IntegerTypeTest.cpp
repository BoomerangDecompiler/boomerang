#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "IntegerTypeTest.h"

#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/CharType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/ssl/type/UnionType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/ArrayType.h"


void IntegerTypeTest::testConstruct()
{
    IntegerType i1(32, Sign::Signed);
    QCOMPARE(i1.getSize(), 32);
    QCOMPARE(i1.getSign(), Sign::Signed);
}


void IntegerTypeTest::testEquals()
{
    QCOMPARE(IntegerType(32, Sign::Signed)   == IntegerType(32, Sign::Signed), true);
    QCOMPARE(IntegerType(0,  Sign::Signed)   == IntegerType(32, Sign::Signed), true);
    QCOMPARE(IntegerType(32, Sign::Signed)   == IntegerType(0,  Sign::Signed), true);
    QCOMPARE(IntegerType(32, Sign::Signed)   == IntegerType(16, Sign::Signed), false);
    QCOMPARE(IntegerType(32, Sign::Unsigned) == IntegerType(32, Sign::Signed), false);
    QCOMPARE(IntegerType(32, Sign::SignedStrong) == IntegerType(32, Sign::Signed), true);
    QCOMPARE(IntegerType(32, Sign::UnsignedStrong) == IntegerType(32, Sign::Unsigned), true);
}


void IntegerTypeTest::testLess()
{
    QCOMPARE(IntegerType(32, Sign::Unknown) < CharType(), false);
    QCOMPARE(IntegerType(32, Sign::Unknown) < IntegerType(32, Sign::Unknown), false);
    QCOMPARE(IntegerType(32, Sign::Unknown) < FloatType(32), true);

    QCOMPARE(IntegerType(32, Sign::Unknown) < IntegerType(16, Sign::Unknown), false);
    QCOMPARE(IntegerType(8,  Sign::Unknown) < IntegerType(64, Sign::Unknown), true);

    // treat Sign::SignedStrong the same as Sign::Signed
    QCOMPARE(IntegerType(32, Sign::Signed) < IntegerType(32, Sign::SignedStrong), false);
    QCOMPARE(IntegerType(16, Sign::Unsigned) < IntegerType(16, Sign::Unknown), true);
    QCOMPARE(IntegerType(8, Sign::Unknown) < IntegerType(8, Sign::Signed), true);
    QCOMPARE(IntegerType(16, Sign::UnsignedStrong) < IntegerType(16, Sign::Unsigned), false);
}


void IntegerTypeTest::testIsComplete()
{
    QCOMPARE(IntegerType(0, Sign::Unknown).isComplete(), false);
    QCOMPARE(IntegerType(0, Sign::Signed).isComplete(), false);
    QCOMPARE(IntegerType(32, Sign::Unknown).isComplete(), false);
    QCOMPARE(IntegerType(32, Sign::Unsigned).isComplete(), true);
}


void IntegerTypeTest::testSigned()
{
    IntegerType i1(32, Sign::Unknown);
    QVERIFY(i1.isMaybeSigned());
    QVERIFY(i1.isMaybeUnsigned());
    QVERIFY(!i1.isSigned());
    QVERIFY(!i1.isUnsigned());
    QVERIFY(i1.isSignUnknown());

    i1.hintAsSigned();
    QVERIFY(i1.getSign() == Sign::Signed);
    QVERIFY(i1.isMaybeSigned());
    QVERIFY(!i1.isMaybeUnsigned());
    QVERIFY(i1.isSigned());
    QVERIFY(!i1.isUnsigned());
    QVERIFY(!i1.isSignUnknown());

    i1.hintAsSigned();
    QVERIFY(i1.getSign() == Sign::SignedStrong);
    QVERIFY(i1.isMaybeSigned());
    QVERIFY(!i1.isMaybeUnsigned());
    QVERIFY(i1.isSigned());
    QVERIFY(!i1.isUnsigned());
    QVERIFY(!i1.isSignUnknown());

    i1.hintAsSigned();
    QVERIFY(i1.getSign() == Sign::SignedStrong);

    i1.setSignedness(Sign::Unsigned);
    QVERIFY(i1.getSign() == Sign::Unsigned);
    QVERIFY(!i1.isMaybeSigned());
    QVERIFY(i1.isMaybeUnsigned());
    QVERIFY(!i1.isSigned());
    QVERIFY(i1.isUnsigned());
    QVERIFY(!i1.isSignUnknown());

    i1.hintAsUnsigned();
    QVERIFY(i1.getSign() == Sign::UnsignedStrong);
    QVERIFY(!i1.isMaybeSigned());
    QVERIFY(i1.isMaybeUnsigned());
    QVERIFY(!i1.isSigned());
    QVERIFY(i1.isUnsigned());
    QVERIFY(!i1.isSignUnknown());

    i1.hintAsUnsigned();
    QVERIFY(i1.getSign() == Sign::UnsignedStrong);
}


void IntegerTypeTest::testGetCtype()
{
    QCOMPARE(IntegerType(1, Sign::Unsigned).getCtype(false), "bool");
    QCOMPARE(IntegerType(1, Sign::Unsigned).getCtype(true),  "bool");
    QCOMPARE(IntegerType(2, Sign::Unsigned).getCtype(false), "?unsigned int");
    QCOMPARE(IntegerType(2, Sign::Unsigned).getCtype(true),  "unsigned int");
    QCOMPARE(IntegerType(8, Sign::Unsigned).getCtype(false), "unsigned char");
    QCOMPARE(IntegerType(8, Sign::Unsigned).getCtype(true),  "unsigned char");
    QCOMPARE(IntegerType(16, Sign::Unsigned).getCtype(false), "unsigned short");
    QCOMPARE(IntegerType(16, Sign::Unsigned).getCtype(true),  "unsigned short");
    QCOMPARE(IntegerType(32, Sign::Unsigned).getCtype(false), "unsigned int");
    QCOMPARE(IntegerType(32, Sign::Unsigned).getCtype(true),  "unsigned int");
    QCOMPARE(IntegerType(64, Sign::Unsigned).getCtype(false), "unsigned long long");
    QCOMPARE(IntegerType(64, Sign::Unsigned).getCtype(true),  "unsigned long long");

    QCOMPARE(IntegerType(1, Sign::Unknown).getCtype(false), "/*signed?*/bool");
    QCOMPARE(IntegerType(1, Sign::Unknown).getCtype(true),  "bool");
    QCOMPARE(IntegerType(2, Sign::Unknown).getCtype(false), "/*signed?*/?int");
    QCOMPARE(IntegerType(2, Sign::Unknown).getCtype(true),  "int");
    QCOMPARE(IntegerType(8, Sign::Unknown).getCtype(false), "/*signed?*/char");
    QCOMPARE(IntegerType(8, Sign::Unknown).getCtype(true),  "char");
    QCOMPARE(IntegerType(16, Sign::Unknown).getCtype(false), "/*signed?*/short");
    QCOMPARE(IntegerType(16, Sign::Unknown).getCtype(true),  "short");
    QCOMPARE(IntegerType(32, Sign::Unknown).getCtype(false), "/*signed?*/int");
    QCOMPARE(IntegerType(32, Sign::Unknown).getCtype(true),  "int");
    QCOMPARE(IntegerType(64, Sign::Unknown).getCtype(false), "/*signed?*/long long");
    QCOMPARE(IntegerType(64, Sign::Unknown).getCtype(true),  "long long");

    QCOMPARE(IntegerType(1, Sign::Signed).getCtype(false), "bool");
    QCOMPARE(IntegerType(1, Sign::Signed).getCtype(true),  "bool");
    QCOMPARE(IntegerType(2, Sign::Signed).getCtype(false), "?int");
    QCOMPARE(IntegerType(2, Sign::Signed).getCtype(true),  "int");
    QCOMPARE(IntegerType(8, Sign::Signed).getCtype(false), "char");
    QCOMPARE(IntegerType(8, Sign::Signed).getCtype(true),  "char");
    QCOMPARE(IntegerType(16, Sign::Signed).getCtype(false), "short");
    QCOMPARE(IntegerType(16, Sign::Signed).getCtype(true),  "short");
    QCOMPARE(IntegerType(32, Sign::Signed).getCtype(false), "int");
    QCOMPARE(IntegerType(32, Sign::Signed).getCtype(true),  "int");
    QCOMPARE(IntegerType(64, Sign::Signed).getCtype(false), "long long");
    QCOMPARE(IntegerType(64, Sign::Signed).getCtype(true),  "long long");
}


void IntegerTypeTest::testIsCompatibleWith()
{
    QCOMPARE(IntegerType(32, Sign::Signed).isCompatibleWith(VoidType()), true);
    QCOMPARE(IntegerType(32, Sign::Signed).isCompatibleWith(IntegerType(16, Sign::Unsigned)), true);
    QCOMPARE(IntegerType(32, Sign::Signed).isCompatibleWith(CharType()), true);
    QCOMPARE(IntegerType(32, Sign::Signed).isCompatibleWith(SizeType(32)), true);
    QCOMPARE(IntegerType(32, Sign::Signed).isCompatibleWith(SizeType(16)), false);
    QCOMPARE(IntegerType(32, Sign::Signed).isCompatibleWith(FloatType(32)), false);

    QCOMPARE(IntegerType(32, Sign::Signed).isCompatibleWith(
        UnionType({ IntegerType::get(32, Sign::Signed), FloatType::get(32) })), true);
    QCOMPARE(IntegerType(32, Sign::Signed).isCompatibleWith(
        UnionType({ FloatType::get(32) })), false);

    QCOMPARE(IntegerType(32, Sign::Unsigned).isCompatibleWith(
        ArrayType(IntegerType::get(32, Sign::Unsigned), 10)), true);
}


QTEST_GUILESS_MAIN(IntegerTypeTest)
