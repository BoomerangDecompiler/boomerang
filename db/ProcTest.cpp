/*==============================================================================
 * FILE:       ProcTest.cc
 * OVERVIEW:   Provides the implementation for the ProcTest class, which
 *              tests the Proc class
 *============================================================================*/
/*
 * $Revision$
 *
 * 23 Apr 02 - Mike: Created
 * 10 Mar 03 - Mike: Mods to not use Prog::pBF (no longer public)
 */

#ifndef BOOMDIR
#error Must define BOOMDIR
#endif

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
    BinaryFile *pBF = new BinaryFileStub();
    CPPUNIT_ASSERT(pBF != 0);
    FrontEnd *pFE = new PentiumFrontEnd(pBF);
    CPPUNIT_ASSERT(pFE != 0);
}

/*==============================================================================
 * FUNCTION:        ProcTest::tearDown
 * OVERVIEW:        Delete expressions created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void ProcTest::tearDown () {
    delete m_proc;
}

/*==============================================================================
 * FUNCTION:        ProcTest::testName
 * OVERVIEW:        Test setting and reading name, constructor, native address
 *============================================================================*/
void ProcTest::testName () {
    std::string nm("default name");
    BinaryFile *pBF = BinaryFile::Load(HELLO_PENTIUM);
    FrontEnd *pFE = new PentiumFrontEnd(pBF);
    Prog* prog = new Prog(pBF, pFE);
    CPPUNIT_ASSERT(prog);
    m_proc = new UserProc(prog, nm, 20000);   // They will print in decimal if error
    std::string actual(m_proc->getName());
    CPPUNIT_ASSERT_EQUAL(std::string("default name"), actual);

    std::string name("printf");
    LibProc lp(prog, name, 30000);
    actual =  lp.getName();
    CPPUNIT_ASSERT_EQUAL(name, actual);

    ADDRESS a = lp.getNativeAddress();
    ADDRESS expected = 30000;
    CPPUNIT_ASSERT_EQUAL(expected, a);
    a = m_proc->getNativeAddress();
    expected = 20000;
    CPPUNIT_ASSERT_EQUAL(expected, a);

    delete prog;
}

