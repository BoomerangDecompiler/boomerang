/*==============================================================================
 * FILE:	   StatementTest.h
 * OVERVIEW:   Provides the interface for the StatementTest class, which
 *				tests the dataflow subsystems
 *============================================================================*/
/*
 * $Revision: 1.7 $
 *
 * 14 Jan 03 - Trent: Created
 */

#include <cppunit/extensions/HelperMacros.h>

class StatementTest : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( StatementTest );
    CPPUNIT_TEST( testEmpty );
    CPPUNIT_TEST( testFlow );
    CPPUNIT_TEST( testKill );
    CPPUNIT_TEST( testUse );
    CPPUNIT_TEST( testUseOverKill );
    CPPUNIT_TEST( testUseOverBB );
    CPPUNIT_TEST( testUseKill );
    CPPUNIT_TEST( testLocationSet );
    CPPUNIT_TEST( testWildLocationSet );
    // TODO check whether these tests are unnecessary; remove them if so.
    //CPPUNIT_TEST( testEndlessLoop );
    //CPPUNIT_TEST( testRecursion );
    //CPPUNIT_TEST( testExpand );
    CPPUNIT_TEST( testClone );
    CPPUNIT_TEST( testIsAssign );
    CPPUNIT_TEST( testIsFlagAssgn );
    CPPUNIT_TEST( testAddUsedLocsAssign );
    CPPUNIT_TEST( testAddUsedLocsBranch );
    CPPUNIT_TEST( testAddUsedLocsCase );
    CPPUNIT_TEST( testAddUsedLocsCall );
    CPPUNIT_TEST( testAddUsedLocsReturn );
    CPPUNIT_TEST( testAddUsedLocsBool );
    CPPUNIT_TEST( testSubscriptVars );
    CPPUNIT_TEST( testBypass );
    CPPUNIT_TEST( testStripSizes );
    CPPUNIT_TEST( testFindConstants );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp ();
    void tearDown ();

protected:
    void testEmpty ();
    void testFlow ();
    void testKill ();
    void testUse ();
    void testUseOverKill ();
    void testUseOverBB ();
    void testUseKill();
    void testEndlessLoop();
    void testLocationSet();
    void testWildLocationSet();
    void testRecursion();
    void testExpand();
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
};

