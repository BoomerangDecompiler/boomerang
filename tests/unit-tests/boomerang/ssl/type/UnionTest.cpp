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
    QCOMPARE(u1.getNumTypes(), 0);
    QCOMPARE(u1.getSize(), 1);
    QCOMPARE(u1.getCtype(), "union { }");

    UnionType u2({ });
    QCOMPARE(u2.getNumTypes(), 0);
    QCOMPARE(u2.getSize(), 1);
    QCOMPARE(u2.getCtype(), "union { }");

    UnionType u3({ IntegerType::get(32, Sign::Signed) });
    QCOMPARE(u3.getNumTypes(), 1);
    QCOMPARE(u3.getSize(), 32);
    QCOMPARE(u3.getCtype(), "union { int; }");

    UnionType u4({ IntegerType::get(32, Sign::Signed), IntegerType::get(32, Sign::Signed) });
    QCOMPARE(u4.getNumTypes(), 1);
    QCOMPARE(u4.getSize(), 32);
    QCOMPARE(u4.getCtype(), "union { int; }");

//     UnionType u5({ IntegerType::get(32, Sign::Signed), SizeType::get(32) });
//     QCOMPARE(u5.getNumTypes(), 1);
//     QCOMPARE(u5.getSize(), 32);
//     QCOMPARE(u5.getCtype(), "union { int; }");

    UnionType u6({ IntegerType::get(32, Sign::Signed), FloatType::get(32) });
    QCOMPARE(u6.getNumTypes(), 2);
    QCOMPARE(u6.getSize(), 32);
    QCOMPARE(u6.getCtype(), "union { float; int; }");
}


void UnionTest::testAddType()
{
    UnionType u1;
//     u1.addType(VoidType::get());
//     QCOMPARE(u1.getNumTypes(), 0);
//     QCOMPARE(u1.getSize(), 1);
//     QCOMPARE(u1.getCtype(), "union { }");

    u1.addType(IntegerType::get(32, Sign::Signed));
    QCOMPARE(u1.getNumTypes(), 1);
    QCOMPARE(u1.getSize(), 32);
    QCOMPARE(u1.getCtype(), "union { int; }");

    u1.addType(IntegerType::get(32, Sign::Signed), "foo");
    QCOMPARE(u1.getNumTypes(), 1);
    QCOMPARE(u1.getSize(), 32);
    QCOMPARE(u1.getCtype(), "union { int; }");

    u1.addType(FloatType::get(32), "bar");
    QCOMPARE(u1.getNumTypes(), 2);
    QCOMPARE(u1.getSize(), 32);
    QCOMPARE(u1.getCtype(), "union { float bar; int; }");

    std::shared_ptr<UnionType> u2 = UnionType::get();
    std::shared_ptr<PointerType> pty = PointerType::get(u2);
    u2->addType(pty);
    QCOMPARE(u2->getNumTypes(), 1);
    QCOMPARE(u2->getSize(), 32);
    QCOMPARE(u2->getCtype(), "union { void *; }");

    // TODO: addType(UnionType)
}


void UnionTest::testGetNumTypes()
{
    UnionType u;
    QCOMPARE(u.getNumTypes(), 0);
    u.addType(SizeType::get(32));
    QCOMPARE(u.getNumTypes(), 1);
}


void UnionTest::testHasType()
{
    UnionType u;
    QVERIFY(!u.hasType(VoidType::get()));
    QVERIFY(!u.hasType(IntegerType::get(32, Sign::Signed)));

    u.addType(IntegerType::get(32, Sign::Signed), "foo");
    QVERIFY(!u.hasType(VoidType::get()));
    QVERIFY(!u.hasType(FloatType::get(32)));
    QVERIFY(u.hasType(IntegerType::get(32, Sign::Signed)));
//     QVERIFY(u.hasType(SizeType::get(32)));
}


QTEST_GUILESS_MAIN(UnionTest)
