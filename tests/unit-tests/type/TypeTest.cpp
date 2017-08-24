/**
 * \file TypeTest.cpp
 * Provides the implementation for the TypeTest class,
 * which tests the Type class and some utility functions
 */

/*
 * $Revision$
 *
 * 09 Apr 02 - Mike: Created
 * 22 Aug 03 - Mike: Extended for Constraint tests
 * 25 Juk 05 - Mike: DataIntervalMap tests
 */

#include "TypeTest.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/db/Signature.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Project.h"

#include "boomerang-frontend/pentium/pentiumfrontend.h"

#include <QTextStream>
#include <QDebug>

#define HELLO_WINDOWS    (BOOMERANG_TEST_BASE "/tests/inputs/windows/hello.exe")


void TypeTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "/lib/boomerang/");
}


void TypeTest::testTypeLong()
{
	auto t = IntegerType::get(64, -1);

	QCOMPARE(t->getCtype(), QString("unsigned long long"));
}


void TypeTest::testNotEqual()
{
	auto t1(IntegerType::get(32, -1));
	auto t2(IntegerType::get(32, -1));
	auto t3(IntegerType::get(16, -1));

	QVERIFY(!(*t1 != *t2));
	QVERIFY(*t2 != *t3);
}


void TypeTest::testCompound()
{
    IProject& project = *Boomerang::get()->getOrCreateProject();
    project.loadBinaryFile(HELLO_WINDOWS);
    IFileLoader* loader = project.getBestLoader(HELLO_WINDOWS);
	QVERIFY(loader != nullptr);

	IFrontEnd *pFE    = new PentiumFrontEnd(loader, new Prog(HELLO_WINDOWS));

	pFE->readLibraryCatalog(); // Read definitions

	std::shared_ptr<Signature> paintSig = pFE->getLibSignature("BeginPaint");

	SharedType paramType = paintSig->getParamType(1);
	QCOMPARE(paintSig->getParamType(1)->getCtype(), QString("LPPAINTSTRUCT"));
	SharedType paintStructType = paramType->as<PointerType>()->getPointsTo();
	QCOMPARE(paintStructType->getCtype(), QString("PAINTSTRUCT"));

	// Offset 8 should have a RECT
	SharedType subTy    = paintStructType->as<CompoundType>()->getTypeAtOffset(8 * 8);
	QString    expected = "struct { "
						  "int left; "
						  "int top; "
						  "int right; "
						  "int bottom; "
						  "}";
	QCOMPARE(subTy->getCtype(true), expected);

	// Name at offset 0x0C should be bottom
	QCOMPARE(subTy->as<CompoundType>()->getNameAtOffset(0x0C * 8), QString("bottom"));

	// Now figure out the name at offset 8+C
	QCOMPARE(paintStructType->as<CompoundType>()->getNameAtOffset((8 + 0x0C) * 8), QString("rcPaint"));

	// Also at offset 8
	QCOMPARE(paintStructType->as<CompoundType>()->getNameAtOffset((8 + 0) * 8), QString("rcPaint"));

	// Also at offset 8+4
	QCOMPARE(paintStructType->as<CompoundType>()->getNameAtOffset((8 + 4) * 8), QString("rcPaint"));

	// And at offset 8+8
	QCOMPARE(paintStructType->as<CompoundType>()->getNameAtOffset((8 + 8) * 8), QString("rcPaint"));

	delete pFE;
}


void TypeTest::testDataInterval()
{

	Prog     *prog = new Prog("test");
	Module   *m    = prog->getOrInsertModule("test");
	UserProc *proc = (UserProc *)m->getOrInsertFunction("test", Address(0x123));
	DataIntervalMap dim;

	proc->setSignature(Signature::instantiate(Platform::PENTIUM, CallConv::C, "test"));
	dim.setProc(proc);

	dim.addItem(Address(0x00001000), "first", IntegerType::get(32, 1));
	dim.addItem(Address(0x00001004), "second", FloatType::get(64));
	QString actual(dim.prints());
	QString expected("0x00001000-0x00001004 first int\n"
					 "0x00001004-0x0000100c second double\n");
	QCOMPARE(actual, expected);

	DataIntervalMap::DataIntervalEntry *pdie = dim.find(Address(0x00001000));
	QVERIFY(pdie);
	QCOMPARE(pdie->second.name, QString("first"));

	pdie = dim.find(Address(0x00001003));
	QVERIFY(pdie);
	QCOMPARE(pdie->second.name, QString("first"));

	pdie = dim.find(Address(0x00001004));
	QVERIFY(pdie);
	expected = "second";
	actual   = pdie->second.name;
	QCOMPARE(actual, expected);

	pdie = dim.find(Address(0x00001007));
	QVERIFY(pdie);
	actual = pdie->second.name;
	QCOMPARE(actual, expected);

	auto ct(CompoundType::get());
	ct->addType(IntegerType::get(16, 1), "short1");
	ct->addType(IntegerType::get(16, 1), "short2");
	ct->addType(IntegerType::get(32, 1), "int1");
	ct->addType(FloatType::get(32), "float1");
	dim.addItem(Address(0x00001010), "struct1", ct);

	ComplexTypeCompList& ctcl = ct->compForAddress(Address(0x00001012), dim);
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
	dim.addItem(Address(0x00001020), "array1", at);
	ComplexTypeCompList& ctcl2 = at->compForAddress(Address(0x00001020 + 0x3C + 8), dim);
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


void TypeTest::testDataIntervalOverlaps()
{
	DataIntervalMap dim;

	Prog     *prog = new Prog("test");
	Module   *m    = prog->getOrInsertModule("test");
	UserProc *proc = (UserProc *)m->getOrInsertFunction("test", Address(0x123));

	proc->setSignature(Signature::instantiate(Platform::PENTIUM, CallConv::C, "test"));
	dim.setProc(proc);

	dim.addItem(Address(0x00001000), "firstInt", IntegerType::get(32, 1));
	dim.addItem(Address(0x00001004), "firstFloat", FloatType::get(32));
	dim.addItem(Address(0x00001008), "secondInt", IntegerType::get(32, 1));
	dim.addItem(Address(0x0000100C), "secondFloat", FloatType::get(32));
	auto ct = CompoundType::get();
	ct->addType(IntegerType::get(32, 1), "int3");
	ct->addType(FloatType::get(32), "float3");
	dim.addItem(Address(0x00001010), "existingStruct", ct);

	// First insert a new struct over the top of the existing middle pair
	auto ctu = CompoundType::get();
	ctu->addType(IntegerType::get(32, 0), "newInt"); // This int has UNKNOWN sign
	ctu->addType(FloatType::get(32), "newFloat");
	dim.addItem(Address(0x00001008), "replacementStruct", ctu);

	DataIntervalMap::DataIntervalEntry *pdie  = dim.find(Address(0x1008));
	QString           actual = pdie->second.type->getCtype();
	QCOMPARE(actual, QString("struct { int newInt; float newFloat; }"));

	// Attempt a weave; should fail
	auto ct3 = CompoundType::get();
	ct3->addType(FloatType::get(32), "newFloat3");
	ct3->addType(IntegerType::get(32, 0), "newInt3");
	dim.addItem(Address(0x00001004), "weaveStruct1", ct3);
	pdie = dim.find(Address(0x00001004));
	QCOMPARE(pdie->second.name, QString("firstFloat"));

	// Totally unaligned
	dim.addItem(Address(0x00001001), "weaveStruct2", ct3);
	pdie = dim.find(Address(0x000001001));
	QCOMPARE(pdie->second.name, QString("firstInt"));

	dim.addItem(Address(0x00001004), "firstInt", IntegerType::get(32, 1)); // Should fail
	pdie = dim.find(Address(0x00001004));
	QCOMPARE(pdie->second.name, QString("firstFloat"));

	// Set up three ints
	dim.deleteItem(Address(0x00001004));
	dim.addItem(Address(0x00001004), "firstInt", IntegerType::get(32, 1)); // Definitely signed
	dim.deleteItem(Address(0x00001008));
	dim.addItem(Address(0x00001008), "firstInt", IntegerType::get(32, 0)); // Unknown signedess
	// then, add an array over the three integers
	auto at = ArrayType::get(IntegerType::get(32, 0), 3);
	dim.addItem(Address(0x00001000), "newArray", at);

	pdie = dim.find(Address(0x00001005)); // Check middle element
	QCOMPARE(pdie->second.name, QString("newArray"));
	pdie = dim.find(Address(0x00001000)); // Check first
	QCOMPARE(pdie->second.name, QString("newArray"));
	pdie = dim.find(Address(0x100B)); // Check last
	QCOMPARE(pdie->second.name, QString("newArray"));

	// Already have an array of 3 ints at 0x1000. Put a new array completely before, then with only one word overlap
	dim.addItem(Address(0x00000F00), "newArray2", at);
	pdie = dim.find(Address(0x00001000)); // Should still be newArray at 0x1000
	QCOMPARE(pdie->second.name, QString("newArray"));

	pdie = dim.find(Address(0x00000F00));
	QCOMPARE(pdie->second.name, QString("newArray2"));

	dim.addItem(Address(0x00000FF8), "newArray3", at); // Should fail
	pdie = dim.find(Address(0x00000FF8));
	QVERIFY(nullptr == (void *)pdie);                // Expect nullptr
}


QTEST_MAIN(TypeTest)
