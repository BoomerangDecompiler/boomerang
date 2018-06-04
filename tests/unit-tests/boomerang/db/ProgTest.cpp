#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ProgTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/LibProc.h"


#define HELLO_PENTIUM    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/hello"))


void ProgTest::cleanupTestCase()
{
    Boomerang::destroy();
}


void ProgTest::testName()
{
    QString progName = HELLO_PENTIUM;
    Prog prog(progName, nullptr);

    QCOMPARE(prog.getName(), progName);

    progName = "Happy Prog";
    prog.setName(progName);
    QCOMPARE(prog.getName(), progName);
}


void ProgTest::testCreateModule()
{
    Prog prog("test", nullptr);

    Module *mod = prog.createModule("");
    QVERIFY(mod != nullptr);
    QCOMPARE(mod->getUpstream(), prog.getRootModule());
    QCOMPARE(mod->getName(), QString(""));
    QCOMPARE(prog.getRootModule()->getNumChildren(), size_t(1));
    QCOMPARE(prog.getRootModule()->getChild(0), mod);
    QCOMPARE(prog.getModuleList().size(), size_t(2));

    // create exisiting module
    Module *existing = prog.createModule("");
    QVERIFY(existing == nullptr);

    Module *sub = prog.createModule("", mod);
    QVERIFY(sub != nullptr);
    QCOMPARE(sub->getUpstream(), mod);
    QCOMPARE(mod->getNumChildren(), size_t(1));
    QCOMPARE(prog.getRootModule()->getNumChildren(), size_t(1));
    QCOMPARE(mod->getChild(0), sub);
}


void ProgTest::testGetOrCreateFunction()
{
    Prog prog("test", nullptr);

    Function *func = prog.getOrCreateFunction(Address::INVALID);
    QVERIFY(func == nullptr);

    func = prog.getOrCreateFunction(Address(0x1000));
    QVERIFY(func != nullptr);
    QCOMPARE(func->getName(), QString("proc_0x00001000"));
    QCOMPARE(func->getEntryAddress(), Address(0x1000));
}


void ProgTest::testGetOrCreateLibraryProc()
{
    Prog prog("test", nullptr);

    LibProc *libProc = prog.getOrCreateLibraryProc("");
    QVERIFY(libProc == nullptr);

    libProc = prog.getOrCreateLibraryProc("testProc");
    QVERIFY(libProc != nullptr);
    QCOMPARE(libProc->getEntryAddress(), Address::INVALID);
    QCOMPARE(prog.getOrCreateLibraryProc("testProc"), libProc);
}


void ProgTest::testGetFunctionByAddr()
{
    Prog prog("test", nullptr);
    QVERIFY(prog.getFunctionByAddr(Address::INVALID) == nullptr);

    Function *func = prog.getOrCreateFunction(Address(0x1000));
    QVERIFY(prog.getFunctionByAddr(Address(0x1000)) == func);
}


void ProgTest::testGetFunctionByName()
{
    Prog prog("test", nullptr);
    QVERIFY(prog.getFunctionByName("test") == nullptr);

    Function *func = prog.getOrCreateFunction(Address(0x1000));
    func->setName("testFunc");
    QVERIFY(prog.getFunctionByName("testFunc") == func);
}


void ProgTest::testRemoveFunction()
{
    Prog prog("test", nullptr);

    QVERIFY(prog.removeFunction("") == false);

    Function *func = prog.getOrCreateFunction(Address(0x1000));
    QVERIFY(prog.removeFunction(func->getName()) == true);
    QVERIFY(prog.getFunctionByAddr(Address(0x1000)) == nullptr);

    func = prog.getOrCreateFunction(Address(0x1000));
    QVERIFY(func != nullptr);
    func->setName("testFunc");
    QVERIFY(prog.removeFunction(func->getName()) == true);
}


void ProgTest::testGetNumFunctions()
{
    Prog prog("test", nullptr);

    QCOMPARE(prog.getNumFunctions(true), 0);
    QCOMPARE(prog.getNumFunctions(false), 0);

    prog.getOrCreateFunction(Address(0x1000));

    QCOMPARE(prog.getNumFunctions(true), 1);
    QCOMPARE(prog.getNumFunctions(false), 1);

    prog.getOrCreateLibraryProc("foo");

    QCOMPARE(prog.getNumFunctions(true), 1);
    QCOMPARE(prog.getNumFunctions(false), 2);
}



QTEST_GUILESS_MAIN(ProgTest)
