#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StatementTest.h"


#include "boomerang/core/Settings.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/module/Module.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/decomp/ProcDecompiler.h"
#include "boomerang/decomp/ProgDecompiler.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/BoolAssign.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/util/log/Log.h"


#include <sstream>
#include <map>


#define HELLO_PENTIUM      getFullSamplePath("pentium/hello")
#define GLOBAL1_PENTIUM    getFullSamplePath("pentium/global1")


void StatementTest::testEmpty()
{
    m_project.getSettings()->setOutputDirectory("./unit_test/");

    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));

    Prog *prog = m_project.getProg();

    const auto& m = *prog->getModuleList().begin();
    QVERIFY(m != nullptr);

    // create UserProc
    UserProc *proc = static_cast<UserProc *>(m->createFunction("test", Address(0x00000123)));

    // create CFG
    ProcCFG                    *cfg   = proc->getCFG();
    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x00000123), { })));

    BasicBlock *entryBB = cfg->createBB(BBType::Ret, std::move(bbRTLs));
    cfg->setEntryAndExitBB(entryBB);
    proc->setDecoded(); // We manually "decoded"

    // compute dataflow
    proc->decompileRecursive();

    // print cfg to a string
    QString     actual;
    OStream st(&actual);
    cfg->print(st);

    QString expected = QString(
            "Control Flow Graph:\n"
            "Ret BB:\n"
            "  in edges: \n"
            "  out edges: \n"
            "0x00000123\n\n"
        );

    QCOMPARE(actual, expected);
}


void StatementTest::testFlow()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));

    Prog *prog = m_project.getProg();

    // create UserProc
    UserProc    *proc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x00000123)));
    proc->setSignature(Signature::instantiate(Machine::PENTIUM, CallConv::C, "test"));

    ProcCFG *cfg   = proc->getCFG();

    Assign *a1 = new Assign(Location::regOf(REG_PENT_EAX), std::make_shared<Const>(5));
    a1->setProc(proc);
    a1->setNumber(1);

    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { a1 })));

    BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));

    ReturnStatement *rs = new ReturnStatement;
    rs->setNumber(2);
    Assign *a2 = new Assign(Location::regOf(REG_PENT_EAX), std::make_shared<Const>(5));
    a2->setProc(proc);
    rs->addReturn(a2);

    bbRTLs.reset(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1010), { rs })));

    BasicBlock *ret = cfg->createBB(BBType::Ret, std::move(bbRTLs));
    QVERIFY(ret);

    // first was empty before
    first->addSuccessor(ret);
    ret->addPredecessor(first);
    cfg->setEntryAndExitBB(first); // Also sets exitBB; important!
    proc->setDecoded();

    // compute dataflow
    proc->decompileRecursive();

    // print cfg to a string
    QString     actual;
    OStream st(&actual);

    proc->numberStatements();
    cfg->print(st);

    // The assignment to 5 gets propagated into the return, and the assignment
    // to r24 is removed
    QString expected =
        "Control Flow Graph:\n"
        "Fall BB:\n"
        "  in edges: \n"
        "  out edges: 0x00001010 \n"
        "0x00001000\n"
        "Ret BB:\n"
        "  in edges: 0x00001000(0x00001000) \n"
        "  out edges: \n"
        "0x00001010    1 RET *v* r24 := 5\n"
        "              Modifieds: <None>\n"
        "              Reaching definitions: r24=5\n"
        "\n";

    compareLongStrings(actual, expected);

    // clean up
    delete a1;
}


void StatementTest::testKill()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    Prog *prog = m_project.getProg();

    // create UserProc
    QString  name  = "test";
    UserProc *proc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x00000123)));
    proc->setSignature(Signature::instantiate(Machine::PENTIUM, CallConv::C, name));

    // create CFG
    ProcCFG              *cfg   = proc->getCFG();

    Assign *e1     = new Assign(Location::regOf(REG_PENT_EAX), Const::get(5));
    e1->setNumber(1);
    e1->setProc(proc);

    Assign *e2 = new Assign(Location::regOf(REG_PENT_EAX), Const::get(6));
    e2->setNumber(2);
    e2->setProc(proc);

    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { e1, e2 })));
    BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));

    ReturnStatement *rs = new ReturnStatement;
    rs->setNumber(3);

    Assign *e = new Assign(Location::regOf(REG_PENT_EAX), Const::get(0));
    e->setProc(proc);
    rs->addReturn(e);

    bbRTLs.reset(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1010), { rs })));

    BasicBlock *ret = cfg->createBB(BBType::Ret, std::move(bbRTLs));
    first->addSuccessor(ret);
    ret->addPredecessor(first);
    cfg->setEntryAndExitBB(first);
    proc->setDecoded();

    // compute dataflow
    proc->decompileRecursive();

    // print cfg to a string
    QString     actual;
    OStream st(&actual);

    proc->numberStatements();
    cfg->print(st);

    QString expected =
        "Control Flow Graph:\n"
        "Fall BB:\n"
        "  in edges: \n"
        "  out edges: 0x00001010 \n"
        "0x00001000\n"
        "Ret BB:\n"
        "  in edges: 0x00001000(0x00001000) \n"
        "  out edges: \n"
        "0x00001010    1 RET *v* r24 := 0\n"
        "              Modifieds: <None>\n"
        "              Reaching definitions: r24=6\n\n";

    compareLongStrings(actual, expected);

    // clean up
    delete e1;
    delete e2;
}


void StatementTest::testUse()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    Prog *prog = m_project.getProg();

    UserProc    *proc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x00000123)));
    proc->setSignature(Signature::instantiate(Machine::PENTIUM, CallConv::C, "test"));

    ProcCFG *cfg   = proc->getCFG();

    Assign *a1 = new Assign(Location::regOf(REG_PENT_EAX), Const::get(5));
    a1->setNumber(1);
    a1->setProc(proc);

    Assign *a2 = new Assign(Location::regOf(REG_PENT_ESP), Location::regOf(REG_PENT_EAX));
    a2->setNumber(2);
    a2->setProc(proc);

    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { a1, a2 })));
    BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));

    ReturnStatement *rs = new ReturnStatement;
    rs->setNumber(3);
    Assign *a = new Assign(Location::regOf(REG_PENT_ESP), Const::get(1000));
    a->setProc(proc);
    rs->addReturn(a);
    bbRTLs.reset(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1010), { rs })));

    BasicBlock *ret = cfg->createBB(BBType::Ret, std::move(bbRTLs));
    first->addSuccessor(ret);
    ret->addPredecessor(first);
    cfg->setEntryAndExitBB(first);
    proc->setDecoded();

    // compute dataflow
    proc->decompileRecursive();
    // print cfg to a string
    QString     actual;
    OStream st(&actual);

    proc->numberStatements();
    cfg->print(st);

    QString expected =
        "Control Flow Graph:\n"
        "Fall BB:\n"
        "  in edges: \n"
        "  out edges: 0x00001010 \n"
        "0x00001000\n"
        "Ret BB:\n"
        "  in edges: 0x00001000(0x00001000) \n"
        "  out edges: \n"
        "0x00001010    1 RET *v* r28 := 1000\n"
        "              Modifieds: <None>\n"
        "              Reaching definitions: r24=5,   r28=5\n\n";

    compareLongStrings(actual, expected);

    // clean up
    delete a1;
    delete a2;
}


void StatementTest::testUseOverKill()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    Prog *prog = m_project.getProg();

    UserProc *proc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x00000123)));
    proc->setSignature(Signature::instantiate(Machine::PENTIUM, CallConv::C, "test"));
    ProcCFG *cfg = proc->getCFG();

    Assign *e1 = new Assign(Location::regOf(REG_PENT_EAX), Const::get(5));
    e1->setNumber(1);
    e1->setProc(proc);

    Assign *e2 = new Assign(Location::regOf(REG_PENT_EAX), Const::get(6));
    e2->setNumber(2);
    e2->setProc(proc);

    Assign *e3 = new Assign(Location::regOf(REG_PENT_ESP), Location::regOf(REG_PENT_EAX));
    e3->setNumber(3);
    e3->setProc(proc);

    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { e1, e2, e3 })));
    BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));

    ReturnStatement *rs = new ReturnStatement;
    rs->setNumber(4);
    Assign *e = new Assign(Location::regOf(REG_PENT_EAX), Const::get(0));
    e->setProc(proc);
    rs->addReturn(e);

    bbRTLs.reset(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1010), { rs })));
    BasicBlock *ret = cfg->createBB(BBType::Ret, std::move(bbRTLs));

    first->addSuccessor(ret);
    ret->addPredecessor(first);
    cfg->setEntryAndExitBB(first);
    proc->setDecoded();

    // compute dataflow
    proc->decompileRecursive();

    // print cfg to a string
    QString     actual;
    OStream st(&actual);

    proc->numberStatements();
    cfg->print(st);

    // compare it to expected
    QString expected =
        "Control Flow Graph:\n"
        "Fall BB:\n"
        "  in edges: \n"
        "  out edges: 0x00001010 \n"
        "0x00001000\n"
        "Ret BB:\n"
        "  in edges: 0x00001000(0x00001000) \n"
        "  out edges: \n"
        "0x00001010    1 RET *v* r24 := 0\n"
        "              Modifieds: <None>\n"
        "              Reaching definitions: r24=6,   r28=6\n\n";

    compareLongStrings(actual, expected);

    // clean up
    delete e1;
    delete e2;
    delete e3;
}


void StatementTest::testUseOverBB()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    Prog *prog = m_project.getProg();

    // create UserProc
    UserProc *proc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x00001000)));
    ProcCFG *cfg       = proc->getCFG();

    Assign *a1 = new Assign(Location::regOf(REG_PENT_EAX), Const::get(5));
    a1->setNumber(1);
    a1->setProc(proc);

    Assign *a2 = new Assign(Location::regOf(REG_PENT_EAX), Const::get(6));
    a2->setNumber(2);
    a2->setProc(proc);

    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { a1, a2 })));
    BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));

    Assign *a3  = new Assign(Location::regOf(REG_PENT_ESP), Location::regOf(REG_PENT_EAX));
    a3->setNumber(3);
    a3->setProc(proc);
    bbRTLs.reset(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1010), { a3 })));


    ReturnStatement *rs = new ReturnStatement;
    rs->setNumber(4);

    Assign *a = new Assign(Location::regOf(REG_PENT_EAX), Const::get(0));
    a->setProc(proc);
    rs->addReturn(a);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x00001012), { rs })));
    BasicBlock *ret = cfg->createBB(BBType::Ret, std::move(bbRTLs));

    first->addSuccessor(ret);
    ret->addPredecessor(first);
    cfg->setEntryAndExitBB(first);
    proc->setDecoded();

    // compute dataflow
    proc->decompileRecursive();

    // print cfg to a string
    QString     actual;
    OStream st(&actual);

    proc->numberStatements();
    cfg->print(st);

    QString expected =
        "Control Flow Graph:\n"
        "Fall BB:\n"
        "  in edges: \n"
        "  out edges: 0x00001010 \n"
        "0x00001000\n"
        "Ret BB:\n"
        "  in edges: 0x00001000(0x00001000) \n"
        "  out edges: \n"
        "0x00001010\n"
        "0x00001012    1 RET *v* r24 := 0\n"
        "              Modifieds: <None>\n"
        "              Reaching definitions: r24=6,   r28=6\n\n";

    compareLongStrings(actual, expected);

    // clean up
    delete a1;
    delete a2;
    delete a3;
}


void StatementTest::testUseKill()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    Prog *prog = m_project.getProg();

    UserProc    *proc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x00000123)));
    ProcCFG *cfg   = proc->getCFG();

    Assign *a1 = new Assign(Location::regOf(REG_PENT_EAX), Const::get(5));
    a1->setNumber(1);
    a1->setProc(proc);

    Assign *a2 = new Assign(Location::regOf(REG_PENT_EAX), Binary::get(opPlus, Location::regOf(REG_PENT_EAX), Const::get(1)));
    a2->setNumber(2);
    a2->setProc(proc);

    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { a1, a2 })));
    BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));

    ReturnStatement *rs = new ReturnStatement;
    rs->setNumber(3);
    Assign *a = new Assign(Location::regOf(REG_PENT_EAX), Const::get(0));
    a->setProc(proc);
    rs->addReturn(a);
    bbRTLs.reset(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1010), { rs })));
    BasicBlock *ret = cfg->createBB(BBType::Ret, std::move(bbRTLs));

    first->addSuccessor(ret);
    ret->addPredecessor(first);
    cfg->setEntryAndExitBB(first);
    proc->setDecoded();

    // compute dataflow
    proc->decompileRecursive();

    // print cfg to a string
    QString     actual;
    OStream st(&actual);

    proc->numberStatements();
    cfg->print(st);

    QString expected =
        "Control Flow Graph:\n"
        "Fall BB:\n"
        "  in edges: \n"
        "  out edges: 0x00001010 \n"
        "0x00001000\n"
        "Ret BB:\n"
        "  in edges: 0x00001000(0x00001000) \n"
        "  out edges: \n"
        "0x00001010    1 RET *v* r24 := 0\n"
        "              Modifieds: <None>\n"
        "              Reaching definitions: r24=6\n\n";

    compareLongStrings(actual, expected);

    // clean up
    delete a1;
    delete a2;
}


void StatementTest::testEndlessLoop()
{
    //
    // BB1 -> BB2 _
    //       ^_____|

    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    Prog *prog = m_project.getProg();

    UserProc *proc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x00001000)));
    ProcCFG *cfg   = proc->getCFG();


    // r[24] := 5
    Assign *a1 = new Assign(Location::regOf(REG_PENT_EAX), Const::get(5, IntegerType::get(32, Sign::Signed)));
    a1->setProc(proc);
    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { a1 })));

    BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));


    // r24 := r24 + 1
    Assign *a2 = new Assign(Location::regOf(REG_PENT_EAX), Binary::get(opPlus,
                                                                       Location::regOf(REG_PENT_EAX),
                                                                       Const::get(1, IntegerType::get(32, Sign::Signed))));
    a2->setProc(proc);
    bbRTLs.reset(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1010), { a2 })));

    BasicBlock *body = cfg->createBB(BBType::Oneway, std::move(bbRTLs));

    first->addSuccessor(body);
    body->addPredecessor(first);
    body->addSuccessor(body);
    body->addPredecessor(body);
    cfg->setEntryAndExitBB(first);
    proc->setDecoded();

    // compute dataflow
    proc->decompileRecursive();

    QString     actual;
    OStream st(&actual);

    proc->numberStatements();
    cfg->print(st);

    // int i = 5; do { i++; } while (true);
    // TODO: is the phi really needed?
    QString expected = "Control Flow Graph:\n"
                       "Fall BB:\n"
                       "  in edges: \n"
                       "  out edges: 0x00001010 \n"
                       "0x00001000    1 *i32* r24 := 5\n"
                       "Oneway BB:\n"
                       "  in edges: 0x00001000(0x00001000) 0x00001010(0x00001010) \n"
                       "  out edges: 0x00001010 \n"
                       "0x00000000    2 *i32* r24 := phi{1 3}\n"
                       "0x00001010    3 *i32* r24 := r24{2} + 1\n"
                       "\n";

    compareLongStrings(actual, expected);
}


void StatementTest::testLocationSet()
{
    auto rof = Location::regOf(REG_SPARC_O4); // r12
    Const&      theReg = *std::dynamic_pointer_cast<Const>(rof->getSubExp1());
    LocationSet ls;


    ls.insert(rof->clone()); // ls has r12
    theReg.setInt(REG_SPARC_O0);
    ls.insert(rof->clone()); // ls has r8 r12
    theReg.setInt(REG_SPARC_I7);
    ls.insert(rof->clone()); // ls has r8 r12 r31
    theReg.setInt(REG_SPARC_I0);
    ls.insert(rof->clone()); // ls has r8 r12 r24 r31
    theReg.setInt(REG_SPARC_O4);
    ls.insert(rof->clone()); // Note: r12 already inserted

    QCOMPARE(ls.size(), 4);
    theReg.setInt(REG_SPARC_O0);
    auto ii = ls.begin();
    QVERIFY(*rof == **ii); // First element should be r8

    theReg.setInt(REG_SPARC_O4);
    SharedExp e = *(++ii);
    QVERIFY(*rof == *e); // Second should be r12

    theReg.setInt(REG_SPARC_I0);
    e = *(++ii);
    QVERIFY(*rof == *e); // Next should be r24
    theReg.setInt(REG_SPARC_I7);
    e = *(++ii);
    QVERIFY(*rof == *e);                                                                      // Last should be r31

    Location mof(opMemOf, Binary::get(opPlus, Location::regOf(REG_SPARC_O6), Const::get(4)), nullptr); // m[r14 + 4]
    ls.insert(mof.clone());                                                                  // ls should be r8 r12 r24 r31 m[r14 + 4]
    ls.insert(mof.clone());

    QCOMPARE(ls.size(), 5); // Should have 5 elements

    ii = --ls.end();
    QVERIFY(mof == **ii);   // Last element should be m[r14 + 4] now
    LocationSet ls2 = ls;
    SharedExp   e2  = *ls2.begin();
    QVERIFY(!(e2 == *ls.begin())); // Must be cloned
    QCOMPARE(ls2.size(), 5);

    theReg.setInt(REG_SPARC_O0);
    QVERIFY(*rof == **ls2.begin()); // First elements should compare equal

    theReg.setInt(REG_SPARC_O4);
    e = *(++ls2.begin());          // Second element
    QVERIFY(e != nullptr);
    QVERIFY(rof != nullptr);
    QCOMPARE(e->toString(), rof->toString());            // ... should be r12

    Assign s10(Const::get(0), Const::get(0));
    Assign s20(Const::get(0), Const::get(0));
    s10.setNumber(10);
    s20.setNumber(20);

    std::shared_ptr<RefExp> r1 = RefExp::get(Location::regOf(REG_SPARC_O0), &s10);
    std::shared_ptr<RefExp> r2 = RefExp::get(Location::regOf(REG_SPARC_O0), &s20);
    ls.insert(r1); // ls now m[r14 + 4] r8 r12 r24 r31 r8{10} (not sure where r8{10} appears)

    QCOMPARE(ls.size(), 6);
    SharedExp dummy;
    QVERIFY(!ls.findDifferentRef(r1, dummy));
    QVERIFY(ls.findDifferentRef(r2, dummy));

    SharedExp r8 = Location::regOf(REG_SPARC_O0);
    QVERIFY(!ls.containsImplicit(r8));

    std::shared_ptr<RefExp> r3(new RefExp(Location::regOf(REG_SPARC_O0), nullptr));
    ls.insert(r3);
    QVERIFY(ls.containsImplicit(r8));
    ls.remove(r3);

    ImplicitAssign          zero(r8);
    std::shared_ptr<RefExp> r4(new RefExp(Location::regOf(REG_SPARC_O0), &zero));
    ls.insert(r4);
    QVERIFY(ls.containsImplicit(r8));
}


void StatementTest::testWildLocationSet()
{
    Location rof12(opRegOf, Const::get(REG_SPARC_O4), nullptr);
    Location rof13(opRegOf, Const::get(REG_SPARC_O5), nullptr);
    Assign   a10, a20;

    a10.setNumber(10);
    a20.setNumber(20);
    std::shared_ptr<RefExp> r12_10(new RefExp(rof12.clone(), &a10));
    std::shared_ptr<RefExp> r12_20(new RefExp(rof12.clone(), &a20));
    std::shared_ptr<RefExp> r12_0(new RefExp(rof12.clone(), nullptr));
    std::shared_ptr<RefExp> r13_10(new RefExp(rof13.clone(), &a10));
    std::shared_ptr<RefExp> r13_20(new RefExp(rof13.clone(), &a20));
    std::shared_ptr<RefExp> r13_0(new RefExp(rof13.clone(), nullptr));
    std::shared_ptr<RefExp> r11_10(new RefExp(Location::regOf(REG_SPARC_O3), &a10));
    std::shared_ptr<RefExp> r22_10(new RefExp(Location::regOf(REG_SPARC_L6), &a10));

    LocationSet ls;
    ls.insert(r12_10);
    ls.insert(r12_20);
    ls.insert(r12_0);
    ls.insert(r13_10);
    ls.insert(r13_20);
    ls.insert(r13_0);

    std::shared_ptr<RefExp> wildr12(new RefExp(rof12.clone(), STMT_WILD));
    QVERIFY(ls.contains(wildr12));
    std::shared_ptr<RefExp> wildr13(new RefExp(rof13.clone(), STMT_WILD));
    QVERIFY(ls.contains(wildr13));
    std::shared_ptr<RefExp> wildr10(new RefExp(Location::regOf(REG_SPARC_O2), STMT_WILD));
    QVERIFY(!ls.contains(wildr10));

    // Test findDifferentRef
    SharedExp x;
    QVERIFY(ls.findDifferentRef(r13_10, x));
    QVERIFY(ls.findDifferentRef(r13_20, x));
    QVERIFY(ls.findDifferentRef(r13_0, x));
    QVERIFY(ls.findDifferentRef(r12_10, x));
    QVERIFY(ls.findDifferentRef(r12_20, x));
    QVERIFY(ls.findDifferentRef(r12_0, x));

    // Next 4 should fail
    QVERIFY(!ls.findDifferentRef(r11_10, x));
    QVERIFY(!ls.findDifferentRef(r22_10, x));
    ls.insert(r11_10);
    ls.insert(r22_10);
    QVERIFY(!ls.findDifferentRef(r11_10, x));
    QVERIFY(!ls.findDifferentRef(r22_10, x));
}


void StatementTest::testRecursion()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
    Prog *prog = m_project.getProg();

    UserProc *proc = new UserProc(Address::ZERO, "test", prog->getOrInsertModule("test"));
    ProcCFG *cfg   = proc->getCFG();

    std::unique_ptr<RTLList> bbRTLs(new RTLList);

    // the fallthrough bb
    {
        // push bp
        // r28 := r28 + -4
        Assign *a1 = new Assign(Location::regOf(REG_PENT_ESP),
                                Binary::get(opPlus,
                                            Location::regOf(REG_PENT_ESP),
                                            Const::get(-4)));
        a1->setProc(proc);

        // m[r28] := r29
        Assign *a2 = new Assign(Location::memOf(Location::regOf(REG_PENT_ESP)), Location::regOf(REG_PENT_EBP));
        a2->setProc(proc);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1004), { a1, a2 })));

        // push arg+1
        // r28 := r28 + -4
        Assign *a3 = (Assign *)a1->clone();
        a3->setProc(proc);

        // Reference our parameter. At esp+0 is this arg; at esp+4 is old ebp;
        // esp+8 is return address; esp+12 is our arg
        // m[r28] := m[r28+12] + 1
        Assign *a4 = new Assign(Location::memOf(Location::regOf(REG_PENT_ESP)),
                                Binary::get(opPlus,
                                            Location::memOf(Binary::get(opPlus,
                                                                        Location::regOf(REG_PENT_ESP),
                                                                        Const::get(12))),
                                            Const::get(1)));

        a4->setProc(proc);
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1006), { a3, a4 })));
    }

    BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));

    // The call BB
    bbRTLs.reset(new RTLList);
    {
        // r28 := r28 + -4
        Assign *a5 = new Assign(Location::regOf(REG_PENT_ESP), Binary::get(opPlus, Location::regOf(REG_PENT_ESP), Const::get(-4)));
        a5->setProc(proc);
        // m[r28] := pc
        Assign *a6 = new Assign(Location::memOf(Location::regOf(REG_PENT_ESP)), Terminal::get(opPC));
        a6->setProc(proc);

        // %pc := (%pc + 5) + 135893848
        Assign *a7 = new Assign(Terminal::get(opPC),
                    Binary::get(opPlus,
                                Binary::get(opPlus, Terminal::get(opPC), Const::get(5)),
                                Const::get(0x8199358)));
        a7->setProc(proc);

        CallStatement *c = new CallStatement;
        c->setDestProc(proc); // Just call self

        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1008), { a5, a6, a7, c })));
    }

    BasicBlock *callbb = cfg->createBB(BBType::Call, std::move(bbRTLs));

    first->addSuccessor(callbb);
    callbb->addPredecessor(first);
    callbb->addSuccessor(callbb);
    callbb->addPredecessor(callbb);

    // the ret bb
    bbRTLs.reset(new RTLList);
    {
        ReturnStatement *retStmt = new ReturnStatement;
        // This ReturnStatement requires the following two sets of semantics to pass the
        // tests for standard Pentium calling convention
        // pc = m[r28]
        Assign *a1 = new Assign(Terminal::get(opPC), Location::memOf(Location::regOf(REG_PENT_ESP)));
        // r28 = r28 + 4
        Assign *a2 = new Assign(Location::regOf(REG_PENT_ESP), Binary::get(opPlus, Location::regOf(REG_PENT_ESP), Const::get(4)));

        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x100C), { a1, a2, retStmt })));
    }

    BasicBlock *ret = cfg->createBB(BBType::Ret, std::move(bbRTLs));

    callbb->addSuccessor(ret);
    ret->addPredecessor(callbb);
    cfg->setEntryAndExitBB(first);

    proc->setEntryAddress(Address(0x1004));

    // decompile the "proc"
    prog->addEntryPoint(Address(0x1004));
    ProgDecompiler dcomp(prog);
    dcomp.decompile();

    proc->numberStatements();

    // print cfg to a string
    QString     actual;
    OStream st(&actual);
    cfg->print(st);

    const QString expected =
        "Control Flow Graph:\n"
        "Fall BB:\n"
        "  in edges: \n"
        "  out edges: 0x00001008 \n"
        "0x00000000    1 *union* r28 := -\n"
        "              2 *32* r29 := -\n"
        "              3 *v* m[r28{1} + 4] := -\n"
        "0x00001004\n"
        "0x00001006    4 *union* r28 := r28{1} - 8\n"
        "Call BB:\n"
        "  in edges: 0x00001006(0x00001004) 0x00001008(0x00001008) \n"
        "  out edges: 0x00001008 0x0000100c \n"
        "0x00000000    5 *union* r28 := phi{4 7}\n"
        "0x00001008    6 *u32* m[r28{5} - 4] := %pc\n"
        "              7 *union* r28 := CALL test(<all>)\n"
        "              Reaching definitions: r28=r28{5} - 4,   r29=r29{2},   m[r28{1} + 4]=m[r28{1} + 4]{3},\n"
        "                m[r28{1} - 4]=r29{2},   m[r28{1} - 8]=m[r28{1} + 4]{3} + 1\n"
        "              Live variables: r28\n"
        "Ret BB:\n"
        "  in edges: 0x00001008(0x00001008) \n"
        "  out edges: \n"
        "0x0000100c    8 RET\n"
        "              Modifieds: <None>\n"
        "              Reaching definitions: r28=r28{7} + 4,   r29=r29{7},   m[r28{1} + 4]=m[r28{1} + 4]{7},\n"
        "                m[r28{1} - 4]=m[r28{1} - 4]{7},   m[r28{1} - 8]=m[r28{1} - 8]{7},   <all>=<all>{7}\n"
        "\n";

    compareLongStrings(actual, expected);
}


void StatementTest::testClone()
{
    Assign *a1 = new Assign(Location::regOf(REG_SPARC_O0), Binary::get(opPlus, Location::regOf(REG_SPARC_O1), Const::get(99)));
    Assign *a2 = new Assign(IntegerType::get(16, Sign::Signed), Location::param("x"),
                            Location::param("y"));
    Assign *a3 = new Assign(IntegerType::get(16, Sign::Unsigned), Location::param("z"),
                            Location::param("q"));

    Statement *c1 = a1->clone();
    Statement *c2 = a2->clone();
    Statement *c3 = a3->clone();

    QString     original, clone;
    OStream original_st(&original);
    OStream clone_st(&clone);

    a1->print(original_st);
    delete a1; // And c1 should still stand!
    c1->print(clone_st);
    a2->print(original_st);
    c2->print(clone_st);
    a3->print(original_st);
    c3->print(clone_st);

    QString expected("   0 *v* r8 := r9 + 99"
                     "   0 *i16* x := y"
                     "   0 *u16* z := q");

    QCOMPARE(original, expected);
    QCOMPARE(clone, expected);

    delete a2;
    delete a3;
    delete c1;
    delete c2;
    delete c3;
}


void StatementTest::testIsAssign()
{
    QString     actual;
    OStream st(&actual);
    // r2 := 99
    Assign a(Location::regOf(REG_SPARC_G2), Const::get(99));

    a.print(st);
    QString expected("   0 *v* r2 := 99");

    QCOMPARE(expected, actual);
    QVERIFY(a.isAssign());

    CallStatement c;
    QVERIFY(!c.isAssign());
}


void StatementTest::testIsFlagAssgn()
{
    // FLAG addFlags(r2 , 99)
    Assign fc(Terminal::get(opFlags),
              Binary::get(opFlagCall, Const::get("addFlags"),
                          Binary::get(opList, Location::regOf(REG_SPARC_G2), Const::get(99))));
    CallStatement   call;
    BranchStatement br;
    Assign          as(Location::regOf(REG_SPARC_O1), Binary::get(opPlus, Location::regOf(REG_SPARC_O2), Const::get(4)));

    QString     actual;
    QString     expected("   0 *v* %flags := addFlags( r2, 99 )");
    OStream ost(&actual);

    fc.print(ost);
    QCOMPARE(expected, actual);

    QVERIFY(fc.isFlagAssign());
    QVERIFY(!call.isFlagAssign());
    QVERIFY(!br.isFlagAssign());
    QVERIFY(!as.isFlagAssign());
}


void StatementTest::testAddUsedLocsAssign()
{
    // m[r28-4] := m[r28-8] * r26
    Assign a(Location::memOf(Binary::get(opMinus,
                                         Location::regOf(REG_PENT_ESP),
                                         Const::get(4))),
             Binary::get(opMult,
                         Location::memOf(Binary::get(opMinus,
                                                     Location::regOf(REG_PENT_ESP),
                                                     Const::get(8))),
                         Location::regOf(REG_PENT_EDX)));
    a.setNumber(1);

    LocationSet l;
    a.addUsedLocs(l);

    QString     actual;
    OStream ost(&actual);
    l.print(ost);
    QString expected = "r26,\tr28,\tm[r28 - 8]";
    QCOMPARE(expected, actual);

    l.clear();
    GotoStatement g;
    g.setNumber(55);
    g.setDest(Location::memOf(Location::regOf(REG_PENT_EDX)));
    g.addUsedLocs(l);

    actual   = "";
    expected = "r26,\tm[r26]";
    l.print(ost);

    QCOMPARE(expected, actual);
}


void StatementTest::testAddUsedLocsBranch()
{
    // BranchStatement with dest m[r26{99}]{55}, condition %flags
    GotoStatement g;
    g.setNumber(55);

    LocationSet     l;
    BranchStatement b;
    b.setNumber(99);
    b.setDest(RefExp::get(Location::memOf(RefExp::get(Location::regOf(REG_PENT_EDX), &b)), &g));
    b.setCondExpr(Terminal::get(opFlags));
    b.addUsedLocs(l);

    QString     actual;
    QString     expected("r26{99},\tm[r26{99}]{55},\t%flags");
    OStream ost(&actual);
    l.print(ost);

    QCOMPARE(actual, expected);
}


void StatementTest::testAddUsedLocsCase()
{
    // CaseStatement with dest = m[r26], switchVar = m[r28 - 12]
    LocationSet   l;
    CaseStatement c;

    c.setDest(Location::memOf(Location::regOf(REG_PENT_EDX)));
    SwitchInfo si;
    si.switchExp = Location::memOf(Binary::get(opMinus, Location::regOf(REG_PENT_ESP), Const::get(12)));
    c.setSwitchInfo(&si);
    c.addUsedLocs(l);

    QString expected("r26,\tr28,\tm[r28 - 12],\tm[r26]");
    QString actual;
    OStream ost(&actual);
    l.print(ost);

    QCOMPARE(actual, expected);
}


void StatementTest::testAddUsedLocsCall()
{
    // CallStatement with dest = m[r26], params = m[r27], r28{55}, defines r31, m[r24]
    LocationSet   l;
    GotoStatement g;

    g.setNumber(55);
    CallStatement ca;
    ca.setDest(Location::memOf(Location::regOf(REG_PENT_EDX)));
    StatementList argl;
    argl.append(new Assign(Location::regOf(REG_PENT_AL), Location::memOf(Location::regOf(REG_PENT_EBX))));
    argl.append(new Assign(Location::regOf(REG_PENT_CL), RefExp::get(Location::regOf(REG_PENT_ESP), &g)));
    ca.setArguments(argl);
    ca.addDefine(new ImplicitAssign(Location::regOf(REG_PENT_EDI)));
    ca.addDefine(new ImplicitAssign(Location::regOf(REG_PENT_EAX)));
    ca.addUsedLocs(l);

    QString actual;
    OStream ost(&actual);
    l.print(ost);
    QCOMPARE(actual, QString("r26,\tr27,\tm[r26],\tm[r27],\tr28{55}"));
}


void StatementTest::testAddUsedLocsReturn()
{
    // ReturnStatement with returns r31, m[r24], m[r25]{55} + r[26]{99}]
    LocationSet   l;
    GotoStatement g;
    g.setNumber(55);

    BranchStatement b;
    b.setNumber(99);

    ReturnStatement r;
    r.addReturn(new Assign(Location::regOf(REG_PENT_EDI), Const::get(100)));
    r.addReturn(new Assign(Location::memOf(Location::regOf(REG_PENT_EAX)), Const::get(0)));
    r.addReturn(new Assign(
                     Location::memOf(Binary::get(opPlus, RefExp::get(Location::regOf(REG_PENT_ECX), &g), RefExp::get(Location::regOf(REG_PENT_EDX), &b))),
                     Const::get(5)));
    r.addUsedLocs(l);

    QString     actual;
    OStream ost(&actual);
    l.print(ost);
    QCOMPARE(actual, QString("r24,\tr25{55},\tr26{99}"));
}


void StatementTest::testAddUsedLocsBool()
{
    // Boolstatement with condition m[r24] = r25, dest m[r26]
    LocationSet l;
    BoolAssign  bs(8);

    bs.setCondExpr(Binary::get(opEquals, Location::memOf(Location::regOf(REG_PENT_EAX)), Location::regOf(REG_PENT_ECX)));
    std::list<Statement *> stmts;
    stmts.push_back(new Assign(Location::memOf(Location::regOf(REG_PENT_EDX)), Terminal::get(opNil)));

    bs.setLeftFromList(stmts);
    bs.addUsedLocs(l);

    QString     actual;
    OStream ost(&actual);
    l.print(ost);
    QCOMPARE(actual, QString("r24,\tr25,\tr26,\tm[r24]"));

    qDeleteAll(stmts);
    l.clear();

    // m[local21 + 16] := phi{0, 372}
    SharedExp base = Location::memOf(Binary::get(opPlus, Location::local("local21", nullptr), Const::get(16)));
    Assign    s372(base, Const::get(0));
    s372.setNumber(372);

    PhiAssign pa(base);
    pa.putAt(nullptr, nullptr, base); // 0
    pa.putAt(nullptr, &s372, base);   // 1
    pa.addUsedLocs(l);
    // Note: phis were not considered to use blah if they ref m[blah], so local21 was not considered used

    actual   = "";
    l.print(ost);
    QCOMPARE(actual, QString("local21,\tm[local21 + 16]{372}"));

    // m[r28{-} - 4] := -
    l.clear();
    ImplicitAssign ia(Location::memOf(Binary::get(opMinus,
                                                  RefExp::get(Location::regOf(REG_PENT_ESP), nullptr),
                                                  Const::get(4))));

    ia.addUsedLocs(l);

    actual   = "";
    l.print(ost);
    QCOMPARE(actual, QString("r28{-}"));
}


void StatementTest::testBypass()
{
    QVERIFY(m_project.loadBinaryFile(GLOBAL1_PENTIUM));

    Prog *prog = m_project.getProg();
    IFrontEnd *fe = prog->getFrontEnd();

    Type::clearNamedTypes();
    prog->setFrontEnd(fe);

    fe->decodeEntryPointsRecursive();
    fe->decodeUndecoded();

    bool    gotMain;
    Address addr = fe->findMainEntryPoint(gotMain);
    QVERIFY(addr != Address::INVALID);

    UserProc *proc = static_cast<UserProc *>(prog->getFunctionByName("foo2"));
    QVERIFY(proc != nullptr);

    proc->promoteSignature(); // Make sure it's a PentiumSignature (needed for bypassing)

    // Number the statements
    proc->numberStatements();

    PassManager::get()->executePass(PassID::StatementInit, proc);
    PassManager::get()->executePass(PassID::Dominators, proc);

    // Note: we need to have up to date call defines before transforming to SSA form,
    // because otherwise definitions of calls get ignored.
    PassManager::get()->executePass(PassID::CallDefineUpdate, proc);
    PassManager::get()->executePass(PassID::BlockVarRename, proc);

    // Find various needed statements
    StatementList stmts;
    proc->getStatements(stmts);
    StatementList::iterator it = stmts.begin();

    while (it != stmts.end() && !(*it)->isCall()) {
        ++it;
    }
    QVERIFY(it != stmts.end());
    Statement *s20 = *std::next(it, 2); // Statement 20
    QVERIFY(s20->getKind() == StmtType::Assign);

    QCOMPARE(s20->toString(), "  20 *32* r28 := r28{18} + 16");

    s20->bypass();        // r28 should bypass the call
    QCOMPARE(s20->toString(), "  20 *32* r28 := r28{15} + 20");

    // Second pass (should do nothing because r28{15} is the only reference to r28
    // that reaches the call)
    s20->bypass();
    QCOMPARE(s20->toString(), "  20 *32* r28 := r28{15} + 20");
}


QTEST_GUILESS_MAIN(StatementTest)
