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
#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/type/type/IntegerType.h"


#define SAMPLE(path)    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/" path))

#define HELLO_PENTIUM   SAMPLE("pentium/hello")
#define FBRANCH_PENTIUM SAMPLE("pentium/fbranch")
#define HELLO_WIN       SAMPLE("windows/hello.exe")


void ProgTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
}


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


void ProgTest::testGetOrInsertModule()
{
    Prog prog("test", nullptr);

    // get module
    QCOMPARE(prog.getOrInsertModule("test"), prog.getRootModule());

    Module *mod = prog.getOrInsertModule("");
    QVERIFY(mod != nullptr);
    QVERIFY(mod != prog.getRootModule());

    QCOMPARE(prog.getOrInsertModule("foo"), prog.getOrInsertModule("foo"));
}


void ProgTest::testGetRootModule()
{
    Prog prog("test", nullptr);
    QVERIFY(prog.getRootModule() != nullptr);
}


void ProgTest::testFindModule()
{
    Prog prog("test", nullptr);
    QCOMPARE(prog.findModule("test"), prog.getRootModule());

    QVERIFY(prog.findModule("foo") == nullptr);

    Module *foo = prog.getOrInsertModule("foo");
    QCOMPARE(prog.findModule("foo"), foo);
}


void ProgTest::testIsModuleUsed()
{
    Prog prog("test", nullptr);

    QVERIFY(!prog.isModuleUsed(prog.getRootModule())); // no functions present in module

    prog.getOrCreateFunction(Address(0x1000));
    QVERIFY(prog.isModuleUsed(prog.getRootModule()));
}


void ProgTest::testAddEntryPoint()
{
    Prog prog("test", nullptr);

    QVERIFY(prog.addEntryPoint(Address::INVALID) == nullptr);

    Function *entry = prog.addEntryPoint(Address(0x1000));
    QVERIFY(entry != nullptr);

    // add existing entry point
    QCOMPARE(prog.addEntryPoint(Address(0x1000)), entry);

    // add entry point that is blocked by a lib proc
    LibProc *libProc = prog.getOrCreateLibraryProc("testProc");
    libProc->setEntryAddress(Address(0x2000));
    QVERIFY(prog.addEntryPoint(Address(0x2000)) == nullptr);
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


void ProgTest::testIsWellFormed()
{
    Project pro;
    QVERIFY(pro.loadBinaryFile(HELLO_PENTIUM));

    const Prog *hello = pro.getProg();
    QVERIFY(hello->isWellFormed());
    QVERIFY(pro.decodeBinaryFile());
    QVERIFY(hello->isWellFormed());
    QVERIFY(pro.decompileBinaryFile());
    QVERIFY(hello->isWellFormed());

    Prog testProg("test", nullptr);
    QVERIFY(testProg.isWellFormed());
}


void ProgTest::testIsWin32()
{
    Prog testProg("test", nullptr);
    QVERIFY(!testProg.isWin32());

    Project pro;
    QVERIFY(pro.loadBinaryFile(HELLO_PENTIUM));
    QVERIFY(!pro.getProg()->isWin32());
    QVERIFY(pro.loadBinaryFile(HELLO_WIN));
    QVERIFY(pro.getProg()->isWin32());
}


void ProgTest::testGetRegName()
{
    QSKIP("TODO");
}


void ProgTest::testGetRegSize()
{
    QSKIP("TODO");
}


void ProgTest::testGetFrontEndId()
{
    Prog testProg("test", nullptr);
    QCOMPARE(testProg.getFrontEndId(), Platform::GENERIC);

    Project pro;
    pro.loadBinaryFile(HELLO_PENTIUM);
    QCOMPARE(pro.getProg()->getFrontEndId(), Platform::PENTIUM);
}


void ProgTest::testGetMachine()
{
    Prog testProg("test", nullptr);
    QCOMPARE(testProg.getMachine(), Machine::INVALID);

    Project pro;
    QVERIFY(pro.loadBinaryFile(HELLO_PENTIUM));
    QCOMPARE(pro.getProg()->getMachine(), Machine::PENTIUM);
}


void ProgTest::testGetDefaultSignature()
{
    Prog testProg("test", nullptr);
    QVERIFY(testProg.getDefaultSignature("foo") == nullptr);

    Project pro;
    QVERIFY(pro.loadBinaryFile(HELLO_PENTIUM));
    auto sig = pro.getProg()->getDefaultSignature("foo");
    QVERIFY(sig != nullptr);
    QCOMPARE(sig->getName(), QString("foo"));
}


void ProgTest::testGetStringConstant()
{
    Prog testProg("test", nullptr);
    QVERIFY(testProg.getStringConstant(Address(0x1000), true ) == nullptr);
    QVERIFY(testProg.getStringConstant(Address(0x1000), false) == nullptr);
    QVERIFY(testProg.getStringConstant(Address::INVALID) == nullptr);

    Project pro;
    QVERIFY(pro.loadBinaryFile(HELLO_PENTIUM));
    const char *hello1 = pro.getProg()->getStringConstant(Address(0x80483FC), false);
    QVERIFY(hello1 != nullptr);
    QCOMPARE(hello1, "Hello, world!\n");

    const char *hello2 = pro.getProg()->getStringConstant(Address(0x80483FC), true);
    QCOMPARE(hello2, hello1);

    // zero length string
    const char *world1 = pro.getProg()->getStringConstant(Address(0x804840A), false);
    QVERIFY(world1 != nullptr);
    QCOMPARE(world1, "");
}


void ProgTest::testGetFloatConstant()
{
    Project pro;
    QVERIFY(pro.loadBinaryFile(FBRANCH_PENTIUM));
    QVERIFY(pro.decodeBinaryFile());

    double result;
    QVERIFY(!pro.getProg()->getFloatConstant(Address::INVALID, result, 32));
    QVERIFY(pro.getProg()->getFloatConstant(Address(0x080485CC), result, 32));
    QCOMPARE(result, 5.0);
}


void ProgTest::testGetSymbolNameByAddr()
{
    Project pro;
    QVERIFY(pro.loadBinaryFile(HELLO_PENTIUM));

    // "Hello, world!" string is not a symbol
    QCOMPARE(pro.getProg()->getSymbolNameByAddr(Address(0x080483FC)), QString(""));
    QCOMPARE(pro.getProg()->getSymbolNameByAddr(Address(0x08048268)), QString("printf"));
}


void ProgTest::testGetSectionByAddr()
{
    Project pro;
    QVERIFY(pro.loadBinaryFile(HELLO_PENTIUM));

    QVERIFY(pro.getProg()->getSectionByAddr(Address::INVALID) == nullptr);

    const BinarySection *sect = pro.getProg()->getSectionByAddr(Address(0x08048331));
    QVERIFY(sect != nullptr);
    QCOMPARE(sect->getName(), QString(".text"));
}


void ProgTest::testGetLimitText()
{
    Project pro;
    QVERIFY(pro.loadBinaryFile(HELLO_PENTIUM));

    QCOMPARE(pro.getProg()->getLimitTextLow(),  Address(0x08048230));
    QCOMPARE(pro.getProg()->getLimitTextHigh(), Address(0x080483f3));
}


void ProgTest::testIsReadOnly()
{
    Project pro;
    QVERIFY(pro.loadBinaryFile(HELLO_PENTIUM));

    QVERIFY(!pro.getProg()->isReadOnly(Address::INVALID));
    QVERIFY(!pro.getProg()->isReadOnly(Address(0x080496a8))); // address in .ctors
    QVERIFY( pro.getProg()->isReadOnly(Address(0x080483f4))); // address in .rodata
}


void ProgTest::testIsInStringsSection()
{
    Project pro;
    QVERIFY(pro.loadBinaryFile(HELLO_PENTIUM));

    QVERIFY(!pro.getProg()->isInStringsSection(Address::INVALID));
    QVERIFY(!pro.getProg()->isInStringsSection(Address(0x080483f4))); // address in .rodata
    QVERIFY( pro.getProg()->isInStringsSection(Address(0x080481a0))); // address in .dynstr
}


void ProgTest::testIsDynamicallyLinkedProcPointer()
{
    Project pro;
    QVERIFY(pro.loadBinaryFile(HELLO_PENTIUM));

    QVERIFY(!pro.getProg()->isDynamicallyLinkedProcPointer(Address::INVALID));
    QVERIFY(!pro.getProg()->isDynamicallyLinkedProcPointer(Address(0x080483f4)));
    QVERIFY( pro.getProg()->isDynamicallyLinkedProcPointer(Address(0x08048268))); // address of printf
}


void ProgTest::testGetDynamicProcName()
{
    Project pro;
    QVERIFY(pro.loadBinaryFile(HELLO_PENTIUM));

    QCOMPARE(pro.getProg()->getDynamicProcName(Address::INVALID),    QString(""));
    QCOMPARE(pro.getProg()->getDynamicProcName(Address(0x080483f4)), QString(""));
    QCOMPARE(pro.getProg()->getDynamicProcName(Address(0x08048268)), QString("printf"));
}


void ProgTest::testGetOrInsertModuleForSymbol()
{
    {
        Prog prog("test", nullptr);
        QCOMPARE(prog.getOrInsertModuleForSymbol(""),     prog.getRootModule());
        QCOMPARE(prog.getOrInsertModuleForSymbol("test"), prog.getRootModule());
    }

    Project pro;
    pro.loadBinaryFile(HELLO_PENTIUM);
    QCOMPARE(pro.getProg()->getOrInsertModuleForSymbol(""), pro.getProg()->getRootModule());
    QCOMPARE(pro.getProg()->getOrInsertModuleForSymbol("printf"), pro.getProg()->getRootModule());

    BinarySymbol *mainSym = pro.getLoadedBinaryFile()->getSymbols()->findSymbolByName("main");
    QVERIFY(mainSym != nullptr);
    mainSym->setAttribute("SourceFile", "foo.c");

    QCOMPARE(pro.getProg()->getOrInsertModuleForSymbol(mainSym->getName()), pro.getProg()->getOrInsertModule("foo"));
}


void ProgTest::testReadNative()
{
    QSKIP("TODO");
}


void ProgTest::testReadNativeAs()
{
    QSKIP("TODO");
}


void ProgTest::testReadSymbolFile()
{
    QSKIP("TODO");
}


void ProgTest::testAddDecodedRTL()
{
    QSKIP("TODO");
}


void ProgTest::testAddReloc()
{
    QSKIP("TODO");
}


void ProgTest::testDecodeEntryPoint()
{
    QSKIP("TODO");
}


void ProgTest::testDecodeFragment()
{
    QSKIP("TODO");
}


void ProgTest::testReDecode()
{
    QSKIP("TODO");
}


void ProgTest::testFinishDecode()
{
    QSKIP("TODO");
}


void ProgTest::testCreateGlobal()
{
    Prog prog("test", nullptr);

    QVERIFY(prog.createGlobal(Address::INVALID) == nullptr);

    Global *global = prog.createGlobal(Address(0x08000000));
    QVERIFY(global != nullptr);
}


void ProgTest::testGetGlobalName()
{
    Prog prog("test", nullptr);
    QCOMPARE(prog.getGlobalName(Address::INVALID), QString(""));

    prog.createGlobal(Address(0x08000000), IntegerType::get(32), "foo");
    QCOMPARE(prog.getGlobalName(Address(0x08000000)), QString("foo"));
}


void ProgTest::testGetGlobalAddr()
{
    Prog prog("test", nullptr);
    prog.createGlobal(Address(0x08000000), IntegerType::get(32), "foo");
    QCOMPARE(prog.getGlobalAddr("foo"), Address(0x08000000));
}


void ProgTest::testGetGlobal()
{
    Prog prog("test", nullptr);
    QVERIFY(prog.getGlobal("foo") == nullptr);

    Global *global = prog.createGlobal(Address(0x08000000), IntegerType::get(32), "foo");
    QCOMPARE(prog.getGlobal("foo"), global);
}


void ProgTest::testNewGlobalName()
{
    Prog prog("test", nullptr);
    QCOMPARE(prog.newGlobalName(Address(0x1000)), QString("global_0x00001000"));
    QCOMPARE(prog.newGlobalName(Address(0x1000)), QString("global_0x00001000"));
}


void ProgTest::testGuessGlobalType()
{
    QSKIP("TODO");
}


void ProgTest::testMakeArrayType()
{
    QSKIP("TODO");
}


void ProgTest::testMarkGlobalUsed()
{
    QSKIP("TODO");
}


void ProgTest::testGlobalType()
{
    QSKIP("TODO");
}


QTEST_GUILESS_MAIN(ProgTest)
