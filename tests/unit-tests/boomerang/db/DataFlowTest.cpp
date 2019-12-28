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

#include "boomerang-plugins/frontend/x86/X86FrontEnd.h"

#include "boomerang/core/Settings.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/DataFlow.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/LowLevelCFG.h"
#include "boomerang/db/module/Module.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/util/log/Log.h"

#include <QDebug>


#define FRONTIER_X86    (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/x86/frontier"))
#define SEMI_X86        (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/x86/semi"))
#define IFTHEN_X86      (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/x86/ifthen"))


// helper function for testCalculateDominatorsComplex()
IRFragment *createBBAndFragment(LowLevelCFG *cfg, BBType bbType, Address addr, UserProc *proc)
{
    BasicBlock *bb = cfg->createBB(bbType, createInsns(addr, 1));
    bb->setProc(proc);
    return proc->getCFG()->createFragment((FragType)bbType, createRTLs(addr, 1, 1), bb);
}


void DataFlowTest::testCalculateDominators1()
{
    Prog prog("test", nullptr);
    UserProc proc(Address(0x1000), "test", nullptr);
    ProcCFG *cfg = proc.getCFG();
    DataFlow *df = proc.getDataFlow();

    BasicBlock *bb = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1000), 1));
    IRFragment *entry = cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1000), 1, 1), bb);

    proc.setEntryFragment();

    QVERIFY(df->calculateDominators());
    QCOMPARE(df->getSemiDominator(entry), entry);

    QCOMPARE(df->getDominanceFrontier(entry), std::set<const IRFragment *>({}));
}


void DataFlowTest::testCalculateDominators2()
{
    Prog prog("test", nullptr);
    UserProc proc(Address(0x1000), "test", nullptr);
    ProcCFG *cfg = proc.getCFG();
    DataFlow *df = proc.getDataFlow();

    BasicBlock *entryBB = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1000), 1));
    IRFragment *entry   = cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1000), 1, 1), entryBB);
    BasicBlock *exitBB  = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1001), 1));
    IRFragment *exit    = cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1001), 1, 1), exitBB);

    cfg->addEdge(entry, exit);
    proc.setEntryFragment();

    QVERIFY(df->calculateDominators());
    QCOMPARE(df->getSemiDominator(entry), entry);
    QCOMPARE(df->getSemiDominator(exit), entry);

    QCOMPARE(df->getDominanceFrontier(entry), std::set<const IRFragment *>({}));
    QCOMPARE(df->getDominanceFrontier(exit), std::set<const IRFragment *>({}));
}


void DataFlowTest::testCalculateDominatorsComplex()
{
    Prog prog("test", nullptr);

    // Appel, Figure 19.8
    LowLevelCFG *cfg = prog.getCFG();
    UserProc proc(Address(0x1000), "test", nullptr);

    IRFragment *a = createBBAndFragment(cfg, BBType::Twoway, Address(0x1000), &proc);
    IRFragment *b = createBBAndFragment(cfg, BBType::Twoway, Address(0x1001), &proc);
    IRFragment *c = createBBAndFragment(cfg, BBType::Twoway, Address(0x1002), &proc);
    IRFragment *d = createBBAndFragment(cfg, BBType::Twoway, Address(0x1003), &proc);
    IRFragment *e = createBBAndFragment(cfg, BBType::Twoway, Address(0x1004), &proc);
    IRFragment *f = createBBAndFragment(cfg, BBType::Twoway, Address(0x1005), &proc);
    IRFragment *g = createBBAndFragment(cfg, BBType::Oneway, Address(0x1006), &proc);
    IRFragment *h = createBBAndFragment(cfg, BBType::Oneway, Address(0x1007), &proc);
    IRFragment *i = createBBAndFragment(cfg, BBType::Oneway, Address(0x1008), &proc);
    IRFragment *j = createBBAndFragment(cfg, BBType::Oneway, Address(0x1009), &proc);
    IRFragment *k = createBBAndFragment(cfg, BBType::Oneway, Address(0x100A), &proc);
    IRFragment *l = createBBAndFragment(cfg, BBType::Twoway, Address(0x100B), &proc);
    IRFragment *m = createBBAndFragment(cfg, BBType::Ret,    Address(0x100C), &proc);

    ProcCFG *procCFG = proc.getCFG();

    procCFG->addEdge(a, b); procCFG->addEdge(a, c);
    procCFG->addEdge(b, d); procCFG->addEdge(b, g);
    procCFG->addEdge(c, e); procCFG->addEdge(c, h);
    procCFG->addEdge(d, f); procCFG->addEdge(d, g);
    procCFG->addEdge(e, h);
    procCFG->addEdge(f, i); procCFG->addEdge(f, k);
    procCFG->addEdge(g, j);
    procCFG->addEdge(h, m);
    procCFG->addEdge(i, l);
    procCFG->addEdge(j, i);
    procCFG->addEdge(k, l);
    procCFG->addEdge(l, b); procCFG->addEdge(l, m);

    proc.setEntryFragment();

    DataFlow *df = proc.getDataFlow();

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

    QCOMPARE(df->getDominanceFrontier(a), std::set<const IRFragment *>({         }));
    QCOMPARE(df->getDominanceFrontier(b), std::set<const IRFragment *>({ b, m    }));
    QCOMPARE(df->getDominanceFrontier(c), std::set<const IRFragment *>({ m       }));
    QCOMPARE(df->getDominanceFrontier(d), std::set<const IRFragment *>({ g, i, l }));
    QCOMPARE(df->getDominanceFrontier(e), std::set<const IRFragment *>({ h       }));
    QCOMPARE(df->getDominanceFrontier(f), std::set<const IRFragment *>({ i, l    }));
    QCOMPARE(df->getDominanceFrontier(g), std::set<const IRFragment *>({ i       }));
    QCOMPARE(df->getDominanceFrontier(h), std::set<const IRFragment *>({ m       }));
    QCOMPARE(df->getDominanceFrontier(i), std::set<const IRFragment *>({ l       }));
    QCOMPARE(df->getDominanceFrontier(j), std::set<const IRFragment *>({ i       }));
    QCOMPARE(df->getDominanceFrontier(k), std::set<const IRFragment *>({ l       }));
    QCOMPARE(df->getDominanceFrontier(l), std::set<const IRFragment *>({ b, m    }));
    QCOMPARE(df->getDominanceFrontier(m), std::set<const IRFragment *>({         }));
}


void DataFlowTest::testPlacePhi()
{
    QVERIFY(m_project.loadBinaryFile(FRONTIER_X86));
    QVERIFY(m_project.decodeBinaryFile());

    Prog *prog = m_project.getProg();
    Type::clearNamedTypes();

    const auto& module = *prog->getModuleList().begin();
    QVERIFY(module != nullptr);
    QVERIFY(module->size() > 0);

    UserProc *mainProc = static_cast<UserProc *>(*module->begin());
    QCOMPARE(mainProc->getName(), QString("main"));

    PassManager::get()->executePass(PassID::StatementInit, mainProc);

    DataFlow *df = mainProc->getDataFlow();
    QVERIFY(df->calculateDominators());

    // test!
    QVERIFY(df->placePhiFunctions());

    SharedExp e = Location::regOf(REG_X86_EAX);
    QString     actualStr;
    OStream actual(&actualStr);

    // r24 == eax
    std::set<FragIndex>& A_phi = df->getA_phi(Location::regOf(REG_X86_EAX));

    for (FragIndex bb : A_phi) {
        actual << (int)bb << " ";
    }

    QCOMPARE(actualStr, QString("6 7 10 14 15 "));
}


void DataFlowTest::testPlacePhi2()
{
    QVERIFY(m_project.loadBinaryFile(IFTHEN_X86));
    QVERIFY(m_project.decodeBinaryFile());

    Prog *prog = m_project.getProg();
    Type::clearNamedTypes();

    const Module *m = (*prog->getModuleList().begin()).get();
    QVERIFY(m != nullptr);
    QVERIFY(m->size() > 0);

    Function *mainFunction = *m->begin();
    UserProc *proc = static_cast<UserProc *>(mainFunction);

    PassManager::get()->executePass(PassID::StatementInit, proc);

    DataFlow *df = proc->getDataFlow();
    QVERIFY(df->calculateDominators());
    QVERIFY(df->placePhiFunctions());
    proc->numberStatements(); // After placing phi functions!

    QString     actual_st;
    OStream actual(&actual_st);
    SharedExp            e = Location::regOf(REG_X86_EAX);
    std::set<FragIndex>& s = df->getA_phi(e);

    for (auto pp = s.begin(); pp != s.end(); ++pp) {
        actual << (uint64)*pp << " ";
    }

    QCOMPARE(actual_st, QString("4 "));
}


void DataFlowTest::testRenameVars()
{
    QVERIFY(m_project.loadBinaryFile(FRONTIER_X86));

    Prog *prog = m_project.getProg();
    IFrontEnd *fe  = prog->getFrontEnd();
    assert(fe != nullptr);

    Type::clearNamedTypes();
    QVERIFY(fe->disassembleEntryPoints());

    const auto& m = *prog->getModuleList().begin();
    QVERIFY(m != nullptr);
    QVERIFY(m->size() > 0);

    UserProc *proc = static_cast<UserProc *>(*m->begin());
    DataFlow *df    = proc->getDataFlow();

    PassManager::get()->executePass(PassID::StatementInit, proc);

    QVERIFY(df->calculateDominators());
    QVERIFY(df->placePhiFunctions());
    proc->numberStatements(); // After placing phi functions!

    QCOMPARE(PassManager::get()->executePass(PassID::BlockVarRename, proc), true);
    QCOMPARE(PassManager::get()->executePass(PassID::BlockVarRename, proc), false);
}


QTEST_GUILESS_MAIN(DataFlowTest)
