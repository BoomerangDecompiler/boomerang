/*==============================================================================
 * FILE:       ProgTest.cc
 * OVERVIEW:   Provides the implementation for the ProgTest class, which
 *              tests the Exp and derived classes
 *============================================================================*/
/*
 * $Revision$
 *
 * 18 Apr 02 - Mike: Created
 */

#include "ProgTest.h"
#include <map>
#include <sstream>

Prog prog;                      // Ugly global

/*==============================================================================
 * FUNCTION:        ProgTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<ProgTest> ("testExp", \
    &ProgTest::name, *this))

void ProgTest::registerTests(CppUnit::TestSuite* suite) {
    MYTEST(testName);
}

int ProgTest::countTestCases () const
{ return 2; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        ProgTest::setUp
 * OVERVIEW:        Set up some expressions for use with all the tests
 * NOTE:            Called before any tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void ProgTest::setUp () {
    m_prog = new Prog("default name");
    m_prog->readLibParams();        // Read library signatures
}

/*==============================================================================
 * FUNCTION:        ProgTest::tearDown
 * OVERVIEW:        Delete expressions created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void ProgTest::tearDown () {
    delete m_prog;
}

/*==============================================================================
 * FUNCTION:        ProgTest::testName
 * OVERVIEW:        Test setting and reading name
 *============================================================================*/
void ProgTest::testName () {
    std::string actual(m_prog->getName());
    CPPUNIT_ASSERT_EQUAL(std::string("default name"), actual);
    std::string name("Happy prog");
    m_prog->setName(name);
    actual =  m_prog->getName();
    CPPUNIT_ASSERT_EQUAL(name, actual);
}

