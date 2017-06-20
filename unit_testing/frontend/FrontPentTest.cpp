/***************************************************************************/ /**
 * \file       FrontPentTest.cpp
 * OVERVIEW:   Provides the implementation for the FrontPentTest class, which
 *                tests the sparc front end
 *============================================================================*/
#include "FrontPentTest.h"

#include "include/types.h"
#include "db/rtl.h"
#include "db/prog.h"
#include "include/frontend.h"
#include "frontend/pentium/pentiumfrontend.h"
#include "boom_base/BinaryFile.h"
#include "boom_base/BinaryFileStub.h"
#include "include/decoder.h"
#include "boom_base/log.h"
#include "boom_base/log.h"

#include <QDir>
#include <QProcessEnvironment>
#include <QDebug>

#define HELLO_PENT      baseDir.absoluteFilePath("tests/inputs/pentium/hello")
#define BRANCH_PENT     baseDir.absoluteFilePath("tests/inputs/pentium/branch")
#define FEDORA2_TRUE    baseDir.absoluteFilePath("tests/inputs/pentium/fedora2_true")
#define FEDORA3_TRUE    baseDir.absoluteFilePath("tests/inputs/pentium/fedora3_true")
#define SUSE_TRUE       baseDir.absoluteFilePath("tests/inputs/pentium/suse_true")

static bool    logset = false;
static QString TEST_BASE;
static QDir    baseDir;
void FrontPentTest::initTestCase()
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


void FrontPentTest::test1()
{
	QString           expected;
	QString           actual;
	QTextStream       strm(&actual);
	BinaryFileFactory bff;
	IFileLoader       *pBF = bff.load(HELLO_PENT);

	QVERIFY(pBF != 0);

	Prog *prog = new Prog(HELLO_PENT);
	QVERIFY(pBF->getMachine() == Machine::PENTIUM);

	FrontEnd *pFE = new PentiumFrontEnd(pBF, prog, &bff);
	prog->setFrontEnd(pFE);

	bool    gotMain;
	ADDRESS addr = pFE->getMainEntryPoint(gotMain);
	QVERIFY(addr != NO_ADDRESS);

	// Decode first instruction
	DecodeResult inst = pFE->decodeInstruction(addr);
	inst.rtl->print(strm);

	expected = "08048328    0 *32* m[r28 - 4] := r29\n"
			   "            0 *32* r28 := r28 - 4\n";
	QCOMPARE(actual, expected);
	actual.clear();

	addr += inst.numBytes;
	inst  = pFE->decodeInstruction(addr);
	inst.rtl->print(strm);
	expected = QString("08048329    0 *32* r29 := r28\n");
	QCOMPARE(actual, expected);
	actual.clear();

	addr = 0x804833b;
	inst = pFE->decodeInstruction(addr);
	inst.rtl->print(strm);
	expected = QString("0804833b    0 *32* m[r28 - 4] := 0x80483fc\n"
					   "            0 *32* r28 := r28 - 4\n");
	QCOMPARE(actual, expected);
	actual.clear();

	delete pFE;
	// delete pBF;
}


void FrontPentTest::test2()
{
	DecodeResult      inst;
	QString           expected;
	QString           actual;
	QTextStream       strm(&actual);
	BinaryFileFactory bff;
	IFileLoader       *pBF = bff.load(HELLO_PENT);

	QVERIFY(pBF != 0);
	Prog *prog = new Prog(HELLO_PENT);
	QVERIFY(pBF->getMachine() == Machine::PENTIUM);

	FrontEnd *pFE = new PentiumFrontEnd(pBF, prog, &bff);
	prog->setFrontEnd(pFE);

	inst = pFE->decodeInstruction(ADDRESS::g(0x8048345));
	inst.rtl->print(strm);
	expected = QString("08048345    0 *32* tmp1 := r28\n"
					   "            0 *32* r28 := r28 + 16\n"
					   "            0 *v* %flags := ADDFLAGS32( tmp1, 16, r28 )\n");
	QCOMPARE(actual, expected);
	actual.clear();

	inst = pFE->decodeInstruction(ADDRESS::g(0x8048348));
	inst.rtl->print(strm);
	expected = QString("08048348    0 *32* r24 := 0\n");
	QCOMPARE(actual, expected);
	actual.clear();

	inst = pFE->decodeInstruction(ADDRESS::g(0x8048329));
	inst.rtl->print(strm);
	expected = QString("08048329    0 *32* r29 := r28\n");
	QCOMPARE(actual, expected);
	actual.clear();

	delete pFE;
	// delete pBF;
}


void FrontPentTest::test3()
{
	DecodeResult      inst;
	QString           expected;
	QString           actual;
	QTextStream       strm(&actual);
	BinaryFileFactory bff;
	IFileLoader       *pBF = bff.load(HELLO_PENT);

	QVERIFY(pBF != 0);
	Prog *prog = new Prog(HELLO_PENT);
	QVERIFY(pBF->getMachine() == Machine::PENTIUM);
	FrontEnd *pFE = new PentiumFrontEnd(pBF, prog, &bff);
	prog->setFrontEnd(pFE);

	inst = pFE->decodeInstruction(ADDRESS::n(0x804834d));
	inst.rtl->print(strm);
	expected = QString("0804834d    0 *32* r28 := r29\n"
					   "            0 *32* r29 := m[r28]\n"
					   "            0 *32* r28 := r28 + 4\n");
	QCOMPARE(actual, expected);
	actual.clear();

	inst = pFE->decodeInstruction(ADDRESS::n(0x804834e));
	inst.rtl->print(strm);
	expected = QString("0804834e    0 *32* %pc := m[r28]\n"
					   "            0 *32* r28 := r28 + 4\n"
					   "            0 RET\n"
					   "              Modifieds: \n"
					   "              Reaching definitions: \n");

	QCOMPARE(actual, expected);
	actual.clear();

	delete pFE;
	// delete pBF;
}


void FrontPentTest::testBranch()
{
	DecodeResult      inst;
	QString           expected;
	QString           actual;
	QTextStream       strm(&actual);
	BinaryFileFactory bff;
	IFileLoader       *pBF = bff.load(BRANCH_PENT);

	QVERIFY(pBF != 0);
	Prog *prog = new Prog(BRANCH_PENT);

	QVERIFY(pBF->getMachine() == Machine::PENTIUM);
	FrontEnd *pFE = new PentiumFrontEnd(pBF, prog, &bff);
	prog->setFrontEnd(pFE);

	// jne
	inst = pFE->decodeInstruction(ADDRESS::n(0x8048979));
	inst.rtl->print(strm);
	expected = QString("08048979    0 BRANCH 0x8048988, condition "
					   "not equals\n"
					   "High level: %flags\n");
	QCOMPARE(actual, expected);
	actual.clear();

	// jg
	inst = pFE->decodeInstruction(ADDRESS::n(0x80489c1));
	inst.rtl->print(strm);
	expected = QString("080489c1    0 BRANCH 0x80489d5, condition signed greater\n"
					   "High level: %flags\n");
	QCOMPARE(actual, expected);
	actual.clear();

	// jbe
	inst = pFE->decodeInstruction(ADDRESS::n(0x8048a1b));
	inst.rtl->print(strm);
	expected = QString("08048a1b    0 BRANCH 0x8048a2a, condition unsigned less or equals\n"
					   "High level: %flags\n");
	QCOMPARE(actual, expected);
	actual.clear();

	delete pFE;
	// delete pBF;
}


void FrontPentTest::testFindMain()
{
	// Test the algorithm for finding main, when there is a call to __libc_start_main
	// Also tests the loader hack
	BinaryFileFactory bff;
	IFileLoader       *pBF = bff.load(FEDORA2_TRUE);

	QVERIFY(pBF != 0);

	Prog *prog = new Prog(FEDORA2_TRUE);
	QVERIFY(pBF->getMachine() == Machine::PENTIUM);
	
	FrontEnd *pFE = new PentiumFrontEnd(pBF, prog, &bff);
	prog->setFrontEnd(pFE);

	bool    found;
	ADDRESS addr     = pFE->getMainEntryPoint(found);
	ADDRESS expected = ADDRESS::n(0x8048b10);
	QCOMPARE(addr, expected);
	pBF->close();
	bff.unload();
	delete pFE;

	pBF = bff.load(FEDORA3_TRUE);
	QVERIFY(pBF != nullptr);
	pFE = new PentiumFrontEnd(pBF, prog, &bff);
	prog->setFrontEnd(pFE);
	QVERIFY(pFE != nullptr);
	addr     = pFE->getMainEntryPoint(found);
	expected = ADDRESS::n(0x8048c4a);
	QCOMPARE(addr, expected);
	
	pBF->close();
	bff.unload();
	delete pFE;

	pBF = bff.load(SUSE_TRUE);
	QVERIFY(pBF != nullptr);
	pFE = new PentiumFrontEnd(pBF, prog, &bff);
	prog->setFrontEnd(pFE);
	QVERIFY(pFE != nullptr);
	addr     = pFE->getMainEntryPoint(found);
	expected = ADDRESS::n(0x8048b60);
	QCOMPARE(addr, expected);
	pBF->close();

	bff.unload();
	delete pFE;
}


QTEST_MAIN(FrontPentTest)
