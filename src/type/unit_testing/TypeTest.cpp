/***************************************************************************/ /**
 * \file       TypeTest.cc
 * OVERVIEW:   Provides the implementation for the TypeTest class, which tests the Type class and some utility
 * functions
 ******************************************************************************/

/*
 * $Revision$
 *
 * 09 Apr 02 - Mike: Created
 * 22 Aug 03 - Mike: Extended for Constraint tests
 * 25 Juk 05 - Mike: DataIntervalMap tests
 */

#include "TypeTest.h"
#include "boom_base/BinaryFile.h" // Ugh - needed before frontend.h
#include "frontend/pentium/pentiumfrontend.h"
#include "include/signature.h"
#include "boom_base/log.h"
#include "boom_base/log.h"
#include "include/prog.h"
#include "include/proc.h"

#include <QTextStream>
#include <QDir>
#include <QProcessEnvironment>
#include <QDebug>

#define HELLO_WINDOWS    baseDir.absoluteFilePath("tests/inputs/windows/hello.exe")

/***************************************************************************/ /**
 * \fn        TypeTest::setUp
 * OVERVIEW:        Set up anything needed before all tests
 * \note            Called before any tests
 *
 ******************************************************************************/
static bool    logset = false;
static QString TEST_BASE;
static QDir    baseDir;
void TypeTest::initTestCase()
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
 * \fn        TypeTest::testTypeLong
 * OVERVIEW:        Test type unsigned long
 ******************************************************************************/
void TypeTest::testTypeLong()
{
	auto t = IntegerType::get(64, -1);

	QCOMPARE(t->getCtype(), QString("unsigned long long"));
}


/***************************************************************************/ /**
 * \fn        TypeTest::testNotEqual
 * OVERVIEW:        Test type inequality
 ******************************************************************************/
void TypeTest::testNotEqual()
{
	auto t1(IntegerType::get(32, -1));
	auto t2(IntegerType::get(32, -1));
	auto t3(IntegerType::get(16, -1));

	QVERIFY(!(*t1 != *t2));
	QVERIFY(*t2 != *t3);
}


/***************************************************************************/ /**
 * \fn        TypeTest::testNotEqual
 * OVERVIEW:        Test type inequality
 ******************************************************************************/
void TypeTest::testCompound()
{
	BinaryFileFactory bff;
	QObject           *pBF = bff.Load(HELLO_WINDOWS);
	FrontEnd          *pFE = new PentiumFrontEnd(pBF, new Prog(HELLO_WINDOWS), &bff);

	pFE->readLibraryCatalog(); // Read definitions

	auto paintSig = pFE->getLibSignature("BeginPaint");
	// Second argument should be an LPPAINTSTRUCT
	auto    ty = paintSig->getParamType(1);
	QString p  = ty->getCtype();
	QString expected("LPPAINTSTRUCT");
	QString actual(p);
	QCOMPARE(actual, expected);

	// Get the type pointed to
	ty = ty->as<PointerType>()->getPointsTo();
	QCOMPARE(ty->getCtype(), QString("PAINTSTRUCT"));

	// Offset 8 should have a RECT
	auto subTy = ty->as<CompoundType>()->getTypeAtOffset(8 * 8);
	expected = "struct { "
			   "int left; "
			   "int top; "
			   "int right; "
			   "int bottom; "
			   "}";
	QCOMPARE(subTy->getCtype(true), expected);

	// Name at offset C should be bottom
	QCOMPARE(subTy->as<CompoundType>()->getNameAtOffset(0x0C * 8), QString("bottom"));

	// Now figure out the name at offset 8+C
	QCOMPARE(ty->as<CompoundType>()->getNameAtOffset((8 + 0x0C) * 8), QString("rcPaint"));

	// Also at offset 8
	QCOMPARE(ty->as<CompoundType>()->getNameAtOffset((8 + 0) * 8), QString("rcPaint"));

	// Also at offset 8+4
	QCOMPARE(ty->as<CompoundType>()->getNameAtOffset((8 + 4) * 8), QString("rcPaint"));

	// And at offset 8+8
	QCOMPARE(ty->as<CompoundType>()->getNameAtOffset((8 + 8) * 8), QString("rcPaint"));

	delete pFE;
}


/***************************************************************************/ /**
 * \fn        TypeTest::testDataInterval
 * OVERVIEW:        Test the DataIntervalMap class
 ******************************************************************************/
void TypeTest::testDataInterval()
{
	DataIntervalMap dim;

	Prog     *prog = new Prog("test");
	Module   *m    = prog->getOrInsertModule("test");
	UserProc *proc = (UserProc *)m->getOrInsertFunction("test", ADDRESS::g(0x123));

	proc->setSignature(Signature::instantiate(PLAT_PENTIUM, CONV_C, "test"));
	dim.setProc(proc);

	dim.addItem(ADDRESS::g(0x1000), "first", IntegerType::get(32, 1));
	dim.addItem(ADDRESS::g(0x1004), "second", FloatType::get(64));
	QString actual(dim.prints());
	QString expected("0x1000-0x1004 first int\n"
					 "0x1004-0x100c second double\n");
	QCOMPARE(actual, expected);

	DataIntervalEntry *pdie = dim.find(ADDRESS::g(0x1000));
	expected = "first";
	QVERIFY(pdie);
	actual = pdie->second.name;
	QCOMPARE(actual, expected);

	pdie = dim.find(ADDRESS::g(0x1003));
	QVERIFY(pdie);
	actual = pdie->second.name;
	QCOMPARE(actual, expected);

	pdie = dim.find(ADDRESS::g(0x1004));
	QVERIFY(pdie);
	expected = "second";
	actual   = pdie->second.name;
	QCOMPARE(actual, expected);

	pdie = dim.find(ADDRESS::g(0x1007));
	QVERIFY(pdie);
	actual = pdie->second.name;
	QCOMPARE(actual, expected);

	auto ct(CompoundType::get());
	ct->addType(IntegerType::get(16, 1), "short1");
	ct->addType(IntegerType::get(16, 1), "short2");
	ct->addType(IntegerType::get(32, 1), "int1");
	ct->addType(FloatType::get(32), "float1");
	dim.addItem(ADDRESS::g(0x1010), "struct1", ct);

	ComplexTypeCompList& ctcl = ct->compForAddress(ADDRESS::g(0x1012), dim);
	unsigned             ua   = ctcl.size();
	unsigned             ue   = 1;
	QCOMPARE(ua, ue);
	ComplexTypeComp& ctc = ctcl.front();
	ue = 0;
	ua = ctc.isArray;
	QCOMPARE(ua, ue);
	expected = "short2";
	actual   = ctc.u.memberName;
	QCOMPARE(actual, expected);

	// An array of 10 struct1's
	auto at = ArrayType::get(ct, 10);
	dim.addItem(ADDRESS::g(0x1020), "array1", at);
	ComplexTypeCompList& ctcl2 = at->compForAddress(ADDRESS::g(0x1020 + 0x3C + 8), dim);
	// Should be 2 components: [5] and .float1
	ue = 2;
	ua = ctcl2.size();
	QCOMPARE(ua, ue);
	ComplexTypeComp& ctc0 = ctcl2.front();
	ComplexTypeComp& ctc1 = ctcl2.back();
	QCOMPARE(ctc0.isArray, true);
	QCOMPARE(ctc0.u.index, 5U);
	QCOMPARE(ctc1.isArray, false);
	QCOMPARE(ctc1.u.memberName, QString("float1"));
}


/***************************************************************************/ /**
 * \fn        TypeTest::testDataIntervalOverlaps
 * OVERVIEW:        Test the DataIntervalMap class with overlapping addItems
 ******************************************************************************/
void TypeTest::testDataIntervalOverlaps()
{
	DataIntervalMap dim;

	Prog     *prog = new Prog("test");
	Module   *m    = prog->getOrInsertModule("test");
	UserProc *proc = (UserProc *)m->getOrInsertFunction("test", ADDRESS::g(0x123));

	proc->setSignature(Signature::instantiate(PLAT_PENTIUM, CONV_C, "test"));
	dim.setProc(proc);

	dim.addItem(ADDRESS::g(0x1000), "firstInt", IntegerType::get(32, 1));
	dim.addItem(ADDRESS::g(0x1004), "firstFloat", FloatType::get(32));
	dim.addItem(ADDRESS::g(0x1008), "secondInt", IntegerType::get(32, 1));
	dim.addItem(ADDRESS::g(0x100C), "secondFloat", FloatType::get(32));
	auto ct = CompoundType::get();
	ct->addType(IntegerType::get(32, 1), "int3");
	ct->addType(FloatType::get(32), "float3");
	dim.addItem(ADDRESS::g(0x1010), "existingStruct", ct);

	// First insert a new struct over the top of the existing middle pair
	auto ctu = CompoundType::get();
	ctu->addType(IntegerType::get(32, 0), "newInt"); // This int has UNKNOWN sign
	ctu->addType(FloatType::get(32), "newFloat");
	dim.addItem(ADDRESS::g(0x1008), "replacementStruct", ctu);

	DataIntervalEntry *pdie  = dim.find(ADDRESS::g(0x1008));
	QString           actual = pdie->second.type->getCtype();
	QCOMPARE(actual, QString("struct { int newInt; float newFloat; }"));

	// Attempt a weave; should fail
	auto ct3 = CompoundType::get();
	ct3->addType(FloatType::get(32), "newFloat3");
	ct3->addType(IntegerType::get(32, 0), "newInt3");
	dim.addItem(ADDRESS::g(0x1004), "weaveStruct1", ct3);
	pdie = dim.find(ADDRESS::g(0x1004));
	QCOMPARE(pdie->second.name, QString("firstFloat"));

	// Totally unaligned
	dim.addItem(ADDRESS::g(0x1001), "weaveStruct2", ct3);
	pdie = dim.find(ADDRESS::g(0x1001));
	QCOMPARE(pdie->second.name, QString("firstInt"));

	dim.addItem(ADDRESS::g(0x1004), "firstInt", IntegerType::get(32, 1)); // Should fail
	pdie = dim.find(ADDRESS::g(0x1004));
	QCOMPARE(pdie->second.name, QString("firstFloat"));

	// Set up three ints
	dim.deleteItem(ADDRESS::g(0x1004));
	dim.addItem(ADDRESS::g(0x1004), "firstInt", IntegerType::get(32, 1)); // Definately signed
	dim.deleteItem(ADDRESS::g(0x1008));
	dim.addItem(ADDRESS::g(0x1008), "firstInt", IntegerType::get(32, 0)); // Unknown signedess
	// then, add an array over the three integers
	auto at = ArrayType::get(IntegerType::get(32, 0), 3);
	dim.addItem(ADDRESS::g(0x1000), "newArray", at);

	pdie = dim.find(ADDRESS::g(0x1005)); // Check middle element
	QCOMPARE(pdie->second.name, QString("newArray"));
	pdie = dim.find(ADDRESS::g(0x1000)); // Check first
	QCOMPARE(pdie->second.name, QString("newArray"));
	pdie = dim.find(ADDRESS::g(0x100B)); // Check last
	QCOMPARE(pdie->second.name, QString("newArray"));

	// Already have an array of 3 ints at 0x1000. Put a new array completely before, then with only one word overlap
	dim.addItem(ADDRESS::g(0xF00), "newArray2", at);
	pdie = dim.find(ADDRESS::g(0x1000)); // Should still be newArray at 0x1000
	QCOMPARE(pdie->second.name, QString("newArray"));

	pdie = dim.find(ADDRESS::g(0xF00));
	QCOMPARE(pdie->second.name, QString("newArray2"));

	dim.addItem(ADDRESS::g(0xFF8), "newArray3", at); // Should fail
	pdie = dim.find(ADDRESS::g(0xFF8));
	QVERIFY(nullptr == (void *)pdie);                // Expect nullptr
}


QTEST_MAIN(TypeTest)
