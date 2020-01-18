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


#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/db/LowLevelCFG.h"


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
    QSKIP("TODO");
}


QTEST_GUILESS_MAIN(ReturnStatementTest)
