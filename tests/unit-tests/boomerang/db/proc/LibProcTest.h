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


class LibProcTest : public BoomerangTest
{
    Q_OBJECT

private slots:
    // tests for Function class
    void testName();
    void testEntryAddr();
    void testRemoveFromModule();
    void testRemoveParameter();
    void testRenameParameter();

    // tests for LibProc class
    void testIsLib();
    void testIsNoReturn();
    void testGetProven();
    void testGetPremised();
    void testIsPreserved();
};
