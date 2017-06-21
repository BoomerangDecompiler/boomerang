/***************************************************************************/ /**
 * \file       DfaTest.cc
 * OVERVIEW:   Provides the implementation for the DfaTest class, which
 *                tests the data flow based type analysis code
 */
#include "DfaTest.h"

#include "util/Log.h"
#include "type/type.h"

#include <QtCore/QDir>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QDebug>
#include <sstream>

static bool logset = false;
QString     TEST_BASE;
QDir        baseDir;

void DfaTest::initTestCase()
{
	if (!logset) {
		TEST_BASE = QProcessEnvironment::systemEnvironment().value("BOOMERANG_TEST_BASE", "");
		baseDir   = QDir(TEST_BASE);

		if (TEST_BASE.isEmpty()) {
			qWarning() << "BOOMERANG_TEST_BASE environment variable not set, will assume '..', many test may fail";
			TEST_BASE = "..";
			baseDir   = QDir("..");
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
void DfaTest::testMeetInt()
{
	auto i32  = IntegerType::get(32, 1);
	auto j32  = IntegerType::get(32, 0);
	auto u32  = IntegerType::get(32, -1);
	auto xint = IntegerType::get(0);
	auto j16  = IntegerType::get(16, 0);
	auto s32  = SizeType::get(32);
	auto s64  = SizeType::get(64);
	auto flt  = FloatType::get(32);
	auto pt   = PointerType::get(flt);
	auto v    = VoidType::get();

	bool ch = false;

	i32->meetWith(i32, ch, false);
	QVERIFY(ch == false);
	QString     actual;
	QTextStream ost1(&actual);
	ost1 << i32;
	QString expected("i32");
	QCOMPARE(actual, expected);
	actual.clear();

	i32->meetWith(j32, ch, false);
	QVERIFY(ch == false);
	j32->meetWith(i32, ch, false);
	QVERIFY(ch == true);

	ost1 << i32;
	QCOMPARE(actual, QString("i32"));
	actual.clear();

	ch = false;
	j32->setSigned(0);
	j32->meetWith(v, ch, false);
	QVERIFY(ch == false);

	ost1 << j32;
	QCOMPARE(actual, QString("j32"));
	actual.clear();

	ch = false;
	j32->meetWith(u32, ch, false);
	QVERIFY(ch == true);

	ost1 << j32;
	QCOMPARE(actual, QString("u32"));
	actual.clear();

	ch = false;
	u32->meetWith(s32, ch, false);
	QVERIFY(ch == false);

	ost1 << u32;
	QCOMPARE(actual, QString("u32"));
	actual.clear();

	u32->meetWith(s64, ch, false);
	QVERIFY(ch == true);

	ost1 << u32;
	QCOMPARE(actual, QString("u64"));
	actual.clear();

	ch = false;
	auto res = i32->meetWith(flt, ch, false);
	QVERIFY(ch == true);

	ost1 << res;
	QCOMPARE(actual, QString("union"));
	actual.clear();

	ch  = false;
	res = i32->meetWith(pt, ch, false);
	QVERIFY(ch == true);

	ost1 << res;
	QCOMPARE(actual, QString("union"));
	actual.clear();
}


/***************************************************************************/ /**
 * \fn        DfaTest::testMeetSize
 * OVERVIEW:        Test meeting IntegerTypes with various other types
 ******************************************************************************/
void DfaTest::testMeetSize()
{
	QString     actual;
	QTextStream ost1(&actual);
	auto        i32 = IntegerType::get(32, 1);
	auto        s32 = SizeType::get(32);
	auto        s16 = SizeType::get(16);
	auto        flt = FloatType::get(32);
	auto        v   = VoidType::get();

	bool ch  = false;
	auto res = s32->meetWith(i32, ch, false);

	QVERIFY(ch == true);

	ost1 << res;
	QCOMPARE(actual, QString("i32"));
	actual.clear();

	ch  = false;
	res = s32->meetWith(s16, ch, false);
	QVERIFY(ch == false);

	// There is a known failure here; to show the warning, use ErrLogger
	Boomerang::get()->setLogger(new ErrLogger);

	res = s16->meetWith(flt, ch, false);
	QVERIFY(ch == true);

	ost1 << res;
	QCOMPARE(actual, QString("union"));
	actual.clear();

	ch  = false;
	res = s16->meetWith(v, ch, false);
	QVERIFY(ch == false);

	ost1 << res;
	QCOMPARE(actual, QString("16"));
	actual.clear();
}


/***************************************************************************/ /**
 * \fn        DfaTest::testMeetPointer
 * OVERVIEW:        Test meeting IntegerTypes with various other types
 ******************************************************************************/
void DfaTest::testMeetPointer()
{
	auto i32  = IntegerType::get(32, 1);
	auto u32  = IntegerType::get(32, -1);
	auto pi32 = PointerType::get(i32);
	auto pu32 = PointerType::get(u32);
	auto v    = VoidType::get();

	QCOMPARE(pu32->getCtype(), QString("unsigned int *"));

	bool ch  = false;
	auto res = pi32->meetWith(pu32, ch, false);
	QVERIFY(ch == true);

	QCOMPARE(res->getCtype(), QString("/*signed?*/int *"));

	ch  = false;
	res = pi32->meetWith(v, ch, false);
	QVERIFY(ch == false);

	res = pi32->meetWith(i32, ch, false);
	QVERIFY(res->isUnion());
}


/***************************************************************************/ /**
 * \fn        DfaTest::testMeetUnion
 * OVERVIEW:        Test meeting IntegerTypes with various other types
 ******************************************************************************/
void DfaTest::testMeetUnion()
{
	auto i32  = IntegerType::get(32, 1);
	auto j32  = IntegerType::get(32, 0);
	auto u32  = IntegerType::get(32, -1);
	auto u1   = UnionType::get();
	auto u2   = UnionType::get();
	auto flt  = FloatType::get(32);
	auto flt2 = FloatType::get(32);

	u1->addType(i32, "bow");
	u1->addType(flt, "wow");
	u2->addType(flt2, "gorm");
	QCOMPARE(u1->getCtype(), QString("union { float wow; int bow; }"));

	bool ch  = false;
	auto res = u1->meetWith(j32, ch, false);
	QVERIFY(ch == false);
	QCOMPARE(res->getCtype(), QString("union { float wow; int bow; }"));

	ch  = false;
	res = u1->meetWith(flt, ch, false);
	QVERIFY(ch == false);
	QCOMPARE(res->getCtype(), QString("union { float wow; int bow; }"));

	res = u1->meetWith(u2, ch, false);
	QVERIFY(ch == false);
	QCOMPARE(u1->getCtype(), QString("union { float wow; int bow; }"));

	// Note: this test relies on the int in the union having signedness 1
	res = u1->meetWith(u32, ch, false);
	QVERIFY(ch == true);
	QCOMPARE(u1->getCtype(), QString("union { /*signed?*/int bow; float wow; }"));
}


QTEST_MAIN(DfaTest)
