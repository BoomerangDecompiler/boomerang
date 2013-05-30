/***************************************************************************//**
 * \file       FrontSparcTest.cc
 * OVERVIEW:   Provides the implementation for the FrontSparcTest class, which
 *                tests the sparc front end
 *============================================================================*/
/*
 * $Revision$
 *
 * 05 Apr 02 - Mike: Created
 * 21 May 02 - Mike: Mods for gcc 3.1
 * 04 Dec 02 - Mike: Changed all r[0] to 0
 */

#include "types.h"
#include "FrontSparcTest.h"
#include "proc.h"
#include "prog.h"
#include "frontend.h"
#include "sparcfrontend.h"
#include "cfg.h"
#include "BinaryFile.h"
#include "BinaryFileStub.h"
#include "boomerang.h"
#include "log.h"
CPPUNIT_TEST_SUITE_REGISTRATION( FrontSparcTest );

#define HELLO_SPARC     "test/sparc/hello"
#define BRANCH_SPARC    "test/sparc/branch"

static bool logset = false;
/***************************************************************************//**
 * \brief   Set up anything needed before all tests
 * \note    Called before any tests
 * \note    Also appears to be called before all tests!
 *============================================================================*/
void FrontSparcTest::setUp () {
    if (!logset) {
        logset = true;
        Boomerang::get()->setLogger(new NullLogger());
    }
}

/***************************************************************************//**
 * FUNCTION:        FrontSparcTest::tearDown
 * OVERVIEW:        Delete objects created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:        <none>
 *
 *============================================================================*/
void FrontSparcTest::tearDown () {
}

/***************************************************************************//**
 * FUNCTION:        FrontSparcTest::test1
 * OVERVIEW:        Test decoding some sparc instructions
 *============================================================================*/
void FrontSparcTest::test1 () {
    std::ostringstream ost;

    BinaryFileFactory bff;
    BinaryFile *pBF = bff.Load(HELLO_SPARC);
    if (pBF == nullptr)
        pBF = new BinaryFileStub();       // fallback on stub
    CPPUNIT_ASSERT(pBF != 0);
    CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_SPARC);
    Prog* prog = new Prog;
    FrontEnd *pFE = new SparcFrontEnd(pBF, prog, &bff);
    prog->setFrontEnd(pFE);

    bool gotMain;
    ADDRESS addr = pFE->getMainEntryPoint(gotMain);
    CPPUNIT_ASSERT (addr != NO_ADDRESS);

    // Decode first instruction
    DecodeResult inst = pFE->decodeInstruction(addr);
    CPPUNIT_ASSERT(inst.rtl != nullptr);
    inst.rtl->print(ost);

    std::string expected(
        "00010684    0 *32* tmp := r14 - 112\n"
        "            0 *32* m[r14] := r16\n"
        "            0 *32* m[r14 + 4] := r17\n"
        "            0 *32* m[r14 + 8] := r18\n"
        "            0 *32* m[r14 + 12] := r19\n"
        "            0 *32* m[r14 + 16] := r20\n"
        "            0 *32* m[r14 + 20] := r21\n"
        "            0 *32* m[r14 + 24] := r22\n"
        "            0 *32* m[r14 + 28] := r23\n"
        "            0 *32* m[r14 + 32] := r24\n"
        "            0 *32* m[r14 + 36] := r25\n"
        "            0 *32* m[r14 + 40] := r26\n"
        "            0 *32* m[r14 + 44] := r27\n"
        "            0 *32* m[r14 + 48] := r28\n"
        "            0 *32* m[r14 + 52] := r29\n"
        "            0 *32* m[r14 + 56] := r30\n"
        "            0 *32* m[r14 + 60] := r31\n"
        "            0 *32* r24 := r8\n"
        "            0 *32* r25 := r9\n"
        "            0 *32* r26 := r10\n"
        "            0 *32* r27 := r11\n"
        "            0 *32* r28 := r12\n"
        "            0 *32* r29 := r13\n"
        "            0 *32* r30 := r14\n"
        "            0 *32* r31 := r15\n"
        "            0 *32* r14 := tmp\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(ost.str()));

    std::ostringstream o2;
    addr += inst.numBytes;
    inst = pFE->decodeInstruction(addr);
    inst.rtl->print(o2);
    expected = std::string("00010688    0 *32* r8 := 0x10400\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    std::ostringstream o3;
    addr += inst.numBytes;
    inst = pFE->decodeInstruction(addr);
    inst.rtl->print(o3);
    expected = std::string("0001068c    0 *32* r8 := r8 | 848\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

    delete pFE;
    //delete pBF;
}

void FrontSparcTest::test2() {
    DecodeResult inst;
    std::string expected;

    BinaryFileFactory bff;
    BinaryFile *pBF = bff.Load(HELLO_SPARC);
    if (pBF == nullptr)
        pBF = new BinaryFileStub();       // fallback on stub
    CPPUNIT_ASSERT(pBF != 0);
    CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_SPARC);
    Prog* prog = new Prog;
    FrontEnd *pFE = new SparcFrontEnd(pBF, prog, &bff);
    prog->setFrontEnd(pFE);

    std::ostringstream o1;
    inst = pFE->decodeInstruction(0x10690);
    inst.rtl->print(o1);
    // This call is to out of range of the program's text limits (to the Program Linkage Table (PLT), calling printf)
    // This is quite normal.
    expected = std::string("00010690    0 CALL printf(\n"
        "              )\n"
        "              Reaching definitions: \n"
        "              Live variables: \n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o1.str()));

    std::ostringstream o2;
    inst = pFE->decodeInstruction(0x10694);
    inst.rtl->print(o2);
    expected = std::string("00010694\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    std::ostringstream o3;
    inst = pFE->decodeInstruction(0x10698);
    inst.rtl->print(o3);
    expected = std::string("00010698    0 *32* r8 := 0\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

    std::ostringstream o4;
    inst = pFE->decodeInstruction(0x1069c);
    inst.rtl->print(o4);
    expected = std::string("0001069c    0 *32* r24 := r8\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o4.str()));

    delete pFE;
    // delete pBF;
}

void FrontSparcTest::test3() {
    DecodeResult inst;
    std::string expected;

    BinaryFileFactory bff;
    BinaryFile *pBF = bff.Load(HELLO_SPARC);
    if (pBF == nullptr)
        pBF = new BinaryFileStub();       // fallback on stub
    CPPUNIT_ASSERT(pBF != 0);
    CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_SPARC);
    Prog* prog = new Prog;
    FrontEnd *pFE = new SparcFrontEnd(pBF, prog, &bff);
    prog->setFrontEnd(pFE);

    std::ostringstream o1;
    inst = pFE->decodeInstruction(0x106a0);
    inst.rtl->print(o1);
    expected = std::string("000106a0\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o1.str()));

    std::ostringstream o2;
    inst = pFE->decodeInstruction(0x106a4);
    inst.rtl->print(o2);
    expected = std::string("000106a4    0 RET\n"
        "              Modifieds: \n"
        "              Reaching definitions: \n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    std::ostringstream o3;
    inst = pFE->decodeInstruction(0x106a8);
    inst.rtl->print(o3);
    expected = std::string(
        "000106a8    0 *32* tmp := 0\n"
        "            0 *32* r8 := r24\n"
        "            0 *32* r9 := r25\n"
        "            0 *32* r10 := r26\n"
        "            0 *32* r11 := r27\n"
        "            0 *32* r12 := r28\n"
        "            0 *32* r13 := r29\n"
        "            0 *32* r14 := r30\n"
        "            0 *32* r15 := r31\n"
        "            0 *32* r0 := tmp\n"
        "            0 *32* r16 := m[r14]\n"
        "            0 *32* r17 := m[r14 + 4]\n"
        "            0 *32* r18 := m[r14 + 8]\n"
        "            0 *32* r19 := m[r14 + 12]\n"
        "            0 *32* r20 := m[r14 + 16]\n"
        "            0 *32* r21 := m[r14 + 20]\n"
        "            0 *32* r22 := m[r14 + 24]\n"
        "            0 *32* r23 := m[r14 + 28]\n"
        "            0 *32* r24 := m[r14 + 32]\n"
        "            0 *32* r25 := m[r14 + 36]\n"
        "            0 *32* r26 := m[r14 + 40]\n"
        "            0 *32* r27 := m[r14 + 44]\n"
        "            0 *32* r28 := m[r14 + 48]\n"
        "            0 *32* r29 := m[r14 + 52]\n"
        "            0 *32* r30 := m[r14 + 56]\n"
        "            0 *32* r31 := m[r14 + 60]\n"
        "            0 *32* r0 := tmp\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

    delete pFE;
    // delete pBF;
}

void FrontSparcTest::testBranch() {
    DecodeResult inst;
    std::string expected;

    BinaryFileFactory bff;
    BinaryFile *pBF = bff.Load(BRANCH_SPARC);
    if (pBF == nullptr)
        pBF = new BinaryFileStub();       // fallback on stub
    CPPUNIT_ASSERT(pBF != 0);
    CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_SPARC);
    Prog* prog = new Prog;
    FrontEnd *pFE = new SparcFrontEnd(pBF, prog, &bff);
    prog->setFrontEnd(pFE);

    // bne
    std::ostringstream o1;
    inst = pFE->decodeInstruction(0x10ab0);
    inst.rtl->print(o1);
    expected = std::string(
      "00010ab0    0 BRANCH 0x10ac8, condition not equals\n"
      "High level: %flags\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o1.str()));

    // bg
    std::ostringstream o2;
    inst = pFE->decodeInstruction(0x10af8);
    inst.rtl->print(o2);
    expected = std::string("00010af8    0 BRANCH 0x10b10, condition "
      "signed greater\n"
      "High level: %flags\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o2.str()));

    // bleu
    std::ostringstream o3;
    inst = pFE->decodeInstruction(0x10b44);
    inst.rtl->print(o3);
    expected = std::string(
        "00010b44    0 BRANCH 0x10b54, condition unsigned less or equals\n"
        "High level: %flags\n");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(o3.str()));

    delete pFE;
    // delete pBF;
}

void FrontSparcTest::testDelaySlot() {

    BinaryFileFactory bff;
    BinaryFile *pBF = bff.Load(BRANCH_SPARC);
    if (pBF == nullptr)
        pBF = new BinaryFileStub();       // fallback on stub
    CPPUNIT_ASSERT(pBF != 0);
    CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_SPARC);
    Prog* prog = new Prog;
    FrontEnd *pFE = new SparcFrontEnd(pBF, prog, &bff);
    prog->setFrontEnd(pFE);
    // decode calls readLibraryCatalog(), which needs to have definitions for non-sparc architectures cleared
    Type::clearNamedTypes();
    pFE->decode(prog);

    bool gotMain;
    ADDRESS addr = pFE->getMainEntryPoint(gotMain);
    CPPUNIT_ASSERT (addr != NO_ADDRESS);

    std::string name("testDelaySlot");
    UserProc* pProc = new UserProc(prog, name, addr);
    std::ofstream dummy;
    bool res = pFE->processProc(addr, pProc, dummy, false);

    CPPUNIT_ASSERT(res == 1);
    Cfg* cfg = pProc->getCFG();
    BB_IT it;
    PBB bb = cfg->getFirstBB(it);
    std::ostringstream o1;
    bb->print(o1);
    std::string expected("Call BB:\n"
        "in edges: \n"
        "out edges: 10a98 \n"
        "00010a80    0 *32* tmp := r14 - 120\n"
        "            0 *32* m[r14] := r16\n"
        "            0 *32* m[r14 + 4] := r17\n"
        "            0 *32* m[r14 + 8] := r18\n"
        "            0 *32* m[r14 + 12] := r19\n"
        "            0 *32* m[r14 + 16] := r20\n"
        "            0 *32* m[r14 + 20] := r21\n"
        "            0 *32* m[r14 + 24] := r22\n"
        "            0 *32* m[r14 + 28] := r23\n"
        "            0 *32* m[r14 + 32] := r24\n"
        "            0 *32* m[r14 + 36] := r25\n"
        "            0 *32* m[r14 + 40] := r26\n"
        "            0 *32* m[r14 + 44] := r27\n"
        "            0 *32* m[r14 + 48] := r28\n"
        "            0 *32* m[r14 + 52] := r29\n"
        "            0 *32* m[r14 + 56] := r30\n"
        "            0 *32* m[r14 + 60] := r31\n"
        "            0 *32* r24 := r8\n"
        "            0 *32* r25 := r9\n"
        "            0 *32* r26 := r10\n"
        "            0 *32* r27 := r11\n"
        "            0 *32* r28 := r12\n"
        "            0 *32* r29 := r13\n"
        "            0 *32* r30 := r14\n"
        "            0 *32* r31 := r15\n"
        "            0 *32* r14 := tmp\n"
        "00010a84    0 *32* r16 := 0x11400\n"
        "00010a88    0 *32* r16 := r16 | 808\n"
        "00010a8c    0 *32* r8 := r16\n"
        "00010a90    0 *32* tmp := r30\n"
        "            0 *32* r9 := r30 - 20\n"
        "00010a90    0 CALL scanf(\n"
        "              )\n"
        "              Reaching definitions: \n"
        "              Live variables: \n");

    std::string actual(o1.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    bb = cfg->getNextBB(it);
    CPPUNIT_ASSERT(bb);
    std::ostringstream o2;
    bb->print(o2);
    expected = std::string("Call BB:\n"
        "in edges: 10a90 \n"
        "out edges: 10aa4 \n"
        "00010a98    0 *32* r8 := r16\n"
        "00010a9c    0 *32* tmp := r30\n"
        "            0 *32* r9 := r30 - 24\n"
        "00010a9c    0 CALL scanf(\n"
        "              )\n"
        "              Reaching definitions: \n"
        "              Live variables: \n");

    actual = std::string(o2.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    bb = cfg->getNextBB(it);
    CPPUNIT_ASSERT(bb);
    std::ostringstream o3;
    bb->print(o3);
    expected = std::string("Twoway BB:\n"
    "in edges: 10a9c \n"
    "out edges: 10ac8 10ab8 \n"
    "00010aa4    0 *32* r8 := m[r30 - 20]\n"
    "00010aa8    0 *32* r16 := 5\n"
    "00010aac    0 *32* tmp := r16\n"
    "            0 *32* r0 := r16 - r8\n"
    "            0 *v* %flags := SUBFLAGS( tmp, r8, r0 )\n"
    "00010ab0    0 *32* r8 := 0x11400\n"
    "00010ab0    0 BRANCH 0x10ac8, condition not equals\n"
    "High level: %flags\n");
    actual = std::string(o3.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    bb = cfg->getNextBB(it);
    CPPUNIT_ASSERT(bb);
    std::ostringstream o4;
    bb->print(o4);
    expected = std::string("L1: Twoway BB:\n"
        "in edges: 10ab0 10ac4 \n"
        "out edges: 10ad8 10ad0 \n"
        "00010ac8    0 *32* r8 := 0x11400\n"
        "00010ac8    0 BRANCH 0x10ad8, condition equals\n"
        "High level: %flags\n");
    actual = std::string(o4.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    bb = cfg->getNextBB(it);
    CPPUNIT_ASSERT(bb);
    std::ostringstream o5;
    bb->print(o5);
    expected = std::string("Call BB:\n"
        "in edges: 10ab0 \n"
        "out edges: 10ac0 \n"
        "00010ab8    0 *32* r8 := r8 | 816\n"
        "00010ab8    0 CALL printf(\n"
        "              )\n"
        "              Reaching definitions: \n"
        "              Live variables: \n");

    actual = std::string(o5.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    delete prog;
}
