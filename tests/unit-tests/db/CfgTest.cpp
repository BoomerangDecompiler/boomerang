#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CfgTest.h"


/**
 * \file CfgTest.cpp
 * Provides the implementation for the CfgTest class, which
 * tests Control Flow Graphs
 */
#include "boomerang/core/Boomerang.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/DataFlow.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/core/Project.h"
#include "boomerang/util/Log.h"

#include "boomerang/frontend/pentium/pentiumfrontend.h"

#include <QDebug>

#define FRONTIER_PENTIUM    (BOOMERANG_TEST_BASE "/tests/inputs/pentium/frontier")
#define SEMI_PENTIUM        (BOOMERANG_TEST_BASE "/tests/inputs/pentium/semi")
#define IFTHEN_PENTIUM      (BOOMERANG_TEST_BASE "/tests/inputs/pentium/ifthen")

void CfgTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
}


#define FRONTIER_FOUR        Address(0x08048347)
#define FRONTIER_FIVE        Address(0x08048351)
#define FRONTIER_TWELVE      Address(0x080483b2)
#define FRONTIER_THIRTEEN    Address(0x080483b9)

void CfgTest::testDominators()
{
    IProject& project = *Boomerang::get()->getOrCreateProject();

    project.loadBinaryFile(FRONTIER_PENTIUM);
    IFileLoader *loader = project.getBestLoader(FRONTIER_PENTIUM);
    QVERIFY(loader != nullptr);

    Prog      prog(FRONTIER_PENTIUM);
    IFrontEnd *pFE = new PentiumFrontEnd(loader, &prog);
    Type::clearNamedTypes();
    prog.setFrontEnd(pFE);
    pFE->decode(&prog);

    bool    gotMain;
    Address addr = pFE->getMainEntryPoint(gotMain);
    QVERIFY(addr != Address::INVALID);

    Module *m = *(prog.getModuleList().begin());
    QVERIFY(m != nullptr);
    QVERIFY(m->size() > 0);

    UserProc *pProc = (UserProc *)*(m->begin());
    Cfg      *cfg   = pProc->getCFG();
    DataFlow *df    = pProc->getDataFlow();
    df->dominators(cfg);

    // Find BB "5" (as per Appel, Figure 19.5).
    BBIterator it;
    BasicBlock *bb = cfg->getFirstBB(it);

    while (bb && bb->getLowAddr() != FRONTIER_FIVE) {
        bb = cfg->getNextBB(it);
    }

    QVERIFY(bb);
    QString     actual_st;
    QTextStream actual(&actual_st);

    int n5 = df->pbbToNode(bb);
    std::set<int>::iterator ii;
    std::set<int>&          DFset = df->getDF(n5);

    for (ii = DFset.begin(); ii != DFset.end(); ii++) {
        actual << df->nodeToBB(*ii)->getLowAddr() << " ";
    }

    QCOMPARE(actual_st,
             FRONTIER_THIRTEEN.toString() + " " +
             FRONTIER_FOUR.toString() + " " +
             FRONTIER_TWELVE.toString() + " " +
             FRONTIER_FIVE.toString() + " "
             );
}


#define SEMI_L    Address(0x80483b0)
#define SEMI_M    Address(0x80483e2)
#define SEMI_B    Address(0x8048345)
#define SEMI_D    Address(0x8048354)
#define SEMI_M    Address(0x80483e2)

void CfgTest::testSemiDominators()
{
    IProject& project = *Boomerang::get()->getOrCreateProject();

    project.loadBinaryFile(SEMI_PENTIUM);
    IFileLoader *loader = project.getBestLoader(SEMI_PENTIUM);
    QVERIFY(loader != nullptr);

    Prog      prog(SEMI_PENTIUM);
    IFrontEnd *pFE = new PentiumFrontEnd(loader, &prog);
    Type::clearNamedTypes();
    prog.setFrontEnd(pFE);
    pFE->decode(&prog);

    bool    gotMain;
    Address addr = pFE->getMainEntryPoint(gotMain);
    QVERIFY(addr != Address::INVALID);

    Module *m = *prog.getModuleList().begin();
    QVERIFY(m != nullptr);
    QVERIFY(m->size() > 0);

    UserProc *pProc = (UserProc *)(*m->begin());
    Cfg      *cfg   = pProc->getCFG();

    DataFlow *df = pProc->getDataFlow();
    df->dominators(cfg);

    // Find BB "L (6)" (as per Appel, Figure 19.8).
    BBIterator it;
    BasicBlock *bb = cfg->getFirstBB(it);

    while (bb && bb->getLowAddr() != SEMI_L) {
        bb = cfg->getNextBB(it);
    }

    QVERIFY(bb);
    int nL = df->pbbToNode(bb);

    // The dominator for L should be B, where the semi dominator is D
    // (book says F)
    Address actual_dom  = df->nodeToBB(df->getIdom(nL))->getLowAddr();
    Address actual_semi = df->nodeToBB(df->getSemi(nL))->getLowAddr();
    QCOMPARE(actual_dom, SEMI_B);
    QCOMPARE(actual_semi, SEMI_D);

    // Check the final dominator frontier as well; should be M and B
    QString     actual_st;
    QTextStream actual(&actual_st);

    std::set<int>& DFset = df->getDF(nL);

    for (auto ii = DFset.begin(); ii != DFset.end(); ii++) {
        actual << df->nodeToBB(*ii)->getLowAddr() << " ";
    }

    QCOMPARE(actual_st,
             SEMI_B.toString() + " " +
             SEMI_M.toString() + " "
             );
}


void CfgTest::testPlacePhi()
{
    QSKIP("Disabled.");

    IProject& project = *Boomerang::get()->getOrCreateProject();
    project.loadBinaryFile(FRONTIER_PENTIUM);
    IFileLoader *loader = project.getBestLoader(FRONTIER_PENTIUM);
    QVERIFY(loader != nullptr);

    Prog      prog(FRONTIER_PENTIUM);
    IFrontEnd *pFE = new PentiumFrontEnd(loader, &prog);
    Type::clearNamedTypes();
    prog.setFrontEnd(pFE);
    pFE->decode(&prog);

    Module *m = *prog.getModuleList().begin();
    QVERIFY(m != nullptr);
    QVERIFY(m->size() > 0);

    UserProc *pProc = (UserProc *)(*m->begin());
    Cfg      *cfg   = pProc->getCFG();

    // Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
    cfg->sortByAddress();
    prog.finishDecode();
    DataFlow *df = pProc->getDataFlow();
    df->dominators(cfg);
    df->placePhiFunctions(pProc);

    // m[r29 - 8] (x for this program)
    SharedExp e = Unary::get(opMemOf, Binary::get(opMinus, Location::regOf(29), Const::get(4)));

    // A_phi[x] should be the set {7 8 10 15 20 21} (all the join points)
    QString     actual_st;
    QTextStream actual(&actual_st);

    std::set<int>& A_phi = df->getA_phi(e);

    for (std::set<int>::iterator ii = A_phi.begin(); ii != A_phi.end(); ++ii) {
        actual << *ii << " ";
    }

    QCOMPARE(actual_st, QString("7 8 10 15 20 21 "));
}


void CfgTest::testPlacePhi2()
{
    QSKIP("Disabled.");

    IProject& project = *Boomerang::get()->getOrCreateProject();
    project.loadBinaryFile(IFTHEN_PENTIUM);
    IFileLoader *loader = project.getBestLoader(IFTHEN_PENTIUM);

    QVERIFY(loader != nullptr);
    Prog      prog(IFTHEN_PENTIUM);
    IFrontEnd *pFE = new PentiumFrontEnd(loader, &prog);
    Type::clearNamedTypes();
    prog.setFrontEnd(pFE);
    pFE->decode(&prog);

    Module *m = *prog.getModuleList().begin();
    QVERIFY(m != nullptr);
    QVERIFY(m->size() > 0);

    UserProc *pProc = (UserProc *)(*m->begin());

    // Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
    prog.finishDecode();

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
     * Call (0)
     |
     | V
     | Call (1)
     |
     | V
     | Twoway (2) if (b < 4 )
     |
     |-T-> Fall (3)
     |      |
     |      V
     |-F-> Call (4) ----> Ret (5)
     */

    QString     actual_st;
    QTextStream actual(&actual_st);
    // m[r29 - 8]
    SharedExp               e = Unary::get(opMemOf, Binary::get(opMinus, Location::regOf(29), Const::get(8)));
    std::set<int>&          s = df->getA_phi(e);
    std::set<int>::iterator pp;

    for (pp = s.begin(); pp != s.end(); pp++) {
        actual << *pp << " ";
    }

    QCOMPARE(actual_st, QString("4 "));

    if (s.size() > 0) {
        BBType actualType   = df->nodeToBB(*s.begin())->getType();
        BBType expectedType = BBType::Call;
        QCOMPARE(actualType, expectedType);
    }

    QString     expected = "";
    QString     actual_st2;
    QTextStream actual2(&actual_st2);
    // m[r29 - 12]
    e = Unary::get(opMemOf, Binary::get(opMinus, Location::regOf(29), Const::get(12)));

    std::set<int>& s2 = df->getA_phi(e);

    for (pp = s2.begin(); pp != s2.end(); pp++) {
        actual2 << *pp << " ";
    }

    QCOMPARE(actual_st2, expected);
    delete pFE;
}


void CfgTest::testRenameVars()
{
    IProject& project = *Boomerang::get()->getOrCreateProject();

    project.loadBinaryFile(FRONTIER_PENTIUM);
    IFileLoader *loader = project.getBestLoader(FRONTIER_PENTIUM);
    QVERIFY(loader != nullptr);

    Prog      *prog = new Prog(FRONTIER_PENTIUM);
    IFrontEnd *pFE  = new PentiumFrontEnd(loader, prog);
    Type::clearNamedTypes();
    prog->setFrontEnd(pFE);
    pFE->decode(prog);

    Module *m = *prog->getModuleList().begin();
    QVERIFY(m != nullptr);
    QVERIFY(m->size() > 0);

    UserProc *pProc = (UserProc *)(*m->begin());
    Cfg      *cfg   = pProc->getCFG();
    DataFlow *df    = pProc->getDataFlow();

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
