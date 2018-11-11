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


class LocationSetTest : public BoomerangTest
{
    Q_OBJECT

private slots:
    void testAssign();
    void testCompare();

    void testEmpty();
    void testSize();
    void testClear();
    void testInsert();
    void testRemove();
    void testContains();
    void testContainsImplicit();
    void testFindNS();
    void testFindDifferentRef();
    void testAddSubscript();
    void testMakeUnion();
    void testMakeDiff();
};
