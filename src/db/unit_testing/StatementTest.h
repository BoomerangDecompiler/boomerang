/***************************************************************************/ /**
 * \file StatementTest.h
 * Provides the interface for the StatementTest class, which
 *  tests the dataflow subsystems
 *============================================================================*/

/*
 * $Revision: 1.7 $
 *
 * 14 Jan 03 - Trent: Created
 */

#include <QtTest/QtTest>
class StatementTest : public QObject
{
private slots:
	void testEmpty();
	void testFlow();
	void testKill();
	void testUse();
	void testUseOverKill();
	void testUseOverBB();
	void testUseKill();
	void testLocationSet();
	void testWildLocationSet();

	// TODO check whether these tests are unnecessary; remove them if so.
	// void  testEndlessLoop ();
	// void  testRecursion ();
	// void  testExpand ();
	void testClone();
	void testIsAssign();
	void testIsFlagAssgn();
	void testAddUsedLocsAssign();
	void testAddUsedLocsBranch();
	void testAddUsedLocsCase();
	void testAddUsedLocsCall();
	void testAddUsedLocsReturn();
	void testAddUsedLocsBool();
	void testSubscriptVars();
	void testBypass();
	void testStripSizes();
	void testFindConstants();
	void initTestCase();
};
