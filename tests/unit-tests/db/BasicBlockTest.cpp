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

    std::unique_ptr<RTLList> rtls(new RTLList);
    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { new BranchStatement() })));

    BasicBlock bb3(BBType::Twoway, std::move(rtls), nullptr);
    QCOMPARE(bb3.getLowAddr(), Address(0x1000));
    QCOMPARE(bb3.getHiAddr(),  Address(0x1000));
}


void BasicBlockTest::testIncomplete()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    QCOMPARE(bb1.isIncomplete(), true);

    std::unique_ptr<RTLList> rtls1(new RTLList);
    rtls1->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { new BranchStatement() })));

    BasicBlock bb2(BBType::Twoway, std::move(rtls1), nullptr);
    QCOMPARE(bb2.isIncomplete(), false);

    std::unique_ptr<RTLList> rtls2(new RTLList);
    rtls2->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { new BranchStatement() })));

    BasicBlock bb3(Address(0x1000), nullptr);
    bb3.setRTLs(std::move(rtls2));
    QCOMPARE(bb3.isIncomplete(), false);
}


void BasicBlockTest::testGetPredecessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    QCOMPARE(bb1.getPredecessor(0), (BasicBlock *)nullptr); // out of range

    BasicBlock pred1(Address::ZERO, nullptr);
    bb1.addPredecessor(&pred1);
    QCOMPARE(bb1.getPredecessor(0), &pred1);
    QCOMPARE(bb1.getPredecessor(1), (BasicBlock *)nullptr);
}


void BasicBlockTest::testGetSuccessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    QCOMPARE(bb1.getSuccessor(0), (BasicBlock *)nullptr); // out of range

    BasicBlock succ1(Address::ZERO, nullptr);
    bb1.addSuccessor(&succ1);
    QCOMPARE(bb1.getSuccessor(0), &succ1);
    QCOMPARE(bb1.getSuccessor(1), (BasicBlock *)nullptr);
}


void BasicBlockTest::testSetPredecessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    BasicBlock pred1(Address::ZERO, nullptr);
    BasicBlock pred2(Address::ZERO, nullptr);

    QCOMPARE(bb1.getNumPredecessors(), 0); // not added
    bb1.addPredecessor(&pred1);
    bb1.setPredecessor(0, &pred2);
    QCOMPARE(bb1.getNumPredecessors(), 1);
    QCOMPARE(bb1.getPredecessor(0), &pred2);

    bb1.setPredecessor(0, nullptr);
    QCOMPARE(bb1.getNumPredecessors(), 1);
    QCOMPARE(bb1.getPredecessor(0), (BasicBlock *)nullptr);
}


void BasicBlockTest::testSetSuccessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    BasicBlock succ1(Address::ZERO, nullptr);
    BasicBlock succ2(Address::ZERO, nullptr);

    QCOMPARE(bb1.getNumSuccessors(), 0); // not added
    bb1.addSuccessor(&succ1);
    bb1.setSuccessor(0, &succ2);
    QCOMPARE(bb1.getNumSuccessors(), 1);
    QCOMPARE(bb1.getSuccessor(0), &succ2);

    bb1.setSuccessor(0, nullptr);
    QCOMPARE(bb1.getNumSuccessors(), 1);
    QCOMPARE(bb1.getSuccessor(0), (BasicBlock *)nullptr);
}


void BasicBlockTest::testAddPredecessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    BasicBlock pred1(Address::ZERO, nullptr);

    bb1.addPredecessor(nullptr);
    QCOMPARE(bb1.getNumPredecessors(), 1);

    bb1.addPredecessor(&pred1);
    QCOMPARE(bb1.getNumPredecessors(), 2);
    QCOMPARE(bb1.getPredecessor(1), &pred1);

    bb1.addPredecessor(&pred1);
    QCOMPARE(bb1.getNumPredecessors(), 3);
    QCOMPARE(bb1.getPredecessor(2), &pred1);
}


void BasicBlockTest::testAddSuccessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    BasicBlock succ1(Address::ZERO, nullptr);

    bb1.addSuccessor(nullptr);
    QCOMPARE(bb1.getNumSuccessors(), 1);

    bb1.addSuccessor(&succ1);
    QCOMPARE(bb1.getNumSuccessors(), 2);
    QCOMPARE(bb1.getSuccessor(1), &succ1);

    bb1.addSuccessor(&succ1);
    QCOMPARE(bb1.getNumSuccessors(), 3);
    QCOMPARE(bb1.getSuccessor(2), &succ1);
}


void BasicBlockTest::testRemovePredecessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);

    bb1.removePredecessor(nullptr);
    QCOMPARE(bb1.getNumPredecessors(), 0);

    BasicBlock pred1(Address::ZERO, nullptr);

    bb1.addPredecessor(&pred1);
    bb1.removePredecessor(nullptr);
    QCOMPARE(bb1.getNumPredecessors(), 1);
    bb1.removePredecessor(&pred1);
    QCOMPARE(bb1.getNumPredecessors(), 0);
}


void BasicBlockTest::testRemoveSuccessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);

    bb1.removeSuccessor(nullptr);
    QCOMPARE(bb1.getNumSuccessors(), 0);

    BasicBlock succ1(Address::ZERO, nullptr);

    bb1.addSuccessor(&succ1);
    bb1.removeSuccessor(nullptr);
    QCOMPARE(bb1.getNumSuccessors(), 1);
    bb1.removeSuccessor(&succ1);
    QCOMPARE(bb1.getNumSuccessors(), 0);
}


void BasicBlockTest::testIsPredecessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    BasicBlock bb2(Address::ZERO, nullptr);

    bb1.addSuccessor(&bb1);
    QVERIFY(bb1.isPredecessorOf(&bb1));
    QVERIFY(!bb1.isPredecessorOf(nullptr));
    QVERIFY(!bb1.isPredecessorOf(&bb2));
    bb1.addSuccessor(&bb2);
    QVERIFY(bb1.isPredecessorOf(&bb2));
}


void BasicBlockTest::testIsSuccessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    BasicBlock bb2(Address::ZERO, nullptr);

    bb1.addPredecessor(&bb1);
    QVERIFY(bb1.isSuccessorOf(&bb1));
    QVERIFY(!bb1.isSuccessorOf(nullptr));
    QVERIFY(!bb1.isSuccessorOf(&bb2));
    bb1.addPredecessor(&bb2);
    QVERIFY(bb1.isSuccessorOf(&bb2));
}



QTEST_MAIN(BasicBlockTest)
