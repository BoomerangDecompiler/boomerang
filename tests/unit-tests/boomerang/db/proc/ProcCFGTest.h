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


/**
 * Tests for the Control Flow Graph
 */
class ProcCFGTest : public BoomerangTest
{
    Q_OBJECT

private slots:
    void testHasBB();
    void testCreateBB();
    void testCreateBBBlocking(); /// tests createBB if another (complete) BB is blocking the newly created BB.
    void testCreateBBBlockingIncomplete(); /// tests createBB if another incomplete BB is blocking the newly created BB.
    void testCreateIncompleteBB(); /// tests creating an incomplete BB
    void testEnsureBBExists();
    void testSetEntryAndExitBB();
    void testRemoveBB();
    void testAddEdge();
    void testIsWellFormed();
};
