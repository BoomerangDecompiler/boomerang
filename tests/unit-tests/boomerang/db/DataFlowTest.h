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


#include <QtTest/QTest>

/**
 * Test for basic data-flow related code
 */
class DataFlowTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    /// Test calculating (semi-)dominators and the Dominance Frontier
    void testCalculateDominators();

    /// Test the placing of phi functions
    void testPlacePhi();

    /// Test a case where a phi function is not needed
    void testPlacePhi2();

    /// Test the renaming of variables
    void testRenameVars();
};
