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
#define SSL_PATH       BOOMDIR "/frontend/machine/pentium/pentium.ssl"

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
    prog = new Prog();
    prog->pBF = new BinaryFileStub();
    CPPUNIT_ASSERT(prog->pBF != 0);
    // Set the text limits
    prog->getTextLimits();
    prog->pFE = new PentiumFrontEnd(prog, prog->textDelta, prog->limitTextHigh); 
    decoder = prog->pFE->getDecoder();
}

/*==============================================================================
 * FUNCTION:        FrontPentTest::tearDown
 * OVERVIEW:        Delete objects created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void FrontPentTest::tearDown () {
    prog->pBF->UnLoad();
	delete prog->pFE; prog->pFE = 0;
    delete decoder;
}

/*==============================================================================
 * FUNCTION:        FrontPentTest::test1
 * OVERVIEW:        Test decoding some pentium instructions
 *============================================================================*/
void FrontPentTest::test1 () {
    std::ostringstream ost;

    bool gotMain;
    ADDRESS addr = prog->pFE->getMainEntryPoint(gotMain);
    CPPUNIT_ASSERT (addr != NO_ADDRESS);

    // Decode first instruction
    DecodeResult inst = decoder->decodeInstruction(addr, prog->textDelta);
    inst.rtl->print(ost);
    
    std::string expected(
        "08048918 *32* r[28] := r[28] - 4\n"
        "         *32* m[r[28]] := r[29]\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(ost.str()));

    std::ostringstream o2;
    addr += inst.numBytes;
    inst = decoder->decodeInstruction(addr, prog->textDelta);
    inst.rtl->print(o2);
    expected = std::string("08048919 *32* r[29] := r[28]\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    std::ostringstream o3;
    addr += inst.numBytes;
    inst = decoder->decodeInstruction(addr, prog->textDelta);
    inst.rtl->print(o3);
    expected = std::string(
        "0804891b *32* r[28] := r[28] - 4\n"
        "         *32* m[r[28]] := 0x80493f8\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

}

void FrontPentTest::test2() {
    DecodeResult inst;
    std::string expected;

    std::ostringstream o1;
    inst = decoder->decodeInstruction(0x8048925, prog->textDelta);
    inst.rtl->print(o1);
    expected = std::string(
        "08048925 *32* r[tmp1] := r[28]\n"
        "         *32* r[28] := r[28] + 4\n"
	"         ADDFLAGS32( r[tmp1], 4, r[28] )\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o1.str()));

    std::ostringstream o2;
    inst = decoder->decodeInstruction(0x8048928, prog->textDelta);
    inst.rtl->print(o2);
    expected = std::string(
        "08048928 *32* r[24] := r[24] ^ r[24]\n"
	"         LOGICALFLAGS32( r[24] )\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    std::ostringstream o3;
    inst = decoder->decodeInstruction(0x804892a, prog->textDelta);
    inst.rtl->print(o3);
    expected = std::string("0804892a JUMP 0x804892c\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

}

void FrontPentTest::test3() {
    DecodeResult inst;
    std::string expected;

    std::ostringstream o1;
    inst = decoder->decodeInstruction(0x804892c, prog->textDelta);
    inst.rtl->print(o1);
    expected = std::string(
        "0804892c *32* r[28] := r[29]\n"
        "         *32* r[29] := m[r[28]]\n"
        "         *32* r[28] := r[28] + 4\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o1.str()));

    std::ostringstream o2;
    inst = decoder->decodeInstruction(0x804892d, prog->textDelta);
    inst.rtl->print(o2);
    expected = std::string(
	  "0804892d *32* %pc := m[r[28]]\n"
	  "         *32* r[28] := r[28] + 4\n"
	  "0804892d RET\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

}

void FrontPentTest::testBranch() {
    DecodeResult inst;
    std::string expected;

/*    prog->pBF->UnLoad();
    prog->pBF = BinaryFile::Load(BRANCH_PENT);
    CPPUNIT_ASSERT(prog->pBF != 0); */
    // Set the text limits
    prog->getTextLimits();

    // jne
    std::ostringstream o1;
    inst = decoder->decodeInstruction(0x8048979, prog->textDelta);
    inst.rtl->print(o1);
    expected = std::string("08048979 JCOND 0x8048988, condition not equals\n");
    CPPUNIT_ASSERT_EQUAL(expected, o1.str());

    // jg
    std::ostringstream o2;
    inst = decoder->decodeInstruction(0x80489c1, prog->textDelta);
    inst.rtl->print(o2);
    expected = std::string(
	  "080489c1 JCOND 0x80489d5, condition signed greater\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    // jbe
    std::ostringstream o3;
    inst = decoder->decodeInstruction(0x8048a1b, prog->textDelta);
    inst.rtl->print(o3);
    expected = std::string(
        "08048a1b JCOND 0x8048a2a, condition unsigned less or equals\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));
}

