#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CompoundTypeTest.h"


#include "boomerang/ssl/type/CompoundType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/UnionType.h"
#include "boomerang/ssl/type/SizeType.h"


void CompoundTypeTest::testConstruct()
{
    CompoundType ct;
    QCOMPARE(ct.getNumMembers(), 0);
    QCOMPARE(ct.getCtype(), "struct { }");
    QCOMPARE(ct.getSize(), 0);
}


void CompoundTypeTest::testEquals()
{
    QCOMPARE(CompoundType() == CompoundType(), true);

    CompoundType ct1, ct2;

    ct1.addMember(VoidType::get(), "foo");
    QCOMPARE(ct1 == ct2, false);

    ct2.addMember(IntegerType::get(32, Sign::Unknown), "foo");
    QCOMPARE(ct1 == ct2, false); // same name, different type
    QCOMPARE(ct2 == *IntegerType::get(32, Sign::Unknown), false); // int != int wrapped in struct


    CompoundType ct3, ct4;
    ct3.addMember(IntegerType::get(32, Sign::Signed), "foo");
    ct4.addMember(IntegerType::get(32, Sign::Signed), "bar");
    QCOMPARE(ct3 == ct4, true); // disregard type names

    CompoundType ct5, ct6;
    ct5.addMember(FloatType::get(32), "foo");
    ct6.addMember(FloatType::get(32), "foo");
    QCOMPARE(ct5 == ct6, true);
}


void CompoundTypeTest::testLess()
{
    QCOMPARE(CompoundType() < ArrayType(VoidType::get()), false);
    QCOMPARE(CompoundType() < CompoundType(), false);
    QCOMPARE(CompoundType() < UnionType(), true);

    CompoundType ct1, ct2;
    ct2.addMember(VoidType::get(), "foo");
    QCOMPARE(ct1 < ct2, true);
    QCOMPARE(ct2 < ct1, false);

    CompoundType ct3, ct4;
    ct3.addMember(VoidType::get(), "foo");
    ct4.addMember(VoidType::get(), "foo");

    QCOMPARE(ct3 < ct4, false);
    QCOMPARE(ct4 < ct3, false);

    ct3.addMember(IntegerType::get(32, Sign::Signed), "bar");
    ct4.addMember(IntegerType::get(32, Sign::Unsigned), "bar");
    QCOMPARE(ct3 < ct4, false);
    QCOMPARE(ct4 < ct3, true);

    CompoundType ct5, ct6;
    ct5.addMember(FloatType::get(32), "foo");
    ct6.addMember(FloatType::get(32), "bar");
    QCOMPARE(ct5 < ct6, false);
    QCOMPARE(ct6 < ct5, false);
}


void CompoundTypeTest::testGetCtype()
{
    QCOMPARE(CompoundType().getCtype(true), "struct { }");
    QCOMPARE(CompoundType().getCtype(false), "struct { }");

    CompoundType ct1;
    ct1.addMember(IntegerType::get(16, Sign::Signed), "");
    QCOMPARE(ct1.getCtype(true),  "struct { short; }");
    QCOMPARE(ct1.getCtype(false), "struct { short; }");

    ct1.addMember(FloatType::get(32), "bar");
    QCOMPARE(ct1.getCtype(true),  "struct { short; float bar; }");
    QCOMPARE(ct1.getCtype(false), "struct { short; float bar; }");
}


void CompoundTypeTest::testIsSuperStructOf()
{
    QCOMPARE(CompoundType::get()->isSuperStructOf(CompoundType::get()), true);
    QCOMPARE(CompoundType::get()->isSubStructOf(CompoundType::get()), true);

    std::shared_ptr<CompoundType> ct1 = CompoundType::get(), ct2 = CompoundType::get();
    ct1->addMember(FloatType::get(32), ""); // ct1 = struct { float; }, ct2 = struct { }
    QCOMPARE(ct1->isSuperStructOf(ct2), false);
    QCOMPARE(ct2->isSuperStructOf(ct1), true);
    QCOMPARE(ct1->isSubStructOf(ct2), true);
    QCOMPARE(ct2->isSubStructOf(ct1), false);

    ct2->addMember(FloatType::get(32), "foo"); // ct1 = struct { float; }, ct2 = struct { float foo; }
    QCOMPARE(ct1->isSuperStructOf(ct2), true);
    QCOMPARE(ct2->isSuperStructOf(ct1), true);
    QCOMPARE(ct2->isSubStructOf(ct1), true);
    QCOMPARE(ct1->isSubStructOf(ct2), true);

    ct1->addMember(IntegerType::get(32, Sign::Signed), "bar");  // ct = struct { float; int bar; }
    ct2->addMember(FloatType::get(32), "bar");                  // ct = struct { float; float; }
    QCOMPARE(ct1->isSuperStructOf(ct2), false);
    QCOMPARE(ct2->isSuperStructOf(ct1), false);
    QCOMPARE(ct1->isSubStructOf(ct2), false);
    QCOMPARE(ct2->isSubStructOf(ct1), false);
}


void CompoundTypeTest::testMemberType()
{
    CompoundType ct1;
    ct1.addMember(FloatType::get(32), "foo");
    QVERIFY(*ct1.getMemberTypeByIdx(0) == *FloatType::get(32));

    QVERIFY(ct1.getMemberTypeByName("foo") != nullptr);
    QVERIFY(*ct1.getMemberTypeByName("foo") == *FloatType::get(32));
    QVERIFY(ct1.getMemberTypeByName("bar") == nullptr);

    QVERIFY(ct1.getMemberTypeByOffset(0) != nullptr);
    QVERIFY(*ct1.getMemberTypeByOffset(0) == *FloatType::get(32));

    QVERIFY(ct1.getMemberTypeByOffset(16) != nullptr);
    QVERIFY(*ct1.getMemberTypeByOffset(16) == *FloatType::get(32));

    QVERIFY(ct1.getMemberTypeByOffset(32) == nullptr);

    ct1.setMemberTypeByOffset(0, FloatType::get(64));
    QCOMPARE(ct1.getCtype(), "struct { double foo; }");
    ct1.setMemberTypeByOffset(16, FloatType::get(32));
    QCOMPARE(ct1.getCtype(), "struct { float foo; __size32 pad; }");
}


void CompoundTypeTest::testMemberName()
{
    CompoundType ct1;
    QVERIFY(ct1.getMemberNameByOffset(0) == "");

    ct1.addMember(FloatType::get(32), "");
    ct1.addMember(FloatType::get(32), "foo");
    QVERIFY(ct1.getMemberNameByIdx(0) == "");
    QVERIFY(ct1.getMemberNameByIdx(1) == "foo");
    QVERIFY(ct1.getMemberNameByOffset(0) == "");
    QVERIFY(ct1.getMemberNameByOffset(32) == "foo");
    QVERIFY(ct1.getMemberNameByOffset(48) == "foo");
    QVERIFY(ct1.getMemberNameByOffset(64) == "");

    ct1.setMemberNameByOffset(0, "bar");
    QVERIFY(ct1.getMemberNameByIdx(0) == "bar");

    ct1.setMemberNameByOffset(48, "baz");
    QCOMPARE(ct1.getCtype(), "struct { float bar; float baz; }");
}


void CompoundTypeTest::testMemberOffset()
{
    CompoundType ct1;
    ct1.addMember(FloatType::get(32), "");
    ct1.addMember(FloatType::get(32), "foo");

    QCOMPARE(ct1.getMemberOffsetByIdx(0), 0);
    QCOMPARE(ct1.getMemberOffsetByIdx(1), 32);
    QCOMPARE(ct1.getMemberOffsetByName(""), 0);
    QCOMPARE(ct1.getMemberOffsetByName("foo"), 32);
}


void CompoundTypeTest::testGetOffsetRemainder()
{
    QCOMPARE(CompoundType().getOffsetRemainder(0), 0);
    QCOMPARE(CompoundType().getOffsetRemainder(10), 10);

    CompoundType ct1;
    ct1.addMember(FloatType::get(32), "");
    ct1.addMember(FloatType::get(32), "foo");

    QCOMPARE(ct1.getOffsetRemainder(0), 0);
    QCOMPARE(ct1.getOffsetRemainder(20), 20);
    QCOMPARE(ct1.getOffsetRemainder(32), 0);
    QCOMPARE(ct1.getOffsetRemainder(48), 16);
    QCOMPARE(ct1.getOffsetRemainder(50), 18);
    QCOMPARE(ct1.getOffsetRemainder(64), 0);
}


void CompoundTypeTest::testIsCompatibleWith()
{
    auto ct1 = CompoundType::get();
    QCOMPARE(ct1->isCompatibleWith(*VoidType::get()), true);
    QCOMPARE(ct1->isCompatibleWith(*IntegerType::get(32, Sign::Signed), true), false);
    QCOMPARE(ct1->isCompatibleWith(*IntegerType::get(32, Sign::Signed), false), false);

    ct1->addMember(FloatType::get(32), "");
    QCOMPARE(ct1->isCompatibleWith(*FloatType::get(32), true), false);
    QCOMPARE(ct1->isCompatibleWith(*FloatType::get(32), false), true);

    auto ct2 = CompoundType::get();
    ct2->addMember(FloatType::get(32), "foo");
    QCOMPARE(ct1->isCompatibleWith(*ct2, true),  true);
    QCOMPARE(ct1->isCompatibleWith(*ct2, false), true);

    ct2->addMember(FloatType::get(32), "");
    QCOMPARE(ct1->isCompatibleWith(*ct2, true),  false);
    QCOMPARE(ct1->isCompatibleWith(*ct2, false), false);

    auto uty = UnionType::get({ FloatType::get(32), SizeType::get(64) });
    QCOMPARE(ct1->isCompatibleWith(*uty, true),  false);
    QCOMPARE(ct1->isCompatibleWith(*ct2, false), false);
}



QTEST_GUILESS_MAIN(CompoundTypeTest)
