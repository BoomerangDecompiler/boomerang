/*==============================================================================
 * FILE:       ProgTest.cc
 * OVERVIEW:   Provides the implementation for the ProgTest class, which
 *              tests the Exp and derived classes
 *============================================================================*/
/*
 * $Revision$
 *
 * 18 Apr 02 - Mike: Created
 * 18 Jul 02 - Mike: Set up prog.pFE before calling readLibParams
 */

//#ifndef BOOMDIR
//#error Must define BOOMDIR
//#endif

#define HELLO_PENTIUM		BOOMDIR "/test/pentium/hello"

#include "ProgTest.h"
//#include "BinaryFile.h"
//#include "pentiumfrontend.h"
#include <map>
#include <sstream>

/*==============================================================================
 * FUNCTION:        ProgTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<ProgTest> ("ProgTest", \
    &ProgTest::name, *this))

void ProgTest::registerTests(CppUnit::TestSuite* suite) {
    MYTEST(testName);
    MYTEST(testLibParams);
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
	//prog.setName("default name");
}

/*==============================================================================
 * FUNCTION:        ProgTest::tearDown
 * OVERVIEW:        Delete expressions created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void ProgTest::tearDown () {
}

/*==============================================================================
 * FUNCTION:        ProgTest::testName
 * OVERVIEW:        Test setting and reading name
 *============================================================================*/
void ProgTest::testName () {
/*    std::string actual(prog.getName());
    CPPUNIT_ASSERT_EQUAL(std::string("default name"), actual);
    std::string name("Happy prog");
    prog.setName(name.c_str());
    actual =  prog.getName();
    CPPUNIT_ASSERT_EQUAL(name, actual); */
}

/*==============================================================================
 * FUNCTION:        ProgTest::testLibParams
 * OVERVIEW:        Test loading of the library parameters
 *============================================================================*/
void ProgTest::testLibParams () {
	// Read the library parameter information
	// Oops - now have to set up a frontend before calling this
/*    prog.clear();
    prog.pBF = BinaryFile::Load(HELLO_PENTIUM);
    CPPUNIT_ASSERT(prog.pBF != 0);
    // Set the text limits
    prog.getTextLimits();

	prog.pFE = new PentiumFrontEnd(prog.textDelta, prog.limitTextHigh);

    prog.readLibParams();        // Read library signatures
	// The test is that it doesn't fall over, really
	CPPUNIT_ASSERT(prog.mapLibParam.size() != 0);

	prog.pBF->UnLoad();
	delete prog.pBF;
	prog.pBF = NULL;
	delete prog.pFE;
	prog.pFE = NULL; */
}

