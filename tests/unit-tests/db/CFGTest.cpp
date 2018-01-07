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

    // Create BB which is smaller than an exising one.
    BasicBlock *newBB1 = cfg->createBB(BBType::Oneway, createRTLs(Address(0x1002), 2));

    QCOMPARE(cfg->getNumBBs(), 2);
    QVERIFY(cfg->isWellFormed());
    QVERIFY(newBB1 != nullptr);

    QCOMPARE(existingBB->getLowAddr(), Address(0x1000));
    QCOMPARE(existingBB->getHiAddr(),  Address(0x1001));
    QCOMPARE(newBB1->getLowAddr(), Address(0x1002));
    QCOMPARE(newBB1->getHiAddr(),  Address(0x1003));

    QVERIFY(newBB1->isType(BBType::Oneway));
    QVERIFY(newBB1->isSuccessorOf(existingBB));
    QVERIFY(existingBB->isPredecessorOf(newBB1));


    // don't create BB: blocked by two BBs with fallthrough branch
    BasicBlock *newBB2 = cfg->createBB(BBType::Oneway, createRTLs(Address(0x10002), 4));
    QVERIFY(newBB2 == nullptr); // RTLs will be erased by blocking BBs until none remain.

    QCOMPARE(cfg->getNumBBs(), 2);
    QVERIFY(cfg->isWellFormed()); // no incomplete BBs created.

    // Create BB that is larger than an existing one.
    BasicBlock *newBB3 = cfg->createBB(BBType::Oneway, createRTLs(Address(0x0FFF), 5));
    QVERIFY(newBB3 != nullptr);

    QCOMPARE(cfg->getNumBBs(), 3);
    QVERIFY(cfg->isWellFormed());

    QCOMPARE(newBB3->getLowAddr(), Address(0x0FFF));
    QCOMPARE(newBB3->getHiAddr(),  Address(0x0FFF));

    QCOMPARE(newBB3->getNumSuccessors(), 1); // don't add out edges to newBB1
    QVERIFY(newBB3->isPredecessorOf(existingBB));
    QVERIFY(existingBB->isSuccessorOf(newBB3));
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
    QSKIP("Not yet implemented.");
}


void CFGTest::testIsWellFormed()
{
    QSKIP("Not yet implemented.");
}


QTEST_MAIN(CFGTest)
