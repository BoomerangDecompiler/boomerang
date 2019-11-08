#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "X86FrontEndTest.h"

#include "boomerang-plugins/frontend/x86/X86FrontEnd.h"

#include "boomerang/db/Prog.h"
#include "boomerang/ifc/IDecoder.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/util/Types.h"
#include "boomerang/util/log/Log.h"

#include <QDebug>


#define HELLO_X86         getFullSamplePath("x86/hello")
#define BRANCH_X86        getFullSamplePath("x86/branch")
#define FEDORA2_TRUE_X86  getFullSamplePath("x86/fedora2_true")
#define FEDORA3_TRUE_X86  getFullSamplePath("x86/fedora3_true")
#define SUSE_TRUE_X86     getFullSamplePath("x86/suse_true")


void X86FrontEndTest::test1()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_X86));
    Prog *prog = m_project.getProg();
    X86FrontEnd *fe = dynamic_cast<X86FrontEnd *>(prog->getFrontEnd());
    QVERIFY(fe != nullptr);

    QString     expected;
    QString     actual;
    OStream strm(&actual);

    bool    gotMain;
    Address addr = fe->findMainEntryPoint(gotMain);
    QVERIFY(gotMain && addr != Address::INVALID);

    // Decode first instruction
    MachineInstruction insn;
    DecodeResult lifted;
    QVERIFY(fe->decodeInstruction(addr, insn, lifted));
    lifted.rtl->print(strm);

    expected = "0x08048328    0 *32* m[r28 - 4] := r29\n"
               "              0 *32* r28 := r28 - 4\n";
    QCOMPARE(actual, expected);
    actual.clear();

    addr += lifted.numBytes;
    QVERIFY(fe->decodeInstruction(addr, insn, lifted));
    lifted.rtl->print(strm);
    expected = QString("0x08048329    0 *32* r29 := r28\n");
    QCOMPARE(actual, expected);
    actual.clear();

    addr = Address(0x804833b);
    QVERIFY(fe->decodeInstruction(addr, insn, lifted));
    lifted.rtl->print(strm);
    expected = QString("0x0804833b    0 *32* m[r28 - 4] := 0x80483fc\n"
                       "              0 *32* r28 := r28 - 4\n");
    QCOMPARE(actual, expected);
    actual.clear();
}


void X86FrontEndTest::test2()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_X86));
    Prog *prog = m_project.getProg();
    X86FrontEnd *fe = dynamic_cast<X86FrontEnd *>(prog->getFrontEnd());
    QVERIFY(fe != nullptr);

    MachineInstruction insn;
    DecodeResult lifted;
    QString      expected;
    QString      actual;
    OStream  strm(&actual);

    QVERIFY(fe->decodeInstruction(Address(0x08048345), insn, lifted));
    lifted.rtl->print(strm);
    expected = QString("0x08048345    0 *32* tmp1 := r28\n"
                       "              0 *32* r28 := r28 + 16\n"
                       "              0 *v* %flags := ADDFLAGS32( tmp1, 16, r28 )\n");
    QCOMPARE(actual, expected);
    actual.clear();

    QVERIFY(fe->decodeInstruction(Address(0x08048348), insn, lifted));
    lifted.rtl->print(strm);
    expected = QString("0x08048348    0 *32* r24 := 0\n");
    QCOMPARE(actual, expected);
    actual.clear();

    QVERIFY(fe->decodeInstruction(Address(0x8048329), insn, lifted));
    lifted.rtl->print(strm);
    expected = QString("0x08048329    0 *32* r29 := r28\n");
    QCOMPARE(actual, expected);
    actual.clear();
}


void X86FrontEndTest::test3()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_X86));
    Prog *prog = m_project.getProg();
    X86FrontEnd *fe = dynamic_cast<X86FrontEnd *>(prog->getFrontEnd());
    QVERIFY(fe != nullptr);

    MachineInstruction insn;
    DecodeResult lifted;
    QString      expected;
    QString      actual;
    OStream  strm(&actual);

    QVERIFY(fe->decodeInstruction(Address(0x804834d), insn, lifted));
    lifted.rtl->print(strm);
    expected = QString("0x0804834d    0 *32* r28 := r29\n"
                       "              0 *32* r29 := m[r28]\n"
                       "              0 *32* r28 := r28 + 4\n");
    QCOMPARE(actual, expected);
    actual.clear();

    QVERIFY(fe->decodeInstruction(Address(0x804834e), insn, lifted));
    lifted.rtl->print(strm);
    expected = QString("0x0804834e    0 *32* %pc := m[r28]\n"
                       "              0 *32* r28 := r28 + 4\n"
                       "              0 RET\n"
                       "              Modifieds: <None>\n"
                       "              Reaching definitions: <None>\n");

    QCOMPARE(actual, expected);
    actual.clear();
}


void X86FrontEndTest::testBranch()
{
    QVERIFY(m_project.loadBinaryFile(BRANCH_X86));
    Prog *prog = m_project.getProg();
    X86FrontEnd *fe = dynamic_cast<X86FrontEnd *>(prog->getFrontEnd());
    QVERIFY(fe != nullptr);

    MachineInstruction insn;
    DecodeResult lifted;
    QString      expected;
    QString      actual;
    OStream  strm(&actual);

    // jne
    QVERIFY(fe->decodeInstruction(Address(0x8048979), insn, lifted));
    lifted.rtl->print(strm);
    expected = QString("0x08048979    0 BRANCH 0x08048988, condition "
                       "not equals\n"
                       "High level: %flags\n");
    QCOMPARE(actual, expected);
    actual.clear();

    // jg
    QVERIFY(fe->decodeInstruction(Address(0x80489c1), insn, lifted));
    lifted.rtl->print(strm);
    expected = QString("0x080489c1    0 BRANCH 0x080489d5, condition signed greater\n"
                       "High level: %flags\n");
    QCOMPARE(actual, expected);
    actual.clear();

    // jbe
    QVERIFY(fe->decodeInstruction(Address(0x8048a1b), insn, lifted));
    lifted.rtl->print(strm);
    expected = QString("0x08048a1b    0 BRANCH 0x08048a2a, condition unsigned less or equals\n"
                       "High level: %flags\n");
    QCOMPARE(actual, expected);
    actual.clear();
}


void X86FrontEndTest::testFindMain()
{
    // Test the algorithm for finding main, when there is a call to __libc_start_main
    // Also tests the loader hack
    {
        QVERIFY(m_project.loadBinaryFile(FEDORA2_TRUE_X86));

        Prog *prog = m_project.getProg();
        IFrontEnd *fe = prog->getFrontEnd();
        QVERIFY(fe != nullptr);

        bool    found;
        Address addr     = fe->findMainEntryPoint(found);
        Address expected = Address(0x08048b10);
        QCOMPARE(addr, expected);
    }

    {
        QVERIFY(m_project.loadBinaryFile(FEDORA3_TRUE_X86));
        Prog *prog = m_project.getProg();
        IFrontEnd *fe = prog->getFrontEnd();
        QVERIFY(fe != nullptr);

        bool found;
        Address addr     = fe->findMainEntryPoint(found);
        Address expected = Address(0x8048c4a);
        QCOMPARE(addr, expected);
    }

    {
        QVERIFY(m_project.loadBinaryFile(SUSE_TRUE_X86));

        Prog *prog = m_project.getProg();
        IFrontEnd *fe = prog->getFrontEnd();
        QVERIFY(fe != nullptr);

        bool found;
        Address addr     = fe->findMainEntryPoint(found);
        Address expected = Address(0x8048b60);
        QCOMPARE(addr, expected);
    }
}


QTEST_GUILESS_MAIN(X86FrontEndTest)
