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

class FrontSparcTest : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();

	/**
     * Test decoding some sparc instructions
     */
	void test1();

	void test2();

	void test3();

	void testBranch();

	void testDelaySlot();
};
