#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "LowLevelCFGTest.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/LowLevelCFG.h"


void LowLevelCFGTest::testGetNumBBs()
{
    LowLevelCFG cfg;
    QCOMPARE(cfg.getNumBBs(), 0);

    cfg.createBB(BBType::Oneway, createInsns(Address(0x1000), 1));
    QCOMPARE(cfg.getNumBBs(), 1);

    cfg.createBB(BBType::DelaySlot, createInsns(Address(0x1000), 1));
    QCOMPARE(cfg.getNumBBs(), 2);

    cfg.createBB(BBType::DelaySlot, createInsns(Address(0x2000), 2));
    QCOMPARE(cfg.getNumBBs(), 3);
}


void LowLevelCFGTest::testHasBB()
{
    LowLevelCFG cfg;
    QVERIFY(!cfg.hasBB(nullptr));

    BasicBlock *bb = cfg.createBB(BBType::Oneway, createInsns(Address(0x1000), 1));
    QVERIFY(cfg.hasBB(bb));

    LowLevelCFG cfg2;
    cfg2.createBB(BBType::Oneway, createInsns(Address(0x1000), 1));
    QVERIFY(!cfg2.hasBB(bb));
}


void LowLevelCFGTest::testCreateBB()
{
    LowLevelCFG cfg;

    {
        BasicBlock *bb = cfg.createBB(BBType::Oneway, createInsns(Address(0x1000), 1));
        QVERIFY(bb != nullptr);
        QVERIFY(bb->isType(BBType::Oneway));
        QCOMPARE(bb->getLowAddr(), Address(0x1000));
        QCOMPARE(bb->getHiAddr(),  Address(0x1001));
    }

    {
        // BB already exists
        BasicBlock *bb = cfg.createBB(BBType::Oneway, createInsns(Address(0x1000), 1));
        QVERIFY(bb == nullptr);
    }

    {
        BasicBlock *bb = cfg.createBB(BBType::DelaySlot, createInsns(Address(0x1000), 1));
        QVERIFY(bb != nullptr);
        QVERIFY(bb->isType(BBType::DelaySlot));
        QCOMPARE(bb->getLowAddr(), Address(0x1000));
        QCOMPARE(bb->getHiAddr(),  Address(0x1001));
    }
}


void LowLevelCFGTest::testCreateBB_Blocking()
{
    LowLevelCFG cfg;

    BasicBlock *existingBB = cfg.createBB(BBType::Oneway, createInsns(Address(0x1000), 4));

    // Try to create a BB which is larger than an existing one.
    BasicBlock *highBB1 = cfg.createBB(BBType::Oneway, createInsns(Address(0x0FFF), 5));
    QVERIFY(highBB1 == nullptr); // ??
    QVERIFY(cfg.getBBStartingAt(Address(0x1000)).bb == existingBB); // bb was not replaced
    QVERIFY(cfg.getBBStartingAt(Address(0x1000)).delay == nullptr);
    QVERIFY(cfg.isWellFormed());
    QCOMPARE(cfg.getNumBBs(), 2);
    highBB1 = existingBB;

    BasicBlock *lowBB1 = cfg.getBBStartingAt(Address(0x0FFF)).bb;
    QVERIFY(lowBB1 != nullptr);
    QVERIFY(cfg.getBBStartingAt(Address(0x0FFF)).delay == nullptr);

    QCOMPARE(lowBB1->getNumSuccessors(), 1);
    QCOMPARE(highBB1->getNumPredecessors(), 1);

    QVERIFY(lowBB1->isPredecessorOf(highBB1));
    QVERIFY(highBB1->isSuccessorOf(lowBB1));

    // don't create BB: Blocked by two BBs with a fallthrough branch
    BasicBlock *newBB2 = cfg.createBB(BBType::Oneway, createInsns(Address(0x0FFF), 5));
    QVERIFY(newBB2 == nullptr);

    QCOMPARE(cfg.getNumBBs(), 2);
    QVERIFY(cfg.isWellFormed());

    // Note: Adding a BB that is smaller than an existing one is handled by ProcCFG::label.
    // However, ProcCFG::createBB should handle it. (TODO)
}


void LowLevelCFGTest::testCreateBB_BlockingIncomplete()
{
    LowLevelCFG cfg;

    {
        // complete the incomplete Basic Block
        cfg.createIncompleteBB(Address(0x1000));
        BasicBlock *complete1 = cfg.createBB(BBType::Oneway, createInsns(Address(0x1000), 4));

        QCOMPARE(cfg.getNumBBs(), 1);
        QVERIFY(cfg.isWellFormed());

        QVERIFY(complete1->isComplete());
        QCOMPARE(complete1->getLowAddr(), Address(0x1000));
        QCOMPARE(complete1->getHiAddr(),  Address(0x1004));
    }

    {
        // Verify that the BB is split by incomplete BBs.
        cfg.createIncompleteBB(Address(0x2002));
        BasicBlock *complete2 = cfg.createBB(BBType::Oneway, createInsns(Address(0x2000), 4));

        QCOMPARE(cfg.getNumBBs(), 3);
        QVERIFY(cfg.isWellFormed());

        QVERIFY(complete2->isComplete());
        QCOMPARE(complete2->getLowAddr(), Address(0x2002));
        QCOMPARE(complete2->getHiAddr(),  Address(0x2004));
    }
}



void LowLevelCFGTest::testCreateIncompleteBB()
{
    LowLevelCFG cfg;

    BasicBlock *bb = cfg.createIncompleteBB(Address(0x1000));
    QVERIFY(!bb->isComplete());
    QCOMPARE(bb->getLowAddr(), Address(0x1000));
    QCOMPARE(cfg.getNumBBs(), 1);
}


void LowLevelCFGTest::testEnsureBBExists()
{
    LowLevelCFG cfg;

    // create incomplete BB
    BasicBlock *dummy = nullptr;
    QCOMPARE(cfg.ensureBBExists(Address(0x1000), dummy), false);
    QVERIFY(dummy == nullptr);
    QCOMPARE(cfg.getNumBBs(), 1);

    BasicBlock *incompleteBB = cfg.getBBStartingAt(Address(0x1000)).bb;
    QVERIFY(incompleteBB != nullptr);
    QVERIFY(!incompleteBB->isComplete());

    // over an existing incomplete BB
    QCOMPARE(cfg.ensureBBExists(Address(0x1000), dummy), false);
    QVERIFY(dummy == nullptr);
    QCOMPARE(cfg.getNumBBs(), 1);

    // start of complete BB
    BasicBlock *completeBB = cfg.createBB(BBType::Oneway, createInsns(Address(0x1000), 4));
    QCOMPARE(cfg.ensureBBExists(Address(0x1000), dummy), true);
    QCOMPARE(cfg.getNumBBs(), 1);
    QVERIFY(dummy == nullptr);

    // into the middle of a complete BB
    BasicBlock *currBB = completeBB;
    QCOMPARE(cfg.ensureBBExists(Address(0x1002), currBB), true);
    QCOMPARE(cfg.getNumBBs(), 2);
    QCOMPARE(currBB->getLowAddr(), Address(0x1002));
    QCOMPARE(currBB->getHiAddr(),  Address(0x1004));
}


void LowLevelCFGTest::testGetBBStartingAt()
{
    {
        LowLevelCFG cfg;
        LowLevelCFG::BBStart b = cfg.getBBStartingAt(Address(0x1000));
        QVERIFY(b.bb == nullptr);
        QVERIFY(b.delay == nullptr);
    }

    {
        LowLevelCFG cfg;
        BasicBlock *bb = cfg.createBB(BBType::Fall, createInsns(Address(0x1000), 2));

        LowLevelCFG::BBStart b1 = cfg.getBBStartingAt(Address(0x1000));
        QVERIFY(b1.bb == bb);
        QVERIFY(b1.delay == nullptr);

        LowLevelCFG::BBStart b2 = cfg.getBBStartingAt(Address(0x1001));
        QVERIFY(b2.bb == nullptr);
        QVERIFY(b2.delay == nullptr);
    }

    {
        LowLevelCFG cfg;
        BasicBlock *bb1 = cfg.createBB(BBType::Fall, createInsns(Address(0x1000), 2));
        BasicBlock *bb2 = cfg.createBB(BBType::DelaySlot, createInsns(Address(0x1000), 1));

        LowLevelCFG::BBStart b1 = cfg.getBBStartingAt(Address(0x1000));
        QVERIFY(b1.bb == bb1);
        QVERIFY(b1.delay == bb2);

        LowLevelCFG::BBStart b2 = cfg.getBBStartingAt(Address(0x1001));
        QVERIFY(b2.bb == nullptr);
        QVERIFY(b2.delay == nullptr);
    }
}


void LowLevelCFGTest::testIsStartOfBB()
{
    {
        LowLevelCFG cfg;
        QVERIFY(!cfg.isStartOfBB(Address(0x1000)));
    }

    {
        LowLevelCFG cfg;

        cfg.createBB(BBType::Fall, createInsns(Address(0x1000), 2));
        QVERIFY(cfg.isStartOfBB(Address(0x1000)));
        QVERIFY(!cfg.isStartOfBB(Address(0x1001)));
        cfg.createBB(BBType::DelaySlot, createInsns(Address(0x1000), 1));
        QVERIFY(cfg.isStartOfBB(Address(0x1000)));
        QVERIFY(!cfg.isStartOfBB(Address(0x1001)));
    }

    {
        LowLevelCFG cfg;

        cfg.createBB(BBType::DelaySlot, createInsns(Address(0x1000), 2));
        QVERIFY(!cfg.isStartOfBB(Address(0x1000)));
        QVERIFY(!cfg.isStartOfBB(Address(0x1001)));
    }
}


void LowLevelCFGTest::testIsStartOfCompleteBB()
{
    {
        LowLevelCFG cfg;
        QVERIFY(!cfg.isStartOfCompleteBB(Address(0x1000)));
    }

    {
        LowLevelCFG cfg;
        cfg.createIncompleteBB(Address(0x1000));
        QVERIFY(!cfg.isStartOfCompleteBB(Address(0x1000)));
    }

    {
        LowLevelCFG cfg;
        cfg.createBB(BBType::DelaySlot, createInsns(Address(0x1000), 1));
        QVERIFY(!cfg.isStartOfCompleteBB(Address(0x1000)));
    }

    {
        LowLevelCFG cfg;
        cfg.createBB(BBType::Fall, createInsns(Address(0x1000), 2));
        QVERIFY(cfg.isStartOfCompleteBB(Address(0x1000)));
    }

    {
        LowLevelCFG cfg;
        cfg.createBB(BBType::DelaySlot, createInsns(Address(0x1000), 1));
        cfg.createBB(BBType::Fall, createInsns(Address(0x1000), 2));
        QVERIFY(cfg.isStartOfCompleteBB(Address(0x1000)));
    }
}


void LowLevelCFGTest::testEntryBB()
{
    LowLevelCFG cfg;

    {
        cfg.setEntryBB(nullptr);

        QVERIFY(cfg.getEntryBB() == nullptr);
    }

    {
        BasicBlock *bb1 = cfg.createBB(BBType::Oneway, createInsns(Address(0x1000), 4));
        cfg.setEntryBB(bb1);
        QVERIFY(cfg.getEntryBB() == bb1);
    }
}


void LowLevelCFGTest::testRemoveBB()
{
    {
        LowLevelCFG cfg;
        cfg.removeBB(nullptr);
        QCOMPARE(cfg.getNumBBs(), 0);
    }

    {
        // remove incomplete BB
        LowLevelCFG cfg;
        BasicBlock *bb = cfg.createIncompleteBB(Address(0x1000));
        cfg.removeBB(bb);
        QVERIFY(cfg.getBBStartingAt(Address(0x1000)).delay == nullptr);
        QCOMPARE(cfg.getNumBBs(), 0);
    }

    {
        // remove complete BB
        LowLevelCFG cfg;
        BasicBlock *bb = cfg.createBB(BBType::Oneway, createInsns(Address(0x1000), 1));
        cfg.removeBB(bb);
        QVERIFY(cfg.getBBStartingAt(Address(0x1000)).bb == nullptr);
        QVERIFY(cfg.getBBStartingAt(Address(0x1000)).delay == nullptr);
        QCOMPARE(cfg.getNumBBs(), 0);
    }

    {
        // remove delay BB, keep other one intact
        LowLevelCFG cfg;
        BasicBlock *bb    = cfg.createBB(BBType::Oneway,    createInsns(Address(0x1000), 1));
        BasicBlock *delay = cfg.createBB(BBType::DelaySlot, createInsns(Address(0x1000), 1));

        cfg.removeBB(delay);

        QVERIFY(cfg.getBBStartingAt(Address(0x1000)).bb == bb);
        QVERIFY(cfg.getBBStartingAt(Address(0x1000)).delay == nullptr);
        QCOMPARE(cfg.getNumBBs(), 1);
    }

    {
        LowLevelCFG cfg;
        // remove original BB, keep delay intact
        BasicBlock *bb    = cfg.createBB(BBType::Oneway,    createInsns(Address(0x1000), 1));
        BasicBlock *delay = cfg.createBB(BBType::DelaySlot, createInsns(Address(0x1000), 1));

        cfg.removeBB(bb);

        QVERIFY(cfg.getBBStartingAt(Address(0x1000)).bb == nullptr);
        QVERIFY(cfg.getBBStartingAt(Address(0x1000)).delay == delay);
        QCOMPARE(cfg.getNumBBs(), 1);
    }
}


void LowLevelCFGTest::testAddEdge()
{
    LowLevelCFG cfg;

    cfg.addEdge(nullptr, nullptr);
    QCOMPARE(cfg.getNumBBs(), 0);

    BasicBlock *bb1 = cfg.createBB(BBType::Oneway, createInsns(Address(0x1000), 2));
    BasicBlock *bb2 = cfg.createBB(BBType::Oneway, createInsns(Address(0x2000), 2));

    cfg.addEdge(bb1, bb2);

    QCOMPARE(bb1->getNumSuccessors(), 1);
    QCOMPARE(bb2->getNumPredecessors(), 1);
    QVERIFY(bb1->isPredecessorOf(bb2));
    QVERIFY(bb2->isSuccessorOf(bb1));

    QVERIFY(cfg.isWellFormed());
    QCOMPARE(cfg.getNumBBs(), 2);

    // we must add the edge twice because there could be a conditional jump to the next instruction
    cfg.addEdge(bb1, bb2);
    QVERIFY(cfg.isWellFormed());

    QCOMPARE(bb1->getNumSuccessors(), 2);
    QCOMPARE(bb2->getNumPredecessors(), 2);

    // add edge to addr of existing BB
    cfg.addEdge(bb2, bb1->getLowAddr());
    QVERIFY(cfg.isWellFormed());

    QCOMPARE(bb2->getNumSuccessors(), 1);
    QCOMPARE(bb1->getNumPredecessors(), 1);
    QVERIFY(bb2->isPredecessorOf(bb1));
    QVERIFY(bb1->isSuccessorOf(bb2));

    cfg.addEdge(bb2, Address(0x3000));
    QVERIFY(!cfg.isWellFormed());

    BasicBlock *incompleteBB = cfg.getBBStartingAt(Address(0x3000)).bb;
    QVERIFY(!incompleteBB->isComplete());
    QCOMPARE(incompleteBB->getLowAddr(), Address(0x3000));
    QCOMPARE(bb2->getNumSuccessors(), 2);
    QVERIFY(bb2->isPredecessorOf(incompleteBB));
    QVERIFY(incompleteBB->isSuccessorOf(bb2));

    // special case: Upgrading oneway to twoway BB
    QVERIFY(bb2->isType(BBType::Twoway));
    QVERIFY(bb1->isType(BBType::Twoway));
}


void LowLevelCFGTest::testIsWellFormed()
{
    LowLevelCFG cfg;

    QVERIFY(cfg.isWellFormed());

    BasicBlock *bb1 = cfg.createBB(BBType::Oneway, createInsns(Address(0x1000), 1));
    QVERIFY(cfg.isWellFormed());

    BasicBlock *bb2 = cfg.createIncompleteBB(Address(0x2000));
    QVERIFY(!cfg.isWellFormed());

    cfg.addEdge(bb1, bb2);
    QVERIFY(!cfg.isWellFormed()); // bb2 is still incomplete
    BasicBlock *callBB = cfg.createBB(BBType::Call, createInsns(Address(0x2000), 1)); // complete the BB
    QVERIFY(cfg.isWellFormed());

    // add interprocedural edge
    UserProc proc(Address(0x1000), "test", nullptr);

    BasicBlock *proc2BB = cfg.createBB(BBType::Oneway, createInsns(Address(0x3000), 1));

    cfg.addEdge(callBB, proc2BB);
    bb1->setFunction(&proc);
    bb2->setFunction(&proc);

    QVERIFY(!cfg.isWellFormed());
}


QTEST_GUILESS_MAIN(LowLevelCFGTest)
