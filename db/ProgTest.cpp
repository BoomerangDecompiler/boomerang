/*==============================================================================
 * FILE:	   ProgTest.cc
 * OVERVIEW:   Provides the implementation for the ProgTest class, which
 *				tests the Exp and derived classes
 *============================================================================*/
/*
 * $Revision$
 *
 * 18 Apr 02 - Mike: Created
 * 18 Jul 02 - Mike: Set up prog.pFE before calling readLibParams
 */

#define HELLO_PENTIUM		"test/pentium/hello"

#include "ProgTest.h"
#include "BinaryFile.h"
#include "pentiumfrontend.h"
#include <map>
#include <sstream>

/*==============================================================================
 * FUNCTION:		ProgTest::registerTests
 * OVERVIEW:		Register the test functions in the given suite
 * PARAMETERS:		Pointer to the test suite
 * RETURNS:			<nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<ProgTest> ("ProgTest", \
	&ProgTest::name, *this))

void ProgTest::registerTests(CppUnit::TestSuite* suite) {
	MYTEST(testName);
}

int ProgTest::countTestCases () const
{ return 2; }	// ? What's this for?

/*==============================================================================
 * FUNCTION:		ProgTest::setUp
 * OVERVIEW:		Set up some expressions for use with all the tests
 * NOTE:			Called before any tests
 * PARAMETERS:		<none>
 * RETURNS:			<nothing>
 *============================================================================*/
void ProgTest::setUp () {
	//prog.setName("default name");
}

/*==============================================================================
 * FUNCTION:		ProgTest::tearDown
 * OVERVIEW:		Delete expressions created in setUp
 * NOTE:			Called after all tests
 * PARAMETERS:		<none>
 * RETURNS:			<nothing>
 *============================================================================*/
void ProgTest::tearDown () {
}

/*==============================================================================
 * FUNCTION:		ProgTest::testName
 * OVERVIEW:		Test setting and reading name
 *============================================================================*/
void ProgTest::testName () {
	BinaryFile *pBF = BinaryFileFactory::Load(HELLO_PENTIUM);	// Don't actually use it
	FrontEnd *pFE = new PentiumFrontEnd(pBF);
	// We need a Prog object with a pBF (for getEarlyParamExp())
	Prog* prog = new Prog(pBF, pFE);
	std::string actual(prog->getName());
	std::string expected(HELLO_PENTIUM);
	CPPUNIT_ASSERT_EQUAL(expected, actual);
	std::string name("Happy prog");
	prog->setName(name.c_str());
	actual =  prog->getName();
	CPPUNIT_ASSERT_EQUAL(name, actual);
}

// Pathetic: the second test we had (for readLibraryParams) is now obsolete;
// the front end does this now.
