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
#include "boomerang/db/LowLevelCFG.h"
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


#define HELLO_X86      getFullSamplePath("x86/hello")
#define GLOBAL1_X86    getFullSamplePath("x86/global1")


void StatementTest::testFragment()
{
    Prog prog("testProg", &m_project);
    BasicBlock *bb = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1000), 1));

    UserProc proc(Address(0x1000), "test", nullptr);
    IRFragment *frag = proc.getCFG()->createFragment(FragType::Oneway, createRTLs(Address(0x1000), 1, 1), bb);

    std::shared_ptr<ReturnStatement> ret(new ReturnStatement);

    QVERIFY(ret->getFragment() == nullptr);
    ret->setFragment(frag);
    QVERIFY(ret->getFragment() == frag);
    ret->setFragment(nullptr);
    QVERIFY(ret->getFragment() == nullptr);
}


void StatementTest::testIsNull()
{
    {
        // %eax := -
        std::shared_ptr<ImplicitAssign> imp(new ImplicitAssign(Location::regOf(REG_X86_EAX)));
        QVERIFY(!imp->isNullStatement());
    }

    {
        std::shared_ptr<Assign> asgn(new Assign(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));
        QVERIFY(!asgn->isNullStatement());
    }

    {
        std::shared_ptr<Assign> asgn(new Assign(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_EAX)));
        QVERIFY(asgn->isNullStatement());
    }

    {
        // 5 %eax := %eax{5}
        std::shared_ptr<Assign> asgn(new Assign(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));
        std::shared_ptr<RefExp> ref = RefExp::get(Location::regOf(REG_X86_EAX), asgn);
        asgn->setRight(ref);

        QVERIFY(asgn->isNullStatement());
    }

    {
        // 5 %eax := %ecx{5}
        std::shared_ptr<Assign> asgn(new Assign(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));
        std::shared_ptr<RefExp> ref = RefExp::get(Location::regOf(REG_X86_ECX), asgn);
        asgn->setRight(ref);

        QVERIFY(asgn->isNullStatement());
    }
}


void StatementTest::testEmpty()
{
    QSKIP("TODO");
//     m_project.getSettings()->setOutputDirectory("./unit_test/");
//
//     QVERIFY(m_project.loadBinaryFile(HELLO_X86));
//
//     Prog *prog = m_project.getProg();
//
//     const auto& m = *prog->getModuleList().begin();
//     QVERIFY(m != nullptr);
//
//     // create UserProc
//     UserProc *proc = static_cast<UserProc *>(m->createFunction("test", Address(0x00000123)));
//
//     // create CFG
//     ProcCFG                    *cfg   = proc->getCFG();
//     std::unique_ptr<RTLList> bbRTLs(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x00000123), { })));
//
//     BasicBlock *entryBB = cfg->createBB(BBType::Ret, std::move(bbRTLs));
//     cfg->setEntryAndExitBB(entryBB);
//     proc->setDecoded(); // We manually "decoded"
//
//     // compute dataflow
//     proc->decompileRecursive();
//
//     // print cfg to a string
//     QString     actual;
//     OStream st(&actual);
//     cfg->print(st);
//
//     QString expected = QString(
//             "Control Flow Graph:\n"
//             "Ret Fragment:\n"
//             "  in edges: \n"
//             "  out edges: \n"
//             "0x00000123\n\n"
//         );
//
//     QCOMPARE(actual, expected);
}


void StatementTest::testFlow()
{
    QSKIP("TODO");
//     QVERIFY(m_project.loadBinaryFile(HELLO_X86));
//
//     Prog *prog = m_project.getProg();
//
//     // create UserProc
//     UserProc    *proc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x00000123)));
//     proc->setSignature(Signature::instantiate(Machine::X86, CallConv::C, "test"));
//
//     ProcCFG *cfg   = proc->getCFG();
//
//     std::shared_ptr<Assign> a1(new Assign(Location::regOf(REG_X86_EAX), std::make_shared<Const>(5)));
//     a1->setProc(proc);
//     a1->setNumber(1);
//
//     std::unique_ptr<RTLList> bbRTLs(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { a1 })));
//
//     BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));
//
//     std::shared_ptr<ReturnStatement> rs(new ReturnStatement);
//     rs->setNumber(2);
//     std::shared_ptr<Assign> a2(new Assign(Location::regOf(REG_X86_EAX), std::make_shared<Const>(5)));
//     a2->setProc(proc);
//     rs->addReturn(a2);
//
//     bbRTLs.reset(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1010), { rs })));
//
//     BasicBlock *ret = cfg->createBB(BBType::Ret, std::move(bbRTLs));
//     QVERIFY(ret);
//
//     // first was empty before
//     first->addSuccessor(ret);
//     ret->addPredecessor(first);
//     cfg->setEntryAndExitBB(first); // Also sets exitBB; important!
//     proc->setDecoded();
//
//     // compute dataflow
//     proc->decompileRecursive();
//
//     // print cfg to a string
//     QString     actual;
//     OStream st(&actual);
//
//     proc->numberStatements();
//     cfg->print(st);
//
//     // The assignment to 5 gets propagated into the return, and the assignment
//     // to r24 is removed
//     QString expected =
//         "Control Flow Graph:\n"
//         "Fall Fragment:\n"
//         "  in edges: \n"
//         "  out edges: 0x00001010 \n"
//         "0x00001000\n"
//         "Ret Fragment:\n"
//         "  in edges: 0x00001000(0x00001000) \n"
//         "  out edges: \n"
//         "0x00001010    1 RET *v* r24 := 5\n"
//         "              Modifieds: <None>\n"
//         "              Reaching definitions: r24=5\n"
//         "\n";
//
//     compareLongStrings(actual, expected);
}


void StatementTest::testKill()
{
    QSKIP("TODO");
//     QVERIFY(m_project.loadBinaryFile(HELLO_X86));
//     Prog *prog = m_project.getProg();
//
//     // create UserProc
//     QString  name  = "test";
//     UserProc *proc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x00000123)));
//     proc->setSignature(Signature::instantiate(Machine::X86, CallConv::C, name));
//
//     // create CFG
//     ProcCFG              *cfg   = proc->getCFG();
//
//     std::shared_ptr<Assign> e1(new Assign(Location::regOf(REG_X86_EAX), Const::get(5)));
//     e1->setNumber(1);
//     e1->setProc(proc);
//
//     std::shared_ptr<Assign> e2(new Assign(Location::regOf(REG_X86_EAX), Const::get(6)));
//     e2->setNumber(2);
//     e2->setProc(proc);
//
//     std::unique_ptr<RTLList> bbRTLs(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { e1, e2 })));
//     BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));
//
//     std::shared_ptr<ReturnStatement> rs(new ReturnStatement);
//     rs->setNumber(3);
//
//     std::shared_ptr<Assign> e(new Assign(Location::regOf(REG_X86_EAX), Const::get(0)));
//     e->setProc(proc);
//     rs->addReturn(e);
//
//     bbRTLs.reset(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1010), { rs })));
//
//     BasicBlock *ret = cfg->createBB(BBType::Ret, std::move(bbRTLs));
//     first->addSuccessor(ret);
//     ret->addPredecessor(first);
//     cfg->setEntryAndExitBB(first);
//     proc->setDecoded();
//
//     // compute dataflow
//     proc->decompileRecursive();
//
//     // print cfg to a string
//     QString     actual;
//     OStream st(&actual);
//
//     proc->numberStatements();
//     cfg->print(st);
//
//     QString expected =
//         "Control Flow Graph:\n"
//         "Fall Fragment:\n"
//         "  in edges: \n"
//         "  out edges: 0x00001010 \n"
//         "0x00001000\n"
//         "Ret Fragment:\n"
//         "  in edges: 0x00001000(0x00001000) \n"
//         "  out edges: \n"
//         "0x00001010    1 RET *v* r24 := 0\n"
//         "              Modifieds: <None>\n"
//         "              Reaching definitions: r24=6\n\n";
//
//     compareLongStrings(actual, expected);
}


void StatementTest::testUse()
{
    QSKIP("TODO");

//     QVERIFY(m_project.loadBinaryFile(HELLO_X86));
//     Prog *prog = m_project.getProg();
//
//     UserProc    *proc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x00000123)));
//     proc->setSignature(Signature::instantiate(Machine::X86, CallConv::C, "test"));
//
//     ProcCFG *cfg   = proc->getCFG();
//
//     std::shared_ptr<Assign> a1(new Assign(Location::regOf(REG_X86_EAX), Const::get(5)));
//     a1->setNumber(1);
//     a1->setProc(proc);
//
//     std::shared_ptr<Assign> a2(new Assign(Location::regOf(REG_X86_ESP), Location::regOf(REG_X86_EAX)));
//     a2->setNumber(2);
//     a2->setProc(proc);
//
//     std::unique_ptr<RTLList> bbRTLs(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { a1, a2 })));
//     BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));
//
//     std::shared_ptr<ReturnStatement> rs(new ReturnStatement);
//     rs->setNumber(3);
//     std::shared_ptr<Assign> a(new Assign(Location::regOf(REG_X86_ESP), Const::get(1000)));
//     a->setProc(proc);
//     rs->addReturn(a);
//     bbRTLs.reset(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1010), { rs })));
//
//     BasicBlock *ret = cfg->createBB(BBType::Ret, std::move(bbRTLs));
//     first->addSuccessor(ret);
//     ret->addPredecessor(first);
//     cfg->setEntryAndExitBB(first);
//     proc->setDecoded();
//
//     // compute dataflow
//     proc->decompileRecursive();
//     // print cfg to a string
//     QString     actual;
//     OStream st(&actual);
//
//     proc->numberStatements();
//     cfg->print(st);
//
//     QString expected =
//         "Control Flow Graph:\n"
//         "Fall Fragment:\n"
//         "  in edges: \n"
//         "  out edges: 0x00001010 \n"
//         "0x00001000\n"
//         "Ret Fragment:\n"
//         "  in edges: 0x00001000(0x00001000) \n"
//         "  out edges: \n"
//         "0x00001010    1 RET *v* r28 := 1000\n"
//         "              Modifieds: <None>\n"
//         "              Reaching definitions: r24=5,   r28=5\n\n";
//
//     compareLongStrings(actual, expected);
}


void StatementTest::testUseOverKill()
{
    QSKIP("TODO");

//     QVERIFY(m_project.loadBinaryFile(HELLO_X86));
//     Prog *prog = m_project.getProg();
//
//     UserProc *proc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x00000123)));
//     proc->setSignature(Signature::instantiate(Machine::X86, CallConv::C, "test"));
//     ProcCFG *cfg = proc->getCFG();
//
//     std::shared_ptr<Assign> e1(new Assign(Location::regOf(REG_X86_EAX), Const::get(5)));
//     e1->setNumber(1);
//     e1->setProc(proc);
//
//     std::shared_ptr<Assign> e2(new Assign(Location::regOf(REG_X86_EAX), Const::get(6)));
//     e2->setNumber(2);
//     e2->setProc(proc);
//
//     std::shared_ptr<Assign> e3(new Assign(Location::regOf(REG_X86_ESP), Location::regOf(REG_X86_EAX)));
//     e3->setNumber(3);
//     e3->setProc(proc);
//
//     std::unique_ptr<RTLList> bbRTLs(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { e1, e2, e3 })));
//     BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));
//
//     std::shared_ptr<ReturnStatement> rs(new ReturnStatement);
//     rs->setNumber(4);
//     std::shared_ptr<Assign> e(new Assign(Location::regOf(REG_X86_EAX), Const::get(0)));
//     e->setProc(proc);
//     rs->addReturn(e);
//
//     bbRTLs.reset(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1010), { rs })));
//     BasicBlock *ret = cfg->createBB(BBType::Ret, std::move(bbRTLs));
//
//     first->addSuccessor(ret);
//     ret->addPredecessor(first);
//     cfg->setEntryAndExitBB(first);
//     proc->setDecoded();
//
//     // compute dataflow
//     proc->decompileRecursive();
//
//     // print cfg to a string
//     QString     actual;
//     OStream st(&actual);
//
//     proc->numberStatements();
//     cfg->print(st);
//
//     // compare it to expected
//     QString expected =
//         "Control Flow Graph:\n"
//         "Fall Fragment:\n"
//         "  in edges: \n"
//         "  out edges: 0x00001010 \n"
//         "0x00001000\n"
//         "Ret Fragment:\n"
//         "  in edges: 0x00001000(0x00001000) \n"
//         "  out edges: \n"
//         "0x00001010    1 RET *v* r24 := 0\n"
//         "              Modifieds: <None>\n"
//         "              Reaching definitions: r24=6,   r28=6\n\n";
//
//     compareLongStrings(actual, expected);
}


void StatementTest::testUseOverBB()
{
    QSKIP("TODO");

//     QVERIFY(m_project.loadBinaryFile(HELLO_X86));
//     Prog *prog = m_project.getProg();
//
//     // create UserProc
//     UserProc *proc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x00001000)));
//     ProcCFG *cfg       = proc->getCFG();
//
//     std::shared_ptr<Assign> a1(new Assign(Location::regOf(REG_X86_EAX), Const::get(5)));
//     a1->setNumber(1);
//     a1->setProc(proc);
//
//     std::shared_ptr<Assign> a2(new Assign(Location::regOf(REG_X86_EAX), Const::get(6)));
//     a2->setNumber(2);
//     a2->setProc(proc);
//
//     std::unique_ptr<RTLList> bbRTLs(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { a1, a2 })));
//     BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));
//
//     std::shared_ptr<Assign> a3(new Assign(Location::regOf(REG_X86_ESP), Location::regOf(REG_X86_EAX)));
//     a3->setNumber(3);
//     a3->setProc(proc);
//     bbRTLs.reset(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1010), { a3 })));
//
//
//     std::shared_ptr<ReturnStatement> rs(new ReturnStatement);
//     rs->setNumber(4);
//
//     std::shared_ptr<Assign> a(new Assign(Location::regOf(REG_X86_EAX), Const::get(0)));
//     a->setProc(proc);
//     rs->addReturn(a);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x00001012), { rs })));
//     BasicBlock *ret = cfg->createBB(BBType::Ret, std::move(bbRTLs));
//
//     first->addSuccessor(ret);
//     ret->addPredecessor(first);
//     cfg->setEntryAndExitBB(first);
//     proc->setDecoded();
//
//     // compute dataflow
//     proc->decompileRecursive();
//
//     // print cfg to a string
//     QString     actual;
//     OStream st(&actual);
//
//     proc->numberStatements();
//     cfg->print(st);
//
//     QString expected =
//         "Control Flow Graph:\n"
//         "Fall Fragment:\n"
//         "  in edges: \n"
//         "  out edges: 0x00001010 \n"
//         "0x00001000\n"
//         "Ret Fragment:\n"
//         "  in edges: 0x00001000(0x00001000) \n"
//         "  out edges: \n"
//         "0x00001010\n"
//         "0x00001012    1 RET *v* r24 := 0\n"
//         "              Modifieds: <None>\n"
//         "              Reaching definitions: r24=6,   r28=6\n\n";
//
//     compareLongStrings(actual, expected);
}


void StatementTest::testUseKill()
{
    QSKIP("TODO");

//     QVERIFY(m_project.loadBinaryFile(HELLO_X86));
//     Prog *prog = m_project.getProg();
//
//     UserProc    *proc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x00000123)));
//     ProcCFG *cfg   = proc->getCFG();
//
//     std::shared_ptr<Assign> a1(new Assign(Location::regOf(REG_X86_EAX), Const::get(5)));
//     a1->setNumber(1);
//     a1->setProc(proc);
//
//     std::shared_ptr<Assign> a2(new Assign(Location::regOf(REG_X86_EAX), Binary::get(opPlus, Location::regOf(REG_X86_EAX), Const::get(1))));
//     a2->setNumber(2);
//     a2->setProc(proc);
//
//     std::unique_ptr<RTLList> bbRTLs(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { a1, a2 })));
//     BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));
//
//     std::shared_ptr<ReturnStatement> rs(new ReturnStatement);
//     rs->setNumber(3);
//     std::shared_ptr<Assign> a(new Assign(Location::regOf(REG_X86_EAX), Const::get(0)));
//     a->setProc(proc);
//     rs->addReturn(a);
//     bbRTLs.reset(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1010), { rs })));
//     BasicBlock *ret = cfg->createBB(BBType::Ret, std::move(bbRTLs));
//
//     first->addSuccessor(ret);
//     ret->addPredecessor(first);
//     cfg->setEntryAndExitBB(first);
//     proc->setDecoded();
//
//     // compute dataflow
//     proc->decompileRecursive();
//
//     // print cfg to a string
//     QString     actual;
//     OStream st(&actual);
//
//     proc->numberStatements();
//     cfg->print(st);
//
//     QString expected =
//         "Control Flow Graph:\n"
//         "Fall Fragment:\n"
//         "  in edges: \n"
//         "  out edges: 0x00001010 \n"
//         "0x00001000\n"
//         "Ret Fragment:\n"
//         "  in edges: 0x00001000(0x00001000) \n"
//         "  out edges: \n"
//         "0x00001010    1 RET *v* r24 := 0\n"
//         "              Modifieds: <None>\n"
//         "              Reaching definitions: r24=6\n\n";
//
//     compareLongStrings(actual, expected);
}


void StatementTest::testEndlessLoop()
{
    QSKIP("TODO");

//     //
//     // BB1 -> BB2 _
//     //       ^_____|
//
//     QVERIFY(m_project.loadBinaryFile(HELLO_X86));
//     Prog *prog = m_project.getProg();
//
//     UserProc *proc = static_cast<UserProc *>(prog->getOrCreateFunction(Address(0x00001000)));
//     ProcCFG *cfg   = proc->getCFG();
//
//     // r[24] := 5
//     std::shared_ptr<Assign> a1(new Assign(Location::regOf(REG_X86_EAX), Const::get(5, IntegerType::get(32, Sign::Signed))));
//     a1->setProc(proc);
//     std::unique_ptr<RTLList> bbRTLs(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { a1 })));
//
//     BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));
//
//
//     // r24 := r24 + 1
//     std::shared_ptr<Assign> a2(new Assign(Location::regOf(REG_X86_EAX), Binary::get(opPlus,
//                                                                        Location::regOf(REG_X86_EAX),
//                                                                        Const::get(1, IntegerType::get(32, Sign::Signed)))));
//     a2->setProc(proc);
//     bbRTLs.reset(new RTLList);
//     bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1010), { a2 })));
//
//     BasicBlock *body = cfg->createBB(BBType::Oneway, std::move(bbRTLs));
//
//     first->addSuccessor(body);
//     body->addPredecessor(first);
//     body->addSuccessor(body);
//     body->addPredecessor(body);
//     cfg->setEntryAndExitBB(first);
//     proc->setDecoded();
//
//     // compute dataflow
//     proc->decompileRecursive();
//
//     QString     actual;
//     OStream st(&actual);
//
//     proc->numberStatements();
//     cfg->print(st);
//
//     // int i = 5; do { i++; } while (true);
//     // TODO: is the phi really needed?
//     QString expected = "Control Flow Graph:\n"
//                        "Fall Fragment:\n"
//                        "  in edges: \n"
//                        "  out edges: 0x00001010 \n"
//                        "0x00001000    1 *i32* r24 := 5\n"
//                        "Oneway Fragment:\n"
//                        "  in edges: 0x00001000(0x00001000) 0x00001010(0x00001010) \n"
//                        "  out edges: 0x00001010 \n"
//                        "0x00000000    2 *i32* r24 := phi{1 3}\n"
//                        "0x00001010    3 *i32* r24 := r24{2} + 1\n"
//                        "\n";
//
//     compareLongStrings(actual, expected);
}


void StatementTest::testRecursion()
{
    QSKIP("TODO");

//     QVERIFY(m_project.loadBinaryFile(HELLO_X86));
//     Prog *prog = m_project.getProg();
//
//     UserProc *proc = new UserProc(Address::ZERO, "test", prog->getOrInsertModule("test"));
//     ProcCFG *cfg   = proc->getCFG();
//
//     std::unique_ptr<RTLList> bbRTLs(new RTLList);
//
//     // the fallthrough bb
//     {
//         // push bp
//         // r28 := r28 + -4
//         std::shared_ptr<Assign> a1(new Assign(Location::regOf(REG_X86_ESP),
//                                 Binary::get(opPlus,
//                                             Location::regOf(REG_X86_ESP),
//                                             Const::get(-4))));
//         a1->setProc(proc);
//
//         // m[r28] := r29
//         std::shared_ptr<Assign> a2(new Assign(Location::memOf(Location::regOf(REG_X86_ESP)), Location::regOf(REG_X86_EBP)));
//         a2->setProc(proc);
//         bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1004), { a1, a2 })));
//
//         // push arg+1
//         // r28 := r28 + -4
//         std::shared_ptr<Assign> a3 = a1->clone()->as<Assign>();
//         a3->setProc(proc);
//
//         // Reference our parameter. At esp+0 is this arg; at esp+4 is old ebp;
//         // esp+8 is return address; esp+12 is our arg
//         // m[r28] := m[r28+12] + 1
//         std::shared_ptr<Assign> a4(new Assign(Location::memOf(Location::regOf(REG_X86_ESP)),
//                                 Binary::get(opPlus,
//                                             Location::memOf(Binary::get(opPlus,
//                                                                         Location::regOf(REG_X86_ESP),
//                                                                         Const::get(12))),
//                                             Const::get(1))));
//
//         a4->setProc(proc);
//         bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1006), { a3, a4 })));
//     }
//
//     BasicBlock *first = cfg->createBB(BBType::Fall, std::move(bbRTLs));
//
//     // The call BB
//     bbRTLs.reset(new RTLList);
//     {
//         // r28 := r28 + -4
//         std::shared_ptr<Assign> a5(new Assign(Location::regOf(REG_X86_ESP), Binary::get(opPlus, Location::regOf(REG_X86_ESP), Const::get(-4))));
//         a5->setProc(proc);
//
//         // m[r28] := pc
//         std::shared_ptr<Assign> a6(new Assign(Location::memOf(Location::regOf(REG_X86_ESP)), Terminal::get(opPC)));
//         a6->setProc(proc);
//
//         // %pc := (%pc + 5) + 135893848
//         std::shared_ptr<Assign> a7(new Assign(Terminal::get(opPC),
//                     Binary::get(opPlus,
//                                 Binary::get(opPlus, Terminal::get(opPC), Const::get(5)),
//                                 Const::get(0x8199358))));
//         a7->setProc(proc);
//
//         std::shared_ptr<CallStatement> c(new CallStatement);
//         c->setDestProc(proc); // Just call self
//
//         bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1008), { a5, a6, a7, c })));
//     }
//
//     BasicBlock *callbb = cfg->createBB(BBType::Call, std::move(bbRTLs));
//
//     first->addSuccessor(callbb);
//     callbb->addPredecessor(first);
//     callbb->addSuccessor(callbb);
//     callbb->addPredecessor(callbb);
//
//     // the ret bb
//     bbRTLs.reset(new RTLList);
//     {
//         std::shared_ptr<ReturnStatement> retStmt(new ReturnStatement);
//         // This ReturnStatement requires the following two sets of semantics to pass the
//         // tests for standard x86 calling convention
//         // pc = m[r28]
//         std::shared_ptr<Assign> a1(new Assign(Terminal::get(opPC), Location::memOf(Location::regOf(REG_X86_ESP))));
//         // r28 = r28 + 4
//         std::shared_ptr<Assign> a2(new Assign(Location::regOf(REG_X86_ESP), Binary::get(opPlus, Location::regOf(REG_X86_ESP), Const::get(4))));
//
//         bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x100C), { a1, a2, retStmt })));
//     }
//
//     BasicBlock *ret = cfg->createBB(BBType::Ret, std::move(bbRTLs));
//
//     callbb->addSuccessor(ret);
//     ret->addPredecessor(callbb);
//     cfg->setEntryAndExitBB(first);
//
//     proc->setEntryAddress(Address(0x1004));
//
//     // decompile the "proc"
//     prog->addEntryPoint(Address(0x1004));
//     ProgDecompiler dcomp(prog);
//     dcomp.decompile();
//
//     proc->numberStatements();
//
//     // print cfg to a string
//     QString     actual;
//     OStream st(&actual);
//     cfg->print(st);
//
//     const QString expected =
//         "Control Flow Graph:\n"
//         "Fall Fragment:\n"
//         "  in edges: \n"
//         "  out edges: 0x00001008 \n"
//         "0x00000000    1 *union* r28 := -\n"
//         "              2 *32* r29 := -\n"
//         "              3 *v* m[r28{1} + 4] := -\n"
//         "0x00001004\n"
//         "0x00001006    4 *union* r28 := r28{1} - 8\n"
//         "Call Fragment:\n"
//         "  in edges: 0x00001006(0x00001004) 0x00001008(0x00001008) \n"
//         "  out edges: 0x00001008 0x0000100c \n"
//         "0x00000000    5 *union* r28 := phi{4 7}\n"
//         "0x00001008    6 *u32* m[r28{5} - 4] := %pc\n"
//         "              7 *union* r28 := CALL test(<all>)\n"
//         "              Reaching definitions: r28=r28{5} - 4,   r29=r29{2},   m[r28{1} + 4]=m[r28{1} + 4]{3},\n"
//         "                m[r28{1} - 4]=r29{2},   m[r28{1} - 8]=m[r28{1} + 4]{3} + 1\n"
//         "              Live variables: r28\n"
//         "Ret Fragment:\n"
//         "  in edges: 0x00001008(0x00001008) \n"
//         "  out edges: \n"
//         "0x0000100c    8 RET\n"
//         "              Modifieds: <None>\n"
//         "              Reaching definitions: r28=r28{7} + 4,   r29=r29{7},   m[r28{1} + 4]=m[r28{1} + 4]{7},\n"
//         "                m[r28{1} - 4]=m[r28{1} - 4]{7},   m[r28{1} - 8]=m[r28{1} - 8]{7},   <all>=<all>{7}\n"
//         "\n";
//
//     compareLongStrings(actual, expected);
}


void StatementTest::testIsAssign()
{
    std::shared_ptr<Assign> a(new Assign(Location::regOf(REG_X86_DX), Const::get(99)));
    std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));

    QVERIFY(a->isAssign());
    QVERIFY(!call->isAssign());
}


void StatementTest::testIsFlagAssgn()
{
    // FLAG addFlags(r2 , 99)
    std::shared_ptr<Assign> fc(new Assign(Terminal::get(opFlags),
                                          Binary::get(opFlagCall,
                                                      Const::get("addFlags"),
                                                      Binary::get(opList,
                                                                  Location::regOf(REG_X86_DX),
                                                                  Const::get(99)))));
    std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
    std::shared_ptr<BranchStatement> br(new BranchStatement(Address(0x1000)));
    std::shared_ptr<Assign> as(new Assign(Location::regOf(REG_X86_CL),
                                          Binary::get(opPlus,
                                                      Location::regOf(REG_X86_DL),
                                                      Const::get(4))));

    QVERIFY(fc->isFlagAssign());
    QVERIFY(!call->isFlagAssign());
    QVERIFY(!br->isFlagAssign());
    QVERIFY(!as->isFlagAssign());
}


void StatementTest::testAddUsedLocsAssign()
{
    {
        // m[r28-4] := m[r28-8] * r26
        std::shared_ptr<Assign> a(new Assign(
            Location::memOf(Binary::get(opMinus,
                                        Location::regOf(REG_X86_ESP),
                                        Const::get(4))),
            Binary::get(opMult,
                        Location::memOf(Binary::get(opMinus,
                                                    Location::regOf(REG_X86_ESP),
                                                    Const::get(8))),
                        Location::regOf(REG_X86_EDX))));
        a->setNumber(1);

        LocationSet l;
        a->addUsedLocs(l);

        QCOMPARE(l.toString(), "r26,\tr28,\tm[r28 - 8]");
    }

    {

        std::shared_ptr<GotoStatement> g(new GotoStatement(Location::memOf(Location::regOf(REG_X86_EDX))));
        g->setNumber(55);

        LocationSet l;
        g->addUsedLocs(l);

        QCOMPARE(l.toString(), "r26,\tm[r26]");
    }
}


void StatementTest::testAddUsedLocsBranch()
{
    // BranchStatement with dest m[r26{99}]{55}, condition %flags
    std::shared_ptr<GotoStatement> g(new GotoStatement(Address(0x1000)));
    g->setNumber(55);

    std::shared_ptr<BranchStatement> b(new BranchStatement(Address(0x1000)));
    b->setNumber(99);
    b->setDest(RefExp::get(Location::memOf(RefExp::get(Location::regOf(REG_X86_EDX), b)), g));
    b->setCondExpr(Terminal::get(opFlags));

    LocationSet l;
    b->addUsedLocs(l);

    QCOMPARE(l.toString(), "r26{99},\tm[r26{99}]{55},\t%flags");
}


void StatementTest::testAddUsedLocsCase()
{
    // CaseStatement with dest = m[r26], switchVar = m[r28 - 12]
    std::shared_ptr<CaseStatement> c(new CaseStatement(Location::memOf(Location::regOf(REG_X86_EDX))));

    std::unique_ptr<SwitchInfo> si(new SwitchInfo);
    si->switchExp = Location::memOf(Binary::get(opMinus, Location::regOf(REG_X86_ESP), Const::get(12)));
    c->setSwitchInfo(std::move(si));

    LocationSet l;
    c->addUsedLocs(l);

    QCOMPARE(l.toString(), "r26,\tr28,\tm[r28 - 12],\tm[r26]");
}


void StatementTest::testAddUsedLocsCall()
{
    // CallStatement with dest = m[r26], params = m[r27], r28{55}, defines r31, m[r24]
    std::shared_ptr<GotoStatement> g(new GotoStatement(Address(0x1000)));
    g->setNumber(55);

    std::shared_ptr<CallStatement> ca(new CallStatement(Location::memOf(Location::regOf(REG_X86_EDX))));
    StatementList argl;

    argl.append(std::make_shared<Assign>(Location::regOf(REG_X86_AL), Location::memOf(Location::regOf(REG_X86_EBX))));
    argl.append(std::make_shared<Assign>(Location::regOf(REG_X86_CL), RefExp::get(Location::regOf(REG_X86_ESP), g)));
    ca->setArguments(argl);

    ca->addDefine(std::make_shared<ImplicitAssign>(Location::regOf(REG_X86_EDI)));
    ca->addDefine(std::make_shared<ImplicitAssign>(Location::regOf(REG_X86_EAX)));

    LocationSet l;
    ca->addUsedLocs(l);

    QCOMPARE(l.toString(), "r26,\tr27,\tm[r26],\tm[r27],\tr28{55}");
}


void StatementTest::testAddUsedLocsReturn()
{
    // ReturnStatement with returns r31, m[r24], m[r25]{55} + r[26]{99}]
    std::shared_ptr<GotoStatement> g(new GotoStatement(Address(0x0800)));
    g->setNumber(55);

    std::shared_ptr<BranchStatement> b(new BranchStatement(Address(0x0800)));
    b->setNumber(99);

    std::shared_ptr<ReturnStatement> r(new ReturnStatement);
    r->addReturn(std::make_shared<Assign>(Location::regOf(REG_X86_EDI), Const::get(100)));
    r->addReturn(std::make_shared<Assign>(Location::memOf(Location::regOf(REG_X86_EAX)), Const::get(0)));
    r->addReturn(std::make_shared<Assign>(
                     Location::memOf(Binary::get(opPlus, RefExp::get(Location::regOf(REG_X86_ECX), g), RefExp::get(Location::regOf(REG_X86_EDX), b))),
                     Const::get(5)));

    LocationSet   l;
    r->addUsedLocs(l);

    QCOMPARE(l.toString(), "r24,\tr25{55},\tr26{99}");
}


void StatementTest::testAddUsedLocsBool()
{
    {
        // Boolstatement with condition m[r24] = r25, dest m[r26]
        SharedExp lhs = Location::memOf(Location::regOf(REG_X86_EDX));
        SharedExp cond = Binary::get(opEquals, Location::memOf(Location::regOf(REG_X86_EAX)), Location::regOf(REG_X86_ECX));

        std::shared_ptr<BoolAssign> bs(new BoolAssign(lhs, BranchType::JE, cond));

        LocationSet l;
        bs->addUsedLocs(l);

        QCOMPARE(l.toString(), "r24,\tr25,\tr26,\tm[r24]");
    }

    {
        // m[local21 + 16] := phi{0, 372}
        SharedExp base = Location::memOf(Binary::get(opPlus, Location::local("local21", nullptr), Const::get(16)));
        std::shared_ptr<Assign> s372(new Assign(base, Const::get(0)));
        s372->setNumber(372);

        std::shared_ptr<PhiAssign> pa(new PhiAssign(base));
        pa->putAt(nullptr, nullptr, base); // 0
        pa->putAt(nullptr, s372, base);    // 1

        LocationSet l;
        pa->addUsedLocs(l);

        // Note: phis were not considered to use blah if they ref m[blah], so local21 was not considered used
        QCOMPARE(l.toString(), "local21,\tm[local21 + 16]{372}");
    }

    // m[r28{-} - 4] := -
    {
        auto ia = std::make_shared<ImplicitAssign>(Location::memOf(Binary::get(opMinus,
                                               RefExp::get(Location::regOf(REG_X86_ESP), nullptr),
                                               Const::get(4))));

        LocationSet l;
        ia->addUsedLocs(l);

        QCOMPARE(l.toString(), "r28{-}");
    }
}


void StatementTest::testBypass()
{
    QSKIP("TODO");

//     QVERIFY(m_project.loadBinaryFile(GLOBAL1_X86));
//
//     Prog *prog = m_project.getProg();
//     IFrontEnd *fe = prog->getFrontEnd();
//     assert(fe != nullptr);
//
//     Type::clearNamedTypes();
//     prog->setFrontEnd(fe);
//
//     fe->decodeEntryPointsRecursive();
//     fe->disassembleAll();
//
//     bool    gotMain;
//     Address addr = fe->findMainEntryPoint(gotMain);
//     QVERIFY(addr != Address::INVALID);
//
//     UserProc *proc = static_cast<UserProc *>(prog->getFunctionByName("foo2"));
//     QVERIFY(proc != nullptr);
//
//     proc->promoteSignature(); // Make sure it's an X86Signature (needed for bypassing)
//
//     // Number the statements
//     proc->numberStatements();
//
//     PassManager::get()->executePass(PassID::StatementInit, proc);
//     PassManager::get()->executePass(PassID::Dominators, proc);
//
//     // Note: we need to have up to date call defines before transforming to SSA form,
//     // because otherwise definitions of calls get ignored.
//     PassManager::get()->executePass(PassID::CallDefineUpdate, proc);
//     PassManager::get()->executePass(PassID::BlockVarRename, proc);
//
//     // Find various needed statements
//     StatementList stmts;
//     proc->getStatements(stmts);
//     StatementList::iterator it = stmts.begin();
//
//     while (it != stmts.end() && !(*it)->isCall()) {
//         ++it;
//     }
//     QVERIFY(it != stmts.end());
//     SharedStmt s19 = *std::next(it, 2);
//     QVERIFY(s19->getKind() == StmtType::Assign);
//
//     QCOMPARE(s19->toString(), "  19 *32* r28 := r28{17} + 16");
//
//     s19->bypass();        // r28 should bypass the call
//     QCOMPARE(s19->toString(), "  19 *32* r28 := r28{15} + 20");
//
//     // Second pass (should do nothing because r28{15} is the only reference to r28
//     // that reaches the call)
//     s19->bypass();
//     QCOMPARE(s19->toString(), "  19 *32* r28 := r28{15} + 20");
}


QTEST_GUILESS_MAIN(StatementTest)
