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


class LowLevelCFGTest : public BoomerangTest
{
    Q_OBJECT

private slots:
    void testGetNumBBs();
    void testCreateBB();
    void testCreateBB_Blocking();
    void testCreateBB_BlockingIncomplete();
    void testCreateIncompleteBB();
    void testEnsureBBExists();
    void testGetBBStartingAt();
    void testIsStartOfBB();
    void testIsStartOfCompleteBB();
    void testEntryBB(); // getEntryBB / setEntryBB
    void testRemoveBB();
    void testAddEdge();
    void testIsWellFormed();
};
