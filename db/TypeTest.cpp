/*==============================================================================
 * FILE:       TypeTest.cc
 * OVERVIEW:   Provides the implementation for the TypeTest class, which
 *              tests the Type class and some utility functions
 *============================================================================*/
/*
 * $Revision$
 *
 * 09 Apr 02 - Mike: Created
 */

#include "TypeTest.h"

/*==============================================================================
 * FUNCTION:        TypeTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<TypeTest> ("testUtil", \
    &TypeTest::name, *this))

void TypeTest::registerTests(CppUnit::TestSuite* suite) {

//  Note: there is nothing left to test in Util (for now)
    MYTEST(testTypeLong);
    MYTEST(testNotEqual);
}

int TypeTest::countTestCases () const
{ return 1; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        TypeTest::setUp
 * OVERVIEW:        Set up anything needed before all tests
 * NOTE:            Called before any tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void TypeTest::setUp () {
}

/*==============================================================================
 * FUNCTION:        TypeTest::tearDown
 * OVERVIEW:        Delete objects created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void TypeTest::tearDown () {
}

/*==============================================================================
 * FUNCTION:        TypeTest::testTypeLong
 * OVERVIEW:        Test type unsigned long
 *============================================================================*/
void TypeTest::testTypeLong () {

    std::string expected("unsigned long long");
    IntegerType t(64, false);
    std::string actual(t.getCtype());
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

/*==============================================================================
 * FUNCTION:        TypeTest::testNotEqual
 * OVERVIEW:        Test type inequality
 *============================================================================*/
void TypeTest::testNotEqual () {

    IntegerType t1(32, false);
    IntegerType t2(32, false);
    IntegerType t3(16, false);
    CPPUNIT_ASSERT(!(t1 != t2));
    CPPUNIT_ASSERT(t2 != t3);
}


