#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ProcCFGTest.h"


#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/LowLevelCFG.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/type/VoidType.h"

#include <QDebug>


void ProcCFGTest::testHasFragment()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Fall, createInsns(Address(0x1000), 1));
    BasicBlock *bb2 = prog.getCFG()->createBB(BBType::Fall, createInsns(Address(0x2000), 1));

    {
        UserProc proc1(Address(0x1000), "test", nullptr);
        QVERIFY(!proc1.getCFG()->hasFragment(nullptr));
    }

    {
        UserProc proc1(Address(0x1000), "test", nullptr);
        IRFragment *frag = proc1.getCFG()->createFragment(FragType::Fall, createRTLs(Address(0x1000), 1, 1), bb1);
        QVERIFY(proc1.getCFG()->hasFragment(frag));
    }

    {
        UserProc proc1(Address(0x1000), "test1", nullptr);
        UserProc proc2(Address(0x2000), "test2", nullptr);

        IRFragment *frag1 = proc1.getCFG()->createFragment(FragType::Fall, createRTLs(Address(0x1000), 1, 1), bb1);
        IRFragment *frag2 = proc2.getCFG()->createFragment(FragType::Fall, createRTLs(Address(0x2000), 1, 1), bb2);

        QVERIFY(frag1 != nullptr);
        QVERIFY(frag2 != nullptr);

        QVERIFY(proc1.getCFG()->hasFragment(frag1));
        QVERIFY(proc2.getCFG()->hasFragment(frag2));
        QVERIFY(!proc1.getCFG()->hasFragment(frag2));
        QVERIFY(!proc2.getCFG()->hasFragment(frag1));
    }
}


void ProcCFGTest::testCreateFragment()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1000), 1));

    {
        UserProc proc(Address(0x1000), "test", nullptr);
        ProcCFG *cfg = proc.getCFG();

        IRFragment *frag = cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1000), 1, 1), bb1);
        QVERIFY(frag != nullptr);
        QVERIFY(frag->isType(FragType::Oneway));
        QCOMPARE(frag->getLowAddr(), Address(0x1000));
        QCOMPARE(frag->getHiAddr(),  Address(0x1000));
        QCOMPARE(frag->getRTLs()->size(), static_cast<std::size_t>(1));

        QCOMPARE(cfg->getNumFragments(), 1);
    }

    {
        UserProc proc(Address(0x1000), "test", nullptr);
        ProcCFG *cfg = proc.getCFG();

        IRFragment *frag = cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1000), 1, 0), bb1);
        QVERIFY(frag != nullptr);
        QVERIFY(frag->isType(FragType::Oneway));
        QCOMPARE(frag->getLowAddr(), Address(0x1000));
        QCOMPARE(frag->getHiAddr(),  Address(0x1000));
        QCOMPARE(frag->getRTLs()->size(), static_cast<std::size_t>(1));

        QCOMPARE(cfg->getNumFragments(), 1);
    }
}


void ProcCFGTest::testSplitFragment()
{
    QSKIP("Not implemented");
}


void ProcCFGTest::testEntryAndExitFragment()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Fall, createInsns(Address(0x1000), 1));
    BasicBlock *bb2 = prog.getCFG()->createBB(BBType::Ret,  createInsns(Address(0x2000), 1));

    prog.getCFG()->addEdge(bb1, bb2);

    {
        UserProc proc(Address(0x1000), "test", nullptr);
        ProcCFG *cfg = proc.getCFG();
        cfg->setEntryAndExitFragment(nullptr);

        QCOMPARE(cfg->getEntryFragment(), nullptr);
        QCOMPARE(cfg->getExitFragment(), nullptr);
    }

    {
        UserProc proc(Address(0x1000), "test", nullptr);
        ProcCFG *cfg = proc.getCFG();
        IRFragment *frag1 = cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1000), 4, 1), bb1);
        cfg->setEntryAndExitFragment(frag1);

        QCOMPARE(cfg->getEntryFragment(), frag1);
        QCOMPARE(cfg->getExitFragment(), nullptr);

        IRFragment *frag2 = cfg->createFragment(FragType::Ret, createRTLs(Address(0x1004), 4, 1), bb2);
        cfg->addEdge(frag1, frag2);

        cfg->setEntryAndExitFragment(frag1);

        QCOMPARE(cfg->getEntryFragment(), frag1);
        QCOMPARE(cfg->getExitFragment(),  frag2);

        cfg->setEntryAndExitFragment(nullptr);

        QCOMPARE(cfg->getEntryFragment(), nullptr);
        QCOMPARE(cfg->getExitFragment(),  frag2);
    }
}


void ProcCFGTest::testRemoveFragment()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Fall, createInsns(Address(0x1000), 1));
    BasicBlock *bb2 = prog.getCFG()->createBB(BBType::Ret,  createInsns(Address(0x2000), 1));

    prog.getCFG()->addEdge(bb1, bb2);

    {
        UserProc proc(Address(0x1000), "test", nullptr);
        ProcCFG *cfg = proc.getCFG();

        bb1->setProc(&proc);
        bb2->setProc(&proc);


        IRFragment *frag = cfg->createFragment(FragType::Fall, createRTLs(Address(0x1000), 1, 1), bb1);
        QCOMPARE(cfg->getNumFragments(), 1);

        cfg->removeFragment(frag);
        QCOMPARE(cfg->getNumFragments(), 0);
        QVERIFY(cfg->isWellFormed());
    }

    {
        // make sure edges are removed such that a well-formed CFG stays well-formed.
        UserProc proc(Address(0x1000), "test", nullptr);
        ProcCFG *cfg = proc.getCFG();

        bb1->setProc(&proc);
        bb2->setProc(&proc);

        IRFragment *frag1 = cfg->createFragment(FragType::Fall, createRTLs(Address(0x1000), 1, 1), bb1);
        IRFragment *frag2 = cfg->createFragment(FragType::Ret,  createRTLs(Address(0x2000), 1, 1), bb2);
        cfg->addEdge(frag1, frag2);
        QVERIFY(cfg->isWellFormed());

        cfg->removeFragment(frag2);
        QVERIFY(cfg->isWellFormed());
        QCOMPARE(frag1->getNumSuccessors(), 0);
    }
}


void ProcCFGTest::testGetFragmentByAddr()
{
    QSKIP("Not implemented");
}


void ProcCFGTest::testAddEdge()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1000), 1));
    BasicBlock *bb2 = prog.getCFG()->createBB(BBType::Ret,  createInsns(Address(0x2000), 1));

    prog.getCFG()->addEdge(bb1, bb2);

    {
        UserProc proc(Address(0x1000), "test", nullptr);
        ProcCFG *cfg = proc.getCFG();

        cfg->addEdge(nullptr, nullptr);
        QCOMPARE(cfg->getNumFragments(), 0);
    }

    {
        UserProc proc(Address(0x1000), "test", nullptr);
        ProcCFG *cfg = proc.getCFG();

        bb1->setProc(&proc);
        bb2->setProc(&proc);

        IRFragment *frag1 = cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1000), 2, 1), bb1);
        IRFragment *frag2 = cfg->createFragment(FragType::Ret,    createRTLs(Address(0x2000), 2, 1), bb2);

        cfg->addEdge(frag1, frag2);

        QCOMPARE(frag1->getNumSuccessors(), 1);
        QCOMPARE(frag2->getNumPredecessors(), 1);
        QVERIFY(frag1->isPredecessorOf(frag2));
        QVERIFY(frag2->isSuccessorOf(frag1));

        QVERIFY(cfg->isWellFormed());
        QCOMPARE(cfg->getNumFragments(), 2);

        // we must add the edge twice because there could be a conditional jump to the next instruction
        cfg->addEdge(frag1, frag2);
        QVERIFY(cfg->isWellFormed());

        QCOMPARE(frag1->getNumSuccessors(), 2);
        QCOMPARE(frag2->getNumPredecessors(), 2);

        // special case: Upgrading oneway to twoway BB
        QVERIFY(frag1->isType(FragType::Twoway));
    }

    {
        UserProc proc(Address(0x1000), "test", nullptr);
        ProcCFG *cfg = proc.getCFG();

        bb1->setProc(&proc);
        bb2->setProc(&proc);

        IRFragment *frag1 = cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1000), 2, 1), bb1);
        cfg->addEdge(frag1, frag1);

        QCOMPARE(frag1->getNumSuccessors(), 1);
        QCOMPARE(frag1->getNumPredecessors(), 1);
        QVERIFY(frag1->isPredecessorOf(frag1));
        QVERIFY(frag1->isSuccessorOf(frag1));
        QCOMPARE(cfg->getNumFragments(), 1);

        QVERIFY(cfg->isWellFormed());
    }
}


void ProcCFGTest::testIsWellFormed()
{
    Prog prog("test", nullptr);
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1000), 1));
    BasicBlock *bb2 = prog.getCFG()->createBB(BBType::Ret,  createInsns(Address(0x2000), 1));

    prog.getCFG()->addEdge(bb1, bb2);

    {
        UserProc proc(Address(0x1000), "test", nullptr);
        ProcCFG *cfg = proc.getCFG();

        bb1->setProc(&proc);
        bb2->setProc(&proc);

        QVERIFY(cfg->isWellFormed());
    }

    {
        UserProc proc(Address(0x1000), "test", nullptr);
        ProcCFG *cfg = proc.getCFG();

        bb1->setProc(&proc);
        bb2->setProc(&proc);

        cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1000), 1, 1), bb1);
        QVERIFY(cfg->isWellFormed());
    }


    {
        UserProc proc(Address(0x1000), "test", nullptr);
        ProcCFG *cfg = proc.getCFG();

        bb1->setProc(&proc);
        bb2->setProc(nullptr);

        cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1000), 1, 1), bb1);
        cfg->createFragment(FragType::Ret,    createRTLs(Address(0x2000), 1, 1), bb2);

        QVERIFY(!cfg->isWellFormed());

        bb1->setProc(nullptr);
    }

    {
        UserProc proc(Address(0x1000), "test", nullptr);
        ProcCFG *cfg = proc.getCFG();

        bb1->setProc(&proc);
        bb2->setProc(&proc);

        IRFragment *frag1 = cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1000), 1, 1), bb1);
        IRFragment *frag2 = cfg->createFragment(FragType::Ret,    createRTLs(Address(0x2000), 1, 1), bb2);

        frag1->addSuccessor(frag2);
        QVERIFY(!cfg->isWellFormed());

        frag1->removeAllSuccessors();
        frag2->addPredecessor(frag1);
        QVERIFY(!cfg->isWellFormed());

        frag1->addSuccessor(frag2);
        QVERIFY(cfg->isWellFormed());
    }
}


QTEST_GUILESS_MAIN(ProcCFGTest)
