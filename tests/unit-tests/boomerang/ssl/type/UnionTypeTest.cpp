#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UnionTypeTest.h"

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
    QCOMPARE(u6.getCtype(), "union { int; float; }");
    QCOMPARE(u6.getNumTypes(), 2);
    QCOMPARE(u6.getSize(), 32);
}


void UnionTest::testCompare()
{
    // equality comparison
    QVERIFY(UnionType({ IntegerType::get(32), FloatType::get(32) })
        ==  UnionType({ FloatType::get(32),   IntegerType::get(32) }));

    QVERIFY(UnionType({ IntegerType::get(32), FloatType::get(32) })
        !=  UnionType({ IntegerType::get(32) }));

    QVERIFY(UnionType({ FloatType::get(32) })
        !=  UnionType({ FloatType::get(32), IntegerType::get(32) }));

    QVERIFY(UnionType({ FloatType::get(32), FloatType::get(64) })
        !=  UnionType({ FloatType::get(64), IntegerType::get(64) }));

    QVERIFY(UnionType({ IntegerType::get(32) })
        !=  *IntegerType::get(32));

    QVERIFY(*IntegerType::get(32)
        != UnionType({ IntegerType::get(32) }));

    // less-than comparison
    QVERIFY(!(UnionType({ IntegerType::get(32) }) < *IntegerType::get(32)));
    QVERIFY(*IntegerType::get(32) < UnionType({ IntegerType::get(32) }));

    QVERIFY(UnionType({ IntegerType::get(32) }) < UnionType({ FloatType::get(32)}));
    QVERIFY(!(UnionType({ FloatType::get(32) }) < UnionType({ FloatType::get(32)})));

    QVERIFY(UnionType({ IntegerType::get(32) })
        <   UnionType({ FloatType::get(32), SizeType::get(64) }));

}


void UnionTest::testGetSize()
{
    UnionType u1;
    QCOMPARE(u1.getSize(), 1);

    UnionType u2{ SizeType::get(32) };
    QCOMPARE(u2.getSize(), 32);

    UnionType u3{ SizeType::get(32), IntegerType::get(16, Sign::Unsigned) };
    QCOMPARE(u3.getSize(), 32);

    UnionType u4{ SizeType::get(8), IntegerType::get(16, Sign::Unsigned) };
    QCOMPARE(u4.getSize(), 16);

    UnionType u5{ SizeType::get(0) };
    QCOMPARE(u5.getSize(), 1);
}


void UnionTest::testGetCtype()
{
    UnionType u1;
    QCOMPARE(u1.getCtype(true),  QString("union { }"));
    QCOMPARE(u1.getCtype(false), QString("union { }"));

    UnionType u2{ IntegerType::get(32, Sign::Unsigned) };
    QCOMPARE(u2.getCtype(true),  QString("union { unsigned int; }"));
    QCOMPARE(u2.getCtype(false), QString("union { unsigned int; }"));

    UnionType u3{ { IntegerType::get(32, Sign::Unsigned), "foo" } };
    QCOMPARE(u3.getCtype(true),  QString("union { unsigned int foo; }"));
    QCOMPARE(u3.getCtype(false), QString("union { unsigned int foo; }"));

    // FIXME this should result in "union { int; }" because the size information is redundant
    UnionType u4{ { SizeType::get(32), IntegerType::get(32, Sign::Unknown) } };
    QCOMPARE(u4.getCtype(true),  QString("union { int; __size32; }"));
    QCOMPARE(u4.getCtype(false), QString("union { /*signed?*/int; __size32; }"));
}


void UnionTest::testIsCompatibleWith()
{
    UnionType leftU1, rightU1{ VoidType::get() };
    QVERIFY(leftU1.isCompatibleWith(rightU1, true));
    QVERIFY(leftU1.isCompatibleWith(rightU1, false));

    UnionType leftU2;
    SharedType right2 = VoidType::get();
    QVERIFY(leftU2.isCompatibleWith(*right2, true));
    QVERIFY(leftU2.isCompatibleWith(*right2, false));

    UnionType leftU3{ IntegerType::get(32, Sign::Signed) };
    UnionType rightU3{ IntegerType::get(32, Sign::Signed) };
    QVERIFY(leftU3.isCompatibleWith(rightU3, true));
    QVERIFY(leftU3.isCompatibleWith(rightU3, false));
    QVERIFY(leftU3.isCompatibleWith(leftU3, true));
    QVERIFY(leftU3.isCompatibleWith(leftU3, false));

    UnionType leftU4{ FloatType::get(32), IntegerType::get(32, Sign::Signed) };
    UnionType rightU4{ IntegerType::get(32, Sign::Signed) };
    QVERIFY(leftU4.isCompatibleWith(rightU4, true));
    QVERIFY(leftU4.isCompatibleWith(rightU4, false));
    QVERIFY(rightU4.isCompatibleWith(leftU4, true));
    QVERIFY(rightU4.isCompatibleWith(leftU4, false));
}


void UnionTest::testGetNumTypes()
{
    UnionType u1{ SizeType::get(32) };
    QCOMPARE(u1.getNumTypes(), 1);

    UnionType u2{ IntegerType::get(32, Sign::Signed), SizeType::get(32) };
    QCOMPARE(u2.getNumTypes(), 1);
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
}


QTEST_GUILESS_MAIN(UnionTest)
