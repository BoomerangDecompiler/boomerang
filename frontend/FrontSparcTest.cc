/*==============================================================================
 * FILE:       FrontSparcTest.cc
 * OVERVIEW:   Provides the implementation for the FrontSparcTest class, which
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

#define HELLO_SPARC     BOOMDIR "/test/sparc/hello"
#define BRANCH_SPARC    BOOMDIR "/test/sparc/branch"
#define SSL_PATH        BOOMDIR "/frontend/machine/sparc/sparc.ssl"

#include "types.h"
#include "FrontSparcTest.h"
#include "prog.h"           // For global prog
#include "frontend.h"

/*==============================================================================
 * FUNCTION:        FrontSparcTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<FrontSparcTest> ("testExp", \
    &FrontSparcTest::name, *this))

void FrontSparcTest::registerTests(CppUnit::TestSuite* suite) {
    MYTEST(test1);
    MYTEST(test2);
    MYTEST(test3);
    MYTEST(testBranch);
    MYTEST(testDelaySlot);
}

int FrontSparcTest::countTestCases () const
{ return 3; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        FrontSparcTest::setUp
 * OVERVIEW:        Set up anything needed before all tests
 * NOTE:            Called before any tests
 * NOTE:            Also appears to be called before all tests!
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void FrontSparcTest::setUp () {
    prog.pBF = BinaryFile::Load(HELLO_SPARC);
    CPPUNIT_ASSERT(prog.pBF != 0);
    fe = FrontEnd::getInstanceFor(HELLO_SPARC, dlHandle, prog.textDelta,
      prog.limitTextHigh, decoder);
}

/*==============================================================================
 * FUNCTION:        FrontSparcTest::tearDown
 * OVERVIEW:        Delete objects created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void FrontSparcTest::tearDown () {
    prog.pBF->UnLoad();
    FrontEnd::closeInstance(fe, dlHandle);
    delete decoder;
}

/*==============================================================================
 * FUNCTION:        FrontSparcTest::test1
 * OVERVIEW:        Test decoding some sparc instructions
 *============================================================================*/
void FrontSparcTest::test1 () {
    std::ostringstream ost;

    // Set the text limits
    prog.getTextLimits();

    bool readResult = prog.RTLDict.readSSLFile(SSL_PATH, false);
    CPPUNIT_ASSERT(readResult);

    ADDRESS addr = fe->getMainEntryPoint();
    CPPUNIT_ASSERT (addr != NO_ADDRESS);

    // Decode first instruction
    DecodeResult inst = decoder->decodeInstruction(addr, prog.textDelta);
    inst.rtl->print(ost);
    
    std::string expected(
        "00010a54 *32* r[tmp] := r[14] + -112\n"
        "         *32* m[r[14] + 0] := r[16]\n"
        "         *32* m[r[14] + 4] := r[17]\n"
        "         *32* m[r[14] + 8] := r[18]\n"
        "         *32* m[r[14] + 12] := r[19]\n"
        "         *32* m[r[14] + 16] := r[20]\n"
        "         *32* m[r[14] + 20] := r[21]\n"
        "         *32* m[r[14] + 24] := r[22]\n"
        "         *32* m[r[14] + 28] := r[23]\n"
        "         *32* m[r[14] + 32] := r[24]\n"
        "         *32* m[r[14] + 36] := r[25]\n"
        "         *32* m[r[14] + 40] := r[26]\n"
        "         *32* m[r[14] + 44] := r[27]\n"
        "         *32* m[r[14] + 48] := r[28]\n"
        "         *32* m[r[14] + 52] := r[29]\n"
        "         *32* m[r[14] + 56] := r[30]\n"
        "         *32* m[r[14] + 60] := r[31]\n"
        "         *32* r[24] := r[8]\n"
        "         *32* r[25] := r[9]\n"
        "         *32* r[26] := r[10]\n"
        "         *32* r[27] := r[11]\n"
        "         *32* r[28] := r[12]\n"
        "         *32* r[29] := r[13]\n"
        "         *32* r[30] := r[14]\n"
        "         *32* r[31] := r[15]\n"
        "         *32* r[14] := r[tmp]\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(ost.str()));

    std::ostringstream o2;
    addr += inst.numBytes;
    inst = decoder->decodeInstruction(addr, prog.textDelta);
    inst.rtl->print(o2);
    expected = std::string("00010a58 *32* r[9] := 70656\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    std::ostringstream o3;
    addr += inst.numBytes;
    inst = decoder->decodeInstruction(addr, prog.textDelta);
    inst.rtl->print(o3);
    expected = std::string("00010a5c *32* r[8] := r[9] | 464\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

    delete fe;
}

void FrontSparcTest::test2() {
    DecodeResult inst;
    std::string expected;

    std::ostringstream o1;
    inst = decoder->decodeInstruction(0x10a60, prog.textDelta);
    inst.rtl->print(o1);
    expected = std::string("00010a60 CALL 0x21668()\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o1.str()));

    std::ostringstream o2;
    inst = decoder->decodeInstruction(0x10a64, prog.textDelta);
    inst.rtl->print(o2);
    expected = std::string("00010a64\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    std::ostringstream o3;
    inst = decoder->decodeInstruction(0x10a68, prog.textDelta);
    inst.rtl->print(o3);
    expected = std::string("00010a68 *32* r[24] := r[0] | 0\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

    std::ostringstream o4;
    inst = decoder->decodeInstruction(0x10a6c, prog.textDelta);
    inst.rtl->print(o4);
    expected = std::string("00010a6c JUMP 0x10a74\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o4.str()));

}

void FrontSparcTest::test3() {
    DecodeResult inst;
    std::string expected;

    std::ostringstream o1;
    inst = decoder->decodeInstruction(0x10a70, prog.textDelta);
    inst.rtl->print(o1);
    expected = std::string("00010a70\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o1.str()));

    std::ostringstream o2;
    inst = decoder->decodeInstruction(0x10a74, prog.textDelta);
    inst.rtl->print(o2);
    expected = std::string("00010a74 RET\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    std::ostringstream o3;
    inst = decoder->decodeInstruction(0x10a78, prog.textDelta);
    inst.rtl->print(o3);
    expected = std::string(
        "00010a78 *32* r[tmp] := r[0] + r[0]\n"
        "         *32* r[8] := r[24]\n"
        "         *32* r[9] := r[25]\n"
        "         *32* r[10] := r[26]\n"
        "         *32* r[11] := r[27]\n"
        "         *32* r[12] := r[28]\n"
        "         *32* r[13] := r[29]\n"
        "         *32* r[14] := r[30]\n"
        "         *32* r[15] := r[31]\n"
        "         *32* r[0] := r[tmp]\n"
        "         *32* r[16] := m[r[14] + 0]\n"
        "         *32* r[17] := m[r[14] + 4]\n"
        "         *32* r[18] := m[r[14] + 8]\n"
        "         *32* r[19] := m[r[14] + 12]\n"
        "         *32* r[20] := m[r[14] + 16]\n"
        "         *32* r[21] := m[r[14] + 20]\n"
        "         *32* r[22] := m[r[14] + 24]\n"
        "         *32* r[23] := m[r[14] + 28]\n"
        "         *32* r[24] := m[r[14] + 32]\n"
        "         *32* r[25] := m[r[14] + 36]\n"
        "         *32* r[26] := m[r[14] + 40]\n"
        "         *32* r[27] := m[r[14] + 44]\n"
        "         *32* r[28] := m[r[14] + 48]\n"
        "         *32* r[29] := m[r[14] + 52]\n"
        "         *32* r[30] := m[r[14] + 56]\n"
        "         *32* r[31] := m[r[14] + 60]\n"
        "         *32* r[0] := r[tmp]\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

}

void FrontSparcTest::testBranch() {
    DecodeResult inst;
    std::string expected;

    prog.pBF->UnLoad();
    prog.pBF = BinaryFile::Load(BRANCH_SPARC);
    CPPUNIT_ASSERT(prog.pBF != 0);

    // bne
    std::ostringstream o1;
    inst = decoder->decodeInstruction(0x10ab0, prog.textDelta);
    inst.rtl->print(o1);
    expected = std::string("00010ab0 JCOND 0x10ac8, condition not equals\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o1.str()));

    // bg
    std::ostringstream o2;
    inst = decoder->decodeInstruction(0x10af8, prog.textDelta);
    inst.rtl->print(o2);
    expected = std::string("00010af8 JCOND 0x10b10, condition signed greater\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    // bleu
    std::ostringstream o3;
    inst = decoder->decodeInstruction(0x10b44, prog.textDelta);
    inst.rtl->print(o3);
    expected = std::string(
        "00010b44 JCOND 0x10b54, condition unsigned less or equals\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));
}

void FrontSparcTest::testDelaySlot() {
    prog.pBF->UnLoad();
    prog.pBF = BinaryFile::Load(BRANCH_SPARC);
    CPPUNIT_ASSERT(prog.pBF != 0);
    prog.getTextLimits();       // Get the limits for the branch test program

    ADDRESS addr = fe->getMainEntryPoint();
    CPPUNIT_ASSERT (addr != NO_ADDRESS);

    std::string name("testDelaySlot");
    UserProc* pProc = new UserProc(name, addr);
    std::ofstream dummy;
    bool res = fe->processProc(addr, pProc, dummy, false);

    CPPUNIT_ASSERT(res == 1);
    Cfg* cfg = pProc->getCFG();
    BB_IT it;
    PBB bb = cfg->getFirstBB(it);
    std::ostringstream o1;
    bb->print(o1);
    std::string expected("Call BB:\n"
        "00010a80 *32* r[tmp] := r[14] + -120\n"
        "         *32* m[r[14] + 0] := r[16]\n"
        "         *32* m[r[14] + 4] := r[17]\n"
        "         *32* m[r[14] + 8] := r[18]\n"
        "         *32* m[r[14] + 12] := r[19]\n"
        "         *32* m[r[14] + 16] := r[20]\n"
        "         *32* m[r[14] + 20] := r[21]\n"
        "         *32* m[r[14] + 24] := r[22]\n"
        "         *32* m[r[14] + 28] := r[23]\n"
        "         *32* m[r[14] + 32] := r[24]\n"
        "         *32* m[r[14] + 36] := r[25]\n"
        "         *32* m[r[14] + 40] := r[26]\n"
        "         *32* m[r[14] + 44] := r[27]\n"
        "         *32* m[r[14] + 48] := r[28]\n"
        "         *32* m[r[14] + 52] := r[29]\n"
        "         *32* m[r[14] + 56] := r[30]\n"
        "         *32* m[r[14] + 60] := r[31]\n"
        "         *32* r[24] := r[8]\n"
        "         *32* r[25] := r[9]\n"
        "         *32* r[26] := r[10]\n"
        "         *32* r[27] := r[11]\n"
        "         *32* r[28] := r[12]\n"
        "         *32* r[29] := r[13]\n"
        "         *32* r[30] := r[14]\n"
        "         *32* r[31] := r[15]\n"
        "         *32* r[14] := r[tmp]\n"
        "00010a84 *32* r[16] := 70656\n"
        "00010a88 *32* r[16] := r[16] | 808\n"
        "00010a8c *32* r[8] := r[0] | r[16]\n"
        "00010a90 *32* r[tmp] := r[30]\n"
        "         *32* r[9] := r[30] + -20\n"
        "00010a90 CALL scanf()\n");
    std::string actual(o1.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    bb = cfg->getNextBB(it);
    CPPUNIT_ASSERT(bb);
    std::ostringstream o2;
    bb->print(o2);
    expected = std::string("Call BB:\n"
        "00010a98 *32* r[8] := r[0] | r[16]\n"
        "00010a9c *32* r[tmp] := r[30]\n"
        "         *32* r[9] := r[30] + -24\n"
        "00010a9c CALL scanf()\n");
    actual = std::string(o2.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    bb = cfg->getNextBB(it);
    CPPUNIT_ASSERT(bb);
    std::ostringstream o3;
    bb->print(o3);
    expected = std::string("Twoway BB:\n"
        "00010aa4 *32* r[8] := m[r[30] + -20]\n"
        "00010aa8 *32* r[16] := r[0] | 5\n"
        "00010aac *32* r[tmp] := r[16]\n"
        "         *32* r[0] := r[16] - r[8]\n"
        "         SUBFLAGS( r[tmp], r[8], r[0] )\n"
        "00010ab0 *32* r[8] := 70656\n"
        "00010ab0 JCOND 0x10ac8, condition not equals\n");
    actual = std::string(o3.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    bb = cfg->getNextBB(it);
    CPPUNIT_ASSERT(bb);
    std::ostringstream o4;
    bb->print(o4);
    expected = std::string("L1: Twoway BB:\n"
        "00010ac8 *32* r[8] := 70656\n"
        "00010ac8 JCOND 0x10ad8, condition equals\n");
    actual = std::string(o4.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    bb = cfg->getNextBB(it);
    CPPUNIT_ASSERT(bb);
    std::ostringstream o5;
    bb->print(o5);
    expected = std::string("Call BB:\n"
        "00010ab8 *32* r[8] := r[8] | 816\n"
        "00010ab8 CALL printf()\n");
    actual = std::string(o5.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);

}
