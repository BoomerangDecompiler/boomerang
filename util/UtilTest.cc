/*==============================================================================
 * FILE:       UtilTest.cc
 * OVERVIEW:   Provides the implementation for the UtilTest class, which
 *              tests the Type class and some utility functions
 *============================================================================*/
/*
 * $Revision$
 *
 * 09 Apr 02 - Mike: Created
 */

#include "UtilTest.h"

/*==============================================================================
 * FUNCTION:        UtilTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<UtilTest> ("testUtil", \
    &UtilTest::name, *this))

void UtilTest::registerTests(CppUnit::TestSuite* suite) {
    MYTEST(testTypeLong);
    MYTEST(testNotEqual);
}

int UtilTest::countTestCases () const
{ return 1; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        UtilTest::setUp
 * OVERVIEW:        Set up anything needed before all tests
 * NOTE:            Called before any tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void UtilTest::setUp () {
}

/*==============================================================================
 * FUNCTION:        UtilTest::tearDown
 * OVERVIEW:        Delete objects created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void UtilTest::tearDown () {
}

/*==============================================================================
 * FUNCTION:        UtilTest::testTypeLong
 * OVERVIEW:        Test type unsigned long
 *============================================================================*/
void UtilTest::testTypeLong () {

    std::string expected("unsigned int64");
    Type t(INTEGER, 64, false);
    std::string actual(t.getCtype());
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

/*==============================================================================
 * FUNCTION:        UtilTest::testNotEqual
 * OVERVIEW:        Test type inequality
 *============================================================================*/
void UtilTest::testNotEqual () {

    Type t1(INTEGER, 32, false);
    Type t2(INTEGER, 32, false);
    Type t3(INTEGER, 16, false);
    CPPUNIT_ASSERT(!(t1 != t2));
    CPPUNIT_ASSERT(t2 != t3);
}


