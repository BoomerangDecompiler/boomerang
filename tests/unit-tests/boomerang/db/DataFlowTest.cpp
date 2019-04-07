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

#include "boomerang-plugins/frontend/x86/PentiumFrontEnd.h"

#include "boomerang/core/Settings.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/DataFlow.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/module/Module.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/util/log/Log.h"

#include <QDebug>


#define FRONTIER_PENTIUM    (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/frontier"))
#define SEMI_PENTIUM        (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/semi"))
#define IFTHEN_PENTIUM      (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/ifthen"))


std::unique_ptr<RTLList> createRTLs(Address baseAddr, int numRTLs)
{
    std::unique_ptr<RTLList> rtls(new RTLList);

    for (int i = 0; i < numRTLs; i++) {
        rtls->push_back(std::unique_ptr<RTL>(new RTL(baseAddr + i,
            { new Assign(VoidType::get(), Terminal::get(opNil), Terminal::get(opNil)) })));
    }

    return rtls;
}


void DataFlowTest::testCalculateDominators1()
{
    UserProc proc(Address(0x1000), "test", nullptr);
    ProcCFG *cfg = proc.getCFG();
    DataFlow *df = proc.getDataFlow();

    BasicBlock *entry = cfg->createBB(BBType::Ret, createRTLs(Address(0x1000), 1));

    proc.setEntryBB();

    QVERIFY(df->calculateDominators());
    QCOMPARE(df->getSemiDominator(entry), entry);

    QCOMPARE(df->getDominanceFrontier(entry), std::set<const BasicBlock *>({}));
}


void DataFlowTest::testCalculateDominators2()
{
    UserProc proc(Address(0x1000), "test", nullptr);
    ProcCFG *cfg = proc.getCFG();
    DataFlow *df = proc.getDataFlow();

    BasicBlock *entry = cfg->createBB(BBType::Call, createRTLs(Address(0x1000), 1));
    BasicBlock *exit  = cfg->createBB(BBType::Ret, createRTLs(Address(0x1001), 1));

    cfg->addEdge(entry, exit);
    proc.setEntryBB();

    QVERIFY(df->calculateDominators());
    QCOMPARE(df->getSemiDominator(entry), entry);
    QCOMPARE(df->getSemiDominator(exit), entry);

    QCOMPARE(df->getDominanceFrontier(entry), std::set<const BasicBlock *>({}));
    QCOMPARE(df->getDominanceFrontier(exit), std::set<const BasicBlock *>({}));
}


void DataFlowTest::testCalculateDominatorsComplex()
{
    // Appel, Figure 19.8
    UserProc proc(Address(0x1000), "test", nullptr);
    ProcCFG *cfg = proc.getCFG();
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
    QVERIFY(df->calculateDominators());

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
    QVERIFY(m_project.loadBinaryFile(FRONTIER_PENTIUM));
    QVERIFY(m_project.decodeBinaryFile());

    Prog *prog = m_project.getProg();
    Type::clearNamedTypes();

    const auto& module = *prog->getModuleList().begin();
    QVERIFY(module != nullptr);
    QVERIFY(module->size() > 0);

    UserProc *mainProc = static_cast<UserProc *>(*module->begin());
    QCOMPARE(mainProc->getName(), QString("main"));

    DataFlow *df = mainProc->getDataFlow();
    df->calculateDominators();

    // test!
    QCOMPARE(df->placePhiFunctions(), true);

    SharedExp e = Location::regOf(REG_PENT_EAX);
    QString     actualStr;
    OStream actual(&actualStr);

    // r24 == eax
    std::set<BBIndex>& A_phi = df->getA_phi(Location::regOf(REG_PENT_EAX));

    for (BBIndex bb : A_phi) {
        actual << (int)bb << " ";
    }

    QCOMPARE(actualStr, QString("8 10 15 20 21 "));
}


void DataFlowTest::testPlacePhi2()
{
    QVERIFY(m_project.loadBinaryFile(IFTHEN_PENTIUM));
    QVERIFY(m_project.decodeBinaryFile());

    Prog *prog = m_project.getProg();
    Type::clearNamedTypes();

    const Module *m = (*prog->getModuleList().begin()).get();
    QVERIFY(m != nullptr);
    QVERIFY(m->size() > 0);

    Function *mainFunction = *m->begin();
    UserProc *proc = static_cast<UserProc *>(mainFunction);

    DataFlow *df = proc->getDataFlow();
    df->calculateDominators();
    QVERIFY(df->placePhiFunctions());

    QString     actual_st;
    OStream actual(&actual_st);
    SharedExp          e = Location::regOf(REG_PENT_EAX);
    std::set<BBIndex>& s = df->getA_phi(e);

    for (auto pp = s.begin(); pp != s.end(); ++pp) {
        actual << *pp << " ";
    }

    QCOMPARE(actual_st, QString("4 "));
}


void DataFlowTest::testRenameVars()
{
    QVERIFY(m_project.loadBinaryFile(FRONTIER_PENTIUM));

    Prog *prog = m_project.getProg();
    IFrontEnd *fe  = prog->getFrontEnd();
    Type::clearNamedTypes();
    fe->decodeEntryPointsRecursive();

    const auto& m = *prog->getModuleList().begin();
    QVERIFY(m != nullptr);
    QVERIFY(m->size() > 0);

    UserProc *proc = static_cast<UserProc *>(*m->begin());
    DataFlow *df    = proc->getDataFlow();

    df->calculateDominators();
    QVERIFY(df->placePhiFunctions());
    proc->numberStatements(); // After placing phi functions!

    QCOMPARE(PassManager::get()->executePass(PassID::BlockVarRename, proc), true);
    QCOMPARE(PassManager::get()->executePass(PassID::BlockVarRename, proc), false);
}


QTEST_GUILESS_MAIN(DataFlowTest)
