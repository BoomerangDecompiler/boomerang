/*==============================================================================
 * FILE:       FrontendTest.cc
 * OVERVIEW:   Provides the implementation for the FrontendTest class, which
 *              tests the FrontEnd and related classes
 *============================================================================*/
/*
 * $Revision$
 *
 * 05 Apr 02 - Mike: Created
 */

// Note: requires -DBOOMDIR=
#define HELLO_SPARC     BOOMDIR"/test/sparc/hello"
#define HELLO_PENTIUM   BOOMDIR"/test/pentium/hello"
#define HELLO_HPPA      BOOMDIR"/test/hppa/hello"
#define STARTER_PALM    BOOMDIR"/test/mc68328/Starter.prc"

#include "FrontendTest.h"
#include "prog.h"       // For global prog
#include <sstream>

/*==============================================================================
 * FUNCTION:        FrontendTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<FrontendTest> ("testExp", \
    &FrontendTest::name, *this))

void FrontendTest::registerTests(CppUnit::TestSuite* suite) {
    MYTEST(test1);
}

int FrontendTest::countTestCases () const
{ return 3; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        FrontendTest::setUp
 * OVERVIEW:        Set up anything needed before all tests
 * NOTE:            Called before any tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void FrontendTest::setUp () {
}

/*==============================================================================
 * FUNCTION:        FrontendTest::tearDown
 * OVERVIEW:        Delete objects created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void FrontendTest::tearDown () {
}

/*==============================================================================
 * FUNCTION:        FrontendTest::test1
 * OVERVIEW:        Test loading the sparc hello world program
 *============================================================================*/
void FrontendTest::test1 () {
}

