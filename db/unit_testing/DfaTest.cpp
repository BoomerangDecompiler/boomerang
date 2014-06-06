/***************************************************************************/ /**
  * \file       DfaTest.cc
  * OVERVIEW:   Provides the implementation for the DfaTest class, which
  *                tests the data flow based type analysis code
  */
#include "DfaTest.h"

#include "boomerang.h"
#include "type.h"

#include <QtCore/QDir>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QDebug>
#include <sstream>

static bool logset = false;
QString TEST_BASE;
QDir baseDir;

void DfaTest::initTestCase() {
    if (!logset) {
        TEST_BASE = QProcessEnvironment::systemEnvironment().value("BOOMERANG_TEST_BASE", "");
        baseDir = QDir(TEST_BASE);
        if (TEST_BASE.isEmpty()) {
            qWarning() << "BOOMERANG_TEST_BASE environment variable not set, will assume '..', many test may fail";
            TEST_BASE = "..";
            baseDir = QDir("..");
        }
        logset = true;
        Boomerang::get()->setProgPath(TEST_BASE);
        Boomerang::get()->setPluginPath(TEST_BASE + "/out");
        Boomerang::get()->setLogger(new NullLogger());
    }
}

/***************************************************************************/ /**
  * \fn        DfaTest::testMeetInt
  * OVERVIEW:        Test meeting IntegerTypes with various other types
  ******************************************************************************/
void DfaTest::testMeetInt() {
    IntegerType *i32 = IntegerType::get(32, 1);
    IntegerType *j32 = IntegerType::get(32, 0);
    IntegerType *u32 = IntegerType::get(32, -1);
    IntegerType *xint = IntegerType::get(0);
    IntegerType *j16 = IntegerType::get(16, 0);
    SizeType s32(32);
    SizeType s64(64);
    FloatType *flt = FloatType::get(32);
    PointerType pt(flt);
    VoidType v;

    bool ch = false;
    i32->meetWith(i32, ch, false);
    QVERIFY(ch == false);
    QString actual;
    QTextStream ost1(&actual);
    ost1 << i32;
    QString expected("i32");
    QCOMPARE(actual,expected);
    actual.clear();

    i32->meetWith(j32, ch, false);
    QVERIFY(ch == false);
    j32->meetWith(i32, ch, false);
    QVERIFY(ch == true);

    ost1 << i32;
    QCOMPARE(actual,QString("i32"));
    actual.clear();

    ch = false;
    j32->setSigned(0);
    j32->meetWith(&v, ch, false);
    QVERIFY(ch == false);

    ost1 << j32;
    QCOMPARE(actual,QString("j32"));
    actual.clear();

    ch = false;
    j32->meetWith(u32, ch, false);
    QVERIFY(ch == true);

    ost1 << j32;
    QCOMPARE(actual,QString("u32"));
    actual.clear();

    ch = false;
    u32->meetWith(&s32, ch, false);
    QVERIFY(ch == false);

    ost1 << u32;
    QCOMPARE(actual,QString("u32"));
    actual.clear();

    u32->meetWith(&s64, ch, false);
    QVERIFY(ch == true);

    ost1 << u32;
    QCOMPARE(actual,QString("u64"));
    actual.clear();

    ch = false;
    Type *res = i32->meetWith(flt, ch, false);
    QVERIFY(ch == true);

    ost1 << res;
    QCOMPARE(actual,QString("union"));
    actual.clear();

    ch = false;
    res = i32->meetWith(&pt, ch, false);
    QVERIFY(ch == true);

    ost1 << res;
    QCOMPARE(actual,QString("union"));
    actual.clear();
}

/***************************************************************************/ /**
  * \fn        DfaTest::testMeetSize
  * OVERVIEW:        Test meeting IntegerTypes with various other types
  ******************************************************************************/
void DfaTest::testMeetSize() {
    QString actual;
    QTextStream ost1(&actual);
    IntegerType *i32 = IntegerType::get(32, 1);
    SizeType s32(32);
    SizeType s16(16);
    FloatType *flt = FloatType::get(32);
    VoidType v;

    bool ch = false;
    Type *res = s32.meetWith(i32, ch, false);
    QVERIFY(ch == true);

    ost1 << res;
    QCOMPARE(actual,QString("i32"));
    actual.clear();

    ch = false;
    res = s32.meetWith(&s16, ch, false);
    QVERIFY(ch == false);

    // There is a known failure here; to show the warning, use ErrLogger
    Boomerang::get()->setLogger(new ErrLogger);

    res = s16.meetWith(flt, ch, false);
    QVERIFY(ch == true);

    ost1 << res;
    QCOMPARE(actual,QString("union"));
    actual.clear();

    ch = false;
    res = s16.meetWith(&v, ch, false);
    QVERIFY(ch == false);

    ost1 << res;
    QCOMPARE(actual,QString("16"));
    actual.clear();
}

/***************************************************************************/ /**
  * \fn        DfaTest::testMeetPointer
  * OVERVIEW:        Test meeting IntegerTypes with various other types
  ******************************************************************************/
void DfaTest::testMeetPointer() {
    IntegerType *i32 = IntegerType::get(32, 1);
    IntegerType *u32 = IntegerType::get(32, -1);
    PointerType pi32(i32);
    PointerType pu32(u32);
    VoidType v;
    QString actual;
    QTextStream ost1(&actual);

    ost1 << pu32.getCtype();
    QCOMPARE(actual,QString("unsigned int *"));
    actual.clear();

    bool ch = false;
    Type *res = pi32.meetWith(&pu32, ch, false);
    QVERIFY(ch == true);

    ost1 << res->getCtype();
    QCOMPARE(actual,QString("/*signed?*/int *"));
    actual.clear();

    ch = false;
    res = pi32.meetWith(&v, ch, false);
    QVERIFY(ch == false);

    res = pi32.meetWith(i32, ch, false);
    QVERIFY(res->isUnion());
}

/***************************************************************************/ /**
  * \fn        DfaTest::testMeetUnion
  * OVERVIEW:        Test meeting IntegerTypes with various other types
  ******************************************************************************/
void DfaTest::testMeetUnion() {
    UnionType u1;
    IntegerType *i32 = IntegerType::get(32, 1);
    IntegerType *j32 = IntegerType::get(32, 0);
    IntegerType *u32 = IntegerType::get(32, -1);
    FloatType *flt = FloatType::get(32);
    u1.addType(i32, "bow");
    u1.addType(flt, "wow");

    std::ostringstream ost1;
    ost1 << u1.getCtype().toStdString();
    std::string actual(ost1.str());
    std::string expected("union { int bow; float wow; }");
    QCOMPARE(actual,expected);

    bool ch = false;
    Type *res = u1.meetWith(j32, ch, false);
    QVERIFY(ch == false);
    std::ostringstream ost2;
    ost2 << res->getCtype().toStdString();
    actual = ost2.str();
    expected = "union { int bow; float wow; }";
    QCOMPARE(actual,expected);

    res = u1.meetWith(j32, ch, false);
    QVERIFY(ch == false);
    std::ostringstream ost3;
    ost3 << u1.getCtype().toStdString();
    actual = ost3.str();
    expected = "union { int bow; float wow; }";
    QCOMPARE(actual,expected);

    // Note: this test relies on the int in the union having signedness 1
    res = u1.meetWith(u32, ch, false);
    QVERIFY(ch == true);
    std::ostringstream ost4;
    ost4 << u1.getCtype().toStdString();
    actual = ost4.str();
    expected = "union { /*signed?*/int bow; float wow; }";
    QCOMPARE(actual,expected);
}
QTEST_MAIN(DfaTest)
