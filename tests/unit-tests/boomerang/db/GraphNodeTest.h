#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "TestUtils.h"


class GraphNodeTest : public BoomerangTest
{
    Q_OBJECT

private slots:
    // predecessors / successors
    void testGetPredecessor();
    void testGetSuccessor();
    void testSetPredecessor();
    void testSetSuccessor();
    void testAddPredecessor();
    void testAddSuccessor();
    void testRemovePredecessor();
    void testRemoveSuccessor();
    void testIsPredecessor();
    void testIsSuccessor();
};
