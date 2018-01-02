#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BasicBlockTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/statements/BranchStatement.h"


void BasicBlockTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
}


void BasicBlockTest::testGetType()
{
    BasicBlock bb(Address::ZERO, nullptr); // incomplete BB

    QVERIFY(bb.getType() == BBType::Invalid);
    QVERIFY(bb.isType(BBType::Invalid));
}


void BasicBlockTest::testExtent()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    QCOMPARE(bb1.getLowAddr(), Address(0x1000));
    QCOMPARE(bb1.getHiAddr(), Address::INVALID);

    BasicBlock bb2(BBType::Invalid, nullptr, nullptr);
    QCOMPARE(bb2.getLowAddr().toString(), Address::ZERO.toString());
    QCOMPARE(bb2.getHiAddr(), Address::INVALID);

    std::unique_ptr<RTLList> rtls(new RTLList({ new RTL(Address(0x1000), { new BranchStatement() }) }));

    BasicBlock bb3(BBType::Twoway, std::move(rtls), nullptr);
    QCOMPARE(bb3.getLowAddr(), Address(0x1000));
    QCOMPARE(bb3.getHiAddr(),  Address(0x1000));
}


void BasicBlockTest::testIncomplete()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    QCOMPARE(bb1.isIncomplete(), true);

    std::unique_ptr<RTLList> rtls1(new RTLList({ new RTL(Address(0x1000), { new BranchStatement() }) }));

    BasicBlock bb2(BBType::Twoway, std::move(rtls1), nullptr);
    QCOMPARE(bb2.isIncomplete(), false);

    std::unique_ptr<RTLList> rtls2(new RTLList({ new RTL(Address(0x1000), { new BranchStatement() }) }));
    BasicBlock bb3(Address(0x1000), nullptr);
    bb3.setRTLs(std::move(rtls2));
    QCOMPARE(bb3.isIncomplete(), false);
}


void BasicBlockTest::testPredecessor()
{
    BasicBlock bb(Address::ZERO, nullptr);

    BasicBlock pred1(Address::ZERO, nullptr);
    BasicBlock pred2(Address::ZERO, nullptr);

    QCOMPARE(bb.getNumPredecessors(), 0);

    bb.addPredecessor(&pred1);

    QCOMPARE(bb.getNumPredecessors(), 1);
    QCOMPARE(bb.getPredecessor(0), &pred1);
    QCOMPARE(pred1.getNumSuccessors(), 0);

    bb.addPredecessor(&pred2);
    QCOMPARE(bb.getNumPredecessors(), 2);
    QCOMPARE(bb.getPredecessor(1), &pred2);

    // We must add the bb twice since there might be a twoway branch to the next instruction
    bb.addPredecessor(&pred2);
    QCOMPARE(bb.getNumPredecessors(), 3);
    QCOMPARE(bb.getPredecessor(2), &pred2);

    // remove all references to pred2
    bb.removePredecessor(&pred2);

    QCOMPARE(bb.getNumPredecessors(), 1);
    QCOMPARE(bb.getPredecessor(0), &pred1); // check if we removed the correct one

    bb.setPredecessor(0, &pred2);

    QCOMPARE(bb.getNumPredecessors(), 1);
    QCOMPARE(bb.getPredecessor(0), &pred2);

    bb.removeAllPredecessors();

    QCOMPARE(bb.getNumPredecessors(), 0);
}


void BasicBlockTest::testSuccessor()
{
    BasicBlock bb(Address::ZERO, nullptr);

    BasicBlock succ1(Address::ZERO, nullptr);
    BasicBlock succ2(Address::ZERO, nullptr);

    QCOMPARE(bb.getNumSuccessors(), 0);

    bb.addSuccessor(&succ1);

    QCOMPARE(bb.getNumSuccessors(), 1);
    QCOMPARE(bb.getSuccessor(0), &succ1);
    QCOMPARE(succ1.getNumPredecessors(), 0);

    bb.addSuccessor(&succ2);
    QCOMPARE(bb.getNumSuccessors(), 2);
    QCOMPARE(bb.getSuccessor(1), &succ2);

    // We must add the bb twice since there might be a twoway branch to the next instruction
    bb.addSuccessor(&succ2);
    QCOMPARE(bb.getNumSuccessors(), 3);
    QCOMPARE(bb.getSuccessor(2), &succ2);

    // remove all references to pred2
    bb.removeSuccessor(&succ2);

    QCOMPARE(bb.getNumSuccessors(), 1);
    QCOMPARE(bb.getSuccessor(0), &succ1); // check if we removed the correct one

    bb.setSuccessor(0, &succ2);

    QCOMPARE(bb.getNumSuccessors(), 1);
    QCOMPARE(bb.getSuccessor(0), &succ2);

    bb.removeAllSuccessors();
    QCOMPARE(bb.getNumSuccessors(), 0);
}


QTEST_MAIN(BasicBlockTest)
