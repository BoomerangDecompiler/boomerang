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

#define FRONTIER_PENTIUM		"test/pentium/frontier"
#define SEMI_PENTIUM		    "test/pentium/semi"
#define IFTHEN_PENTIUM		    "test/pentium/ifthen"

#include "CfgTest.h"
#include <sstream>
#include <string>
#include "BinaryFile.h"
#include "frontend.h"
#include "pentiumfrontend.h"        // For PentiumFrontEnd
#include "proc.h"
#include "prog.h"

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
    MYTEST(testPlacePhi);
    MYTEST(testPlacePhi2);
    MYTEST(testRenameVars);
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
    Type::clearNamedTypes();
    Prog *prog = pFE->decode();

    bool gotMain;
    ADDRESS addr = pFE->getMainEntryPoint(gotMain);
    CPPUNIT_ASSERT (addr != NO_ADDRESS);

    UserProc* pProc = (UserProc*) prog->getProc(0);
    Cfg* cfg = pProc->getCFG();

    cfg->dominators();

    // Find BB "5" (as per Appel, Figure 19.5).
    BB_IT it;
    PBB bb = cfg->getFirstBB(it);
    while (bb && bb->getLowAddr() != FRONTIER_FIVE) {
        bb = cfg->getNextBB(it);
    }
    CPPUNIT_ASSERT(bb);

    std::ostringstream expected, actual;
  //expected << std::hex << FRONTIER_FIVE << " " << FRONTIER_THIRTEEN << " " <<
  //      FRONTIER_TWELVE << " " << FRONTIER_FOUR << " ";
    expected << std::hex << FRONTIER_THIRTEEN << " " << FRONTIER_FOUR << " " <<
        FRONTIER_TWELVE << " " << FRONTIER_FIVE << " ";
    int n5 = cfg->pbbToNode(bb);
    std::set<int>::iterator ii;
    std::set<int>& DFset = cfg->getDF(n5);
    for (ii=DFset.begin(); ii != DFset.end(); ii++)
        actual << std::hex << (unsigned)cfg->nodeToBB(*ii)->getLowAddr() << " ";
    CPPUNIT_ASSERT_EQUAL(expected.str(), actual.str());

    pBF->UnLoad();
    delete pFE;
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
    Type::clearNamedTypes();
    Prog* prog = pFE->decode();

    bool gotMain;
    ADDRESS addr = pFE->getMainEntryPoint(gotMain);
    CPPUNIT_ASSERT (addr != NO_ADDRESS);

    UserProc* pProc = (UserProc*) prog->getProc(0);
    Cfg* cfg = pProc->getCFG();

    cfg->dominators();

    // Find BB "L (6)" (as per Appel, Figure 19.8).
    BB_IT it;
    PBB bb = cfg->getFirstBB(it);
    while (bb && bb->getLowAddr() != SEMI_L) {
        bb = cfg->getNextBB(it);
    }
    CPPUNIT_ASSERT(bb);
    int nL = cfg->pbbToNode(bb);

    // The dominator for L should be B, where the semi dominator is D
    // (book says F)
    unsigned actual_dom  = (unsigned)cfg->nodeToBB(cfg->getIdom(nL))->getLowAddr();
    unsigned actual_semi = (unsigned)cfg->nodeToBB(cfg->getSemi(nL))->getLowAddr();
    CPPUNIT_ASSERT_EQUAL((unsigned)SEMI_B, actual_dom);
    CPPUNIT_ASSERT_EQUAL((unsigned)SEMI_D, actual_semi);
    // Check the final dominator frontier as well; should be M and B
    std::ostringstream expected, actual;
  //expected << std::hex << SEMI_M << " " << SEMI_B << " ";
    expected << std::hex << SEMI_B << " " << SEMI_M << " ";
    std::set<int>::iterator ii;
    std::set<int>& DFset = cfg->getDF(nL);
    for (ii=DFset.begin(); ii != DFset.end(); ii++)
        actual << std::hex << (unsigned)cfg->nodeToBB(*ii)->getLowAddr() << " ";
    CPPUNIT_ASSERT_EQUAL(expected.str(), actual.str());
}

/*==============================================================================
 * FUNCTION:        CfgTest::testPlacePhi
 * OVERVIEW:        Test the placing of phi functions
 *============================================================================*/
void CfgTest::testPlacePhi () {
    BinaryFile* pBF = BinaryFile::Load(FRONTIER_PENTIUM);
    CPPUNIT_ASSERT(pBF != 0);
    FrontEnd* pFE = new PentiumFrontEnd(pBF);
    Type::clearNamedTypes();
    Prog* prog = pFE->decode();

    UserProc* pProc = (UserProc*) prog->getProc(0);
    Cfg* cfg = pProc->getCFG();

    // Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
    prog->analyse();

    cfg->dominators();
    cfg->placePhiFunctions(1, pProc);

    // m[r29 - 8] (x for this program)
    Exp* e = new Unary(opMemOf,
        new Binary(opMinus,
            Location::regOf(29),
            new Const(4)));

    // A_phi[x] should be the set {7 8 10 15 20 21} (all the join points)
    std::set<int> expected;
    expected.insert(7);  expected.insert(8); expected.insert(10);
    expected.insert(15); expected.insert(20); expected.insert(21);
    bool result = expected == cfg->getA_phi(e);
    CPPUNIT_ASSERT_EQUAL(true, result);
    delete e;
}

/*==============================================================================
 * FUNCTION:        CfgTest::testPlacePhi2
 * OVERVIEW:        Test a case where a phi function is not needed
 *============================================================================*/
void CfgTest::testPlacePhi2 () {
    BinaryFile* pBF = BinaryFile::Load(IFTHEN_PENTIUM);
    CPPUNIT_ASSERT(pBF != 0);
    FrontEnd* pFE = new PentiumFrontEnd(pBF);
    Type::clearNamedTypes();
    Prog* prog = pFE->decode();

    UserProc* pProc = (UserProc*) prog->getProc(0);
    Cfg* cfg = pProc->getCFG();

    // Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
    prog->analyse();

    cfg->dominators();
    cfg->placePhiFunctions(1, pProc);

    // In this program, x is allocated at [ebp-4], a at [ebp-8], and
    // b at [ebp-12]
    // We check that A_phi[ m[ebp-8] ] is 4, and that
    // A_phi A_phi[ m[ebp-8] ] is null
    // (block 4 comes out with n=4)

    std::string expected = "4 ";
    std::ostringstream actual;
    // m[r29 - 8]
    Exp* e = new Unary(opMemOf,
        new Binary(opMinus,
            Location::regOf(29),
            new Const(8)));
    std::set<int>& s = cfg->getA_phi(e);
    std::set<int>::iterator pp;
    for (pp = s.begin(); pp != s.end(); pp++)
        actual << *pp << " ";
    CPPUNIT_ASSERT_EQUAL(expected, actual.str());
    delete e;

    expected = "";
    std::ostringstream actual2;
    // m[r29 - 12]
    e = new Unary(opMemOf,
        new Binary(opMinus,
            Location::regOf(29),
            new Const(12)));
 
    std::set<int>& s2 = cfg->getA_phi(e);
    for (pp = s2.begin(); pp != s2.end(); pp++)
        actual2 << *pp << " ";
    CPPUNIT_ASSERT_EQUAL(expected, actual2.str());
    delete e;
}

/*==============================================================================
 * FUNCTION:        CfgTest::testRenameVars
 * OVERVIEW:        Test the renaming of variables
 *============================================================================*/
void CfgTest::testRenameVars () {
    BinaryFile* pBF = BinaryFile::Load(FRONTIER_PENTIUM);
    CPPUNIT_ASSERT(pBF != 0);
    FrontEnd* pFE = new PentiumFrontEnd(pBF);
    Type::clearNamedTypes();
    Prog* prog = pFE->decode();

    UserProc* pProc = (UserProc*) prog->getProc(0);
    Cfg* cfg = pProc->getCFG();

    // Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
    prog->analyse();

    cfg->dominators();
    cfg->placePhiFunctions(1, pProc);
    int stmtNumber = 0;
    pProc->numberStatements(stmtNumber);// After placing phi functions!
    cfg->renameBlockVars(0, 1);      // Block 0, mem depth 1

    // MIKE: something missing here?
}
