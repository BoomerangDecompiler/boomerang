#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License


/**
 * \file FrontPentTest.cpp
 * Provides the implementation for the FrontPentTest class, which
 * tests the Pentium frontend
 */
#include "FrontPentTest.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/Prog.h"
#include "boomerang/core/Project.h"
#include "boomerang/frontend/Decoder.h"
#include "boomerang/util/Types.h"
#include "boomerang/util/Log.h"

#include "boomerang/frontend/pentium/pentiumfrontend.h"

#include <QDebug>


#define HELLO_PENT      (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/hello"))
#define BRANCH_PENT     (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/branch"))
#define FEDORA2_TRUE    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/fedora2_true"))
#define FEDORA3_TRUE    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/fedora3_true"))
#define SUSE_TRUE       (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/suse_true"))


void FrontPentTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
}


void FrontPentTest::test1()
{
    QString     expected;
    QString     actual;
    QTextStream strm(&actual);

    IProject& project = *Boomerang::get()->getOrCreateProject();

    project.loadBinaryFile(HELLO_PENT);
    IFileLoader *loader = project.getBestLoader(HELLO_PENT);

    QVERIFY(loader != nullptr);

    Prog *prog = new Prog(HELLO_PENT);
    QVERIFY(loader->getMachine() == Machine::PENTIUM);

    IFrontEnd *pFE = new PentiumFrontEnd(loader, prog);
    prog->setFrontEnd(pFE);

    bool    gotMain;
    Address addr = pFE->getMainEntryPoint(gotMain);
    QVERIFY(gotMain && addr != Address::INVALID);

    // Decode first instruction
    DecodeResult inst;
    pFE->decodeInstruction(addr, inst);
    inst.rtl->print(strm);

    expected = "0x08048328    0 *32* m[r28 - 4] := r29\n"
               "              0 *32* r28 := r28 - 4\n";
    QCOMPARE(actual, expected);
    actual.clear();

    addr += inst.numBytes;
    pFE->decodeInstruction(addr, inst);
    inst.rtl->print(strm);
    expected = QString("0x08048329    0 *32* r29 := r28\n");
    QCOMPARE(actual, expected);
    actual.clear();

    addr = Address(0x804833b);
    pFE->decodeInstruction(addr, inst);
    inst.rtl->print(strm);
    expected = QString("0x0804833b    0 *32* m[r28 - 4] := 0x80483fc\n"
                       "              0 *32* r28 := r28 - 4\n");
    QCOMPARE(actual, expected);
    actual.clear();

    delete pFE;
}


void FrontPentTest::test2()
{
    DecodeResult inst;
    QString      expected;
    QString      actual;
    QTextStream  strm(&actual);

    IProject& project = *Boomerang::get()->getOrCreateProject();

    project.loadBinaryFile(HELLO_PENT);
    IFileLoader *pBF = project.getBestLoader(HELLO_PENT);

    QVERIFY(pBF != nullptr);
    Prog *prog = new Prog(HELLO_PENT);
    QVERIFY(pBF->getMachine() == Machine::PENTIUM);

    IFrontEnd *pFE = new PentiumFrontEnd(pBF, prog);
    prog->setFrontEnd(pFE);

    pFE->decodeInstruction(Address(0x08048345), inst);
    inst.rtl->print(strm);
    expected = QString("0x08048345    0 *32* tmp1 := r28\n"
                       "              0 *32* r28 := r28 + 16\n"
                       "              0 *v* %flags := ADDFLAGS32( tmp1, 16, r28 )\n");
    QCOMPARE(actual, expected);
    actual.clear();

    pFE->decodeInstruction(Address(0x08048348), inst);
    inst.rtl->print(strm);
    expected = QString("0x08048348    0 *32* r24 := 0\n");
    QCOMPARE(actual, expected);
    actual.clear();

    pFE->decodeInstruction(Address(0x8048329), inst);
    inst.rtl->print(strm);
    expected = QString("0x08048329    0 *32* r29 := r28\n");
    QCOMPARE(actual, expected);
    actual.clear();

    delete pFE;
    // delete pBF;
}


void FrontPentTest::test3()
{
    DecodeResult inst;
    QString      expected;
    QString      actual;
    QTextStream  strm(&actual);

    IProject& project = *Boomerang::get()->getOrCreateProject();

    project.loadBinaryFile(HELLO_PENT);
    IFileLoader *loader = project.getBestLoader(HELLO_PENT);

    QVERIFY(loader != nullptr);
    Prog *prog = new Prog(HELLO_PENT);
    QVERIFY(loader->getMachine() == Machine::PENTIUM);
    IFrontEnd *pFE = new PentiumFrontEnd(loader, prog);
    prog->setFrontEnd(pFE);

    pFE->decodeInstruction(Address(0x804834d), inst);
    inst.rtl->print(strm);
    expected = QString("0x0804834d    0 *32* r28 := r29\n"
                       "              0 *32* r29 := m[r28]\n"
                       "              0 *32* r28 := r28 + 4\n");
    QCOMPARE(actual, expected);
    actual.clear();

    pFE->decodeInstruction(Address(0x804834e), inst);
    inst.rtl->print(strm);
    expected = QString("0x0804834e    0 *32* %pc := m[r28]\n"
                       "              0 *32* r28 := r28 + 4\n"
                       "              0 RET\n"
                       "              Modifieds: \n"
                       "              Reaching definitions: \n");

    QCOMPARE(actual, expected);
    actual.clear();

    delete pFE;
    // delete pBF;
}


void FrontPentTest::testBranch()
{
    DecodeResult inst;
    QString      expected;
    QString      actual;
    QTextStream  strm(&actual);

    IProject& project = *Boomerang::get()->getOrCreateProject();

    project.loadBinaryFile(BRANCH_PENT);
    IFileLoader *loader = project.getBestLoader(BRANCH_PENT);
    QVERIFY(loader != nullptr);
    Prog *prog = new Prog(BRANCH_PENT);

    QVERIFY(loader->getMachine() == Machine::PENTIUM);
    IFrontEnd *pFE = new PentiumFrontEnd(loader, prog);
    prog->setFrontEnd(pFE);

    // jne
    pFE->decodeInstruction(Address(0x8048979), inst);
    inst.rtl->print(strm);
    expected = QString("0x08048979    0 BRANCH 0x08048988, condition "
                       "not equals\n"
                       "High level: %flags\n");
    QCOMPARE(actual, expected);
    actual.clear();

    // jg
    pFE->decodeInstruction(Address(0x80489c1), inst);
    inst.rtl->print(strm);
    expected = QString("0x080489c1    0 BRANCH 0x080489d5, condition signed greater\n"
                       "High level: %flags\n");
    QCOMPARE(actual, expected);
    actual.clear();

    // jbe
    pFE->decodeInstruction(Address(0x8048a1b), inst);
    inst.rtl->print(strm);
    expected = QString("0x08048a1b    0 BRANCH 0x08048a2a, condition unsigned less or equals\n"
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
    IProject& project = *Boomerang::get()->getOrCreateProject();

    project.loadBinaryFile(FEDORA2_TRUE);
    IFileLoader *loader = project.getBestLoader(FEDORA2_TRUE);
    QVERIFY(loader != nullptr);

    Prog *prog = new Prog(FEDORA2_TRUE);
    QVERIFY(loader->getMachine() == Machine::PENTIUM);

    IFrontEnd *pFE = new PentiumFrontEnd(loader, prog);
    prog->setFrontEnd(pFE);

    bool    found;
    Address addr     = pFE->getMainEntryPoint(found);
    Address expected = Address(0x08048b10);
    QCOMPARE(addr, expected);
    loader->close();
    delete pFE;

    project.loadBinaryFile(FEDORA3_TRUE);
    loader = project.getBestLoader(FEDORA3_TRUE);
    QVERIFY(loader != nullptr);
    pFE = new PentiumFrontEnd(loader, prog);
    prog->setFrontEnd(pFE);
    QVERIFY(pFE != nullptr);
    addr     = pFE->getMainEntryPoint(found);
    expected = Address(0x8048c4a);
    QCOMPARE(addr, expected);
    loader->close();
    delete pFE;

    project.loadBinaryFile(SUSE_TRUE);
    loader = project.getBestLoader(SUSE_TRUE);
    QVERIFY(loader != nullptr);

    pFE = new PentiumFrontEnd(loader, prog);
    prog->setFrontEnd(pFE);
    QVERIFY(pFE != nullptr);
    addr     = pFE->getMainEntryPoint(found);
    expected = Address(0x8048b60);
    QCOMPARE(addr, expected);
    loader->close();

    delete pFE;
}


QTEST_MAIN(FrontPentTest)
