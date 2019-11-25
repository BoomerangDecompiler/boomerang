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
    void testHasFragment();
    void testCreateFragment();
    void testSplitFragment();
    void testEntryAndExitFragment();
    void testRemoveFragment();
    void testGetFragmentByAddr();
    void testAddEdge();
    void testIsWellFormed();
};
