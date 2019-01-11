#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "TypeTest.h"

#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/BooleanType.h"
#include "boomerang/ssl/type/CharType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/ssl/type/IntegerType.h"


void TypeTest::testNotEqual()
{
    QCOMPARE(*VoidType::get() != *VoidType::get(), false);
    QCOMPARE(*VoidType::get() != *FloatType::get(32), true);
}


void TypeTest::testIsCString()
{
    QCOMPARE(FloatType::get(32)->isCString(), false);
    QCOMPARE(PointerType::get(VoidType::get())->isCString(), false);
    QCOMPARE(PointerType::get(PointerType::get(CharType::get()))->isCString(), false);
    QCOMPARE(PointerType::get(CharType::get())->isCString(), true);

    QCOMPARE(ArrayType::get(VoidType::get())->isCString(), false);
    QCOMPARE(ArrayType::get(FloatType::get(32))->isCString(), false);
    QCOMPARE(ArrayType::get(CharType::get())->isCString(), true);

    QCOMPARE(ArrayType::get(VoidType::get(), 16)->isCString(), false);
    QCOMPARE(ArrayType::get(FloatType::get(32), 20)->isCString(), false);
    QCOMPARE(ArrayType::get(CharType::get(), 10)->isCString(), true);
}


void TypeTest::testNewIntegerLikeType()
{
    QVERIFY(*Type::newIntegerLikeType(0, Sign::Unknown)  == *IntegerType::get(0, Sign::Unknown));
    QVERIFY(*Type::newIntegerLikeType(1, Sign::Unknown)  == *BooleanType::get());
    QVERIFY(*Type::newIntegerLikeType(8, Sign::Unknown)  == *CharType::get());
    QVERIFY(*Type::newIntegerLikeType(8, Sign::Signed)   == *CharType::get());
    QVERIFY(*Type::newIntegerLikeType(8, Sign::Unsigned) == *IntegerType::get(8, Sign::Unsigned));
    QVERIFY(*Type::newIntegerLikeType(64, Sign::Unsigned) == *IntegerType::get(64, Sign::Unsigned));
}



QTEST_GUILESS_MAIN(TypeTest)
