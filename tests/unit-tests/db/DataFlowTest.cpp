#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DataFlowTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/core/Project.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/DataFlow.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/RTL.h"
#include "boomerang/frontend/pentium/pentiumfrontend.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/util/Log.h"

#include <QDebug>


#define FRONTIER_PENTIUM    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/frontier"))
#define SEMI_PENTIUM        (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/semi"))
#define IFTHEN_PENTIUM      (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/ifthen"))


std::unique_ptr<RTLList> createRTLs(Address baseAddr, int numRTLs)
{
    std::unique_ptr<RTLList> rtls(new RTLList);

    for (int i = 0; i < numRTLs; i++) {
        rtls->push_back(std::unique_ptr<RTL>(new RTL(baseAddr + i,
            { new Assign(VoidType::get(), Terminal::get(opNil), Terminal::get(opNil)) })));
    }

    return rtls;
}


void DataFlowTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
}


void DataFlowTest::testCalculateDominators()
{
    // Appel, Figure 19.8
    UserProc proc(Address(0x1000), "test", nullptr);
    Cfg *cfg = proc.getCFG();
    DataFlow *df = proc.getDataFlow();

    BasicBlock *a = cfg->createBB(BBType::Twoway, createRTLs(Address(0x1000), 1));
    BasicBlock *b = cfg->createBB(BBType::Twoway, createRTLs(Address(0x1001), 1));
    BasicBlock *c = cfg->createBB(BBType::Twoway, createRTLs(Address(0x1002), 1));
    BasicBlock *d = cfg->createBB(BBType::Twoway, createRTLs(Address(0x1003), 1));
    BasicBlock *e = cfg->createBB(BBType::Twoway, createRTLs(Address(0x1004), 1));
    BasicBlock *f = cfg->createBB(BBType::Twoway, createRTLs(Address(0x1005), 1));
    BasicBlock *g = cfg->createBB(BBType::Oneway, createRTLs(Address(0x1006), 1));
    BasicBlock *h = cfg->createBB(BBType::Oneway, createRTLs(Address(0x1007), 1));
    BasicBlock *i = cfg->createBB(BBType::Oneway, createRTLs(Address(0x1008), 1));
    BasicBlock *j = cfg->createBB(BBType::Oneway, createRTLs(Address(0x1009), 1));
    BasicBlock *k = cfg->createBB(BBType::Oneway, createRTLs(Address(0x100A), 1));
    BasicBlock *l = cfg->createBB(BBType::Twoway, createRTLs(Address(0x100B), 1));
    BasicBlock *m = cfg->createBB(BBType::Ret,    createRTLs(Address(0x100C), 1));

    cfg->addEdge(a, b); cfg->addEdge(a, c);
    cfg->addEdge(b, d); cfg->addEdge(b, g);
    cfg->addEdge(c, e); cfg->addEdge(c, h);
    cfg->addEdge(d, f); cfg->addEdge(d, g);
    cfg->addEdge(e, h);
    cfg->addEdge(f, i); cfg->addEdge(f, k);
    cfg->addEdge(g, j);
    cfg->addEdge(h, m);
    cfg->addEdge(i, l);
    cfg->addEdge(j, i);
    cfg->addEdge(k, l);
    cfg->addEdge(l, b); cfg->addEdge(l, m);

    proc.setEntryBB();

    // test!
    df->calculateDominators();

    QCOMPARE(df->getSemiDominator(b), a); QCOMPARE(df->getDominator(b), a);
    QCOMPARE(df->getSemiDominator(c), a); QCOMPARE(df->getDominator(c), a);
    QCOMPARE(df->getSemiDominator(d), b); QCOMPARE(df->getDominator(d), b);
    QCOMPARE(df->getSemiDominator(e), c); QCOMPARE(df->getDominator(e), c);
    QCOMPARE(df->getSemiDominator(f), d); QCOMPARE(df->getDominator(f), d);
    QCOMPARE(df->getSemiDominator(g), b); QCOMPARE(df->getDominator(g), b);
    QCOMPARE(df->getSemiDominator(h), c); QCOMPARE(df->getDominator(h), c);
    QCOMPARE(df->getSemiDominator(i), b); QCOMPARE(df->getDominator(i), b);
    QCOMPARE(df->getSemiDominator(j), g); QCOMPARE(df->getDominator(j), g);
    QCOMPARE(df->getSemiDominator(k), f); QCOMPARE(df->getDominator(k), f);
    QCOMPARE(df->getSemiDominator(l), f); QCOMPARE(df->getDominator(l), b); // semidom != dom
    QCOMPARE(df->getSemiDominator(m), a); QCOMPARE(df->getDominator(m), a);

    QCOMPARE(df->getDominanceFrontier(a), std::set<const BasicBlock *>({         }));
    QCOMPARE(df->getDominanceFrontier(b), std::set<const BasicBlock *>({ b, m    }));
    QCOMPARE(df->getDominanceFrontier(c), std::set<const BasicBlock *>({ m       }));
    QCOMPARE(df->getDominanceFrontier(d), std::set<const BasicBlock *>({ g, i, l }));
    QCOMPARE(df->getDominanceFrontier(e), std::set<const BasicBlock *>({ h       }));
    QCOMPARE(df->getDominanceFrontier(f), std::set<const BasicBlock *>({ i, l    }));
    QCOMPARE(df->getDominanceFrontier(g), std::set<const BasicBlock *>({ i       }));
    QCOMPARE(df->getDominanceFrontier(h), std::set<const BasicBlock *>({ m       }));
    QCOMPARE(df->getDominanceFrontier(i), std::set<const BasicBlock *>({ l       }));
    QCOMPARE(df->getDominanceFrontier(j), std::set<const BasicBlock *>({ i       }));
    QCOMPARE(df->getDominanceFrontier(k), std::set<const BasicBlock *>({ l       }));
    QCOMPARE(df->getDominanceFrontier(l), std::set<const BasicBlock *>({ b, m    }));
    QCOMPARE(df->getDominanceFrontier(m), std::set<const BasicBlock *>({         }));
}


void DataFlowTest::testPlacePhi()
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

    const auto& m = *prog.getModuleList().begin();
    QVERIFY(m != nullptr);
    QVERIFY(m->size() > 0);

    UserProc *mainProc = (UserProc *)(*m->begin());
    QCOMPARE(mainProc->getName(), QString("main"));

    // Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
    prog.finishDecode();

    DataFlow *df = mainProc->getDataFlow();
    df->calculateDominators();

    // test!
    QCOMPARE(df->placePhiFunctions(), true);

    // r29 == ebp
    // m[r29 - 4] (x for this program)
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


void DataFlowTest::testPlacePhi2()
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

    const auto& m = *prog.getModuleList().begin();
    QVERIFY(m != nullptr);
    QVERIFY(m->size() > 0);

    UserProc *pProc = (UserProc *)(*m->begin());

    // Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
    prog.finishDecode();

    DataFlow *df = pProc->getDataFlow();
    df->calculateDominators();
    df->placePhiFunctions();

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


void DataFlowTest::testRenameVars()
{
    IProject& project = *Boomerang::get()->getOrCreateProject();

    project.loadBinaryFile(FRONTIER_PENTIUM);
    IFileLoader *loader = project.getBestLoader(FRONTIER_PENTIUM);
    QVERIFY(loader != nullptr);

    Prog      *prog = new Prog("FRONTIER_PENTIUM");
    IFrontEnd *pFE  = new PentiumFrontEnd(loader, prog);
    Type::clearNamedTypes();
    prog->setFrontEnd(pFE);
    pFE->decode(prog);

    const auto& m = *prog->getModuleList().begin();
    QVERIFY(m != nullptr);
    QVERIFY(m->size() > 0);

    UserProc *pProc = (UserProc *)(*m->begin());
    DataFlow *df    = pProc->getDataFlow();

    // Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
    prog->finishDecode();

    df->calculateDominators();
    df->placePhiFunctions();
    pProc->numberStatements();        // After placing phi functions!
    df->renameBlockVars(0, 1); // Block 0, mem depth 1

    // MIKE: something missing here?

    delete prog;
}


QTEST_MAIN(DataFlowTest)
