/*==============================================================================
 * FILE:       ProcTest.cc
 * OVERVIEW:   Provides the implementation for the ProcTest class, which
 *              tests the Proc class
 *============================================================================*/
/*
 * $Revision$
 *
 * 23 Apr 02 - Mike: Created
 */

//#ifndef BOOMDIR
//#error Must define BOOMDIR
//#endif

#define HELLO_PENTIUM       BOOMDIR "/test/pentium/hello"

#include "ProcTest.h"
#include "BinaryFile.h"
#include "BinaryFileStub.h"
#include "pentiumfrontend.h"

#include <sstream>
#include <map>

/*==============================================================================
 * FUNCTION:        ProcTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<ProcTest> ("testProc", \
    &ProcTest::name, *this))

void ProcTest::registerTests(CppUnit::TestSuite* suite) {

    MYTEST(testName);
}

int ProcTest::countTestCases () const
{ return 2; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        ProcTest::setUp
 * OVERVIEW:        Set up some expressions for use with all the tests
 * NOTE:            Called before any tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void ProcTest::setUp () {
    m_prog = new Prog();
    m_prog->pBF = new BinaryFileStub();
    CPPUNIT_ASSERT(m_prog->pBF != 0);
    // Set the text limits
    m_prog->getTextLimits();
    m_prog->pFE = new PentiumFrontEnd(m_prog, m_prog->textDelta, m_prog->limitTextHigh);
    m_prog->readLibParams();
}

/*==============================================================================
 * FUNCTION:        ProcTest::tearDown
 * OVERVIEW:        Delete expressions created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void ProcTest::tearDown () {
    m_prog->pBF->UnLoad();
    delete m_proc;
    delete m_prog;
}

/*==============================================================================
 * FUNCTION:        ProcTest::testName
 * OVERVIEW:        Test setting and reading name, constructor, native address
 *============================================================================*/
void ProcTest::testName () {
    std::string nm("default name");
    m_proc = new UserProc(m_prog, nm, 20000);   // They will print in decimal if error
    std::string actual(m_proc->getName());
    CPPUNIT_ASSERT_EQUAL(std::string("default name"), actual);

    std::string name("printf");
    LibProc lp(m_prog, name, 30000);
    actual =  lp.getName();
    CPPUNIT_ASSERT_EQUAL(name, actual);

    ADDRESS a = lp.getNativeAddress();
    ADDRESS expected = 30000;
    CPPUNIT_ASSERT_EQUAL(expected, a);
    a = m_proc->getNativeAddress();
    expected = 20000;
    CPPUNIT_ASSERT_EQUAL(expected, a);
}

