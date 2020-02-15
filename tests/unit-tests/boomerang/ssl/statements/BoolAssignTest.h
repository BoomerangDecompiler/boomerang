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


class BoolAssignTest : public BoomerangTest
{
    Q_OBJECT

private slots:
    void testAssign(); //< operator=
    void testClone();
    void testGetDefinitions();
    void testDefinesLoc();
    void testSearch();
    void testSearchAll();
    void testSearchAndReplace();
    void testCond(); //< get/setCondExpr
    void testSubExps(); //< getLeft, getRight
    void testPrint();
    void testPrint_data();
};
