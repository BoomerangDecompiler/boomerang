/*==============================================================================
 * FILE:       AnalysisTest.cc
 * OVERVIEW:   Provides the implementation for the AnalysisTest class, which
 *              tests the analysis code
 *============================================================================*/
/*
 * $Revision$
 *
 * 10 Jul 02 - Mike: Created
 */

#include "AnalysisTest.h"
#include "prog.h"
#include "proc.h"
#include "sparcfrontend.h"
#include "BinaryFile.h"
#include "BinaryFileStub.h"
#include "analysis.h"

#define CCX_SPARC       "test/sparc/condcodexform_gcc"

/*==============================================================================
 * FUNCTION:        AnalysisTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<AnalysisTest> ("testAnalysis", \
    &AnalysisTest::name, *this))

void AnalysisTest::registerTests(CppUnit::TestSuite* suite) {

    MYTEST(testFlags);
}

int AnalysisTest::countTestCases () const
{ return 1; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        AnalysisTest::setUp
 * OVERVIEW:        Set up anything needed before all tests
 * NOTE:            Called before any tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void AnalysisTest::setUp () {
    BinaryFile *pBF = BinaryFile::Load(CCX_SPARC);
    if (pBF == NULL) 
	   pBF = new BinaryFileStub();
    CPPUNIT_ASSERT(pBF != 0);
    CPPUNIT_ASSERT (pBF->GetMachine() == MACHINE_SPARC);

    // Set up the front-end object
    pFE = new SparcFrontEnd(pBF);

    // Set up the prog
    prog = pFE->decode();
}

/*==============================================================================
 * FUNCTION:        AnalysisTest::tearDown
 * OVERVIEW:        Delete objects created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void AnalysisTest::tearDown () {
    delete pFE;
}

/*==============================================================================
 * FUNCTION:        AnalysisTest::testFlags
 * OVERVIEW:        Test matching uses and definitions of flags
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void AnalysisTest::testFlags () {

    Proc* p = prog->findProc("main");
    CPPUNIT_ASSERT(p && !p->isLib());
    UserProc *pProc = (UserProc*)p;
    CPPUNIT_ASSERT(pProc);

    Analysis *analysis = new Analysis();
    // Call analysis
    analysis->analyse(pProc);

    // Hunt for a specific BB
    Cfg* cfg = pProc->getCFG();
    BB_IT it;
    PBB bb = cfg->getFirstBB(it);
    int found = 0;
    while (bb) {
        if (bb->getLowAddr() == 0x10cf4) {
            found = 1;
            break;
        }
        bb = cfg->getNextBB(it);
    }
    CPPUNIT_ASSERT_EQUAL(1, found);

//bb->print();		// It hasn't done anything, because the flag calls are expanded!

}

