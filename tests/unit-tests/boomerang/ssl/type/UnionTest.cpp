#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UnionTest.h"


#include "boomerang/ssl/type/UnionType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/CompoundType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/VoidType.h"


void UnionTest::testConstruct()
{
    UnionType u1;
    QCOMPARE(u1.getCtype(), "union { }");
    QCOMPARE(u1.getNumTypes(), 0);
    QCOMPARE(u1.getSize(), 1);

    UnionType u2{ VoidType::get() };
    QCOMPARE(u2.getCtype(), "union { }");
    QCOMPARE(u2.getNumTypes(), 0);
    QCOMPARE(u2.getSize(), 1);

    UnionType u3({ IntegerType::get(32, Sign::Signed) });
    QCOMPARE(u3.getCtype(), "union { int; }");
    QCOMPARE(u3.getNumTypes(), 1);
    QCOMPARE(u3.getSize(), 32);

    UnionType u4({ IntegerType::get(32, Sign::Signed), IntegerType::get(32, Sign::Signed) });
    QCOMPARE(u4.getCtype(), "union { int; }");
    QCOMPARE(u4.getNumTypes(), 1);
    QCOMPARE(u4.getSize(), 32);

    UnionType u5({ IntegerType::get(32, Sign::Signed), SizeType::get(32) });
    QCOMPARE(u5.getNumTypes(), 1);
    QCOMPARE(u5.getSize(), 32);
    QCOMPARE(u5.getCtype(), "union { int; }");

    UnionType u6({ IntegerType::get(32, Sign::Signed), FloatType::get(32) });
    QCOMPARE(u6.getCtype(), "union { float; int; }");
    QCOMPARE(u6.getNumTypes(), 2);
    QCOMPARE(u6.getSize(), 32);
}


void UnionTest::testGetNumTypes()
{
    UnionType u1;
//     QCOMPARE(u1.getSize(), 1);

    UnionType u2{ SizeType::get(32) };
    QCOMPARE(u2.getNumTypes(), 1);

    UnionType u3{ IntegerType::get(32, Sign::Signed), SizeType::get(32) };
    QCOMPARE(u3.getNumTypes(), 1);

}


void UnionTest::testHasType()
{
    UnionType u1;
    QVERIFY(!u1.hasType(VoidType::get()));
    QVERIFY(!u1.hasType(IntegerType::get(32, Sign::Signed)));

    UnionType u2{ { IntegerType::get(32, Sign::Signed), "foo" } };
    QVERIFY(!u2.hasType(VoidType::get()));
    QVERIFY(!u2.hasType(FloatType::get(32)));
    QVERIFY(u2.hasType(IntegerType::get(32, Sign::Signed)));
//     QVERIFY(u2.hasType(SizeType::get(32)));
}


QTEST_GUILESS_MAIN(UnionTest)
