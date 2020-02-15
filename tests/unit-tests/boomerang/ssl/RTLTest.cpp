#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "RTLTest.h"

#include "boomerang/ssl/RTL.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/module/Module.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/statements/BoolAssign.h"
#include "boomerang/ssl/statements/GotoStatement.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/stmtvisitor/StmtVisitor.h"

#include <sstream>


void RTLTest::testAppend()
{
    std::shared_ptr<Assign> a(new Assign(Location::regOf(REG_X86_AL), Binary::get(opPlus, Location::regOf(REG_X86_CL), Const::get(99))));
    RTL    r(Address::ZERO, { a });

    QString     res;
    OStream ost(&res);
    r.print(ost);

    QCOMPARE(res, QString("0x00000000    0 *v* r8 := r9 + 99\n"));
}


class StmtVisitorStub : public StmtVisitor
{
public:
    bool a, b, c, d, e, f, g, h;

    void clear() { a = b = c = d = e = f = g = h = false; }
    StmtVisitorStub() { clear(); }
    virtual ~StmtVisitorStub() {}
    bool visit(const RTL *) override
    {
        a = true;
        return false;
    }

    bool visit(const GotoStatement *) override
    {
        b = true;
        return false;
    }

    bool visit(const BranchStatement *) override
    {
        c = true;
        return false;
    }

    bool visit(const CaseStatement *) override
    {
        d = true;
        return false;
    }

    bool visit(const CallStatement *) override
    {
        e = true;
        return false;
    }

    bool visit(const ReturnStatement *) override
    {
        f = true;
        return false;
    }

    bool visit(const BoolAssign *) override
    {
        g = true;
        return false;
    }

    bool visit(const Assign *) override
    {
        h = true;
        return false;
    }
};


void RTLTest::testVisitor()
{
    StmtVisitorStub *visitor = new StmtVisitorStub();

    /* jump stmt */
    GotoStatement *jump = new GotoStatement(Address(0x0800));

    jump->accept(visitor);
    QVERIFY(visitor->b);
    delete jump;

    /* branch stmt */
    BranchStatement *jcond = new BranchStatement(Address(0x0800));
    jcond->accept(visitor);
    QVERIFY(visitor->c);
    delete jcond;

    /* nway jump stmt */
    CaseStatement *nwayjump = new CaseStatement(Location::regOf(REG_X86_ECX));
    nwayjump->accept(visitor);
    QVERIFY(visitor->d);
    delete nwayjump;

    /* call stmt */
    CallStatement *call = new CallStatement(Address(0x1000));
    call->accept(visitor);
    QVERIFY(visitor->e);
    delete call;

    /* return stmt */
    ReturnStatement *ret = new ReturnStatement;
    ret->accept(visitor);
    QVERIFY(visitor->f);
    delete ret;

    /* "bool" assgn */
    BoolAssign *scond = new BoolAssign(Location::regOf(REG_X86_EAX), BranchType::JE, Location::regOf(REG_X86_ECX));
    scond->accept(visitor);
    QVERIFY(visitor->g);
    delete scond;

    /* assignment stmt */
    Assign *as = new Assign(Location::regOf(REG_X86_EAX), Const::get(0));
    as->accept(visitor);
    QVERIFY(visitor->h);
    delete as;

    /* polymorphic */
    Statement *s = new CallStatement(Address(0x1000));
    s->accept(visitor);
    QVERIFY(visitor->e);
    delete s;

    /* cleanup */
    delete visitor;
}


QTEST_GUILESS_MAIN(RTLTest)
