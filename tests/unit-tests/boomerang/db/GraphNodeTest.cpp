#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "GraphNodeTest.h"


class DummyNode : public GraphNode<DummyNode> {};


void GraphNodeTest::testGetPredecessor()
{
    DummyNode n1;
    QCOMPARE(n1.getPredecessor(0), nullptr); // out of range

    DummyNode pred1;
    n1.addPredecessor(&pred1);
    QCOMPARE(n1.getPredecessor(0), &pred1);
    QCOMPARE(n1.getPredecessor(1), nullptr);
}


void GraphNodeTest::testGetSuccessor()
{
    DummyNode bb1;
    QCOMPARE(bb1.getSuccessor(0), nullptr); // out of range

    DummyNode succ1;
    bb1.addSuccessor(&succ1);
    QCOMPARE(bb1.getSuccessor(0), &succ1);
    QCOMPARE(bb1.getSuccessor(1), nullptr);
}


void GraphNodeTest::testSetPredecessor()
{
    DummyNode bb1;
    DummyNode pred1;
    DummyNode pred2;

    QCOMPARE(bb1.getNumPredecessors(), 0); // not added
    bb1.addPredecessor(&pred1);
    bb1.setPredecessor(0, &pred2);
    QCOMPARE(bb1.getNumPredecessors(), 1);
    QCOMPARE(bb1.getPredecessor(0), &pred2);

    bb1.setPredecessor(0, nullptr);
    QCOMPARE(bb1.getNumPredecessors(), 1);
    QCOMPARE(bb1.getPredecessor(0), nullptr);
}


void GraphNodeTest::testSetSuccessor()
{
    DummyNode bb1;
    DummyNode succ1;
    DummyNode succ2;

    QCOMPARE(bb1.getNumSuccessors(), 0); // not added
    bb1.addSuccessor(&succ1);
    bb1.setSuccessor(0, &succ2);
    QCOMPARE(bb1.getNumSuccessors(), 1);
    QCOMPARE(bb1.getSuccessor(0), &succ2);

    bb1.setSuccessor(0, nullptr);
    QCOMPARE(bb1.getNumSuccessors(), 1);
    QCOMPARE(bb1.getSuccessor(0), nullptr);
}


void GraphNodeTest::testAddPredecessor()
{
    DummyNode bb1;
    DummyNode pred1;

    bb1.addPredecessor(nullptr);
    QCOMPARE(bb1.getNumPredecessors(), 1);

    bb1.addPredecessor(&pred1);
    QCOMPARE(bb1.getNumPredecessors(), 2);
    QCOMPARE(bb1.getPredecessor(1), &pred1);

    bb1.addPredecessor(&pred1);
    QCOMPARE(bb1.getNumPredecessors(), 3);
    QCOMPARE(bb1.getPredecessor(2), &pred1);
}


void GraphNodeTest::testAddSuccessor()
{
    DummyNode bb1;
    DummyNode succ1;

    bb1.addSuccessor(nullptr);
    QCOMPARE(bb1.getNumSuccessors(), 1);

    bb1.addSuccessor(&succ1);
    QCOMPARE(bb1.getNumSuccessors(), 2);
    QCOMPARE(bb1.getSuccessor(1), &succ1);

    bb1.addSuccessor(&succ1);
    QCOMPARE(bb1.getNumSuccessors(), 3);
    QCOMPARE(bb1.getSuccessor(2), &succ1);
}


void GraphNodeTest::testRemovePredecessor()
{
    DummyNode bb1;

    bb1.removePredecessor(nullptr);
    QCOMPARE(bb1.getNumPredecessors(), 0);

    DummyNode pred1;

    bb1.addPredecessor(&pred1);
    bb1.removePredecessor(nullptr);
    QCOMPARE(bb1.getNumPredecessors(), 1);
    bb1.removePredecessor(&pred1);
    QCOMPARE(bb1.getNumPredecessors(), 0);
}


void GraphNodeTest::testRemoveSuccessor()
{
    DummyNode bb1;

    bb1.removeSuccessor(nullptr);
    QCOMPARE(bb1.getNumSuccessors(), 0);

    DummyNode succ1;

    bb1.addSuccessor(&succ1);
    bb1.removeSuccessor(nullptr);
    QCOMPARE(bb1.getNumSuccessors(), 1);
    bb1.removeSuccessor(&succ1);
    QCOMPARE(bb1.getNumSuccessors(), 0);
}


void GraphNodeTest::testIsPredecessor()
{
    DummyNode bb1;
    DummyNode bb2;

    bb1.addSuccessor(&bb1);
    QVERIFY(bb1.isPredecessorOf(&bb1));
    QVERIFY(!bb1.isPredecessorOf(nullptr));
    QVERIFY(!bb1.isPredecessorOf(&bb2));
    bb1.addSuccessor(&bb2);
    QVERIFY(bb1.isPredecessorOf(&bb2));
}


void GraphNodeTest::testIsSuccessor()
{
    DummyNode bb1;
    DummyNode bb2;

    bb1.addPredecessor(&bb1);
    QVERIFY(bb1.isSuccessorOf(&bb1));
    QVERIFY(!bb1.isSuccessorOf(nullptr));
    QVERIFY(!bb1.isSuccessorOf(&bb2));
    bb1.addPredecessor(&bb2);
    QVERIFY(bb1.isSuccessorOf(&bb2));
}


QTEST_GUILESS_MAIN(GraphNodeTest)
