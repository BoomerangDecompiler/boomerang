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
    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::initTestCase
    * OVERVIEW:        Set up some expressions for use with all the tests
    * NOTE:            Called before any tests
    * PARAMETERS:        <none>
    *
    *============================================================================*/
    void initTestCase();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testEmpty
    * OVERVIEW:
    *============================================================================*/
	void testEmpty();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testFlow
    * OVERVIEW:
    *============================================================================*/
	void testFlow();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testKill
    * OVERVIEW:
    *============================================================================*/
	void testKill();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testUse
    * OVERVIEW:
    *============================================================================*/
	void testUse();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testUseOverKill
    * OVERVIEW:
    *============================================================================*/
	void testUseOverKill();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testUseOverBB
    * OVERVIEW:
    *============================================================================*/
	void testUseOverBB();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testUseKill
    * OVERVIEW:
    *============================================================================*/
	void testUseKill();


    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testLocationSet
    * OVERVIEW:
    *============================================================================*/
	void testLocationSet();



    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testWildLocationSet
    * OVERVIEW:
    *============================================================================*/
	void testWildLocationSet();

	// TODO check whether these three tests below are unnecessary; remove them if so.
    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testEndlessLoop
    * OVERVIEW:
    *============================================================================*/
	void  testEndlessLoop ();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testRecursion
    * OVERVIEW:        Test push of argument (X86 style), then call self
    *============================================================================*/
	void  testRecursion ();
	// void  testExpand ();


    /***************************************************************************/ /**
    * FUNCTION:        StatamentTest::testClone
    * OVERVIEW:        Test cloning of Assigns (and exps)
    *============================================================================*/
	void testClone();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testIsAssign
    * OVERVIEW:        Test assignment test
    *============================================================================*/
	void testIsAssign();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testIsFlagCall
    * OVERVIEW:        Test the isFlagAssgn function, and opFlagCall
    *============================================================================*/
	void testIsFlagAssgn();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testAddUsedLocsAssign .. testAddUsedLocsBool
    * OVERVIEW:        Test the finding of locations used by this statement
    *============================================================================*/
	void testAddUsedLocsAssign();
	void testAddUsedLocsBranch();
	void testAddUsedLocsCase();
	void testAddUsedLocsCall();
	void testAddUsedLocsReturn();
	void testAddUsedLocsBool();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testSubscriptVars
    * OVERVIEW:        Test the subscripting of locations in Statements
    *============================================================================*/
	void testSubscriptVars();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testBypass
    * OVERVIEW:        Test the visitor code that fixes references that were to locations defined by calls
    *============================================================================*/
	void testBypass();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testStripSizes
    * OVERVIEW:        Test the visitor code that strips out size casts
    *============================================================================*/
	void testStripSizes();

    /***************************************************************************/ /**
    * FUNCTION:        StatementTest::testFindConstants
    * OVERVIEW:        Test the visitor code that finds constants
    *============================================================================*/
	void testFindConstants();
};
