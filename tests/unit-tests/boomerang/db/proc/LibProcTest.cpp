#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "LibProcTest.h"


#include "boomerang/core/Settings.h"
#include "boomerang/db/module/Module.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/db/signature/X86Signature.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/CallStatement.h"


#define HELLO_PENTIUM  (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/hello"))


void LibProcTest::testName()
{
    LibProc proc(Address::INVALID, "test", nullptr);
    QCOMPARE(proc.getName(), QString("test"));
    proc.setName("foo");
    QCOMPARE(proc.getName(), QString("foo"));
}


void LibProcTest::testEntryAddr()
{
    LibProc proc(Address(0x1000), "test", nullptr);
    QCOMPARE(proc.getEntryAddress(), Address(0x1000));
    proc.setEntryAddress(Address::INVALID);
    QCOMPARE(proc.getEntryAddress(), Address::INVALID);
}


void LibProcTest::testSetModule()
{
    Module mod1("test1");
    Module mod2("test2");

    LibProc proc(Address(0x1000), "test", nullptr);
    proc.setModule(nullptr);
    QVERIFY(proc.getModule() == nullptr);

    proc.setModule(&mod1);
    QVERIFY(proc.getModule() == &mod1);
    QVERIFY(mod1.getFunctionList().size() == 1);
    QVERIFY(mod1.getFunction(Address(0x1000)) == &proc);

    proc.setModule(&mod2);
    QVERIFY(proc.getModule() == &mod2);
    QVERIFY(mod1.getFunctionList().empty());
    QVERIFY(mod2.getFunctionList().size() == 1);
    QVERIFY(mod1.getFunction(Address(0x1000)) == nullptr);
    QVERIFY(mod2.getFunction(Address(0x1000)) == &proc);

    proc.setModule(nullptr);
    QVERIFY(proc.getModule() == nullptr);
}


void LibProcTest::testRemoveFromModule()
{
    Module mod("testMod");
    LibProc proc(Address(0x1000), "test", &mod);
    proc.removeFromModule();
    QVERIFY(mod.getFunctionList().empty());
    QVERIFY(mod.getFunction(Address(0x1000)) == nullptr);
}


void LibProcTest::testRemoveParameterFromSignature()
{
    LibProc proc(Address(0x1000), "test", nullptr);
    auto sig = std::make_shared<CallingConvention::StdC::X86Signature>("test");
    auto exp = Location::memOf(Binary::get(opPlus, Location::regOf(REG_PENT_ESP), Const::get(8)));
    sig->addParameter("foo", exp);

    proc.setSignature(sig);
    QCOMPARE(proc.getSignature()->getNumParams(), 1);

    proc.removeParameterFromSignature(exp->clone()); // make sure to not compare by address
    QCOMPARE(proc.getSignature()->getNumParams(), 0);

    sig->addParameter("bar", exp);
    std::shared_ptr<CallStatement> call(new CallStatement);
    call->setDestProc(&proc);
    proc.addCaller(call);
    call->setNumArguments(1);

    proc.removeParameterFromSignature(exp->clone());
    QCOMPARE(proc.getSignature()->getNumParams(), 0);
    QCOMPARE(call->getNumArguments(), 0);
}


void LibProcTest::testRenameParameter()
{
    LibProc proc(Address::INVALID, "test", nullptr);
    auto sig = std::make_shared<CallingConvention::StdC::X86Signature>("test");
    auto exp = Location::memOf(Binary::get(opPlus, Location::regOf(REG_PENT_ESP), Const::get(8)));
    sig->addParameter("foo", exp);
    sig->addParameter("foo", exp);
    proc.setSignature(sig);

    proc.renameParameter("foo", "bar");

    QCOMPARE(proc.getSignature()->getParamName(0), QString("bar"));
    QCOMPARE(proc.getSignature()->getParamName(1), QString("foo"));
}


void LibProcTest::testIsLib()
{
    LibProc proc(Address::INVALID, "print", nullptr);
    QVERIFY(proc.isLib());
}


void LibProcTest::testIsNoReturn()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));

    LibProc *proc = m_project.getProg()->getOrCreateLibraryProc("abort");
    QVERIFY(proc->isNoReturn());
    proc->setName("test");
    QVERIFY(!proc->isNoReturn());

    std::shared_ptr<Signature> sig(new Signature("test"));
    proc->setSignature(sig);
    QVERIFY(!proc->isNoReturn());
}


void LibProcTest::testGetProven()
{
    LibProc proc(Address::INVALID, "test", nullptr);
    QVERIFY(proc.getProven(nullptr) == nullptr);
    QVERIFY(proc.getProven(Location::regOf(REG_PENT_EBX)) == nullptr);

    proc.setSignature(std::make_shared<CallingConvention::StdC::X86Signature>("test"));
    QVERIFY(proc.getProven(Location::regOf(REG_PENT_EBX)) != nullptr);
    QCOMPARE(proc.getProven(Location::regOf(REG_PENT_EBX))->toString(),
             Location::regOf(REG_PENT_EBX)->toString());
}


void LibProcTest::testGetPremised()
{
    LibProc proc(Address::INVALID, "test", nullptr);
    QVERIFY(proc.getPremised(nullptr) == nullptr);
    QVERIFY(proc.getPremised(Location::regOf(REG_PENT_EBX)) == nullptr);
}


void LibProcTest::testIsPreserved()
{
    LibProc proc(Address::INVALID, "test", nullptr);
    QVERIFY(!proc.isPreserved(nullptr));
    QVERIFY(!proc.isPreserved(Location::regOf(REG_PENT_EBX)));

    proc.setSignature(std::make_shared<CallingConvention::StdC::X86Signature>("test"));
    QVERIFY(!proc.isPreserved(Location::regOf(REG_PENT_EAX)));
    QVERIFY(proc.isPreserved(Location::regOf(REG_PENT_EBX)));
}

QTEST_GUILESS_MAIN(LibProcTest)
