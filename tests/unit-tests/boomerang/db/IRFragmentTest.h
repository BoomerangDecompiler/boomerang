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


class IRFragmentTest : public BoomerangTest
{
    Q_OBJECT

private slots:
    void testConstruct();

    void testGetType();
    void testRemoveRTL();
    void testExtent();
    void testUpdateAddresses();
    void testGetStmt();

    // adding phis/implict assigns
    void testAddPhi();
    void testAddImplicit();
    void testAddPhiOverImplict();
    void testAddImplicitOverPhi();

    void testHasStatement();
    void testIsEmpty();
    void testIsEmptyJump();
    void testGetCallDestProc();
    void testGetCond();
    void testSetCond();
    void testGetDest();
    void testSimplify();
};
