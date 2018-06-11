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
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/type/type/ArrayType.h"
#include "boomerang/type/type/CharType.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/PointerType.h"


#define SAMPLE(path)    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/" path))

#define HELLO_PENTIUM   SAMPLE("pentium/hello")
#define FBRANCH_PENTIUM SAMPLE("pentium/fbranch")
#define HELLO_WIN       SAMPLE("windows/hello.exe")


void ProgTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
    m_project.loadPlugins();
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
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));

    const Prog *hello = m_project.getProg();
    QVERIFY(hello->isWellFormed());
    QVERIFY(m_project.decodeBinaryFile());
    QVERIFY(hello->isWellFormed());
    QVERIFY(m_project.decompileBinaryFile());
    QVERIFY(hello->isWellFormed());

    Prog testProg("test", nullptr);
    QVERIFY(testProg.isWellFormed());
}


void ProgTest::testIsWin32()
{
    Prog testProg("test", nullptr);
    QVERIFY(!testProg.isWin32());

    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    QVERIFY(!m_project.getProg()->isWin32());
    QVERIFY(m_project.loadBinaryFile(HELLO_WIN));
    QVERIFY(m_project.getProg()->isWin32());
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

    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    QCOMPARE(m_project.getProg()->getFrontEndId(), Platform::PENTIUM);
}


void ProgTest::testGetMachine()
{
    Prog testProg("test", nullptr);
    QCOMPARE(testProg.getMachine(), Machine::INVALID);

    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    QCOMPARE(m_project.getProg()->getMachine(), Machine::PENTIUM);
}


void ProgTest::testGetDefaultSignature()
{
    Prog testProg("test", nullptr);
    QVERIFY(testProg.getDefaultSignature("foo") == nullptr);

    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    auto sig = m_project.getProg()->getDefaultSignature("foo");
    QVERIFY(sig != nullptr);
    QCOMPARE(sig->getName(), QString("foo"));
}


void ProgTest::testGetStringConstant()
{
    Prog testProg("test", nullptr);
    QVERIFY(testProg.getStringConstant(Address(0x1000), true ) == nullptr);
    QVERIFY(testProg.getStringConstant(Address(0x1000), false) == nullptr);
    QVERIFY(testProg.getStringConstant(Address::INVALID) == nullptr);

    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    const char *hello1 = m_project.getProg()->getStringConstant(Address(0x80483FC), false);
    QVERIFY(hello1 != nullptr);
    QCOMPARE(hello1, "Hello, world!\n");

    const char *hello2 = m_project.getProg()->getStringConstant(Address(0x80483FC), true);
    QCOMPARE(hello2, hello1);

    // zero length string
    const char *world1 = m_project.getProg()->getStringConstant(Address(0x804840A), false);
    QVERIFY(world1 != nullptr);
    QCOMPARE(world1, "");
}


void ProgTest::testGetFloatConstant()
{
    QVERIFY(m_project.loadBinaryFile(FBRANCH_PENTIUM));
    QVERIFY(m_project.decodeBinaryFile());

    double result;
    QVERIFY(!m_project.getProg()->getFloatConstant(Address::INVALID, result, 32));
    QVERIFY(m_project.getProg()->getFloatConstant(Address(0x080485CC), result, 32));
    QCOMPARE(result, 5.0);
}


void ProgTest::testGetSymbolNameByAddr()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));

    // "Hello, world!" string is not a symbol
    QCOMPARE(m_project.getProg()->getSymbolNameByAddr(Address(0x080483FC)), QString(""));
    QCOMPARE(m_project.getProg()->getSymbolNameByAddr(Address(0x08048268)), QString("printf"));
}


void ProgTest::testGetSectionByAddr()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));

    QVERIFY(m_project.getProg()->getSectionByAddr(Address::INVALID) == nullptr);

    const BinarySection *sect = m_project.getProg()->getSectionByAddr(Address(0x08048331));
    QVERIFY(sect != nullptr);
    QCOMPARE(sect->getName(), QString(".text"));
}


void ProgTest::testGetLimitText()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));

    QCOMPARE(m_project.getProg()->getLimitTextLow(),  Address(0x08048230));
    QCOMPARE(m_project.getProg()->getLimitTextHigh(), Address(0x080483f3));
}


void ProgTest::testIsReadOnly()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));

    QVERIFY(!m_project.getProg()->isReadOnly(Address::INVALID));
    QVERIFY(!m_project.getProg()->isReadOnly(Address(0x080496a8))); // address in .ctors
    QVERIFY( m_project.getProg()->isReadOnly(Address(0x080483f4))); // address in .rodata
}


void ProgTest::testIsInStringsSection()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));

    QVERIFY(!m_project.getProg()->isInStringsSection(Address::INVALID));
    QVERIFY(!m_project.getProg()->isInStringsSection(Address(0x080483f4))); // address in .rodata
    QVERIFY( m_project.getProg()->isInStringsSection(Address(0x080481a0))); // address in .dynstr
}


void ProgTest::testIsDynamicallyLinkedProcPointer()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));

    QVERIFY(!m_project.getProg()->isDynamicallyLinkedProcPointer(Address::INVALID));
    QVERIFY(!m_project.getProg()->isDynamicallyLinkedProcPointer(Address(0x080483f4)));
    QVERIFY( m_project.getProg()->isDynamicallyLinkedProcPointer(Address(0x08048268))); // address of printf
}


void ProgTest::testGetDynamicProcName()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));

    QCOMPARE(m_project.getProg()->getDynamicProcName(Address::INVALID),    QString(""));
    QCOMPARE(m_project.getProg()->getDynamicProcName(Address(0x080483f4)), QString(""));
    QCOMPARE(m_project.getProg()->getDynamicProcName(Address(0x08048268)), QString("printf"));
}


void ProgTest::testGetOrInsertModuleForSymbol()
{
    Prog prog("test", nullptr);
    QCOMPARE(prog.getOrInsertModuleForSymbol(""),     prog.getRootModule());
    QCOMPARE(prog.getOrInsertModuleForSymbol("test"), prog.getRootModule());

    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    QCOMPARE(m_project.getProg()->getOrInsertModuleForSymbol(""), m_project.getProg()->getRootModule());
    QCOMPARE(m_project.getProg()->getOrInsertModuleForSymbol("printf"), m_project.getProg()->getRootModule());

    BinarySymbol *mainSym = m_project.getLoadedBinaryFile()->getSymbols()->findSymbolByName("main");
    QVERIFY(mainSym != nullptr);
    mainSym->setAttribute("SourceFile", "foo.c");

    QCOMPARE(m_project.getProg()->getOrInsertModuleForSymbol(mainSym->getName()), m_project.getProg()->getOrInsertModule("foo"));
}


void ProgTest::testReadNative4()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));

    QCOMPARE(m_project.getProg()->readNative4(Address::INVALID), 0);
    QCOMPARE(m_project.getProg()->readNative4(Address(0x80483FC)), 0x6c6c6548);
}


void ProgTest::testReadNativeAs()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    QVERIFY(m_project.getProg()->readNativeAs(Address(0x80483FC), PointerType::get(CharType::get())) == nullptr);

    SharedExp e = m_project.getProg()->readNativeAs(Address(0x80483FC), ArrayType::get(CharType::get()));
    QVERIFY(e->isConst());
    QCOMPARE(e->access<Const>()->getStr(), QString("Hello, world!\n"));
}


void ProgTest::testAddReloc()
{
    Prog prog("test", nullptr);
    SharedExp e = prog.addReloc(Const::get(0x1000), Address::INVALID);
    QVERIFY(*e == *Const::get(0x1000));

    QSKIP("TODO");
}


void ProgTest::testDecodeEntryPoint()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    QVERIFY(!m_project.getProg()->decodeEntryPoint(Address::INVALID));
    QVERIFY(m_project.getProg()->decodeEntryPoint(m_project.getLoadedBinaryFile()->getEntryPoint()));
}


void ProgTest::testDecodeFragment()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    UserProc *mainProc = (UserProc *)m_project.getProg()->getOrCreateFunction(Address(0x08048328));
    QVERIFY(!m_project.getProg()->decodeFragment(mainProc, Address::INVALID));
    QVERIFY(m_project.getProg()->decodeFragment(mainProc, Address(0x08048328)));
}


void ProgTest::testReDecode()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    UserProc *mainProc = (UserProc *)m_project.getProg()->getOrCreateFunction(Address(0x08048328));

    QVERIFY(!m_project.getProg()->reDecode(nullptr));
    QVERIFY(m_project.getProg()->reDecode(mainProc)); // actually processing for the first time
    QVERIFY(m_project.getProg()->reDecode(mainProc)); // actual re-decode
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
    {
        Prog prog("test", nullptr);
        SharedType ty = prog.guessGlobalType("test", Address(0x1000));
        QVERIFY(*ty == *VoidType::get());
    }

    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));

    SharedType ty = m_project.getProg()->guessGlobalType("helloworld", Address(0x80483FC));
    QVERIFY(*ty == *PointerType::get(CharType::get()));
}


void ProgTest::testMakeArrayType()
{
    Prog prog("test", nullptr);

    std::shared_ptr<ArrayType> ty = prog.makeArrayType(Address::INVALID, CharType::get());
    QVERIFY(*ty == *ArrayType::get(CharType::get()));

    m_project.loadBinaryFile(HELLO_PENTIUM);
    BinarySymbol *helloworld = m_project.getLoadedBinaryFile()->getSymbols()
        ->createSymbol(Address(0x80483FC), "helloworld");

    helloworld->setSize(15);

    // type of hello world
    ty = m_project.getProg()->makeArrayType(Address(0x80483FC), CharType::get());
    QCOMPARE(ty->prints(), QString("char[15]"));

    ty = m_project.getProg()->makeArrayType(Address(0x80483FC), VoidType::get());
    QCOMPARE(ty->prints(), QString("void[15]"));
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
