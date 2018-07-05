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


class AssignSetTest : public BoomerangTest
{
public:
    Q_OBJECT

private slots:
    void testClear();
    void testEmpty();
    void testSize();

    void testInsert();
    void testRemove();

    void testMakeUnion();
    void testMakeDiff();
    void testMakeIsect();
    void testIsSubSetOf();

    void testDefinesLoc();
    void testLookupLoc();
};
