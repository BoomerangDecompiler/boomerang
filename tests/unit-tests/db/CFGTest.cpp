#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CFGTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/type/type/VoidType.h"

#include <QDebug>


std::unique_ptr<RTLList> createRTLs(Address baseAddr, int numRTLs)
{
    std::unique_ptr<RTLList> rtls(new RTLList);

    for (int i = 0; i < numRTLs; i++) {
        rtls->push_back(new RTL(baseAddr + i,
            { new Assign(VoidType::get(), Terminal::get(opNil), Terminal::get(opNil)) }));
    }

    return rtls;
}


void CFGTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
}


void CFGTest::testCreateBB()
{
    UserProc proc(Address(0x1000), "test", nullptr);
    Cfg *cfg = proc.getCFG();

    BasicBlock *bb = cfg->createBB(BBType::Oneway, createRTLs(Address(0x1000), 1));
    QVERIFY(bb != nullptr);
    QVERIFY(bb->isType(BBType::Oneway));
    QCOMPARE(bb->getLowAddr(), Address(0x1000));
    QCOMPARE(bb->getHiAddr(),  Address(0x1000));
    QCOMPARE(bb->getRTLs()->size(), (size_t)1);

    QCOMPARE(cfg->getNumBBs(), 1);
}


void CFGTest::testCreateBBBlocking()
{
    UserProc proc(Address(0x1000), "test", nullptr);
    Cfg *cfg = proc.getCFG();

    BasicBlock *existingBB = cfg->createBB(BBType::Oneway, createRTLs(Address(0x1000), 4));

    // Try to create a BB which is larger than an existing one.
    BasicBlock *highBB1 = cfg->createBB(BBType::Oneway, createRTLs(Address(0x0FFF), 5));
    QVERIFY(highBB1 == nullptr); // ??
    QVERIFY(cfg->getBBStartingAt(Address(0x1000)) == existingBB); // bb was not replaced
    highBB1 = existingBB;

    QVERIFY(cfg->isWellFormed());
    QCOMPARE(cfg->getNumBBs(), 2);

    BasicBlock *lowBB1 = cfg->getBBStartingAt(Address(0x0FFF));
    QVERIFY(lowBB1 != nullptr);

    QCOMPARE(lowBB1->getNumSuccessors(), 1);
    QCOMPARE(highBB1->getNumPredecessors(), 1);

    QVERIFY(lowBB1->isPredecessorOf(highBB1));
    QVERIFY(highBB1->isSuccessorOf(lowBB1));

    // don't create BB: Blocked by two BBs with a fallthrough branch
    BasicBlock *newBB2 = cfg->createBB(BBType::Oneway, createRTLs(Address(0x0FFF), 5));
    QVERIFY(newBB2 == nullptr);

    QCOMPARE(cfg->getNumBBs(), 2);
    QVERIFY(cfg->isWellFormed());

    // Note: Adding a BB that is smaller than an existing one is handled by Cfg::label.
    // However, Cfg::createBB should handle it. (TODO)
}


void CFGTest::testCreateBBBlockingIncomplete()
{
    UserProc proc(Address(0x1000), "test", nullptr);
    Cfg *cfg = proc.getCFG();

    // complete the incomplete Basic Block
    cfg->createIncompleteBB(Address(0x1000));
    BasicBlock *complete1 = cfg->createBB(BBType::Oneway, createRTLs(Address(0x1000), 4));

    QCOMPARE(cfg->getNumBBs(), 1);
    QVERIFY(cfg->isWellFormed());

    QVERIFY(!complete1->isIncomplete());
    QCOMPARE(complete1->getLowAddr(), Address(0x1000));
    QCOMPARE(complete1->getHiAddr(),  Address(0x1003));

    // Verify that the BB is split by incomplete BBs.
    cfg->createIncompleteBB(Address(0x2002));
    BasicBlock *complete2 = cfg->createBB(BBType::Oneway, createRTLs(Address(0x2000), 4));

    QCOMPARE(cfg->getNumBBs(), 3);
    QVERIFY(cfg->isWellFormed());

    QVERIFY(!complete2->isIncomplete());
    QCOMPARE(complete2->getLowAddr(), Address(0x2002));
    QCOMPARE(complete2->getHiAddr(),  Address(0x2003));
}


void CFGTest::testCreateIncompleteBB()
{
    UserProc proc(Address(0x1000), "test", nullptr);
    Cfg *cfg = proc.getCFG();

    BasicBlock *bb = cfg->createIncompleteBB(Address(0x1000));
    QVERIFY(bb->isIncomplete());
    QCOMPARE(bb->getLowAddr(), Address(0x1000));
    QCOMPARE(cfg->getNumBBs(), 1);
}


void CFGTest::testRemoveBB()
{
    UserProc proc(Address(0x1000), "test", nullptr);
    Cfg *cfg = proc.getCFG();

    cfg->removeBB(nullptr);
    QCOMPARE(cfg->getNumBBs(), 0);

    // remove incomplete BB
    BasicBlock *bb = cfg->createIncompleteBB(Address(0x1000));
    cfg->removeBB(bb);
    QCOMPARE(cfg->getNumBBs(), 0);

    // remove complete BB
    bb = cfg->createBB(BBType::Oneway, createRTLs(Address(0x1000), 1));
    cfg->removeBB(bb);
    QCOMPARE(cfg->getNumBBs(), 0);
}


void CFGTest::testAddEdge()
{
    UserProc proc(Address(0x1000), "test", nullptr);
    Cfg *cfg = proc.getCFG();

    cfg->addEdge(nullptr, nullptr);
    QCOMPARE(cfg->getNumBBs(), 0);

    BasicBlock *bb1 = cfg->createBB(BBType::Oneway, createRTLs(Address(0x1000), 2));
    BasicBlock *bb2 = cfg->createBB(BBType::Oneway, createRTLs(Address(0x2000), 2));

    cfg->addEdge(bb1, bb2);

    QCOMPARE(bb1->getNumSuccessors(), 1);
    QCOMPARE(bb2->getNumPredecessors(), 1);
    QVERIFY(bb1->isPredecessorOf(bb2));
    QVERIFY(bb2->isSuccessorOf(bb1));

    QVERIFY(cfg->isWellFormed());
    QCOMPARE(cfg->getNumBBs(), 2);

    // we must add the edge twice because there could be a conditional jump to the next instruction
    cfg->addEdge(bb1, bb2);
    QVERIFY(cfg->isWellFormed());

    QCOMPARE(bb1->getNumSuccessors(), 2);
    QCOMPARE(bb2->getNumPredecessors(), 2);

    // add edge to addr of existing BB
    cfg->addEdge(bb2, bb1->getLowAddr());
    QVERIFY(cfg->isWellFormed());

    QCOMPARE(bb2->getNumSuccessors(), 1);
    QCOMPARE(bb1->getNumPredecessors(), 1);
    QVERIFY(bb2->isPredecessorOf(bb1));
    QVERIFY(bb1->isSuccessorOf(bb2));

    cfg->addEdge(bb2, Address(0x3000));
    QVERIFY(!cfg->isWellFormed());

    BasicBlock *incompleteBB = cfg->getBBStartingAt(Address(0x3000));
    QVERIFY(incompleteBB->isIncomplete());
    QCOMPARE(incompleteBB->getLowAddr(), Address(0x3000));
    QCOMPARE(bb2->getNumSuccessors(), 2);
    QVERIFY(bb2->isPredecessorOf(incompleteBB));
    QVERIFY(incompleteBB->isSuccessorOf(bb2));

    // special case: Upgrading oneway to twoway BB
    QVERIFY(bb2->isType(BBType::Twoway));
    QVERIFY(bb1->isType(BBType::Twoway));
}


void CFGTest::testIsWellFormed()
{
    UserProc proc(Address(0x1000), "test", nullptr);
    Cfg *cfg = proc.getCFG();

    QVERIFY(cfg->isWellFormed());

    BasicBlock *bb1 = cfg->createBB(BBType::Oneway, createRTLs(Address(0x1000), 1));
    QVERIFY(cfg->isWellFormed());

    BasicBlock *bb2 = cfg->createIncompleteBB(Address(0x2000));
    QVERIFY(!cfg->isWellFormed());

    cfg->addEdge(bb1, bb2);
    QVERIFY(!cfg->isWellFormed()); // bb2 is still incomplete
    BasicBlock *callBB = cfg->createBB(BBType::Call, createRTLs(Address(0x2000), 1)); // complete the BB
    QVERIFY(cfg->isWellFormed());

    // add interprocedural edge
    UserProc proc2(Address(0x3000), "test2", nullptr);
    Cfg *proc2Cfg = proc2.getCFG();

    BasicBlock *proc2BB = proc2Cfg->createBB(BBType::Oneway, createRTLs(Address(0x3000), 1));

    cfg->addEdge(callBB, proc2BB);
    QVERIFY(!cfg->isWellFormed());
    QVERIFY(!proc2Cfg->isWellFormed());
}


QTEST_MAIN(CFGTest)
