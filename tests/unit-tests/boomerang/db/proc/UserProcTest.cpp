#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UserProcTest.h"


#define SAMPLE(path)    (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/" path))
#define HELLO_X86   SAMPLE("x86/hello")


#include "boomerang/core/Settings.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/db/signature/X86Signature.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/decomp/ProcDecompiler.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/db/LowLevelCFG.h"


void UserProcTest::testIsNoReturn()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Ret,  createInsns(Address(0x1000), 1));
    BasicBlock *bb2 = prog.getCFG()->createBB(BBType::Call, createInsns(Address(0x2000), 1));
    BasicBlock *bb3 = prog.getCFG()->createBB(BBType::Ret,  createInsns(Address(0x2001), 1));

    {
        UserProc testProc(Address(0x1000), "test", nullptr);
        QVERIFY(!testProc.isNoReturn());
    }

    {
        UserProc testProc(Address(0x1000), "test", nullptr);
        testProc.setStatus(ProcStatus::Decoded);
        QVERIFY(testProc.isNoReturn());
    }

    {
        UserProc testProc(Address(0x1000), "test", nullptr);
        testProc.setStatus(ProcStatus::Decoded);

        std::shared_ptr<ReturnStatement> retStmt(new ReturnStatement);
        std::unique_ptr<RTLList> bbRTLs(new RTLList);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { retStmt })));

        testProc.getCFG()->createFragment(FragType::Ret, std::move(bbRTLs), bb1);
        testProc.setEntryFragment();

        QVERIFY(!testProc.isNoReturn());
    }

    {
        UserProc noReturnProc(Address(0x8000), "noReturn", nullptr);
        UserProc testProc(Address(0x2000), "test", nullptr);
        testProc.setStatus(ProcStatus::Decoded);
        noReturnProc.setStatus(ProcStatus::Decoded);

        std::unique_ptr<RTLList> bbRTLs(new RTLList);
        std::shared_ptr<CallStatement> call(new CallStatement);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x2000), { call })));
        call->setDestProc(&noReturnProc);

        IRFragment *entryFrag = testProc.getCFG()->createFragment(FragType::Call, std::move(bbRTLs), bb2);
        IRFragment *exitFrag  = testProc.getCFG()->createFragment(FragType::Ret, createRTLs(Address(0x2001), 1, 1), bb3);

        testProc.setEntryAddress(Address(0x2000));
        testProc.getCFG()->addEdge(entryFrag, exitFrag);
        testProc.getCFG()->setEntryAndExitFragment(entryFrag);
        testProc.setEntryFragment();

        QVERIFY(testProc.isNoReturn());
    }
}


void UserProcTest::testRemoveStatement()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1000), 1));

    {
        UserProc proc(Address::INVALID, "test", nullptr);
        QVERIFY(!proc.removeStatement(nullptr));
    }

    {
        // statement cannot be removed since it does not belong to a fragment
        UserProc proc(Address::INVALID, "test", nullptr);
        std::shared_ptr<Assign> asgn(new Assign(VoidType::get(),
                                                Location::regOf(REG_X86_EAX),
                                                Location::regOf(REG_X86_ECX)));

        QVERIFY(!proc.removeStatement(asgn));
    }

    {
        UserProc proc(Address::INVALID, "test", nullptr);

        std::shared_ptr<Assign> asgn(new Assign(VoidType::get(),
                                                Location::regOf(REG_X86_EAX),
                                                Location::regOf(REG_X86_ECX)));

        std::unique_ptr<RTLList> bbRTLs(new RTLList);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x00000123), { asgn })));

        IRFragment *frag = proc.getCFG()->createFragment(FragType::Fall, std::move(bbRTLs), bb1);
        asgn->setFragment(frag);

        QVERIFY(proc.removeStatement(asgn));
    }

    // todo: test that proven true cache is updated
}


void UserProcTest::testInsertAssignAfter()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1000), 1));

    {
        UserProc proc(Address(0x1000), "test", nullptr);

        std::unique_ptr<RTLList> bbRTLs(new RTLList);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { })));
        IRFragment *entryFrag = proc.getCFG()->createFragment(FragType::Oneway, createRTLs(Address(0x1000), 1, 0), bb1);
        proc.setEntryFragment();

        std::shared_ptr<Assign> as = proc.insertAssignAfter(nullptr, Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX));

        QVERIFY(as != nullptr);
        QVERIFY(as->getProc() == &proc);
        QVERIFY(as->getFragment() == entryFrag);

        QVERIFY(proc.getEntryFragment()->getRTLs()->front()->size() == 1);
        QVERIFY(*proc.getEntryFragment()->getRTLs()->front()->begin() == as);

        std::shared_ptr<Assign> as2 = proc.insertAssignAfter(as, Location::regOf(REG_X86_EBX), Location::regOf(REG_X86_EDX));
        QVERIFY(as2 != nullptr);
        QVERIFY(as->getProc() == &proc);
        QVERIFY(as->getFragment() == entryFrag);
        QVERIFY(proc.getEntryFragment()->getRTLs()->front()->size() == 2);
        QVERIFY(*proc.getEntryFragment()->getRTLs()->front()->begin() == as);
        QVERIFY(*std::next(proc.getEntryFragment()->getRTLs()->front()->begin()) == as2);
    }
}


void UserProcTest::testInsertStatementAfter()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1000), 1));

    {
        UserProc proc(Address(0x1000), "test", nullptr);

        std::unique_ptr<RTLList> bbRTLs(new RTLList);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { })));
        IRFragment *entryFrag = proc.getCFG()->createFragment(FragType::Oneway, std::move(bbRTLs), bb1);
        proc.setEntryFragment();

        std::shared_ptr<Assign> as = proc.insertAssignAfter(nullptr, Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX));
        std::shared_ptr<Assign> as2(new Assign(VoidType::get(), Location::regOf(REG_X86_EDX), Location::regOf(REG_X86_EBX)));

        proc.insertStatementAfter(as, as2);
        QVERIFY(as2->getFragment() == entryFrag);
        QVERIFY(proc.getEntryFragment()->getRTLs()->front()->size() == 2);
        QVERIFY(*proc.getEntryFragment()->getRTLs()->front()->begin() == as);
        QVERIFY(*std::next(proc.getEntryFragment()->getRTLs()->front()->begin()) == as2);
    }
}


void UserProcTest::testReplacePhiByAssign()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1000), 1));

    {
        UserProc proc(Address(0x1000), "test", nullptr);
        std::unique_ptr<RTLList> bbRTLs(new RTLList);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { })));
        proc.getCFG()->createFragment(FragType::Oneway, std::move(bbRTLs), bb1);
        proc.setEntryFragment();

        std::shared_ptr<Assign> as = proc.replacePhiByAssign(nullptr, Location::regOf(REG_X86_EAX));
        QVERIFY(as == nullptr);
    }

    {
        // replace phi
        UserProc proc(Address(0x1000), "test", nullptr);
        std::unique_ptr<RTLList> bbRTLs(new RTLList);

        std::shared_ptr<PhiAssign> phi(new PhiAssign(Location::regOf(REG_X86_EAX)));
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address::ZERO, { phi })));
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { })));

        IRFragment *frag = proc.getCFG()->createFragment(FragType::Oneway, std::move(bbRTLs), bb1);
        proc.setEntryFragment();
        phi->setFragment(frag);

        auto as = proc.replacePhiByAssign(phi, Location::regOf(REG_X86_EAX));
        QVERIFY(as != nullptr);
        QVERIFY(as->isAssign());
        QVERIFY(*as->getLeft() == *Location::regOf(REG_X86_EAX));
        QVERIFY(*as->getLeft() == *as->getRight());
    }

    {
        // replace first phi, make sure that the assign gets moved after phis
        UserProc proc(Address(0x1000), "test", nullptr);
        std::unique_ptr<RTLList> bbRTLs(new RTLList);

        std::shared_ptr<PhiAssign> phi1(new PhiAssign(Location::regOf(REG_X86_EAX)));
        std::shared_ptr<PhiAssign> phi2(new PhiAssign(Location::regOf(REG_X86_EDX)));
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address::ZERO, { phi1, phi2 })));
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { })));

        IRFragment *frag = proc.getCFG()->createFragment(FragType::Oneway, std::move(bbRTLs), bb1);
        proc.setEntryFragment();
        phi1->setFragment(frag);
        phi2->setFragment(frag);

        auto as = proc.replacePhiByAssign(phi1, Location::regOf(REG_X86_EAX));
        QVERIFY(as != nullptr);
        QVERIFY(as->isAssign());
        QVERIFY(*as->getLeft() == *Location::regOf(REG_X86_EAX));
        QVERIFY(*as->getLeft() == *as->getRight());

        QVERIFY(frag->getFirstStmt());
        QVERIFY(frag->getFirstStmt()->isPhi());
        QVERIFY(*frag->getFirstStmt()->as<Assignment>()->getLeft() == *Location::regOf(REG_X86_EDX));

        QVERIFY(frag->getLastStmt());
        QVERIFY(frag->getLastStmt() != frag->getFirstStmt());
        QVERIFY(frag->getLastStmt()->isAssign());
        QCOMPARE(frag->getLastStmt()->toString(), "   0 *v* r24 := r24");
    }
}


void UserProcTest::testAddParameterToSignature()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    proc.addParameterToSignature(Location::memOf(Binary::get(opPlus,
        Location::regOf(REG_X86_ESP), Const::get(4)), &proc),
        VoidType::get());

    QCOMPARE(proc.getSignature()->getNumParams(), 1);

    // try to add the same parameter again
    proc.addParameterToSignature(Location::memOf(Binary::get(opPlus,
        Location::regOf(REG_X86_ESP), Const::get(4)), &proc),
        VoidType::get());

    QCOMPARE(proc.getSignature()->getNumParams(), 1);
}


void UserProcTest::testInsertParameter()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    proc.insertParameter(Location::memOf(Binary::get(opPlus,
        Location::regOf(REG_X86_ESP), Const::get(4)), &proc),
        VoidType::get());

    QCOMPARE(proc.getParameters().size(), (size_t)1);
    QCOMPARE(proc.getSignature()->getNumParams(), 1);

    // try to add the same parameter again
    proc.insertParameter(Location::memOf(Binary::get(opPlus,
        Location::regOf(REG_X86_ESP), Const::get(4)), &proc),
        VoidType::get());

    QCOMPARE(proc.getParameters().size(), (size_t)1);
    QCOMPARE(proc.getSignature()->getNumParams(), 1);
}


void UserProcTest::testParamType()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    QVERIFY(proc.getParamType("invalidParam") == nullptr);

    proc.insertParameter(Location::memOf(Binary::get(opPlus,
        Location::regOf(REG_X86_ESP), Const::get(4)), &proc),
        VoidType::get());

    SharedConstType ty = proc.getParamType("param1");
    QVERIFY(ty != nullptr);
    QCOMPARE(ty->toString(), VoidType::get()->toString());

    proc.setParamType("param1", IntegerType::get(32, Sign::Signed));
    QCOMPARE(proc.getParamType("param1")->toString(), IntegerType::get(32, Sign::Signed)->toString());

    proc.setParamType(5, VoidType::get()); // proc only has 1 parameter
    QCOMPARE(proc.getParamType("param1")->toString(), IntegerType::get(32, Sign::Signed)->toString());

    proc.setParamType(0, VoidType::get());
    QCOMPARE(proc.getParamType("param1")->toString(), VoidType::get()->toString());
}


void UserProcTest::testLookupParam()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1000), 1));

    {
        UserProc proc(Address(0x1000), "test", nullptr);

        std::unique_ptr<RTLList> bbRTLs(new RTLList);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { })));

        proc.getCFG()->createFragment(FragType::Oneway, std::move(bbRTLs), bb1);
        proc.setEntryFragment();

        SharedExp paramExp = Location::memOf(Binary::get(opPlus,
                                                         Location::regOf(REG_X86_ESP),
                                                         Const::get(4)),
                                             &proc);

        SharedStmt ias = proc.getCFG()->findOrCreateImplicitAssign(paramExp->clone());
        proc.insertParameter(RefExp::get(paramExp->clone(), ias), VoidType::get());
        proc.mapSymbolTo(RefExp::get(paramExp->clone(), ias), Location::param("param1", &proc));
        proc.addParameterToSignature(paramExp->clone(), VoidType::get());

        QCOMPARE(proc.lookupParam(paramExp), QString("param1"));
        QCOMPARE(proc.lookupParam(Location::regOf(REG_X86_ECX)), QString(""));
    }
}


void UserProcTest::testCanBeParam()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_X86));
    Prog *prog = m_project.getProg();

    UserProc *mainProc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x08048328)));
    QVERIFY(mainProc != nullptr && !mainProc->isLib());

    QVERIFY(!mainProc->canBeParam(Terminal::get(opPC)));
    QVERIFY(!mainProc->canBeParam(Location::tempOf(Terminal::get(opTrue))));
    QVERIFY(!mainProc->canBeParam(Location::regOf(REG_X86_ESP)));
    QVERIFY(mainProc->canBeParam(Location::regOf(REG_X86_EDX)));
    QVERIFY(!mainProc->canBeParam(Location::memOf(Const::get(0x08048328))));
    QVERIFY(!mainProc->canBeParam(Location::memOf(RefExp::get(Location::regOf(REG_X86_ESP), nullptr))));
    QVERIFY(mainProc->canBeParam(Location::memOf(Binary::get(opPlus,
                                                             Location::regOf(REG_X86_ESP),
                                                             Const::get(4)))));
    QVERIFY(!mainProc->canBeParam(Location::global("test", mainProc)));
    QVERIFY(mainProc->canBeParam(Const::get(5)));
}


void UserProcTest::testRetStmt()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    QCOMPARE(proc.getRetAddr(), Address::INVALID);

    std::shared_ptr<ReturnStatement> retStmt(new ReturnStatement);
    proc.setRetStmt(retStmt, Address(0x2000));
    QVERIFY(proc.getRetStmt() == retStmt);
    QCOMPARE(proc.getRetAddr(), Address(0x2000));
}


void UserProcTest::testCanBeReturn()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_X86));
    QVERIFY(m_project.decodeBinaryFile());
    QVERIFY(m_project.decompileBinaryFile());

    Prog *prog = m_project.getProg();

    UserProc *mainProc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x08048328)));
    QVERIFY(mainProc != nullptr && !mainProc->isLib());

    // test cached preservation TODO
    QVERIFY(mainProc->getRetStmt());
    QVERIFY(mainProc->preservesExp(Location::regOf(REG_X86_EBP)));
    QVERIFY(!mainProc->canBeReturn(Location::regOf(REG_X86_EBP)));

    QVERIFY(!mainProc->canBeReturn(Terminal::get(opPC)));
    QVERIFY(!mainProc->canBeReturn(Location::get(opTemp, Terminal::get(opTrue), mainProc)));
    QVERIFY(mainProc->canBeReturn(Location::regOf(REG_X86_ESP)));
    QVERIFY(mainProc->canBeReturn(Location::regOf(REG_X86_EDX)));
    QVERIFY(!mainProc->canBeReturn(Location::memOf(Const::get(0x08048328))));
}


void UserProcTest::testCreateLocal()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    SharedExp exp = proc.createLocal(VoidType::get(), Location::regOf(REG_X86_EAX), "eax");
    QVERIFY(exp != nullptr);
    QCOMPARE(exp->toString(), QString("eax"));
    QCOMPARE(proc.getLocalType("eax")->toString(), VoidType::get()->toString());
    QVERIFY(proc.getLocals().size() == 1);

    // set type of local
    exp = proc.createLocal(IntegerType::get(32, Sign::Signed), Location::regOf(REG_X86_EAX), "eax");
    QVERIFY(exp != nullptr);
    QCOMPARE(exp->toString(), QString("eax"));
    QCOMPARE(proc.getLocalType("eax")->toString(), IntegerType::get(32, Sign::Signed)->toString());
    QVERIFY(proc.getLocals().size() == (size_t)1);
}


void UserProcTest::testAddLocal()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    proc.addLocal(VoidType::get(), "eax", Location::regOf(REG_X86_EAX));
    QVERIFY(proc.getLocals().size() == (size_t)1);
    QVERIFY(proc.getSymbolMap().size() == 1);
    QCOMPARE(proc.findFirstSymbol(Location::regOf(REG_X86_EAX)), QString("eax"));

    // test for no duplicates
    proc.addLocal(IntegerType::get(32, Sign::Signed), "eax", Location::regOf(REG_X86_EAX));
    QVERIFY(proc.getLocals().size() == (size_t)1);
    QVERIFY(proc.getSymbolMap().size() == 1);
    QCOMPARE(proc.findFirstSymbol(Location::regOf(REG_X86_EAX)), QString("eax"));
}


void UserProcTest::testEnsureExpIsMappedToLocal()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_X86));

    BasicBlock *bb1 = m_project.getProg()->getCFG()->createBB(BBType::Fall, createInsns(Address(0x1000), 1));

    UserProc proc(Address(0x1000), "test", m_project.getProg()->getRootModule());

    proc.getCFG()->createFragment(FragType::Fall, createRTLs(Address(0x1000), 1, 0), bb1);
    proc.setEntryFragment();

    {
        // do not create local if nullptr def
        proc.ensureExpIsMappedToLocal(RefExp::get(Location::regOf(REG_X86_EAX), nullptr));
        QCOMPARE(proc.findLocal(Location::regOf(REG_X86_EAX), VoidType::get()), QString(""));
    }

    {
        // local does not exist
        SharedStmt ias1 = proc.getCFG()->findOrCreateImplicitAssign(Location::regOf(REG_X86_EAX));
        QVERIFY(ias1 != nullptr);

        proc.ensureExpIsMappedToLocal(RefExp::get(Location::regOf(REG_X86_EAX), ias1));
        QCOMPARE(proc.findLocal(Location::regOf(REG_X86_EAX), VoidType::get()), QString("eax"));

        // local already exists
        proc.ensureExpIsMappedToLocal(RefExp::get(Location::regOf(REG_X86_EAX), ias1));
        QCOMPARE(proc.findLocal(Location::regOf(REG_X86_EAX), VoidType::get()), QString("eax"));
    }

    {
        SharedExp memOf = Location::memOf(Binary::get(opPlus,
                                        Location::regOf(REG_X86_ESP),
                                        Const::get(4)));

        SharedStmt ias2 = proc.getCFG()->findOrCreateImplicitAssign(memOf);
        QVERIFY(ias2 != nullptr);
        proc.ensureExpIsMappedToLocal(RefExp::get(memOf->clone(), ias2));
        QCOMPARE(proc.findLocal(memOf->clone(), VoidType::get()), QString("local0"));
    }
}


void UserProcTest::testGetSymbolExp()
{
    UserProc proc(Address(0x1000), "test", nullptr);
    proc.setSignature(std::make_shared<CallingConvention::StdC::X86Signature>("test"));

    SharedExp local0 = proc.getSymbolExp(Location::regOf(REG_X86_EAX), VoidType::get());
    QVERIFY(local0 != nullptr);
    QCOMPARE(local0->toString(), Location::local("local0", &proc)->toString());
    QCOMPARE(proc.getLocalType("local0")->toString(), VoidType::get()->toString());

    SharedExp local0_2 = proc.getSymbolExp(Location::regOf(REG_X86_EAX), VoidType::get());
    QVERIFY(local0_2 != nullptr);
    QCOMPARE(local0_2->toString(), local0->toString());

    SharedExp spMinus4 = Location::memOf(
        Binary::get(opMinus,
                    RefExp::get(Location::regOf(REG_X86_ESP), nullptr),
                    Const::get(4)));

    SharedExp spMinus7 = Location::memOf(
        Binary::get(opMinus,
                    RefExp::get(Location::regOf(REG_X86_ESP), nullptr),
                    Const::get(7)));

    SharedExp local1 = proc.getSymbolExp(spMinus4, VoidType::get(), true);
    QVERIFY(local1 != nullptr);
    QCOMPARE(local1->toString(), QString("local1"));
    QCOMPARE(proc.getLocalType("local1")->toString(), IntegerType::get(STD_SIZE)->toString());



    SharedExp local2 = proc.getSymbolExp(spMinus7, IntegerType::get(8), true);
    QVERIFY(local2 != nullptr);
    QCOMPARE(local2->toString(), Location::memOf(Binary::get(opPlus,
        Unary::get(opAddrOf, Location::local("local1", &proc)), Const::get(3)))->toString());
}


void UserProcTest::testFindLocal()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    QCOMPARE(proc.findLocal(Location::regOf(REG_X86_EAX), VoidType::get()), QString(""));
    QCOMPARE(proc.findLocal(Location::local("testLocal", &proc), VoidType::get()), QString("testLocal"));

    proc.mapSymbolTo(Location::regOf(REG_X86_EAX), Location::local("foo", &proc));
    proc.createLocal(IntegerType::get(32, Sign::Signed), Location::regOf(REG_X86_EAX), "foo");
    QCOMPARE(proc.findLocal(Location::regOf(REG_X86_EAX), VoidType::get()), QString("foo"));
    QCOMPARE(proc.findLocal(Location::regOf(REG_X86_EAX), IntegerType::get(32, Sign::Signed)), QString("foo"));

    proc.mapSymbolTo(Location::regOf(REG_X86_EDX), Location::param("bar", &proc));
    QCOMPARE(proc.findLocal(Location::regOf(REG_X86_EDX), VoidType::get()), QString(""));
}


void UserProcTest::testLocalType()
{
    UserProc proc(Address(0x1000), "test", nullptr);


    QVERIFY(proc.getLocalType("") == nullptr);
    QVERIFY(proc.getLocalType("nonexistent") == nullptr);

    proc.createLocal(IntegerType::get(32, Sign::Signed), Location::regOf(REG_X86_EAX), "retVal");
    SharedConstType localType = proc.getLocalType("retVal");
    QVERIFY(localType != nullptr);
    QCOMPARE(localType->toString(), IntegerType::get(32, Sign::Signed)->toString());

    // set type
    proc.setLocalType("invalid", IntegerType::get(32, Sign::Unsigned));
    QVERIFY(proc.getLocals().size() == 1);

    proc.setLocalType("retVal", IntegerType::get(16, Sign::Unsigned));
    QCOMPARE(proc.getLocalType("retVal")->toString(), IntegerType::get(16, Sign::Unsigned)->toString());
}


void UserProcTest::testIsLocalOrParamPattern()
{
    UserProc proc(Address(0x1000), "test", nullptr);
    SharedConstExp spPlus4 = Location::memOf(Binary::get(opPlus, RefExp::get(Location::regOf(REG_X86_ESP), nullptr), Const::get(4)));

    QVERIFY(!proc.isLocalOrParamPattern(Location::regOf(REG_X86_EAX)));
    QVERIFY(!proc.isLocalOrParamPattern(spPlus4)); // signature is not promoted

    proc.setSignature(std::make_shared<CallingConvention::StdC::X86Signature>("test"));
    QVERIFY(proc.isLocalOrParamPattern(spPlus4));

    SharedConstExp spTimes4 = Location::memOf(Binary::get(opMults, RefExp::get(Location::regOf(REG_X86_EAX), nullptr), Const::get(4)));
    QVERIFY(!proc.isLocalOrParamPattern(spTimes4));

    SharedConstExp mofSP = Location::memOf(RefExp::get(Location::regOf(REG_X86_ESP), nullptr)); // m[sp{-}]
    QVERIFY(proc.isLocalOrParamPattern(mofSP));
}


void UserProcTest::testExpFromSymbol()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    proc.mapSymbolTo(Location::regOf(REG_X86_EAX), Location::local("foo", &proc));
    SharedConstExp origExp = proc.expFromSymbol("foo");
    QVERIFY(origExp != nullptr);
    QCOMPARE(origExp->toString(), Location::regOf(REG_X86_EAX)->toString());

    // bar is not a local variable
    proc.mapSymbolTo(Location::regOf(REG_X86_EDX), Location::param("bar", nullptr));
    QVERIFY(proc.expFromSymbol("bar") == nullptr);

    QVERIFY(proc.expFromSymbol("") == nullptr);
    QVERIFY(proc.expFromSymbol("nonexistent") == nullptr);
}


void UserProcTest::testMapSymbolTo()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    proc.mapSymbolTo(Location::regOf(REG_X86_EAX), Location::local("foo", &proc));
    QVERIFY(proc.getSymbolMap().size() == 1);
    QVERIFY(proc.getSymbolMap().count(Location::regOf(REG_X86_EAX)) == 1);

    /// Mapping the same value twice should not change anything
    proc.mapSymbolTo(Location::regOf(REG_X86_EAX), Location::local("foo", &proc));
    QVERIFY(proc.getSymbolMap().size() == 1);
    QVERIFY(proc.getSymbolMap().count(Location::regOf(REG_X86_EAX)) == 1);

    // conflicting expressions
    proc.mapSymbolTo(Location::regOf(REG_X86_EAX), Location::param("bar", &proc));
    QVERIFY(proc.getSymbolMap().size() == 2);
    QVERIFY(proc.getSymbolMap().count(Location::regOf(REG_X86_EAX)) == 2);

    // more than 1 conflicting expression
    proc.mapSymbolTo(Location::regOf(REG_X86_EAX), Location::param("bar2", &proc));
    QVERIFY(proc.getSymbolMap().size() == 3);
    QVERIFY(proc.getSymbolMap().count(Location::regOf(REG_X86_EAX)) == 3);
}


void UserProcTest::testLookupSym()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    QCOMPARE(proc.lookupSym(Location::regOf(REG_X86_EAX), IntegerType::get(32, Sign::Signed)), QString(""));

    proc.addLocal(IntegerType::get(32, Sign::Signed), "foo", Location::regOf(REG_X86_EAX));
    QCOMPARE(proc.lookupSym(Location::regOf(REG_X86_EAX), VoidType::get()), QString("foo"));
    QCOMPARE(proc.lookupSym(Location::regOf(REG_X86_EAX), FloatType::get(32)), QString(""));

    proc.mapSymbolTo(Location::regOf(REG_X86_EDX), Location::param("param0", &proc));
    QCOMPARE(proc.lookupSym(Location::regOf(REG_X86_EDX), VoidType::get()), QString(""));

    proc.addParameterToSignature(Location::regOf(REG_X86_EDX), IntegerType::get(32, Sign::Signed));
    QCOMPARE(proc.lookupSym(Location::regOf(REG_X86_EDX), VoidType::get()), QString("param0"));
    QCOMPARE(proc.lookupSym(Location::regOf(REG_X86_EDX), FloatType::get(32)), QString(""));
}


void UserProcTest::testLookupSymFromRef()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Fall, createInsns(Address(0x1000), 1));
    UserProc proc(Address(0x1000), "test", nullptr);

    proc.getCFG()->createFragment(FragType::Fall, createRTLs(Address(0x1000), 1, 0), bb1);
    proc.setEntryFragment();

    SharedStmt ias1 = proc.getCFG()->findOrCreateImplicitAssign(Location::regOf(REG_X86_EAX));
    QVERIFY(ias1 != nullptr);

    std::shared_ptr<RefExp> refEaxNull = RefExp::get(Location::regOf(REG_X86_EAX), nullptr);
    std::shared_ptr<RefExp> refEaxImp  = RefExp::get(Location::regOf(REG_X86_EAX), ias1);

    QCOMPARE(proc.lookupSymFromRef(refEaxNull), QString(""));
    QCOMPARE(proc.lookupSymFromRef(refEaxImp),  QString("")); // since it's not mapped to a symbol

    proc.addLocal(IntegerType::get(32, Sign::Signed), "foo", Location::regOf(REG_X86_EAX));
    QCOMPARE(proc.lookupSymFromRef(refEaxImp), QString(""));
    proc.addLocal(IntegerType::get(32, Sign::Signed), "bar", refEaxImp);
    QCOMPARE(proc.lookupSymFromRef(refEaxImp), QString("bar")); // not an exact ref match
}


void UserProcTest::testLookupSymFromRefAny()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Fall, createInsns(Address(0x1000), 1));
    UserProc proc(Address(0x1000), "test", nullptr);

    proc.getCFG()->createFragment(FragType::Fall, createRTLs(Address(0x1000), 1, 0), bb1);
    proc.setEntryFragment();

    SharedStmt ias1 = proc.getCFG()->findOrCreateImplicitAssign(Location::regOf(REG_X86_EAX));
    QVERIFY(ias1 != nullptr);

    std::shared_ptr<RefExp> refEaxNull = RefExp::get(Location::regOf(REG_X86_EAX), nullptr);
    std::shared_ptr<RefExp> refEaxImp  = RefExp::get(Location::regOf(REG_X86_EAX), ias1);

    QCOMPARE(proc.lookupSymFromRefAny(refEaxNull), QString(""));
    QCOMPARE(proc.lookupSymFromRefAny(refEaxImp),  QString("")); // since it's not mapped to a symbol

    proc.addLocal(IntegerType::get(32, Sign::Signed), "foo", Location::regOf(REG_X86_EAX));
    QCOMPARE(proc.lookupSymFromRefAny(refEaxImp), QString("foo"));
    proc.addLocal(IntegerType::get(32, Sign::Signed), "bar", refEaxImp);
    QCOMPARE(proc.lookupSymFromRefAny(refEaxImp), QString("bar"));
}


void UserProcTest::testMarkAsNonChildless()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Call, createInsns(Address(0x1000), 1));
    BasicBlock *bb2 = prog.getCFG()->createBB(BBType::Ret,  createInsns(Address(0x1800), 1));
    BasicBlock *bb3 = prog.getCFG()->createBB(BBType::Call, createInsns(Address(0x2000), 1));
    BasicBlock *bb4 = prog.getCFG()->createBB(BBType::Call, createInsns(Address(0x2400), 1));
    BasicBlock *bb5 = prog.getCFG()->createBB(BBType::Ret,  createInsns(Address(0x2800), 1));
    BasicBlock *bb6 = prog.getCFG()->createBB(BBType::Fall, createInsns(Address(0x3000), 1));

    // call graph:
    // proc1 <--> proc2 --> proc3

    UserProc proc1(Address(0x1000), "test1", nullptr);
    UserProc proc2(Address(0x2000), "test2", nullptr);
    UserProc proc3(Address(0x3000), "test3", nullptr);

    std::shared_ptr<CallStatement> call1(new CallStatement);
    call1->setDestProc(&proc2);

    std::shared_ptr<CallStatement> call2(new CallStatement);
    call2->setDestProc(&proc1);

    std::shared_ptr<CallStatement> call3(new CallStatement);
    call3->setDestProc(&proc3);

    std::shared_ptr<ReturnStatement> ret1(new ReturnStatement);
    std::shared_ptr<ReturnStatement> ret2(new ReturnStatement);
    std::shared_ptr<ReturnStatement> ret3(new ReturnStatement);

    {
        std::unique_ptr<RTLList> bbRTLs(new RTLList);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { call1 })));
        proc1.getCFG()->createFragment(FragType::Call, std::move(bbRTLs), bb1);

        bbRTLs.reset(new RTLList);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1800), { ret1 })));
        proc1.getCFG()->createFragment(FragType::Ret, std::move(bbRTLs), bb2);
        proc1.setEntryFragment();
    }

    {
        std::unique_ptr<RTLList> bbRTLs(new RTLList);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x2000), { call3 })));
        proc2.getCFG()->createFragment(FragType::Call, std::move(bbRTLs), bb3);
        bbRTLs.reset(new RTLList);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x2400), { call2 })));
        proc2.getCFG()->createFragment(FragType::Call, std::move(bbRTLs), bb4);
        bbRTLs.reset(new RTLList);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x2800), { ret2 })));
        proc2.getCFG()->createFragment(FragType::Ret, std::move(bbRTLs), bb5);
        proc2.setEntryFragment();
    }

    {
        std::unique_ptr<RTLList> bbRTLs(new RTLList);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x3000), { ret3 })));
        proc3.getCFG()->createFragment(FragType::Ret, std::move(bbRTLs), bb6);
        proc3.setEntryFragment();
    }

    proc1.setRetStmt(ret1, Address(0x1800));
    proc2.setRetStmt(ret2, Address(0x2800));
    proc3.setRetStmt(ret3, Address(0x3000));

    std::shared_ptr<ProcSet> recursionGroup(new ProcSet);
    proc1.addCallee(&proc2);
    proc2.addCallee(&proc1);
    proc2.addCallee(&proc3);
    recursionGroup->insert(&proc1);
    recursionGroup->insert(&proc2);
    proc1.setRecursionGroup(recursionGroup);
    proc2.setRecursionGroup(recursionGroup);

    proc1.markAsNonChildless(recursionGroup);
    QVERIFY(call1->getCalleeReturn() == ret2);

    proc2.markAsNonChildless(recursionGroup);
    QVERIFY(call2->getCalleeReturn() == ret1); // and not ret3
}


void UserProcTest::testAddCallee()
{
    UserProc proc1(Address(0x1000), "test1", nullptr);
    UserProc proc2(Address(0x2000), "test2", nullptr);

    QVERIFY(proc1.getCallees().empty());
    proc1.addCallee(&proc2);
    QVERIFY(std::count(proc1.getCallees().begin(), proc1.getCallees().end(), &proc2) == 1);
    proc1.addCallee(&proc2);
    QVERIFY(std::count(proc1.getCallees().begin(), proc1.getCallees().end(), &proc2) == 1);
}


void UserProcTest::testPreservesExp()
{
    QVERIFY(m_project.loadBinaryFile(SAMPLE("x86/fib")));
    QVERIFY(m_project.decodeBinaryFile());
    QVERIFY(m_project.decompileBinaryFile());
    UserProc *fib = static_cast<UserProc *>(m_project.getProg()->getFunctionByName("fib"));
    QVERIFY(fib && !fib->isLib());
    QVERIFY(fib->preservesExp(Location::regOf(REG_X86_EBX)));

    QVERIFY(m_project.loadBinaryFile(SAMPLE("x86/recursion2")));
    QVERIFY(m_project.decodeBinaryFile());
    QVERIFY(m_project.decompileBinaryFile());
    UserProc *c = static_cast<UserProc *>(m_project.getProg()->getFunctionByName("c"));
    QVERIFY(c && !c->isLib());
    QVERIFY(!c->preservesExp(Location::regOf(REG_X86_ECX)));
    // TODO more test cases here
}


void UserProcTest::testPreservesExpWithOffset()
{
    QVERIFY(m_project.loadBinaryFile(SAMPLE("x86/recursion2")));
    QVERIFY(m_project.decodeBinaryFile());
    QVERIFY(m_project.decompileBinaryFile());
    UserProc *f = static_cast<UserProc *>(m_project.getProg()->getFunctionByName("f"));
    QVERIFY(f && !f->isLib());
    QVERIFY(f->preservesExpWithOffset(Location::regOf(REG_X86_ESP), 4));
    // TODO more test cases here
}


void UserProcTest::testPromoteSignature()
{
    QVERIFY(m_project.loadBinaryFile(SAMPLE("x86/fib")));
    QVERIFY(m_project.decodeBinaryFile());
    UserProc *fib = static_cast<UserProc *>(m_project.getProg()->getFunctionByName("fib"));
    QVERIFY(fib->getSignature()->getConvention() == CallConv::INVALID);
    fib->promoteSignature();
    QVERIFY(fib->getSignature()->getConvention() == CallConv::C);
}


void UserProcTest::testFindFirstSymbol()
{
    UserProc proc(Address(0x1000), "test", nullptr);
    QCOMPARE(proc.findFirstSymbol(Location::regOf(REG_X86_EAX)), QString(""));

    proc.addLocal(VoidType::get(), "testLocal", Location::regOf(REG_X86_EAX));
    QCOMPARE(proc.findFirstSymbol(Location::regOf(REG_X86_EAX)), QString("testLocal"));

    proc.mapSymbolTo(Location::regOf(REG_X86_EAX), Location::param("testParam", &proc));
    QCOMPARE(proc.findFirstSymbol(Location::regOf(REG_X86_EAX)), QString("testLocal"));
}


void UserProcTest::testSearchAndReplace()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = m_project.getProg()->getCFG()->createBB(BBType::Fall, createInsns(Address(0x1000), 1));

    UserProc proc(Address(0x1000), "test", nullptr);

    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp edx = Location::regOf(REG_X86_EDX);
    QVERIFY(proc.searchAndReplace(*eax, edx) == false);

    std::shared_ptr<Assign> as(new Assign(VoidType::get(), eax, edx));
    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { as })));
    IRFragment *frag = proc.getCFG()->createFragment(FragType::Fall, std::move(bbRTLs), bb1);
    as->setFragment(frag);

    QVERIFY(proc.searchAndReplace(*eax, eax) == true);
    QCOMPARE(as->getLeft()->toString(), eax->toString());
    QCOMPARE(as->getRight()->toString(), edx->toString());

    QVERIFY(proc.searchAndReplace(*eax, edx) == true);
    QCOMPARE(as->getLeft()->toString(),  edx->toString());
    QCOMPARE(as->getRight()->toString(), edx->toString());

    // replace more than one expression
    QVERIFY(proc.searchAndReplace(*edx, eax) == true);
    QCOMPARE(as->getLeft()->toString(),  eax->toString());
    QCOMPARE(as->getRight()->toString(), eax->toString());
}


void UserProcTest::testAllPhisHaveDefs()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Fall, createInsns(Address(0x1000), 1));

    UserProc proc(Address(0x1000), "test", nullptr);
    QVERIFY(proc.allPhisHaveDefs());

    IRFragment *frag = proc.getCFG()->createFragment(FragType::Fall, createRTLs(Address(0x1000), 1, 0), bb1);
    proc.setEntryFragment();

    SharedStmt ias = proc.getCFG()->findOrCreateImplicitAssign(Location::regOf(REG_X86_EAX));
    QVERIFY(ias != nullptr);
    QVERIFY(proc.allPhisHaveDefs());

    std::shared_ptr<PhiAssign> phi1 = frag->addPhi(Location::regOf(REG_X86_EDX));
    QVERIFY(phi1 != nullptr);
    QVERIFY(proc.allPhisHaveDefs());

    phi1->putAt(frag, nullptr, Location::regOf(REG_X86_EAX));
    QVERIFY(!proc.allPhisHaveDefs());

    phi1->putAt(frag, ias, Location::regOf(REG_X86_EAX));
    QVERIFY(proc.allPhisHaveDefs());
}

QTEST_GUILESS_MAIN(UserProcTest)
