/*==============================================================================
 * FILE:	   FrontPentTest.cc
 * OVERVIEW:   Provides the implementation for the FrontPentTest class, which
 *				tests the sparc front end
 *============================================================================*/
/*
 * $Revision$
 *
 * 05 Apr 02 - Mike: Created
 * 21 May 02 - Mike: Mods for gcc 3.1
 */

#define HELLO_PENT		"test/pentium/hello"
#define BRANCH_PENT		"test/pentium/branch"
#define FEDORA2_TRUE	"test/pentium/fedora2_true"
#define FEDORA3_TRUE	"test/pentium/fedora3_true"
#define SUSE_TRUE		"test/pentium/suse_true"

#include "types.h"
#include "rtl.h"
#include "FrontPentTest.h"
#include "prog.h"
#include "frontend.h"
#include "pentiumfrontend.h"
#include "BinaryFile.h"
#include "BinaryFileStub.h"

/*==============================================================================
 * FUNCTION:		FrontPentTest::registerTests
 * OVERVIEW:		Register the test functions in the given suite
 * PARAMETERS:		Pointer to the test suite
 * RETURNS:			<nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<FrontPentTest> ("FrontPentTest", \
	&FrontPentTest::name, *this))

void FrontPentTest::registerTests(CppUnit::TestSuite* suite) {
	MYTEST(test1);
	MYTEST(test2);
	MYTEST(test3);
	MYTEST(testBranch);
	MYTEST(testFindMain);
}

int FrontPentTest::countTestCases () const
{ return 3; }	// ? What's this for?

/*==============================================================================
 * FUNCTION:		FrontPentTest::setUp
 * OVERVIEW:		Set up anything needed before all tests
 * NOTE:			Called before any tests
 * NOTE:			Also appears to be called before all tests!
 * PARAMETERS:		<none>
 * RETURNS:			<nothing>
 *============================================================================*/
void FrontPentTest::setUp () {
}

/*==============================================================================
 * FUNCTION:		FrontPentTest::tearDown
 * OVERVIEW:		Delete objects created in setUp
 * NOTE:			Called after all tests
 * PARAMETERS:		<none>
 * RETURNS:			<nothing>
 *============================================================================*/
void FrontPentTest::tearDown () {
}

/*==============================================================================
 * FUNCTION:		FrontPentTest::test1
 * OVERVIEW:		Test decoding some pentium instructions
 *============================================================================*/
void FrontPentTest::test1 () {
	std::ostringstream ost;

	BinaryFile *pBF = BinaryFileFactory::Load(HELLO_PENT);
	if (pBF == NULL)
		pBF = new BinaryFileStub();
	CPPUNIT_ASSERT(pBF != 0);
	CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_PENTIUM);
	FrontEnd *pFE = new PentiumFrontEnd(pBF, new Prog); 

	bool gotMain;
	ADDRESS addr = pFE->getMainEntryPoint(gotMain);
	CPPUNIT_ASSERT (addr != NO_ADDRESS);

	// Decode first instruction
	DecodeResult inst = pFE->decodeInstruction(addr);
	inst.rtl->print(ost);
	
	std::string expected(
		"08048328    0 *32* r28 := r28 - 4\n"
		"            0 *32* m[r28] := r29\n");
	CPPUNIT_ASSERT_EQUAL(expected, std::string(ost.str()));

	std::ostringstream o2;
	addr += inst.numBytes;
	inst = pFE->decodeInstruction(addr);
	inst.rtl->print(o2);
	expected = std::string("08048329    0 *32* r29 := r28\n");
	CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

	std::ostringstream o3;
	addr = 0x804833b;
	inst = pFE->decodeInstruction(addr);
	inst.rtl->print(o3);
	expected = std::string(
		"0804833b    0 *32* r28 := r28 - 4\n"
		"            0 *32* m[r28] := 134513660\n");
	CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

	delete pFE;
	delete pBF;
}

void FrontPentTest::test2() {
	DecodeResult inst;
	std::string expected;

	BinaryFile *pBF = BinaryFileFactory::Load(HELLO_PENT);
	if (pBF == NULL)
		pBF = new BinaryFileStub();
	CPPUNIT_ASSERT(pBF != 0);
	CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_PENTIUM);
	FrontEnd *pFE = new PentiumFrontEnd(pBF, new Prog); 

	std::ostringstream o1;
	inst = pFE->decodeInstruction(0x8048345);
	inst.rtl->print(o1);
	expected = std::string(
		"08048345    0 *32* tmp1 := r28\n"
		"            0 *32* r28 := r28 + 16\n"
		"            0 *v* %flags := ADDFLAGS32( tmp1, 16, r28 )\n");
	CPPUNIT_ASSERT_EQUAL(expected, std::string(o1.str()));

	std::ostringstream o2;
	inst = pFE->decodeInstruction(0x8048348);
	inst.rtl->print(o2);
	expected = std::string(
		"08048348    0 *32* r24 := 0\n");
	CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

	std::ostringstream o3;
	inst = pFE->decodeInstruction(0x8048329);
	inst.rtl->print(o3);
	expected = std::string("08048329    0 *32* r29 := r28\n");
	CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

	delete pFE;
	delete pBF;
}

void FrontPentTest::test3() {
	DecodeResult inst;
	std::string expected;

	BinaryFile *pBF = BinaryFileFactory::Load(HELLO_PENT);
	if (pBF == NULL)
		pBF = new BinaryFileStub();
	CPPUNIT_ASSERT(pBF != 0);
	CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_PENTIUM);
	FrontEnd *pFE = new PentiumFrontEnd(pBF, new Prog); 

	std::ostringstream o1;
	inst = pFE->decodeInstruction(0x804834d);
	inst.rtl->print(o1);
	expected = std::string(
		"0804834d    0 *32* r28 := r29\n"
		"            0 *32* r29 := m[r28]\n"
		"            0 *32* r28 := r28 + 4\n");
	CPPUNIT_ASSERT_EQUAL(expected, std::string(o1.str()));

	std::ostringstream o2;
	inst = pFE->decodeInstruction(0x804834e);
	inst.rtl->print(o2);
	expected = std::string(
	  "0804834e    0 *32* %pc := m[r28]\n"
	  "            0 *32* r28 := r28 + 4\n"
	  "            0 RET \n");
	CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

	delete pFE;
	delete pBF;
}

void FrontPentTest::testBranch() {
	DecodeResult inst;
	std::string expected;

	BinaryFile *pBF = BinaryFileFactory::Load(BRANCH_PENT);
	if (pBF == NULL)
		pBF = new BinaryFileStub();
	CPPUNIT_ASSERT(pBF != 0);
	CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_PENTIUM);
	FrontEnd *pFE = new PentiumFrontEnd(pBF, new Prog); 

	// jne
	std::ostringstream o1;
	inst = pFE->decodeInstruction(0x8048979);
	inst.rtl->print(o1);
	expected = std::string("08048979    0 BRANCH 0x8048988, condition "
	  "not equals\n"
	  "High level: %flags\n");
	CPPUNIT_ASSERT_EQUAL(expected, o1.str());

	// jg
	std::ostringstream o2;
	inst = pFE->decodeInstruction(0x80489c1);
	inst.rtl->print(o2);
	expected = std::string(
	  "080489c1    0 BRANCH 0x80489d5, condition signed greater\n"
	  "High level: %flags\n");
	CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

	// jbe
	std::ostringstream o3;
	inst = pFE->decodeInstruction(0x8048a1b);
	inst.rtl->print(o3);
	expected = std::string(
		"08048a1b    0 BRANCH 0x8048a2a, condition unsigned less or equals\n"
		"High level: %flags\n");
	CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

	delete pFE;
	delete pBF;
}

void FrontPentTest::testFindMain() {
	// Test the algorithm for finding main, when there is a call to __libc_start_main
	// Also tests the loader hack
    BinaryFile* pBF = BinaryFileFactory::Load(FEDORA2_TRUE);
    CPPUNIT_ASSERT(pBF != NULL);
	FrontEnd* pFE = new PentiumFrontEnd(pBF, new Prog); 
    CPPUNIT_ASSERT(pFE != NULL);
	bool found;
    ADDRESS addr = pFE->getMainEntryPoint(found);
	ADDRESS expected = 0x8048b10;
    CPPUNIT_ASSERT_EQUAL(expected, addr);
	pBF->Close();

    pBF = BinaryFileFactory::Load(FEDORA3_TRUE);
    CPPUNIT_ASSERT(pBF != NULL);
	pFE = new PentiumFrontEnd(pBF, new Prog); 
    CPPUNIT_ASSERT(pFE != NULL);
    addr = pFE->getMainEntryPoint(found);
	expected = 0x8048c4a;
    CPPUNIT_ASSERT_EQUAL(expected, addr);
	pBF->Close();

    pBF = BinaryFileFactory::Load(SUSE_TRUE);
    CPPUNIT_ASSERT(pBF != NULL);
	pFE = new PentiumFrontEnd(pBF, new Prog); 
    CPPUNIT_ASSERT(pFE != NULL);
    addr = pFE->getMainEntryPoint(found);
	expected = 0x8048b60;
    CPPUNIT_ASSERT_EQUAL(expected, addr);
	pBF->Close();

}
