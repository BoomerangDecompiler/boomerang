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
#include "sparcfrontend.h"
#include "BinaryFile.h"
#include "BinaryFileStub.h"

#define CCX_SPARC       BOOMDIR "/test/sparc/condcodexform_gcc"
#define SSL_PATH        BOOMDIR "/frontend/machine/sparc/sparc.ssl"

// There is no .h file for analysis; there is just this prototype
void analysis(UserProc* proc);

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
    prog = new Prog();
    prog->pBF = new BinaryFileStub();
    CPPUNIT_ASSERT(prog->pBF != 0);

    // Set the text limits
    prog->getTextLimits();

	// Set up the front-end object
	prog->pFE = new SparcFrontEnd(prog,  prog->textDelta, prog->limitTextHigh);
	decoder = prog->pFE->getDecoder();

}

/*==============================================================================
 * FUNCTION:        AnalysisTest::tearDown
 * OVERVIEW:        Delete objects created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void AnalysisTest::tearDown () {
    prog->pBF->UnLoad();
    delete prog->pFE; prog->pFE = 0;
}

/*==============================================================================
 * FUNCTION:        AnalysisTest::testFlags
 * OVERVIEW:        Test matching uses and definitions of flags
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void AnalysisTest::testFlags () {
    
    CPPUNIT_ASSERT (prog->pBF->GetMachine() == MACHINE_SPARC);

    bool gotMain;
    ADDRESS addr = prog->pFE->getMainEntryPoint(gotMain);
    CPPUNIT_ASSERT (addr != NO_ADDRESS);

    std::string name("main");
    UserProc* pProc = new UserProc(prog, name, addr);
    std::ofstream dummy;
    bool res = prog->pFE->processProc(addr, pProc, dummy, false);
	CPPUNIT_ASSERT(res);

    // Call analysis
    analysis(pProc);

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

    // FIXME: Last test should delete decoder
    delete decoder;

}

