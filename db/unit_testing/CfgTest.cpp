/***************************************************************************/ /**
  * \file       CfgTest.cc
  * OVERVIEW:   Provides the implementation for the CfgTest class, which
  *                tests the Exp and derived classes
  ******************************************************************************/

#include "CfgTest.h"

#include "BinaryFile.h"
#include "frontend.h"
#include "proc.h"
#include "prog.h"
#include "dataflow.h"
#include "pentiumfrontend.h"
#include "log.h"
#include "boomerang.h"
#include "basicblock.h"

#include <QDir>
#include <QProcessEnvironment>
#include <QDebug>

#define FRONTIER_PENTIUM baseDir.absoluteFilePath("tests/inputs/pentium/frontier")
#define SEMI_PENTIUM baseDir.absoluteFilePath("tests/inputs/pentium/semi")
#define IFTHEN_PENTIUM baseDir.absoluteFilePath("tests/inputs/pentium/ifthen")
static bool logset = false;
static QString TEST_BASE;
static QDir baseDir;
void CfgTest::initTestCase() {
    if (!logset) {
        TEST_BASE = QProcessEnvironment::systemEnvironment().value("BOOMERANG_TEST_BASE", "");
        baseDir = QDir(TEST_BASE);
        if (TEST_BASE.isEmpty()) {
            qWarning() << "BOOMERANG_TEST_BASE environment variable not set, will assume '..', many test may fail";
            TEST_BASE = "..";
            baseDir = QDir("..");
        }
        logset = true;
        Boomerang::get()->setProgPath(TEST_BASE);
        Boomerang::get()->setPluginPath(TEST_BASE + "/out");
        Boomerang::get()->setLogger(new NullLogger());
    }
}

    /***************************************************************************/ /**
      * \fn        CfgTest::testDominators
      * OVERVIEW:        Test the dominator frontier code
      ******************************************************************************/
#define FRONTIER_FOUR ADDRESS::n(0x08048347)
#define FRONTIER_FIVE ADDRESS::n(0x08048351)
#define FRONTIER_TWELVE ADDRESS::n(0x080483b2)
#define FRONTIER_THIRTEEN ADDRESS::n(0x080483b9)

void CfgTest::testDominators() {
    BinaryFileFactory bff;
    QObject *pBF = bff.Load(FRONTIER_PENTIUM);
    QVERIFY(pBF != 0);
    Prog *prog = new Prog(FRONTIER_PENTIUM);
    FrontEnd *pFE = new PentiumFrontEnd(pBF, prog, &bff);
    Type::clearNamedTypes();
    prog->setFrontEnd(pFE);
    pFE->decode(prog);

    bool gotMain;
    ADDRESS addr = pFE->getMainEntryPoint(gotMain);
    QVERIFY(addr != NO_ADDRESS);
    Module *m = *prog->begin();
    QVERIFY(m!=nullptr);
    QVERIFY(m->size()>0);

    UserProc *pProc = (UserProc *)*(m->begin());
    Cfg *cfg = pProc->getCFG();
    DataFlow *df = pProc->getDataFlow();
    df->dominators(cfg);

    // Find BB "5" (as per Appel, Figure 19.5).
    BB_IT it;
    BasicBlock *bb = cfg->getFirstBB(it);
    while (bb && bb->getLowAddr() != FRONTIER_FIVE) {
        bb = cfg->getNextBB(it);
    }
    QVERIFY(bb);
    QString expect_st,actual_st;
    QTextStream expected(&expect_st), actual(&actual_st);
    // expected << std::hex << FRONTIER_FIVE << " " << FRONTIER_THIRTEEN << " " << FRONTIER_TWELVE << " " <<
    //    FRONTIER_FOUR << " ";
    expected << FRONTIER_THIRTEEN << " " << FRONTIER_FOUR << " " << FRONTIER_TWELVE << " " << FRONTIER_FIVE
             << " ";
    int n5 = df->pbbToNode(bb);
    std::set<int>::iterator ii;
    std::set<int> &DFset = df->getDF(n5);
    for (ii = DFset.begin(); ii != DFset.end(); ii++)
        actual << df->nodeToBB(*ii)->getLowAddr() << " ";
    QCOMPARE(actual_st,expect_st);

    pBF->deleteLater();
    delete pFE;
}

    /***************************************************************************/ /**
      * \fn        CfgTest::testSemiDominators
      * OVERVIEW:        Test a case where semi dominators are different to dominators
      ******************************************************************************/
#define SEMI_L ADDRESS::g(0x80483b0)
#define SEMI_M ADDRESS::g(0x80483e2)
#define SEMI_B ADDRESS::g(0x8048345)
#define SEMI_D ADDRESS::g(0x8048354)
#define SEMI_M ADDRESS::g(0x80483e2)

void CfgTest::testSemiDominators() {
    BinaryFileFactory bff;
    QObject *pBF = bff.Load(SEMI_PENTIUM);
    QVERIFY(pBF != 0);
    Prog *prog = new Prog(SEMI_PENTIUM);
    FrontEnd *pFE = new PentiumFrontEnd(pBF, prog, &bff);
    Type::clearNamedTypes();
    prog->setFrontEnd(pFE);
    pFE->decode(prog);

    bool gotMain;
    ADDRESS addr = pFE->getMainEntryPoint(gotMain);
    QVERIFY(addr != NO_ADDRESS);

    Module *m = *prog->begin();
    QVERIFY(m!=nullptr);
    QVERIFY(m->size()>0);

    UserProc *pProc = (UserProc *)(*m->begin());
    Cfg *cfg = pProc->getCFG();

    DataFlow *df = pProc->getDataFlow();
    df->dominators(cfg);

    // Find BB "L (6)" (as per Appel, Figure 19.8).
    BB_IT it;
    BasicBlock *bb = cfg->getFirstBB(it);
    while (bb && bb->getLowAddr() != SEMI_L) {
        bb = cfg->getNextBB(it);
    }
    QVERIFY(bb);
    int nL = df->pbbToNode(bb);

    // The dominator for L should be B, where the semi dominator is D
    // (book says F)
    ADDRESS actual_dom = df->nodeToBB(df->getIdom(nL))->getLowAddr();
    ADDRESS actual_semi = df->nodeToBB(df->getSemi(nL))->getLowAddr();
    QCOMPARE(actual_dom,SEMI_B);
    QCOMPARE(actual_semi,SEMI_D);
    // Check the final dominator frontier as well; should be M and B
    QString expected_st,actual_st;
    QTextStream expected(&expected_st), actual(&actual_st);
    // expected << std::hex << SEMI_M << " " << SEMI_B << " ";
    expected << SEMI_B << " " << SEMI_M << " ";
    std::set<int>::iterator ii;
    std::set<int> &DFset = df->getDF(nL);
    for (ii = DFset.begin(); ii != DFset.end(); ii++)
        actual << df->nodeToBB(*ii)->getLowAddr() << " ";
    QCOMPARE(actual_st,expected_st);
    delete pFE;
}

/***************************************************************************/ /**
  * \fn        CfgTest::testPlacePhi
  * OVERVIEW:        Test the placing of phi functions
  ******************************************************************************/
void CfgTest::testPlacePhi() {
    BinaryFileFactory bff;
    QObject *pBF = bff.Load(FRONTIER_PENTIUM);
    QVERIFY(pBF != 0);
    Prog *prog = new Prog(FRONTIER_PENTIUM);
    FrontEnd *pFE = new PentiumFrontEnd(pBF, prog, &bff);
    Type::clearNamedTypes();
    prog->setFrontEnd(pFE);
    pFE->decode(prog);

    Module *m = *prog->begin();
    QVERIFY(m!=nullptr);
    QVERIFY(m->size()>0);

    UserProc *pProc = (UserProc *)(*m->begin());
    Cfg *cfg = pProc->getCFG();

    // Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
    cfg->sortByAddress();
    prog->finishDecode();
    DataFlow *df = pProc->getDataFlow();
    df->dominators(cfg);
    df->placePhiFunctions(pProc);

    // m[r29 - 8] (x for this program)
    Exp *e = new Unary(opMemOf, new Binary(opMinus, Location::regOf(29), new Const(4)));

    // A_phi[x] should be the set {7 8 10 15 20 21} (all the join points)
    QString actual_st;
    QTextStream actual(&actual_st);
    std::set<int>::iterator ii;
    std::set<int> &A_phi = df->getA_phi(e);
    for (ii = A_phi.begin(); ii != A_phi.end(); ++ii)
        actual << *ii << " ";
    QCOMPARE(actual_st,QString("7 8 10 15 20 21 "));
    delete pFE;
}

/***************************************************************************/ /**
  * \fn        CfgTest::testPlacePhi2
  * OVERVIEW:        Test a case where a phi function is not needed
  ******************************************************************************/
void CfgTest::testPlacePhi2() {
    BinaryFileFactory bff;
    QObject *pBF = bff.Load(IFTHEN_PENTIUM);
    QVERIFY(pBF != 0);
    Prog *prog = new Prog(IFTHEN_PENTIUM);
    FrontEnd *pFE = new PentiumFrontEnd(pBF, prog, &bff);
    Type::clearNamedTypes();
    prog->setFrontEnd(pFE);
    pFE->decode(prog);

    Module *m = *prog->begin();
    QVERIFY(m!=nullptr);
    QVERIFY(m->size()>0);

    UserProc *pProc = (UserProc *)(*m->begin());

    // Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
    prog->finishDecode();

    Cfg *cfg = pProc->getCFG();
    cfg->sortByAddress();

    DataFlow *df = pProc->getDataFlow();
    df->dominators(cfg);
    df->placePhiFunctions(pProc);

    // In this program, x is allocated at [ebp-4], a at [ebp-8], and
    // b at [ebp-12]
    // We check that A_phi[ m[ebp-8] ] is 4, and that
    // A_phi A_phi[ m[ebp-8] ] is null
    // (block 4 comes out with n=4)

    /*
     Call (0)
      |
      V
     Call (1)
      |
      V
   Twoway (2) if (b < 4 )
      |
      |-T-> Fall (3)
      |		 |
      |		 V
      |-F-> Call (4) ----> Ret (5)
    */

    QString expected = "4 ";
    QString actual_st;
    QTextStream actual(&actual_st);
    // m[r29 - 8]
    Exp *e = new Unary(opMemOf, new Binary(opMinus, Location::regOf(29), new Const(8)));
    std::set<int> &s = df->getA_phi(e);
    std::set<int>::iterator pp;
    for (pp = s.begin(); pp != s.end(); pp++)
        actual << *pp << " ";
    QCOMPARE(actual_st,expected);
    if (s.size() > 0)
    {
        BBTYPE actualType = df->nodeToBB(*s.begin())->getType();
        BBTYPE expectedType = BBTYPE::CALL;
        QCOMPARE(actualType, expectedType);
    }
    delete e;

    expected = "";
    QString actual_st2;
    QTextStream actual2(&actual_st2);
    // m[r29 - 12]
    e = new Unary(opMemOf, new Binary(opMinus, Location::regOf(29), new Const(12)));

    std::set<int> &s2 = df->getA_phi(e);
    for (pp = s2.begin(); pp != s2.end(); pp++)
        actual2 << *pp << " ";
    QCOMPARE(actual_st2,expected);
    delete e;
    delete pFE;
}

/***************************************************************************/ /**
  * \fn        CfgTest::testRenameVars
  * OVERVIEW:        Test the renaming of variables
  ******************************************************************************/
void CfgTest::testRenameVars() {
    BinaryFileFactory bff;
    QObject *pBF = bff.Load(FRONTIER_PENTIUM);
    QVERIFY(pBF != 0);
    Prog *prog = new Prog(FRONTIER_PENTIUM);
    FrontEnd *pFE = new PentiumFrontEnd(pBF, prog, &bff);
    Type::clearNamedTypes();
    prog->setFrontEnd(pFE);
    pFE->decode(prog);

    Module *m = *prog->begin();
    QVERIFY(m!=nullptr);
    QVERIFY(m->size()>0);

    UserProc *pProc = (UserProc *)(*m->begin());
    Cfg *cfg = pProc->getCFG();
    DataFlow *df = pProc->getDataFlow();

    // Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
    prog->finishDecode();

    df->dominators(cfg);
    df->placePhiFunctions(pProc);
    pProc->numberStatements();        // After placing phi functions!
    df->renameBlockVars(pProc, 0, 1); // Block 0, mem depth 1

    // MIKE: something missing here?

    delete pFE;
}
QTEST_MAIN(CfgTest)
