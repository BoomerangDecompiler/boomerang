/*==============================================================================
 * FILE:       StatementTest.h
 * OVERVIEW:   Provides the interface for the StatementTest class, which
 *              tests the dataflow subsystems
 *============================================================================*/
/*
 * $Revision$
 *
 * 14 Jan 03 - Trent: Created
 */

#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

#include "proc.h"
#include "prog.h"

class StatementTest : public CppUnit::TestCase {
  protected:

  public:
    StatementTest(std::string name) : CppUnit::TestCase (name)
    {}

    virtual void registerTests(CppUnit::TestSuite* suite);

    int countTestCases () const;

    void setUp ();
    void tearDown ();

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
    void testAddUsedLocs();
};

