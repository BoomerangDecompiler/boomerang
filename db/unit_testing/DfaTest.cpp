/***************************************************************************//**
 * \file       DfaTest.cc
 * OVERVIEW:   Provides the implementation for the DfaTest class, which
 *                tests the data flow based type analysis code
 *============================================================================*/
#include "DfaTest.h"

#include "boomerang.h"
#include "type.h"

#include <sstream>

static bool logset = false;

DfaTest::DfaTest()
{
    if(!logset) {
        logset=true;
        Boomerang::get()->setLogger(new NullLogger());
    }
}

void DfaTest::SetUp () { }
void DfaTest::TearDown () { }

/***************************************************************************//**
 * FUNCTION:        DfaTest::testMeetInt
 * OVERVIEW:        Test meeting IntegerTypes with various other types
 *============================================================================*/
void DfaTest::testMeetInt() {
    IntegerType *i32=IntegerType::get(32, 1);
    IntegerType *j32=IntegerType::get(32, 0);
    IntegerType *u32=IntegerType::get(32, -1);
    IntegerType *xint=IntegerType::get(0);
    IntegerType *j16=IntegerType::get(16, 0);
    SizeType s32(32);
    SizeType s64(64);
    FloatType *flt=FloatType::get(32);
    PointerType pt(flt);
    VoidType v;

    bool ch = false;
    i32->meetWith(i32, ch, false);
    QVERIFY(ch == false);
    std::ostringstream ost1;
    ost1<< i32;
    std::string actual(ost1.str());
    std::string expected("i32");
    QCOMPARE(expected, actual);

    i32->meetWith(j32, ch, false);
    QVERIFY(ch == false);
    j32->meetWith(i32, ch, false);
    QVERIFY(ch == true);
    std::ostringstream ost2;
    ost2<< i32;
    actual = ost2.str();
    expected = "i32";
    QCOMPARE(expected, actual);

    ch = false;
    j32->setSigned(0);
    j32->meetWith(&v, ch, false);
    QVERIFY(ch == false);
    std::ostringstream ost2a;
    ost2a<< j32;
    actual = ost2a.str();
    expected = "j32";
    QCOMPARE(expected, actual);

    ch = false;
    j32->meetWith(u32, ch, false);
    QVERIFY(ch == true);
    std::ostringstream ost3;
    ost3<< j32;
    actual = ost3.str();
    expected = "u32";
    QCOMPARE(expected, actual);

    ch = false;
    u32->meetWith(&s32, ch, false);
    QVERIFY(ch == false);
    std::ostringstream ost4;
    ost4<< &u32;
    actual = ost4.str();
    expected = "u32";
    QCOMPARE(expected, actual);

    u32->meetWith(&s64, ch, false);
    QVERIFY(ch == true);
    std::ostringstream ost5;
    ost5<< &u32;
    actual = ost5.str();
    expected = "u64";
    QCOMPARE(expected, actual);

    ch = false;
    Type* res = i32->meetWith(flt, ch, false);
    QVERIFY(ch == true);
    std::ostringstream ost6;
    ost6<< res;
    actual = ost6.str();
    expected = "union";
    QCOMPARE(expected, actual);

    ch = false;
    res = i32->meetWith(&pt, ch, false);
    QVERIFY(ch == true);
    std::ostringstream ost7;
    ost7<< res;
    actual = ost7.str();
    expected = "union";
    QCOMPARE(expected, actual);
}

/***************************************************************************//**
 * FUNCTION:        DfaTest::testMeetSize
 * OVERVIEW:        Test meeting IntegerTypes with various other types
 *============================================================================*/
void DfaTest::testMeetSize() {
    IntegerType *i32=IntegerType::get(32, 1);
    SizeType s32(32);
    SizeType s16(16);
    FloatType *flt=FloatType::get(32);
    VoidType v;

    bool ch = false;
    Type* res = s32.meetWith(i32, ch, false);
    QVERIFY(ch == true);
    std::ostringstream ost1;
    ost1 << res;
    std::string actual(ost1.str());
    std::string expected("i32");
    QCOMPARE(expected, actual);

    ch = false;
    res = s32.meetWith(&s16, ch, false);
    QVERIFY(ch == false);

    // There is a known failure here; to show the warning, use ErrLogger
    Boomerang::get()->setLogger(new ErrLogger);

    res = s16.meetWith(flt, ch, false);
    QVERIFY(ch == true);
    std::ostringstream ost2;
    ost2 << res;
    actual = ost2.str();
    expected = "union";
    QCOMPARE(expected, actual);

    ch = false;
    res = s16.meetWith(&v, ch, false);
    QVERIFY(ch == false);
    std::ostringstream ost3;
    ost3 << res;
    actual = ost3.str();
    expected = "16";
    QCOMPARE(expected, actual);

}

/***************************************************************************//**
 * FUNCTION:        DfaTest::testMeetPointer
 * OVERVIEW:        Test meeting IntegerTypes with various other types
 *============================================================================*/
void DfaTest::testMeetPointer() {
    IntegerType *i32=IntegerType::get(32, 1);
    IntegerType *u32=IntegerType::get(32, -1);
    PointerType pi32(i32);
    PointerType pu32(u32);
    VoidType v;

    std::ostringstream ost1;
    ost1 << pu32.getCtype();
    std::string actual(ost1.str());
    std::string expected("unsigned int *");
    QCOMPARE(expected, actual);

    bool ch = false;
    Type* res = pi32.meetWith(&pu32, ch, false);
    QVERIFY(ch == true);
    std::ostringstream ost2;
    ost2 << res->getCtype();
    actual = ost2.str();
    expected = "/*signed?*/int *";
    QCOMPARE(expected, actual);

    ch = false;
    res = pi32.meetWith(&v, ch, false);
    QVERIFY(ch == false);

    res = pi32.meetWith(i32, ch, false);
    std::ostringstream ost3;
    ost3 << res;
    actual = ost3.str();
    expected = "union";
    QCOMPARE(expected, actual);

}

/***************************************************************************//**
 * FUNCTION:        DfaTest::testMeetUnion
 * OVERVIEW:        Test meeting IntegerTypes with various other types
 *============================================================================*/
void DfaTest::testMeetUnion() {
    UnionType u1;
    IntegerType *i32=IntegerType::get(32, 1);
    IntegerType *j32=IntegerType::get(32, 0);
    IntegerType *u32=IntegerType::get(32, -1);
    FloatType *flt=FloatType::get(32);
    u1.addType(i32, "bow");
    u1.addType(flt, "wow");

    std::ostringstream ost1;
    ost1 << u1.getCtype();
    std::string actual(ost1.str());
    std::string expected("union { int bow; float wow; }");
    QCOMPARE(expected, actual);

    bool ch = false;
    Type* res = u1.meetWith(j32, ch, false);
    QVERIFY(ch == false);
    std::ostringstream ost2;
    ost2 << res->getCtype();
    actual = ost2.str();
    expected = "union { int bow; float wow; }";
    QCOMPARE(expected, actual);

    res = u1.meetWith(j32, ch, false);
    QVERIFY(ch == false);
    std::ostringstream ost3;
    ost3 << u1.getCtype();
    actual = ost3.str();
    expected = "union { int bow; float wow; }";
    QCOMPARE(expected, actual);

    // Note: this test relies on the int in the union having signedness 1
    res = u1.meetWith(u32, ch, false);
    QVERIFY(ch == true);
    std::ostringstream ost4;
    ost4 << u1.getCtype();
    actual = ost4.str();
    expected = "union { /*signed?*/int bow; float wow; }";
    QCOMPARE(expected, actual);
}
QTEST_MAIN(DfaTest)

