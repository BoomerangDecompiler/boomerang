/*==============================================================================
 * FILE:       CfgTest.cc
 * OVERVIEW:   Provides the implementation for the CfgTest class, which
 *              tests the Exp and derived classes
 *============================================================================*/
/*
 * $Revision$
 *
 * 17 Jul 03 - Mike: Created
 */

#ifndef BOOMDIR
#error Must define BOOMDIR
#endif

#define FRONTIER_PENTIUM		BOOMDIR "/test/pentium/frontier"
#define SEMI_PENTIUM		    BOOMDIR "/test/pentium/semi"

#include "CfgTest.h"
#include <sstream>
#include <string>
#include "BinaryFile.h"
#include "frontend.h"
#include "pentiumfrontend.h"        // For PentiumFrontEnd
#include "proc.h"
#include "prog.h"
#include "dom.h"

/*==============================================================================
 * FUNCTION:        CfgTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<CfgTest> ("CfgTest", \
    &CfgTest::name, *this))

void CfgTest::registerTests(CppUnit::TestSuite* suite) {
    MYTEST(testDominators);
    MYTEST(testSemiDominators);
}

int CfgTest::countTestCases () const
{ return 2; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        CfgTest::setUp
 * OVERVIEW:        Set up some expressions for use with all the tests
 * NOTE:            Called before any tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void CfgTest::setUp () {
	//prog.setName("default name");
}

/*==============================================================================
 * FUNCTION:        CfgTest::tearDown
 * OVERVIEW:        Delete expressions created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void CfgTest::tearDown () {
}

/*==============================================================================
 * FUNCTION:        CfgTest::testDominators
 * OVERVIEW:        Test the dominator frontier code
 *============================================================================*/
#define FRONTIER_FOUR   0x08048347
#define FRONTIER_FIVE   0x08048351
#define FRONTIER_TWELVE 0x080483b2
#define FRONTIER_THIRTEEN 0x080483b9

void CfgTest::testDominators () {
    BinaryFile *pBF = BinaryFile::Load(FRONTIER_PENTIUM);
    CPPUNIT_ASSERT(pBF != 0);
    FrontEnd *pFE = new PentiumFrontEnd(pBF);
    Prog *prog = pFE->decode();

    bool gotMain;
    ADDRESS addr = pFE->getMainEntryPoint(gotMain);
    CPPUNIT_ASSERT (addr != NO_ADDRESS);

    UserProc* pProc = (UserProc*) prog->getProc(0);
    Cfg* cfg = pProc->getCFG();

    DOM* d = new DOM;
    cfg->dominators(d);

    // Find BB "5" (as per Appel, Figure 19.5).
    BB_IT it;
    PBB bb = cfg->getFirstBB(it);
    while (bb && bb->getLowAddr() != FRONTIER_FIVE) {
        bb = cfg->getNextBB(it);
    }
    CPPUNIT_ASSERT(bb);

    std::ostringstream expected, actual;
    expected << std::hex << FRONTIER_FIVE << " " << FRONTIER_THIRTEEN << " " <<
        FRONTIER_TWELVE << " " << FRONTIER_FOUR << " ";
    int n5 = d->indices[bb];
    std::set<int>::iterator ii;
    for (ii=d->DF[n5].begin(); ii != d->DF[n5].end(); ii++)
        actual << std::hex << (unsigned)d->BBs[*ii]->getLowAddr() << " ";
    CPPUNIT_ASSERT_EQUAL(expected.str(), actual.str());

    pBF->UnLoad();
    delete pFE;
    delete d;
}


/*==============================================================================
 * FUNCTION:        CfgTest::testSemiDominators
 * OVERVIEW:        Test a case where semi dominators are different to dominators
 *============================================================================*/
#define SEMI_L  0x80483b0
#define SEMI_M  0x80483e2
#define SEMI_B  0x8048345
#define SEMI_D  0x8048354
#define SEMI_M  0x80483e2

void CfgTest::testSemiDominators () {
    BinaryFile* pBF = BinaryFile::Load(SEMI_PENTIUM);
    CPPUNIT_ASSERT(pBF != 0);
    FrontEnd* pFE = new PentiumFrontEnd(pBF);
    Prog* prog = pFE->decode();

    bool gotMain;
    ADDRESS addr = pFE->getMainEntryPoint(gotMain);
    CPPUNIT_ASSERT (addr != NO_ADDRESS);

    UserProc* pProc = (UserProc*) prog->getProc(0);
    Cfg* cfg = pProc->getCFG();

    DOM* d = new DOM;
    cfg->dominators(d);

    // Find BB "L (6)" (as per Appel, Figure 19.8).
    BB_IT it;
    PBB bb = cfg->getFirstBB(it);
    while (bb && bb->getLowAddr() != SEMI_L) {
        bb = cfg->getNextBB(it);
    }
    CPPUNIT_ASSERT(bb);
    int nL = d->indices[bb];

    // The dominator for L should be B, where the semi dominator is D (book says F)
    unsigned actual_dom  = (unsigned)d->BBs[d->idom[nL]]->getLowAddr();
    unsigned actual_semi = (unsigned)d->BBs[d->semi[nL]]->getLowAddr();
    CPPUNIT_ASSERT_EQUAL((unsigned)SEMI_B, actual_dom);
    CPPUNIT_ASSERT_EQUAL((unsigned)SEMI_D, actual_semi);
    // Check the final dominator frontier as well; should be M and B
    std::ostringstream expected, actual;
    expected << std::hex << SEMI_M << " " << SEMI_B << " ";
    std::set<int>::iterator ii;
    for (ii=d->DF[nL].begin(); ii != d->DF[nL].end(); ii++)
        actual << std::hex << (unsigned)d->BBs[*ii]->getLowAddr() << " ";
    CPPUNIT_ASSERT_EQUAL(expected.str(), actual.str());
}
