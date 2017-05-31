/***************************************************************************/ /**
  * \file       FrontSparcTest.cpp
  * OVERVIEW:   Provides the implementation for the FrontSparcTest class, which
  *                tests the sparc front end
  *============================================================================*/

#include "FrontSparcTest.h"

#include "include/types.h"
#include "include/proc.h"
#include "include/prog.h"
#include "include/frontend.h"
#include "sparcfrontend.h"
#include "include/cfg.h"
#include "include/rtl.h"
#include "boom_base/BinaryFile.h"
#include "boom_base/BinaryFileStub.h"
#include "boom_base/log.h"
#include "include/basicblock.h"
#include "boom_base/log.h"

#include <QDir>
#include <QProcessEnvironment>
#include <QDebug>

#define HELLO_SPARC baseDir.absoluteFilePath("tests/inputs/sparc/hello")
#define BRANCH_SPARC baseDir.absoluteFilePath("tests/inputs/sparc/branch")
static bool logset = false;
static QString TEST_BASE;
static QDir baseDir;
void FrontSparcTest::initTestCase() {
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
  * FUNCTION:        FrontSparcTest::test1
  * OVERVIEW:        Test decoding some sparc instructions
  *============================================================================*/
void FrontSparcTest::test1() {

    QString expected;
    QString actual;
    QTextStream strm(&actual);
    BinaryFileFactory bff;
    QObject *pBF = bff.Load(HELLO_SPARC);
    QVERIFY(pBF != 0);
    Prog *prog = new Prog(HELLO_SPARC);
    LoaderInterface *iface = qobject_cast<LoaderInterface *>(pBF);
    QVERIFY(iface->getMachine() == MACHINE_SPARC);
    FrontEnd *pFE = new SparcFrontEnd(pBF, prog, &bff);
    prog->setFrontEnd(pFE);

    bool gotMain;
    ADDRESS addr = pFE->getMainEntryPoint(gotMain);
    QVERIFY(addr != NO_ADDRESS);

    // Decode first instruction
    DecodeResult inst = pFE->decodeInstruction(addr);
    QVERIFY(inst.rtl != nullptr);
    inst.rtl->print(strm);

    expected =   "00010684    0 *32* tmp := r14 - 112\n"
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
                 "            0 *32* r14 := tmp\n";
    QCOMPARE(actual,expected);
    actual.clear();

    addr += inst.numBytes;
    inst = pFE->decodeInstruction(addr);
    inst.rtl->print(strm);
    expected = QString("00010688    0 *32* r8 := 0x10400\n");
    QCOMPARE(actual,expected);
    actual.clear();

    addr += inst.numBytes;
    inst = pFE->decodeInstruction(addr);
    inst.rtl->print(strm);
    expected = QString("0001068c    0 *32* r8 := r8 | 848\n");
    QCOMPARE(actual,expected);
    actual.clear();

    delete pFE;
    // delete pBF;
}

void FrontSparcTest::test2() {
    DecodeResult inst;
    QString expected;
    QString actual;
    QTextStream strm(&actual);
    BinaryFileFactory bff;
    QObject *pBF = bff.Load(HELLO_SPARC);
    QVERIFY(pBF != 0);
    Prog *prog = new Prog(HELLO_SPARC);
    LoaderInterface *iface = qobject_cast<LoaderInterface *>(pBF);
    QVERIFY(iface->getMachine() == MACHINE_SPARC);
    FrontEnd *pFE = new SparcFrontEnd(pBF, prog, &bff);
    prog->setFrontEnd(pFE);

    inst = pFE->decodeInstruction(ADDRESS::g(0x10690));
    inst.rtl->print(strm);
    // This call is to out of range of the program's text limits (to the Program Linkage Table (PLT), calling printf)
    // This is quite normal.
    expected = QString("00010690    0 CALL printf(\n"
                           "              )\n"
                           "              Reaching definitions: \n"
                           "              Live variables: \n");
    QCOMPARE(actual,expected);
    actual.clear();

    inst = pFE->decodeInstruction(ADDRESS::g(0x10694));
    inst.rtl->print(strm);
    expected = QString("00010694\n");
    QCOMPARE(actual,expected);
    actual.clear();

    inst = pFE->decodeInstruction(ADDRESS::g(0x10698));
    inst.rtl->print(strm);
    expected = QString("00010698    0 *32* r8 := 0\n");
    QCOMPARE(actual,expected);
    actual.clear();

    inst = pFE->decodeInstruction(ADDRESS::g(0x1069c));
    inst.rtl->print(strm);
    expected = QString("0001069c    0 *32* r24 := r8\n");
    QCOMPARE(actual,expected);

    delete pFE;
    // delete pBF;
}

void FrontSparcTest::test3() {
    DecodeResult inst;
    QString expected;
    QString actual;
    QTextStream strm(&actual);
    BinaryFileFactory bff;
    QObject *pBF = bff.Load(HELLO_SPARC);
    QVERIFY(pBF != 0);
    Prog *prog = new Prog(HELLO_SPARC);
    LoaderInterface *iface = qobject_cast<LoaderInterface *>(pBF);
    QVERIFY(iface->getMachine() == MACHINE_SPARC);
    FrontEnd *pFE = new SparcFrontEnd(pBF, prog, &bff);
    prog->setFrontEnd(pFE);

    inst = pFE->decodeInstruction(ADDRESS::g(0x106a0));
    inst.rtl->print(strm);
    expected = QString("000106a0\n");
    QCOMPARE(actual,expected);
    actual.clear();
    inst = pFE->decodeInstruction(ADDRESS::g(0x106a4));
    inst.rtl->print(strm);
    expected = QString("000106a4    0 RET\n"
                           "              Modifieds: \n"
                           "              Reaching definitions: \n");
    QCOMPARE(actual,expected);
    actual.clear();

    inst = pFE->decodeInstruction(ADDRESS::g(0x106a8));
    inst.rtl->print(strm);
    expected = QString("000106a8    0 *32* tmp := 0\n"
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
    QCOMPARE(actual,expected);

    delete pFE;
    // delete pBF;
}

void FrontSparcTest::testBranch() {
    DecodeResult inst;
    QString expected;
    QString actual;
    QTextStream strm(&actual);
    BinaryFileFactory bff;
    QObject *pBF = bff.Load(BRANCH_SPARC);
    QVERIFY(pBF != 0);
    Prog *prog = new Prog(BRANCH_SPARC);
    LoaderInterface *iface = qobject_cast<LoaderInterface *>(pBF);
    QVERIFY(iface->getMachine() == MACHINE_SPARC);
    FrontEnd *pFE = new SparcFrontEnd(pBF, prog, &bff);
    prog->setFrontEnd(pFE);

    // bne
    inst = pFE->decodeInstruction(ADDRESS::g(0x10ab0));
    inst.rtl->print(strm);
    expected = QString("00010ab0    0 BRANCH 0x10ac8, condition not equals\n"
                           "High level: %flags\n");
    QCOMPARE(actual,expected);
    actual.clear();

    // bg
    inst = pFE->decodeInstruction(ADDRESS::g(0x10af8));
    inst.rtl->print(strm);
    expected = QString("00010af8    0 BRANCH 0x10b10, condition "
                           "signed greater\n"
                           "High level: %flags\n");
    QCOMPARE(actual,expected);
    actual.clear();

    // bleu
    inst = pFE->decodeInstruction(ADDRESS::g(0x10b44));
    inst.rtl->print(strm);
    expected = QString("00010b44    0 BRANCH 0x10b54, condition unsigned less or equals\n"
                           "High level: %flags\n");
    QCOMPARE(actual,expected);
    actual.clear();

    delete pFE;
    // delete pBF;
}

void FrontSparcTest::testDelaySlot() {

    BinaryFileFactory bff;
    QObject *pBF = bff.Load(BRANCH_SPARC);
    QVERIFY(pBF != 0);
    Prog *prog = new Prog(BRANCH_SPARC);
    LoaderInterface *iface = qobject_cast<LoaderInterface *>(pBF);
    QVERIFY(iface->getMachine() == MACHINE_SPARC);
    FrontEnd *pFE = new SparcFrontEnd(pBF, prog, &bff);
    prog->setFrontEnd(pFE);
    // decode calls readLibraryCatalog(), which needs to have definitions for non-sparc architectures cleared
    Type::clearNamedTypes();
    pFE->decode(prog);

    bool gotMain;
    ADDRESS addr = pFE->getMainEntryPoint(gotMain);
    QVERIFY(addr != NO_ADDRESS);
    QString actual;
    QTextStream strm(&actual);
    Module *m = prog->getOrInsertModule("test");

    UserProc *pProc = new UserProc(m, "testDelaySlot", addr);
    QString dum;
    QTextStream dummy(&dum);
    bool res = pFE->processProc(addr, pProc, dummy, false);

    QVERIFY(res == 1);
    Cfg *cfg = pProc->getCFG();
    BB_IT it;
    BasicBlock *bb = cfg->getFirstBB(it);
    bb->print(strm);
    QString expected("Call BB:\n"
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

    QCOMPARE(actual,expected);
    actual.clear();

    bb = cfg->getNextBB(it);
    QVERIFY(bb);
    bb->print(strm);
    expected = "Call BB:\n"
               "in edges: 10a90(10a80) \n"
               "out edges: 10aa4 \n"
               "00010a98    0 *32* r8 := r16\n"
               "00010a9c    0 *32* tmp := r30\n"
               "            0 *32* r9 := r30 - 24\n"
               "00010a9c    0 CALL scanf(\n"
               "              )\n"
               "              Reaching definitions: \n"
               "              Live variables: \n";

    QCOMPARE(actual,expected);
    actual.clear();

    bb = cfg->getNextBB(it);
    QVERIFY(bb);
    bb->print(strm);
    expected = "Twoway BB:\n"
               "in edges: 10a9c(10a98) \n"
               "out edges: 10ac8 10ab8 \n"
               "00010aa4    0 *32* r8 := m[r30 - 20]\n"
               "00010aa8    0 *32* r16 := 5\n"
               "00010aac    0 *32* tmp := r16\n"
               "            0 *32* r0 := r16 - r8\n"
               "            0 *v* %flags := SUBFLAGS( tmp, r8, r0 )\n"
               "00010ab0    0 *32* r8 := 0x11400\n"
               "00010ab0    0 BRANCH 0x10ac8, condition not equals\n"
               "High level: %flags\n";
    QCOMPARE(actual,expected);
    actual.clear();

    bb = cfg->getNextBB(it);
    QVERIFY(bb);
    bb->print(strm);
    expected = "L1: Twoway BB:\n"
               "in edges: 10ab0(10aa4) 10ac4(10ac0) \n"
               "out edges: 10ad8 10ad0 \n"
               "00010ac8    0 *32* r8 := 0x11400\n"
               "00010ac8    0 BRANCH 0x10ad8, condition equals\n"
               "High level: %flags\n";
    QCOMPARE(actual,expected);
    actual.clear();

    bb = cfg->getNextBB(it);
    QVERIFY(bb);
    bb->print(strm);
    expected = "Call BB:\n"
               "in edges: 10ab0(10aa4) \n"
               "out edges: 10ac0 \n"
               "00010ab8    0 *32* r8 := r8 | 816\n"
               "00010ab8    0 CALL printf(\n"
               "              )\n"
               "              Reaching definitions: \n"
               "              Live variables: \n";

    QCOMPARE(actual,expected);

    delete prog;
}
QTEST_MAIN(FrontSparcTest)
