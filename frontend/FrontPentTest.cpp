/*==============================================================================
 * FILE:       FrontPentTest.cc
 * OVERVIEW:   Provides the implementation for the FrontPentTest class, which
 *              tests the sparc front end
 *============================================================================*/
/*
 * $Revision$
 *
 * 05 Apr 02 - Mike: Created
 * 21 May 02 - Mike: Mods for gcc 3.1
 */

#ifndef BOOMDIR
#error Must define BOOMDIR
#endif

#define HELLO_PENT     BOOMDIR "/test/pentium/hello"
#define BRANCH_PENT    BOOMDIR "/test/pentium/branch"

#include "types.h"
#include "rtl.h"
#include "FrontPentTest.h"
#include "prog.h"
#include "frontend.h"
#include "pentiumfrontend.h"
#include "BinaryFile.h"
#include "BinaryFileStub.h"

/*==============================================================================
 * FUNCTION:        FrontPentTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<FrontPentTest> ("FrontPentTest", \
    &FrontPentTest::name, *this))

void FrontPentTest::registerTests(CppUnit::TestSuite* suite) {
    MYTEST(test1);
    MYTEST(test2);
    MYTEST(test3);
    MYTEST(testBranch);
}

int FrontPentTest::countTestCases () const
{ return 3; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        FrontPentTest::setUp
 * OVERVIEW:        Set up anything needed before all tests
 * NOTE:            Called before any tests
 * NOTE:            Also appears to be called before all tests!
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void FrontPentTest::setUp () {
}

/*==============================================================================
 * FUNCTION:        FrontPentTest::tearDown
 * OVERVIEW:        Delete objects created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void FrontPentTest::tearDown () {
}

/*==============================================================================
 * FUNCTION:        FrontPentTest::test1
 * OVERVIEW:        Test decoding some pentium instructions
 *============================================================================*/
void FrontPentTest::test1 () {
    std::ostringstream ost;

    BinaryFile *pBF = BinaryFile::Load(HELLO_PENT);
    if (pBF == NULL)
        pBF = new BinaryFileStub();
    CPPUNIT_ASSERT(pBF != 0);
    CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_PENTIUM);
    FrontEnd *pFE = new PentiumFrontEnd(pBF); 

    bool gotMain;
    ADDRESS addr = pFE->getMainEntryPoint(gotMain);
    CPPUNIT_ASSERT (addr != NO_ADDRESS);

    // Decode first instruction
    DecodeResult inst = pFE->decodeInstruction(addr);
    inst.rtl->print(ost);
    
    std::string expected(
        "08048918    0 *32* r28 := r28 - 4\n"
        "            0 *32* m[r28] := r29\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(ost.str()));

    std::ostringstream o2;
    addr += inst.numBytes;
    inst = pFE->decodeInstruction(addr);
    inst.rtl->print(o2);
    expected = std::string("08048919    0 *32* r29 := r28\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    std::ostringstream o3;
    addr += inst.numBytes;
    inst = pFE->decodeInstruction(addr);
    inst.rtl->print(o3);
    expected = std::string(
        "0804891b    0 *32* r28 := r28 - 4\n"
        "            0 *32* m[r28] := 134517752\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

    delete pFE;
    delete pBF;
}

void FrontPentTest::test2() {
    DecodeResult inst;
    std::string expected;

    BinaryFile *pBF = BinaryFile::Load(HELLO_PENT);
    if (pBF == NULL)
        pBF = new BinaryFileStub();
    CPPUNIT_ASSERT(pBF != 0);
    CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_PENTIUM);
    FrontEnd *pFE = new PentiumFrontEnd(pBF); 

    std::ostringstream o1;
    inst = pFE->decodeInstruction(0x8048925);
    inst.rtl->print(o1);
    expected = std::string(
        "08048925    0 *32* tmp1 := r28\n"
        "            0 *32* r28 := r28 + 4\n"
        "            0 *32* %flags := ADDFLAGS32( tmp1, 4, r28 )\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o1.str()));

    std::ostringstream o2;
    inst = pFE->decodeInstruction(0x8048928);
    inst.rtl->print(o2);
    expected = std::string(
        "08048928    0 *32* r24 := r24 ^ r24\n"
        "            0 *32* %flags := LOGICALFLAGS32( r24 )\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    std::ostringstream o3;
    inst = pFE->decodeInstruction(0x804892a);
    inst.rtl->print(o3);
    expected = std::string("0804892a    0 JUMP 0x804892c\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

    delete pFE;
    delete pBF;
}

void FrontPentTest::test3() {
    DecodeResult inst;
    std::string expected;

    BinaryFile *pBF = BinaryFile::Load(HELLO_PENT);
    if (pBF == NULL)
        pBF = new BinaryFileStub();
    CPPUNIT_ASSERT(pBF != 0);
    CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_PENTIUM);
    FrontEnd *pFE = new PentiumFrontEnd(pBF); 

    std::ostringstream o1;
    inst = pFE->decodeInstruction(0x804892c);
    inst.rtl->print(o1);
    expected = std::string(
        "0804892c    0 *32* r28 := r29\n"
        "            0 *32* r29 := m[r28]\n"
        "            0 *32* r28 := r28 + 4\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o1.str()));

    std::ostringstream o2;
    inst = pFE->decodeInstruction(0x804892d);
    inst.rtl->print(o2);
    expected = std::string(
      "0804892d    0 *32* %pc := m[r28]\n"
      "            0 *32* r28 := r28 + 4\n"
      "            0 RET\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    delete pFE;
    delete pBF;
}

void FrontPentTest::testBranch() {
    DecodeResult inst;
    std::string expected;

    BinaryFile *pBF = BinaryFile::Load(BRANCH_PENT);
    if (pBF == NULL)
        pBF = new BinaryFileStub();
    CPPUNIT_ASSERT(pBF != 0);
    CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_PENTIUM);
    FrontEnd *pFE = new PentiumFrontEnd(pBF); 

    // jne
    std::ostringstream o1;
    inst = pFE->decodeInstruction(0x8048979);
    inst.rtl->print(o1);
    expected = std::string("08048979    0 JCOND 0x8048988, condition "
      "not equals\n"
      "High level: %flags\n");
    CPPUNIT_ASSERT_EQUAL(expected, o1.str());

    // jg
    std::ostringstream o2;
    inst = pFE->decodeInstruction(0x80489c1);
    inst.rtl->print(o2);
    expected = std::string(
      "080489c1    0 JCOND 0x80489d5, condition signed greater\n"
      "High level: %flags\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    // jbe
    std::ostringstream o3;
    inst = pFE->decodeInstruction(0x8048a1b);
    inst.rtl->print(o3);
    expected = std::string(
        "08048a1b    0 JCOND 0x8048a2a, condition unsigned less or equals\n"
        "High level: %flags\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

    delete pFE;
    delete pBF;
}

