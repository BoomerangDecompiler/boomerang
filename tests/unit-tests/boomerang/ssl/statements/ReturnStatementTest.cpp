#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ReturnStatementTest.h"


#include "boomerang/db/LowLevelCFG.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/ssl/type/IntegerType.h"


void ReturnStatementTest::testClone()
{
    {
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement());
        SharedStmt clone = ret->clone();

        QVERIFY(&(*clone) != &(*ret));
        QVERIFY(clone->isReturn());
        QVERIFY(clone->getID() != (uint32)-1);
        QVERIFY(clone->getID() != ret->getID());

        std::shared_ptr<ReturnStatement> retClone = clone->as<ReturnStatement>();
        QVERIFY(retClone->getNumReturns() == 0);
        QVERIFY(retClone->getRetAddr() == ret->getRetAddr());
    }
}


void ReturnStatementTest::testGetDefinitions()
{
    // ReturnStatement
    {
        Prog prog("testProg", &m_project);
        BasicBlock *bb = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1000), 1));

        UserProc *proc = static_cast<UserProc *>(prog.getOrCreateFunction(Address(0x1000)));
        proc->setSignature(std::make_shared<Signature>("test"));
        IRFragment *frag = proc->getCFG()->createFragment(FragType::Ret, createRTLs(Address(0x1000), 1, 1), bb);
        LocationSet defs;
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setFragment(frag);
        ret->setProc(proc);

        ret->getDefinitions(defs, false);
        QCOMPARE(defs.toString(), "");

        ret->getCollector()->collectDef(std::make_shared<Assign>(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));
        ret->updateModifieds();
        QVERIFY(ret->getModifieds().existsOnLeft(Location::regOf(REG_X86_EAX)));

        ret->getDefinitions(defs, false);
        QCOMPARE(defs.toString(), "r24");
    }
}


void ReturnStatementTest::testDefinesLoc()
{
    {
        Prog prog("testProg", &m_project);
        BasicBlock *bb = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1000), 1));

        UserProc *proc = static_cast<UserProc *>(prog.getOrCreateFunction(Address(0x1000)));
        proc->setSignature(std::make_shared<Signature>("test"));
        IRFragment *frag = proc->getCFG()->createFragment(FragType::Ret, createRTLs(Address(0x1000), 1, 1), bb);
        LocationSet defs;
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setFragment(frag);
        ret->setProc(proc);

        QVERIFY(!ret->definesLoc(nullptr));

        const SharedExp def = Ternary::get(opAt,
                                           Location::regOf(REG_X86_EAX),
                                           Const::get(0),
                                           Const::get(7));

        ret->getCollector()->collectDef(std::make_shared<Assign>(def, Location::regOf(REG_X86_CH)));
        ret->updateModifieds();
        QVERIFY(ret->getModifieds().existsOnLeft(def));

        QVERIFY(!ret->definesLoc(Location::regOf(REG_X86_CH)));
        QVERIFY(ret->definesLoc(Location::regOf(REG_X86_EAX)));
        QVERIFY(ret->definesLoc(def));
    }
}


void ReturnStatementTest::testSearch()
{
    // ReturnStatement
    {
        // TODO verify it only searches the returns and not the modifieds etc.
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);

        SharedExp result;
        QVERIFY(!ret->search(*Terminal::get(opWild), result));

        ret->addReturn(std::make_shared<Assign>(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));

        QVERIFY(ret->search(*Terminal::get(opWildRegOf), result));
        QVERIFY(result != nullptr);
        QCOMPARE(*result, *Location::regOf(REG_X86_EAX));
    }
}


void ReturnStatementTest::testSearchAll()
{
    // ReturnStatement
    {
        SharedExp eax = Location::regOf(REG_X86_EAX);
        SharedExp ecx = Location::regOf(REG_X86_ECX);

        // TODO verify it only searches the returns and not the modifieds etc.
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);

        std::list<SharedExp> result;
        QVERIFY(!ret->searchAll(*Terminal::get(opWild), result));
        QVERIFY(result.empty());

        ret->addReturn(std::make_shared<Assign>(eax, ecx));

        QVERIFY(ret->searchAll(*Terminal::get(opWildRegOf), result));
        QCOMPARE(result, std::list<SharedExp>({ ecx, eax }));
    }
}


void ReturnStatementTest::testSearchAndReplace()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);
    SharedExp edx = Location::regOf(REG_X86_EDX);

    Prog prog("test", &m_project);
    BasicBlock *retBB = prog.getCFG()->createBB(BBType::Ret, createInsns(Address(0x1000), 1));

    UserProc proc(Address(0x1000), "test", prog.getRootModule());
    IRFragment *retFrag = proc.getCFG()->createFragment(FragType::Ret, createRTLs(Address(0x1000), 1, 0), retBB);

    RTL *retRTL = retFrag->getLastRTL();

    {
        retRTL->clear();
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);

        QVERIFY(!ret->searchAndReplace(*ecx, eax, false));
        QCOMPARE(ret->toString(), "   0 RET\n              Modifieds: <None>\n              Reaching definitions: <None>");

        ret->addReturn(std::make_shared<Assign>(eax, ecx));

        QVERIFY(ret->searchAndReplace(*ecx, ecx, false));
        QCOMPARE(ret->toString(), "   0 RET *v* r24 := r25\n              Modifieds: <None>\n              Reaching definitions: <None>");
    }

    {
        retRTL->clear();
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);

        ret->getCollector()->collectDef(std::make_shared<Assign>(eax, ecx));

        QVERIFY(!ret->searchAndReplace(*eax, edx, false));
        QCOMPARE(ret->toString(), "   0 RET\n              Modifieds: <None>\n              Reaching definitions: r24=r25");

        QVERIFY(ret->searchAndReplace(*eax, edx, true));
        QCOMPARE(ret->toString(), "   0 RET\n              Modifieds: <None>\n              Reaching definitions: r26=r25");
    }

    {
        retRTL->clear();
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);

        ret->getCollector()->collectDef(std::make_shared<Assign>(eax, ecx));
        ret->updateModifieds();

        QVERIFY(!ret->searchAndReplace(*eax, edx, false));
        QCOMPARE(ret->toString(), "   0 RET\n              Modifieds: *v* r24\n              Reaching definitions: r24=r25");

        QVERIFY(ret->searchAndReplace(*eax, edx, true));
        QCOMPARE(ret->toString(), "   0 RET\n              Modifieds: *v* r24\n              Reaching definitions: r26=r25");
    }

    {
        retRTL->clear();
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);

        ret->getCollector()->collectDef(std::make_shared<Assign>(eax, ecx));
        ret->updateModifieds();
        ret->updateReturns();

        QVERIFY(ret->searchAndReplace(*eax, eax, false));
        QCOMPARE(ret->toString(), "   0 RET *0* r24 := r25\n              Modifieds: *v* r24\n              Reaching definitions: r24=r25");

        QVERIFY(ret->searchAndReplace(*eax, edx, true));
        QCOMPARE(ret->toString(), "   0 RET *0* r26 := r25\n              Modifieds: *v* r24\n              Reaching definitions: r26=r25");
    }
}


void ReturnStatementTest::testTypeForExp()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);

    Prog prog("test", &m_project);
    BasicBlock *retBB = prog.getCFG()->createBB(BBType::Ret, createInsns(Address(0x1000), 1));

    UserProc proc(Address(0x1000), "test", prog.getRootModule());
    IRFragment *retFrag = proc.getCFG()->createFragment(FragType::Ret, createRTLs(Address(0x1000), 1, 0), retBB);

    RTL *retRTL = retFrag->getLastRTL();

    {
        retRTL->clear();
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);

        QVERIFY(ret->getTypeForExp(ecx) == nullptr);
        ret->setTypeForExp(ecx, IntegerType::get(32, Sign::Signed));
        QVERIFY(ret->getTypeForExp(ecx) == nullptr); // since we don't have a def of ecx
    }

    {
        retRTL->clear();
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);

        ret->getCollector()->collectDef(std::make_shared<Assign>(ecx, eax));
        QVERIFY(ret->getTypeForExp(eax) == nullptr);
        QVERIFY(ret->getTypeForExp(ecx) == nullptr);

        ret->setTypeForExp(ecx, VoidType::get());
        QVERIFY(ret->getTypeForExp(eax) == nullptr);
        QVERIFY(ret->getTypeForExp(ecx) == nullptr);

        ret->updateModifieds();
        QVERIFY(ret->getTypeForExp(eax) == nullptr);
        QVERIFY(ret->getTypeForExp(ecx) != nullptr);
        QVERIFY(*ret->getTypeForExp(ecx) == *VoidType::get());

        ret->setTypeForExp(ecx, IntegerType::get(32, Sign::Signed));
        QVERIFY(ret->getTypeForExp(eax) == nullptr);
        QVERIFY(ret->getTypeForExp(ecx) != nullptr);
        QVERIFY(*ret->getTypeForExp(ecx) == *IntegerType::get(32, Sign::Signed));

        ret->updateReturns();
        QVERIFY(ret->getTypeForExp(eax) == nullptr);
        QVERIFY(ret->getTypeForExp(ecx) != nullptr);
        QVERIFY(*ret->getTypeForExp(ecx) == *IntegerType::get(32, Sign::Signed));

        ret->setTypeForExp(ecx, VoidType::get());
        QCOMPARE(ret->getModifieds().toString(), "   0 *v* r25 := -");
        QCOMPARE(ret->getReturns().toString(), "   0 *v* r25 := r24");

        ret->addReturn(std::make_shared<Assign>(IntegerType::get(32, Sign::Signed), eax, Const::get(0)));
        ret->setTypeForExp(eax, IntegerType::get(32, Sign::Unsigned));
        QVERIFY(ret->getTypeForExp(eax) == nullptr); // This only affects modifieds
        QVERIFY(ret->getTypeForExp(ecx) != nullptr);
        QVERIFY(*ret->getTypeForExp(ecx) == *VoidType::get());
    }

    // const version of getTypeForExp
    {
        retRTL->clear();
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);

        ret->getCollector()->collectDef(std::make_shared<Assign>(ecx, eax));
        QVERIFY(ret->as<const ReturnStatement>()->getTypeForExp(eax) == nullptr);
        QVERIFY(ret->as<const ReturnStatement>()->getTypeForExp(ecx) == nullptr);

        ret->setTypeForExp(ecx, VoidType::get());
        QVERIFY(ret->as<const ReturnStatement>()->getTypeForExp(eax) == nullptr);
        QVERIFY(ret->as<const ReturnStatement>()->getTypeForExp(ecx) == nullptr);

        ret->updateModifieds();
        QVERIFY(ret->as<const ReturnStatement>()->getTypeForExp(eax) == nullptr);
        QVERIFY(ret->as<const ReturnStatement>()->getTypeForExp(ecx) != nullptr);
        QVERIFY(*ret->as<const ReturnStatement>()->getTypeForExp(ecx) == *VoidType::get());

        ret->setTypeForExp(ecx, IntegerType::get(32, Sign::Signed));
        QVERIFY(ret->as<const ReturnStatement>()->getTypeForExp(eax) == nullptr);
        QVERIFY(ret->as<const ReturnStatement>()->getTypeForExp(ecx) != nullptr);
        QVERIFY(*ret->as<const ReturnStatement>()->getTypeForExp(ecx) == *IntegerType::get(32, Sign::Signed));

        ret->updateReturns();
        QVERIFY(ret->as<const ReturnStatement>()->getTypeForExp(eax) == nullptr);
        QVERIFY(ret->as<const ReturnStatement>()->getTypeForExp(ecx) != nullptr);
        QVERIFY(*ret->as<const ReturnStatement>()->getTypeForExp(ecx) == *IntegerType::get(32, Sign::Signed));

        ret->setTypeForExp(ecx, VoidType::get());
        QCOMPARE(ret->as<const ReturnStatement>()->getModifieds().toString(), "   0 *v* r25 := -");
        QCOMPARE(ret->as<const ReturnStatement>()->getReturns().toString(), "   0 *v* r25 := r24");

        ret->addReturn(std::make_shared<Assign>(IntegerType::get(32, Sign::Signed), eax, Const::get(0)));
        ret->setTypeForExp(eax, IntegerType::get(32, Sign::Unsigned));
        QVERIFY(ret->as<const ReturnStatement>()->getTypeForExp(eax) == nullptr); // This only affects modifieds
        QVERIFY(ret->as<const ReturnStatement>()->getTypeForExp(ecx) != nullptr);
        QVERIFY(*ret->as<const ReturnStatement>()->getTypeForExp(ecx) == *VoidType::get());
    }
}


void ReturnStatementTest::testSimplify()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);

    Prog prog("test", &m_project);
    BasicBlock *retBB = prog.getCFG()->createBB(BBType::Ret, createInsns(Address(0x1000), 1));

    UserProc proc(Address(0x1000), "test", prog.getRootModule());
    IRFragment *retFrag = proc.getCFG()->createFragment(FragType::Ret, createRTLs(Address(0x1000), 1, 0), retBB);

    RTL *retRTL = retFrag->getLastRTL();

    {
        retRTL->clear();
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);

        ret->simplify();

        const QString expected = "   0 RET\n              Modifieds: <None>\n              Reaching definitions: <None>";
        QCOMPARE(ret->toString(), expected);
    }

    {
        retRTL->clear();
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);

        ret->getCollector()->collectDef(std::make_shared<Assign>(ecx, Binary::get(opBitXor, eax, eax)));
        ret->updateModifieds();

        ret->simplify();

        const QString expected = "   0 RET\n              Modifieds: *v* r25\n              Reaching definitions: r25=r24 ^ r24";
        QCOMPARE(ret->toString(), expected);
    }

    {
        retRTL->clear();
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);

        ret->getCollector()->collectDef(std::make_shared<Assign>(ecx, Binary::get(opBitXor, eax, eax)));
        ret->updateModifieds();
        ret->updateReturns();

        ret->simplify();

        const QString expected = "   0 RET *0* r25 := 0\n              Modifieds: *v* r25\n              Reaching definitions: r25=r24 ^ r24";
        QCOMPARE(ret->toString(), expected);
    }
}


void ReturnStatementTest::testGetNumReturns()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);

    std::shared_ptr<ReturnStatement> ret(new ReturnStatement);

    QCOMPARE(ret->getNumReturns(), (size_t)0);
    ret->addReturn(std::make_shared<Assign>(eax, ecx));
    QCOMPARE(ret->getNumReturns(), (size_t)1);
    ret->addReturn(std::make_shared<Assign>(eax, ecx));
    QCOMPARE(ret->getNumReturns(), (size_t)2);
}


void ReturnStatementTest::testUpdateModifieds()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);

    Prog prog("test", &m_project);
    BasicBlock *retBB = prog.getCFG()->createBB(BBType::Ret, createInsns(Address(0x1000), 1));

    UserProc proc(Address(0x1000), "test", prog.getRootModule());
    IRFragment *retFrag = proc.getCFG()->createFragment(FragType::Ret, createRTLs(Address(0x1000), 1, 0), retBB);

    RTL *retRTL = retFrag->getLastRTL();

    {
        retRTL->clear();

        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);

        ret->updateModifieds();

        QVERIFY(ret->getCollector()->begin() == ret->getCollector()->end());
        QVERIFY(ret->getModifieds().empty());
    }

    {
        retRTL->clear();

        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);

        ret->getCollector()->collectDef(std::make_shared<Assign>(Terminal::get(opPC), Terminal::get(opPC)));

        ret->updateModifieds();

        QVERIFY(ret->getCollector()->begin() != ret->getCollector()->end());
        QVERIFY(ret->getModifieds().empty());
    }

    {
        retRTL->clear();

        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);

        ret->getCollector()->collectDef(std::make_shared<Assign>(ecx, eax));
        ret->updateModifieds();

        QVERIFY(ret->getCollector()->begin() != ret->getCollector()->end());
        QVERIFY(!ret->getModifieds().empty());
        QCOMPARE(ret->getModifieds().toString(), "   0 *v* r25 := -");

        ret->updateModifieds();

        QVERIFY(ret->getCollector()->begin() != ret->getCollector()->end());
        QVERIFY(!ret->getModifieds().empty());
        QCOMPARE(ret->getModifieds().toString(), "   0 *v* r25 := -");

        ret->getCollector()->collectDef(std::make_shared<Assign>(eax, ecx));
        ret->updateModifieds();

        QVERIFY(ret->getCollector()->begin() != ret->getCollector()->end());
        QVERIFY(!ret->getModifieds().empty());
        QCOMPARE(ret->getModifieds().toString(), "   0 *v* r24 := -,\t   0 *v* r25 := -");

        ret->getCollector()->clear();
        ret->updateModifieds();

        QVERIFY(ret->getCollector()->begin() == ret->getCollector()->end());
        QVERIFY(ret->getModifieds().empty());
    }
}


void ReturnStatementTest::testUpdateReturns()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);

    Prog prog("test", &m_project);
    BasicBlock *retBB = prog.getCFG()->createBB(BBType::Ret, createInsns(Address(0x1000), 1));

    UserProc proc(Address(0x1000), "test", prog.getRootModule());
    proc.setSignature(Signature::instantiate(Machine::X86, CallConv::C, "test"));
    IRFragment *retFrag = proc.getCFG()->createFragment(FragType::Ret, createRTLs(Address(0x1000), 1, 0), retBB);

    RTL *retRTL = retFrag->getLastRTL();

    {
        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);

        ret->updateReturns();

        QVERIFY(ret->getNumReturns() == 0);
    }

    {
        retRTL->clear();

        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);
        ret->getCollector()->collectDef(std::make_shared<Assign>(Location::regOf(REG_X86_ESP), eax));

        ret->updateModifieds();
        ret->updateReturns();

        QVERIFY(ret->getCollector()->begin() != ret->getCollector()->end());
        QVERIFY(ret->getNumReturns() == 0);
        QCOMPARE(ret->getReturns().toString(), "");
    }

    {
        retRTL->clear();

        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);
        ret->getCollector()->collectDef(std::make_shared<Assign>(ecx, eax));

        ret->updateModifieds();
        ret->updateReturns();

        QVERIFY(ret->getCollector()->begin() != ret->getCollector()->end());
        QVERIFY(ret->getNumReturns() == 1);
        QCOMPARE(ret->getReturns().toString(), "   0 *0* r25 := r24");

        ret->updateReturns();

        QVERIFY(ret->getCollector()->begin() != ret->getCollector()->end());
        QVERIFY(ret->getNumReturns() == 1);
        QCOMPARE(ret->getReturns().toString(), "   0 *0* r25 := r24");

        ret->getCollector()->collectDef(std::make_shared<Assign>(eax, ecx));
        ret->updateModifieds();
        ret->updateReturns();

        QVERIFY(ret->getCollector()->begin() != ret->getCollector()->end());
        QVERIFY(ret->getNumReturns() == 2);
        QCOMPARE(ret->getReturns().toString(), "   0 *0* r24 := r25,\t   0 *0* r25 := r24");

        ret->getCollector()->clear();
        ret->updateModifieds();
        ret->updateReturns(); // was non-empty before, must now be empty

        QVERIFY(ret->getCollector()->begin() == ret->getCollector()->end());
        QVERIFY(ret->getNumReturns() == 0);
        QCOMPARE(ret->getReturns().toString(), "");

        ret->getCollector()->collectDef(std::make_shared<Assign>(ecx, eax));
        ret->updateModifieds();
        ret->addReturn(std::make_shared<Assign>(ecx, RefExp::get(ecx, nullptr)));
        ret->updateReturns(); // ecx := ecx{-} (implicit def) should be removed

        QVERIFY(ret->getCollector()->begin() != ret->getCollector()->end());
        QVERIFY(ret->getNumReturns() == 0);
        QCOMPARE(ret->getReturns().toString(), "");
    }
}


void ReturnStatementTest::testAddReturn()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);

    std::shared_ptr<ReturnStatement> ret(new ReturnStatement);

    std::shared_ptr<Assign> a(new Assign(eax, ecx));
    ret->addReturn(a);
    QCOMPARE(ret->getReturns().toString(), "   0 *v* r24 := r25");
    ret->addReturn(a);
    QCOMPARE(ret->getReturns().toString(), "   0 *v* r24 := r25,\t   0 *v* r24 := r25");
}


void ReturnStatementTest::testRemoveFromModifiedsAndReturns()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);

    Prog prog("test", &m_project);
    BasicBlock *retBB = prog.getCFG()->createBB(BBType::Ret, createInsns(Address(0x1000), 1));

    UserProc proc(Address(0x1000), "test", prog.getRootModule());
    proc.setSignature(Signature::instantiate(Machine::X86, CallConv::C, "test"));
    IRFragment *retFrag = proc.getCFG()->createFragment(FragType::Ret, createRTLs(Address(0x1000), 1, 0), retBB);

    RTL *retRTL = retFrag->getLastRTL();

    {
        retRTL->clear();

        std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
        ret->setProc(&proc);
        ret->setFragment(retFrag);
        retRTL->append(ret);

        ret->getCollector()->collectDef(std::make_shared<Assign>(ecx, eax));

        ret->removeFromModifiedsAndReturns(ecx);
        QVERIFY(ret->getModifieds().empty());
        QVERIFY(ret->getReturns().empty());

        ret->updateModifieds();
        ret->removeFromModifiedsAndReturns(ecx);
        QVERIFY(ret->getModifieds().empty());
        QVERIFY(ret->getReturns().empty());

        ret->updateModifieds();
        ret->updateReturns();
        ret->removeFromModifiedsAndReturns(ecx);
        QVERIFY(ret->getModifieds().empty());
        QVERIFY(ret->getReturns().empty());

        ret->getCollector()->collectDef(std::make_shared<Assign>(eax, ecx));
        ret->updateModifieds();
        ret->removeFromModifiedsAndReturns(ecx);
        QCOMPARE(ret->getModifieds().toString(), "   0 *v* r24 := -");
        QVERIFY(ret->getReturns().empty());

        ret->updateModifieds();
        ret->updateReturns();
        ret->removeFromModifiedsAndReturns(ecx);
        QCOMPARE(ret->getModifieds().toString(), "   0 *v* r24 := -");
        QCOMPARE(ret->getReturns().toString(), "   0 *0* r24 := r25");
    }
}


void ReturnStatementTest::testRetAddr()
{
    std::shared_ptr<ReturnStatement> ret(new ReturnStatement);
    QCOMPARE(ret->getRetAddr(), Address::INVALID);

    ret->setRetAddr(Address(0x1000));
    QCOMPARE(ret->getRetAddr(), Address(0x1000));
}


QTEST_GUILESS_MAIN(ReturnStatementTest)
