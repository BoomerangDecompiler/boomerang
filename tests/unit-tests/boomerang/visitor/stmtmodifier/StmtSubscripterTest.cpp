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
    const SharedExp esp = Location::regOf(REG_X86_ESP);
    const SharedExp edx = Location::regOf(REG_X86_EDX);
    const SharedExp ebx = Location::regOf(REG_X86_EBX);
    const SharedExp ebp = Location::regOf(REG_X86_EBP);
    const SharedExp edi = Location::regOf(REG_X86_EDI);

    Prog prog("testSubscriptVars", nullptr);
    Module *mod = prog.getOrInsertModuleForSymbol("test");

    std::shared_ptr<Assign> s9(new Assign(Const::get(0), Const::get(0)));
    s9->setNumber(9);

    {
        // m[r28-4] := m[r28-8] * r26
        std::shared_ptr<Assign> a(new Assign(Location::memOf(Binary::get(opMinus, esp, Const::get(4))),
            Binary::get(opMult, Location::memOf(Binary::get(opMinus, esp, Const::get(8))), edx)));

        a->setNumber(1);

        subscriptVarForStmt(a, esp, s9);

        QCOMPARE(a->toString(), "   1 *v* m[r28{9} - 4] := m[r28{9} - 8] * r26");
    }

    {
        // GotoStatement
        std::shared_ptr<GotoStatement> g(new GotoStatement(esp));
        g->setNumber(55);
        subscriptVarForStmt(g, esp, s9);

        QCOMPARE(g->toString(), "  55 GOTO r28{9}");
    }

    {
        // FIXME
        // BranchStatement with dest m[r26{99}]{55}, condition %flags
        std::shared_ptr<GotoStatement> g(new GotoStatement(esp));
        g->setNumber(55);

        std::shared_ptr<BranchStatement> b(new BranchStatement(Address::INVALID));
        SharedExp dest = RefExp::get(Location::memOf(RefExp::get(edx, b)), g);

        b->setDest(dest);
        b->setNumber(99);
        b->setCondExpr(Terminal::get(opFlags));

        subscriptVarForStmt(b, dest, s9); // Should be ignored now: new behaviour
        subscriptVarForStmt(b, Terminal::get(opFlags), g);

        QCOMPARE(b->toString(),
                "  99 BRANCH m[r26{99}]{55}, condition equals\n"
                "High level: %flags{55}");
    }

    {
        // CaseStatement with dest = m[r26], switchVar = m[r28 - 12]
        std::shared_ptr<CaseStatement> c1(new CaseStatement(Location::memOf(edx)));
        std::unique_ptr<SwitchInfo> si(new SwitchInfo);
        si->switchExp = Location::memOf(Binary::get(opMinus, esp, Const::get(12)));
        c1->setSwitchInfo(std::move(si));

        subscriptVarForStmt(c1, esp, s9);

        QCOMPARE(c1->toString(), "   0 SWITCH(m[r28{9} - 12])\n");
    }

    {
        // CaseStatement (before recog) with dest = r28, switchVar is nullptr
        std::shared_ptr<CaseStatement> c2(new CaseStatement(esp));
        c2->setSwitchInfo(nullptr);

        subscriptVarForStmt(c2, esp, s9);

        QCOMPARE(c2->toString(), "   0 CASE [r28{9}]");
    }

    {
        // CallStatement with dest = m[r26], params = m[r27], r28, defines r28, m[r28]
        std::shared_ptr<CallStatement> ca(new CallStatement(Location::memOf(esp)));

        StatementList argl;
        argl.append(std::make_shared<Assign>(Location::memOf(ebx), Const::get(1)));
        argl.append(std::make_shared<Assign>(esp, Const::get(2)));
        ca->setArguments(argl);
        ca->addDefine(std::make_shared<ImplicitAssign>(esp));
        ca->addDefine(std::make_shared<ImplicitAssign>(Location::memOf(esp)));

        std::shared_ptr<ReturnStatement> retStmt(new ReturnStatement);
        UserProc destProc(Address(0x2000), "dest", mod);
        ca->setDestProc(&destProc);   // Must have a dest to be non-childless
        ca->setCalleeReturn(retStmt); // So it's not a childless call, and we can see the defs and params

        subscriptVarForStmt(ca, esp, s9);

        QCOMPARE(ca->toString(),
                "   0 { *v* r28, *v* m[r28] } := CALL dest(\n"
                "                *v* m[r27] := 1\n"
                "                *v* r28 := 2\n"
                "              )\n"
                "              Reaching definitions: <None>\n"
                "              Live variables: <None>");
    }

    {
        // CallStatement with dest = r28, params = m[r27], r29, defines r31, m[r31]
        std::shared_ptr<CallStatement> ca2(new CallStatement(esp));

        StatementList argl;
        argl.append(std::make_shared<Assign>(Location::memOf(ebx), Const::get(1)));
        argl.append(std::make_shared<Assign>(ebp, Const::get(2)));
        ca2->setArguments(argl);
        ca2->addDefine(std::make_shared<ImplicitAssign>(edi));
        ca2->addDefine(std::make_shared<ImplicitAssign>(Location::memOf(edi)));

        std::shared_ptr<ReturnStatement> retStmt2(new ReturnStatement);
        UserProc dest2(Address(0x2000), "dest", mod);
        ca2->setDestProc(&dest2);       // Must have a dest to be non-childless
        ca2->setCalleeReturn(retStmt2); // So it's not a childless call, and we can see the defs and params

        subscriptVarForStmt(ca2, esp, s9);

        QCOMPARE(ca2->toString(),
                 "   0 { *v* r31, *v* m[r31] } := CALL dest(\n"
                 "                *v* m[r27] := 1\n"
                 "                *v* r29 := 2\n"
                 "              )\n"
                 "              Reaching definitions: <None>\n"
                 "              Live variables: <None>");
    }

    {
        // ReturnStatement with returns r28, m[r28], m[r28]{55} + r[26]{99}]
        // FIXME: shouldn't this test have some propagation? Now, it seems it's just testing the print code!
        std::shared_ptr<GotoStatement> g(new GotoStatement(esp));
        g->setNumber(55);

        std::shared_ptr<BranchStatement> b(new BranchStatement(Address::INVALID));
        b->setNumber(99);

        std::shared_ptr<ReturnStatement> r(new ReturnStatement);
        r->addReturn(std::make_shared<Assign>(esp, Const::get(1000)));
        r->addReturn(std::make_shared<Assign>(Location::memOf(esp), Const::get(2000)));
        r->addReturn(std::make_shared<Assign>(Location::memOf(Binary::get(opPlus,
                                                                        RefExp::get(esp, g),
                                                                        RefExp::get(edx, b))),
                                            Const::get(100)));

        subscriptVarForStmt(r, esp, s9); // New behaviour: gets ignored now

        QCOMPARE(r->toString(),
                "   0 RET *v* r28 := 1000,   *v* m[r28{9}] := 0x7d0,   *v* m[r28{55} + r26{99}] := 100\n"
                "              Modifieds: <None>\n"
                "              Reaching definitions: <None>");
    }

    {
        // BoolAssign with condition m[r28] = r28, dest m[r28]
        SharedExp m28 = Location::memOf(Location::regOf(REG_X86_ESP));
        SharedExp cond = Binary::get(opEquals, m28, Location::regOf(REG_X86_ESP));

        std::shared_ptr<BoolAssign> bs(new BoolAssign(m28, BranchType::JE, cond));

        subscriptVarForStmt(bs, esp, s9);

        QCOMPARE(bs->toString(), "   0 BOOL m[r28{9}] := CC(equals)\nHigh level: m[r28{9}] = r28{9}\n");
    }
}


void StmtSubscripterTest::testSubscriptVar()
{
    const SharedExp esp = Location::regOf(REG_X86_ESP);
    const SharedExp ebp = Location::regOf(REG_X86_EBP);
    const SharedExp lhs = Location::memOf(Binary::get(opMinus, esp, Const::get(4)));

    // m[r28 - 4] := r28 + r29
    std::shared_ptr<Assign> ae(new Assign(lhs, Binary::get(opPlus, esp, ebp)));
    ae->setNumber(1);

    {
        // Subtest 1: should do nothing
        SharedStmt def1(new Assign(esp, esp));
        def1->setNumber(12);

        subscriptVarForStmt(def1, lhs, def1); // Should do nothing
        QCOMPARE(ae->toString(), QString("   1 *v* m[r28 - 4] := r28 + r29"));
    }

    {
        // Subtest 2: Ordinary substitution, on LHS and RHS
        SharedStmt def1(new Assign(esp, esp));
        def1->setNumber(12);

        subscriptVarForStmt(ae, esp, def1);

        QCOMPARE(ae->toString(), QString("   1 *v* m[r28{12} - 4] := r28{12} + r29"));
    }

    {
        // Subtest 3: try to change to a different definition
        // Note: behaviour has changed. Now, we don't allow re-renaming, so it should stay the same

        // 99: r28 := 0
        SharedStmt def3(new Assign(esp, Const::get(0)));
        def3->setNumber(99);

        subscriptVarForStmt(ae, esp, def3);

        QCOMPARE(ae->toString(), QString("   1 *v* m[r28{12} - 4] := r28{12} + r29"));
    }
}


QTEST_GUILESS_MAIN(StmtSubscripterTest)
