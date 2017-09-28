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


#include "boomerang/db/CFG.h"

#include <QtTest/QTest>

/**
 * Test for Control Flow Graphs
 */
class CfgTest : public QObject
{
	Q_OBJECT

protected:
	Cfg *m_prog;

private slots:
	void initTestCase();

    /// Test the dominator frontier code
	void testDominators();

    /// Test a case where semi dominators are different to dominators
	void testSemiDominators();

	/// Test the placing of phi functions
	void testPlacePhi();

	/// Test a case where a phi function is not needed
	void testPlacePhi2();

    /// Test the renaming of variables
	void testRenameVars();
};
