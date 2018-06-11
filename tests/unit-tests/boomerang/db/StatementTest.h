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


/*
 * $Revision: 1.7 $
 *
 * 14 Jan 03 - Trent: Created
 */

#include <QtTest/QtTest>

#include "boomerang/core/Project.h"


/**
 * Provides the interface for the StatementTest class, which
 *  tests the dataflow subsystems
 */
class StatementTest : public QObject
{
    Q_OBJECT

private slots:
    /**
     * Set up some expressions for use with all the tests
     * \note Called before any tests
     */
    void initTestCase();
    void cleanupTestCase();

    void testEmpty();
    void testFlow();
    void testKill();
    void testUse();
    void testUseOverKill();
    void testUseOverBB();
    void testUseKill();
    void testLocationSet();
    void testWildLocationSet();

    void testEndlessLoop();

    /// Test push of argument (X86 style), then call self
    void testRecursion();


    /// Test cloning of Assigns (and exps)
    void testClone();

    /// Test assignment test
    void testIsAssign();

    /// Test the isFlagAssgn function, and opFlagCall
    void testIsFlagAssgn();

    /// Test the finding of locations used by this statement
    void testAddUsedLocsAssign();
    void testAddUsedLocsBranch();
    void testAddUsedLocsCase();
    void testAddUsedLocsCall();
    void testAddUsedLocsReturn();
    void testAddUsedLocsBool();

    /// Test the subscripting of locations in Statements
    void testSubscriptVars();

    /// Test the visitor code that fixes references that were to locations defined by calls
    void testBypass();

    /// Test the visitor code that strips out size casts
    void testStripSizes();

    /// Test the visitor code that finds constants
    void testFindConstants();

private:
    Project m_project;
};
