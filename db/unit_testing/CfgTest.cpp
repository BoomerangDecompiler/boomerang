/***************************************************************************//**
 * \file       CfgTest.cc
 * OVERVIEW:   Provides the implementation for the CfgTest class, which
 *                tests the Exp and derived classes
 *============================================================================*/

#include "CfgTest.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "proc.h"
#include "prog.h"
#include "dataflow.h"
#include "pentiumfrontend.h"
#include "log.h"
#include <QDir>
#include <QProcessEnvironment>
#include <sstream>

#define FRONTIER_PENTIUM    "tests/inputs/pentium/frontier"
#define SEMI_PENTIUM        "tests/inputs/pentium/semi"
#define IFTHEN_PENTIUM      "tests/inputs/pentium/ifthen"
static bool logset = false;
QString TEST_BASE;
QDir baseDir;
CfgTest::CfgTest()
{
    if(!logset) {
        TEST_BASE = QProcessEnvironment::systemEnvironment().value("BOOMERANG_TEST_BASE","");
        baseDir = QDir(TEST_BASE);
        Boomerang::get()->setProgPath(TEST_BASE.toStdString());
        Boomerang::get()->setPluginPath(TEST_BASE+"/out");
        if(TEST_BASE.isEmpty())
            fprintf(stderr,"BOOMERANG_TEST_BASE environment variable net set, many test will fail\n");
        logset=true;
        Boomerang::get()->setLogger(new NullLogger());
    }
}

void CfgTest::SetUp ()
{
}
void CfgTest::TearDown () {
}

/***************************************************************************//**
 * FUNCTION:        CfgTest::testDominators
 * OVERVIEW:        Test the dominator frontier code
 *============================================================================*/
#define FRONTIER_FOUR    0x08048347
#define FRONTIER_FIVE    ADDRESS::g(0x08048351)
#define FRONTIER_TWELVE 0x080483b2
#define FRONTIER_THIRTEEN 0x080483b9

void CfgTest::testDominators() {
    BinaryFileFactory bff;
    QObject *pBF = bff.Load(baseDir.absoluteFilePath(FRONTIER_PENTIUM));
    QVERIFY(pBF != 0);
    Prog* prog = new Prog;
    FrontEnd *pFE = new PentiumFrontEnd(pBF, prog, &bff);
    Type::clearNamedTypes();
    prog->setFrontEnd(pFE);
    pFE->decode(prog);

    bool gotMain;
    ADDRESS addr = pFE->getMainEntryPoint(gotMain);
    QVERIFY (addr != NO_ADDRESS);

    UserProc* pProc = (UserProc*) prog->getProc(0);
    Cfg* cfg = pProc->getCFG();
    DataFlow* df = pProc->getDataFlow();
    df->dominators(cfg);

    // Find BB "5" (as per Appel, Figure 19.5).
    BB_IT it;
    BasicBlock *bb = cfg->getFirstBB(it);
    while (bb && bb->getLowAddr() != FRONTIER_FIVE) {
        bb = cfg->getNextBB(it);
    }
    QVERIFY(bb);

    std::ostringstream expected, actual;
  //expected << std::hex << FRONTIER_FIVE << " " << FRONTIER_THIRTEEN << " " << FRONTIER_TWELVE << " " <<
  //    FRONTIER_FOUR << " ";
    expected << std::hex << FRONTIER_THIRTEEN << " " << FRONTIER_FOUR << " " << FRONTIER_TWELVE << " " <<
        FRONTIER_FIVE << " ";
    int n5 = df->pbbToNode(bb);
    std::set<int>::iterator ii;
    std::set<int>& DFset = df->getDF(n5);
    for (ii=DFset.begin(); ii != DFset.end(); ii++)
        actual << std::hex << df->nodeToBB(*ii)->getLowAddr() << " ";
    QCOMPARE(expected.str(), actual.str());

    pBF->deleteLater();
    delete pFE;
}


/***************************************************************************//**
 * FUNCTION:        CfgTest::testSemiDominators
 * OVERVIEW:        Test a case where semi dominators are different to dominators
 *============================================================================*/
#define SEMI_L    ADDRESS::g(0x80483b0)
#define SEMI_M    ADDRESS::g(0x80483e2)
#define SEMI_B    ADDRESS::g(0x8048345)
#define SEMI_D    ADDRESS::g(0x8048354)
#define SEMI_M    ADDRESS::g(0x80483e2)

void CfgTest::testSemiDominators() {
    BinaryFileFactory bff;
    QObject* pBF = bff.Load(baseDir.absoluteFilePath(SEMI_PENTIUM));
    QVERIFY(pBF != 0);
    Prog* prog = new Prog;
    FrontEnd* pFE = new PentiumFrontEnd(pBF, prog, &bff);
    Type::clearNamedTypes();
    prog->setFrontEnd(pFE);
    pFE->decode(prog);

    bool gotMain;
    ADDRESS addr = pFE->getMainEntryPoint(gotMain);
    QVERIFY (addr != NO_ADDRESS);

    UserProc* pProc = (UserProc*) prog->getProc(0);
    Cfg* cfg = pProc->getCFG();

    DataFlow* df = pProc->getDataFlow();
    df->dominators(cfg);

    // Find BB "L (6)" (as per Appel, Figure 19.8).
    BB_IT it;
    BasicBlock * bb = cfg->getFirstBB(it);
    while (bb && bb->getLowAddr() != SEMI_L) {
        bb = cfg->getNextBB(it);
    }
    QVERIFY(bb);
    int nL = df->pbbToNode(bb);

    // The dominator for L should be B, where the semi dominator is D
    // (book says F)
    ADDRESS actual_dom  = df->nodeToBB(df->getIdom(nL))->getLowAddr();
    ADDRESS actual_semi = df->nodeToBB(df->getSemi(nL))->getLowAddr();
    QCOMPARE(SEMI_B, actual_dom);
    QCOMPARE(SEMI_D, actual_semi);
    // Check the final dominator frontier as well; should be M and B
    std::ostringstream expected, actual;
  //expected << std::hex << SEMI_M << " " << SEMI_B << " ";
    expected << std::hex << SEMI_B << " " << SEMI_M << " ";
    std::set<int>::iterator ii;
    std::set<int>& DFset = df->getDF(nL);
    for (ii=DFset.begin(); ii != DFset.end(); ii++)
        actual << std::hex << df->nodeToBB(*ii)->getLowAddr() << " ";
    QCOMPARE(expected.str(), actual.str());
    delete pFE;
}

/***************************************************************************//**
 * FUNCTION:        CfgTest::testPlacePhi
 * OVERVIEW:        Test the placing of phi functions
 *============================================================================*/
void CfgTest::testPlacePhi() {
    BinaryFileFactory bff;
    QObject* pBF = bff.Load(baseDir.absoluteFilePath(FRONTIER_PENTIUM));
    QVERIFY(pBF != 0);
    Prog* prog = new Prog;
    FrontEnd* pFE = new PentiumFrontEnd(pBF, prog, &bff);
    Type::clearNamedTypes();
    prog->setFrontEnd(pFE);
    pFE->decode(prog);

    UserProc* pProc = (UserProc*) prog->getProc(0);
    Cfg* cfg = pProc->getCFG();

    // Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
    prog->finishDecode();
    DataFlow* df = pProc->getDataFlow();
    df->dominators(cfg);
    df->placePhiFunctions(pProc);

    // m[r29 - 8] (x for this program)
    Exp* e = new Unary(opMemOf,
        new Binary(opMinus,
            Location::regOf(29),
            new Const(4)));
    //pProc->dump();
    cfg->dump();
    df->dumpA_phi();
    df->dumpA_orig();
    df->dumpDefsites();
    // A_phi[x] should be the set {7 8 10 15 20 21} (all the join points)
    std::ostringstream ost;
    std::set<int>::iterator ii;
    std::set<int>& A_phi = df->getA_phi(e);
    for (ii = A_phi.begin(); ii != A_phi.end(); ++ii)
        ost << *ii << " ";
    std::string expected("7 8 10 15 20 21 ");
    QCOMPARE(expected, ost.str());
    delete pFE;
}

/***************************************************************************//**
 * FUNCTION:        CfgTest::testPlacePhi2
 * OVERVIEW:        Test a case where a phi function is not needed
 *============================================================================*/
void CfgTest::testPlacePhi2() {
    BinaryFileFactory bff;
    QObject* pBF = bff.Load(baseDir.absoluteFilePath(IFTHEN_PENTIUM));
    QVERIFY(pBF != 0);
    Prog* prog = new Prog;
    FrontEnd* pFE = new PentiumFrontEnd(pBF, prog, &bff);
    Type::clearNamedTypes();
    prog->setFrontEnd(pFE);
    pFE->decode(prog);

    UserProc* pProc = (UserProc*) prog->getProc(0);
    Cfg* cfg = pProc->getCFG();
    DataFlow* df = pProc->getDataFlow();

    // Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
    prog->finishDecode();

    df->dominators(cfg);
    df->placePhiFunctions(pProc);

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
    std::set<int>& s = df->getA_phi(e);
    std::set<int>::iterator pp;
    for (pp = s.begin(); pp != s.end(); pp++)
        actual << *pp << " ";
    QCOMPARE(expected, actual.str());
    delete e;

    expected = "";
    std::ostringstream actual2;
    // m[r29 - 12]
    e = new Unary(opMemOf,
        new Binary(opMinus,
            Location::regOf(29),
            new Const(12)));

    std::set<int>& s2 = df->getA_phi(e);
    for (pp = s2.begin(); pp != s2.end(); pp++)
        actual2 << *pp << " ";
    QCOMPARE(expected, actual2.str());
    delete e;
    delete pFE;
}

/***************************************************************************//**
 * FUNCTION:        CfgTest::testRenameVars
 * OVERVIEW:        Test the renaming of variables
 *============================================================================*/
void CfgTest::testRenameVars() {
    BinaryFileFactory bff;
    QObject* pBF = bff.Load(baseDir.absoluteFilePath(FRONTIER_PENTIUM));
    QVERIFY(pBF != 0);
    Prog* prog = new Prog;
    FrontEnd* pFE = new PentiumFrontEnd(pBF, prog, &bff);
    Type::clearNamedTypes();
    prog->setFrontEnd(pFE);
    pFE->decode(prog);

    UserProc* pProc = (UserProc*) prog->getProc(0);
    Cfg* cfg = pProc->getCFG();
    DataFlow* df = pProc->getDataFlow();

    // Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
    prog->finishDecode();

    df->dominators(cfg);
    df->placePhiFunctions(pProc);
    pProc->numberStatements();                // After placing phi functions!
    df->renameBlockVars(pProc, 0, 1);        // Block 0, mem depth 1

    // MIKE: something missing here?

    delete pFE;
}
QTEST_MAIN(CfgTest)

