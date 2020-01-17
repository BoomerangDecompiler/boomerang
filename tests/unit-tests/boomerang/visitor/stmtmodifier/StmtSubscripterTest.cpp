#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StmtSubscripterTest.h"


#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/GotoStatement.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/module/Module.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/BoolAssign.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/visitor/stmtmodifier/StmtSubscripter.h"
#include "boomerang/visitor/expmodifier/ExpSubscripter.h"


void StmtSubscripterTest::subscriptVarForStmt(const SharedStmt& stmt, SharedExp e, const SharedStmt& varDef)
{
    ExpSubscripter es(e, varDef);
    StmtSubscripter ss(&es);

    stmt->accept(&ss);
}


void StmtSubscripterTest::testSubscriptVars()
{
    SharedExp srch = Location::regOf(REG_X86_ESP);
    std::shared_ptr<Assign> s9(new Assign(Const::get(0), Const::get(0)));

    s9->setNumber(9);

    // m[r28-4] := m[r28-8] * r26
    std::shared_ptr<Assign> a(new Assign(Location::memOf(Binary::get(opMinus,
                                         Location::regOf(REG_X86_ESP),
                                         Const::get(4))),
             Binary::get(opMult,
                        Location::memOf(Binary::get(opMinus,
                                                     Location::regOf(REG_X86_ESP),
                                                     Const::get(8))),
                         Location::regOf(REG_X86_EDX))));
    a->setNumber(1);
    QString     actual;
    OStream ost(&actual);

    subscriptVarForStmt(a, srch, s9);

    ost << a;
    QString expected = "   1 *v* m[r28{9} - 4] := m[r28{9} - 8] * r26";
    QCOMPARE(actual, expected);

    // GotoStatement
    std::shared_ptr<GotoStatement> g(new GotoStatement);
    g->setNumber(55);
    g->setDest(Location::regOf(REG_X86_ESP));
    subscriptVarForStmt(g, srch, s9);

    actual   = "";
    ost << g;

    expected = "  55 GOTO r28{9}";
    QCOMPARE(actual, expected);

    // BranchStatement with dest m[r26{99}]{55}, condition %flags
    std::shared_ptr<BranchStatement> b(new BranchStatement);
    b->setNumber(99);
    SharedExp srchb = Location::memOf(RefExp::get(Location::regOf(REG_X86_EDX), b));
    b->setDest(RefExp::get(srchb, g));
    b->setCondExpr(Terminal::get(opFlags));

    subscriptVarForStmt(b, srchb, s9); // Should be ignored now: new behaviour
    subscriptVarForStmt(b, Terminal::get(opFlags), g);

    actual   = "";
    expected = "  99 BRANCH m[r26{99}]{55}, condition equals\n"
               "High level: %flags{55}";
    ost << b;
    QCOMPARE(actual, expected);

    // CaseStatement with dest = m[r26], switchVar = m[r28 - 12]
    std::shared_ptr<CaseStatement> c1(new CaseStatement);
    c1->setDest(Location::memOf(Location::regOf(REG_X86_EDX)));
    std::unique_ptr<SwitchInfo> si(new SwitchInfo);
    si->switchExp = Location::memOf(Binary::get(opMinus, Location::regOf(REG_X86_ESP), Const::get(12)));
    c1->setSwitchInfo(std::move(si));

    subscriptVarForStmt(c1, srch, s9);

    actual   = "";
    expected = "   0 SWITCH(m[r28{9} - 12])\n";
    ost << c1;
    QCOMPARE(actual, expected);

    // CaseStatement (before recog) with dest = r28, switchVar is nullptr
    std::shared_ptr<CaseStatement> c2(new CaseStatement);
    c2->setDest(Location::regOf(REG_X86_ESP));
    c2->setSwitchInfo(nullptr);

    subscriptVarForStmt(c2, srch, s9);
    actual   = "";
    expected = "   0 CASE [r28{9}]";
    ost << c2;
    QCOMPARE(expected, actual);

    // CallStatement with dest = m[r26], params = m[r27], r28, defines r28, m[r28]
    std::shared_ptr<CallStatement> ca(new CallStatement);
    ca->setDest(Location::memOf(Location::regOf(REG_X86_ESP)));
    StatementList argl;

    Prog   *prog = new Prog("testSubscriptVars", nullptr);
    Module *mod  = prog->getOrInsertModuleForSymbol("test");

    argl.append(std::make_shared<Assign>(Location::memOf(Location::regOf(REG_X86_EBX)), Const::get(1)));
    argl.append(std::make_shared<Assign>(Location::regOf(REG_X86_ESP), Const::get(2)));
    ca->setArguments(argl);
    ca->addDefine(std::make_shared<ImplicitAssign>(Location::regOf(REG_X86_ESP)));
    ca->addDefine(std::make_shared<ImplicitAssign>(Location::memOf(Location::regOf(REG_X86_ESP))));

    std::shared_ptr<ReturnStatement> retStmt(new ReturnStatement);
    UserProc destProc(Address(0x2000), "dest", mod);
    ca->setDestProc(&destProc);    // Must have a dest to be non-childless
    ca->setCalleeReturn(retStmt); // So it's not a childless call, and we can see the defs and params
    subscriptVarForStmt(ca, srch, s9);

    actual   = "";
    expected = "   0 {*v* r28, *v* m[r28]} := CALL dest(\n"
               "                *v* m[r27] := 1\n"
               "                *v* r28 := 2\n"
               "              )\n"
               "              Reaching definitions: <None>\n"
               "              Live variables: <None>";
    ost << ca;
    QCOMPARE(expected, actual);

    argl.clear();

    // CallStatement with dest = r28, params = m[r27], r29, defines r31, m[r31]
    std::shared_ptr<CallStatement> ca2(new CallStatement);
    ca2->setDest(Location::regOf(REG_X86_ESP));
    argl.append(std::make_shared<Assign>(Location::memOf(Location::regOf(REG_X86_EBX)), Const::get(1)));
    argl.append(std::make_shared<Assign>(Location::regOf(REG_X86_EBP), Const::get(2)));
    ca2->setArguments(argl);
    ca2->addDefine(std::make_shared<ImplicitAssign>(Location::regOf(REG_X86_EDI)));
    ca2->addDefine(std::make_shared<ImplicitAssign>(Location::memOf(Location::regOf(REG_X86_EDI))));

    std::shared_ptr<ReturnStatement> retStmt2(new ReturnStatement);
    UserProc dest2(Address(0x2000), "dest", mod);
    ca2->setDestProc(&dest2);       // Must have a dest to be non-childless
    ca2->setCalleeReturn(retStmt2); // So it's not a childless call, and we can see the defs and params
    subscriptVarForStmt(ca2, srch, s9);

    actual   = "";
    expected = "   0 {*v* r31, *v* m[r31]} := CALL dest(\n"
               "                *v* m[r27] := 1\n"
               "                *v* r29 := 2\n"
               "              )\n"
               "              Reaching definitions: <None>\n"
               "              Live variables: <None>";
    ost << ca2;

    QCOMPARE(actual, expected);
    argl.clear();

    // ReturnStatement with returns r28, m[r28], m[r28]{55} + r[26]{99}]
    // FIXME: shouldn't this test have some propagation? Now, it seems it's just testing the print code!
    std::shared_ptr<ReturnStatement> r(new ReturnStatement);
    r->addReturn(std::make_shared<Assign>(Location::regOf(REG_X86_ESP), Const::get(1000)));
    r->addReturn(std::make_shared<Assign>(Location::memOf(Location::regOf(REG_X86_ESP)), Const::get(2000)));
    r->addReturn(std::make_shared<Assign>(
                     Location::memOf(Binary::get(opPlus, RefExp::get(Location::regOf(REG_X86_ESP), g),
                                                 RefExp::get(Location::regOf(REG_X86_EDX), b))),
                     Const::get(100)));

    subscriptVarForStmt(r, srch, s9); // New behaviour: gets ignored now

    actual   = "";
    expected = "   0 RET *v* r28 := 1000,   *v* m[r28{9}] := 0x7d0,   *v* m[r28{55} + r26{99}] := 100\n"
               "              Modifieds: <None>\n"
               "              Reaching definitions: <None>";
    ost << r;
    QCOMPARE(actual, expected);

    // BoolAssign with condition m[r28] = r28, dest m[r28]
    SharedExp m28 = Location::memOf(Location::regOf(REG_X86_ESP));
    SharedExp cond = Binary::get(opEquals, m28, Location::regOf(REG_X86_ESP));

    std::shared_ptr<BoolAssign> bs(new BoolAssign(m28, BranchType::JE, cond));

    subscriptVarForStmt(bs, srch, s9);

    expected = "   0 BOOL m[r28{9}] := CC(equals)\n"
               "High level: m[r28{9}] = r28{9}\n";

    QCOMPARE(bs->toString(), QString(""));

    delete prog;
}


void StmtSubscripterTest::testSubscriptVar()
{
    // m[r28 - 4] := r28 + r29
    SharedExp lhs = Location::memOf(Binary::get(opMinus,
                                                 Location::regOf(REG_X86_ESP),
                                                 Const::get(4)));

    std::shared_ptr<Assign> ae(new Assign(lhs->clone(),
                                Binary::get(opPlus,
                                            Location::regOf(REG_X86_ESP),
                                            Location::regOf(REG_X86_EBP))));

    // Subtest 1: should do nothing
    SharedExp r28   = Location::regOf(REG_X86_ESP);
    SharedStmt def1(new Assign(r28->clone(), r28->clone()));

    def1->setNumber(12);
    subscriptVarForStmt(def1, lhs, def1); // Should do nothing
    QCOMPARE(ae->toString(), QString("   0 *v* m[r28 - 4] := r28 + r29"));

    // m[r28 - 4]

    // Subtest 2: Ordinary substitution, on LHS and RHS
    subscriptVarForStmt(ae, r28, def1);
    QCOMPARE(ae->toString(), QString("   0 *v* m[r28{12} - 4] := r28{12} + r29"));

    // Subtest 3: change to a different definition
    // 99: r28 := 0
    // Note: behaviour has changed. Now, we don't allow re-renaming, so it should stay the same
    SharedStmt def3(new Assign(Location::regOf(REG_X86_ESP), Const::get(0)));
    def3->setNumber(99);
    subscriptVarForStmt(ae, r28, def3);
    QCOMPARE(ae->toString(), QString("   0 *v* m[r28{12} - 4] := r28{12} + r29"));
}


QTEST_GUILESS_MAIN(StmtSubscripterTest)
