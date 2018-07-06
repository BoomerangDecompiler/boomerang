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



#include "boomerang/db/RTL.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Module.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/Ternary.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/statements/BoolAssign.h"
#include "boomerang/db/statements/GotoStatement.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/statements/CaseStatement.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/visitor/stmtvisitor/StmtConscriptSetter.h"
#include "boomerang/frontend/pentium/pentiumfrontend.h"
#include "boomerang/frontend/sparc/sparcfrontend.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/util/Log.h"

#include <sstream>


// #define SWITCH_SPARC    (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/sparc/switch_cc"))
// #define SWITCH_PENT     (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/switch_cc"))


void compareStrings(const QString& actual, const QString& expected)
{
    QStringList actualList = actual.split('\n');
    QStringList expectedList = expected.split('\n');

    for (int i = 0; i < std::min(actualList.length(), expectedList.length()); i++) {
        QCOMPARE(actualList[i], expectedList[i]);
    }

    QVERIFY(actualList.length() == expectedList.length());
}


void RtlTest::testAppend()
{
    Assign *a = new Assign(Location::regOf(REG_SPARC_O0), Binary::get(opPlus, Location::regOf(REG_SPARC_O1), Const::get(99)));
    RTL    r(Address::ZERO, { a });

    QString     res;
    QTextStream ost(&res);
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
    virtual bool visit(const RTL *) override
    {
        a = true;
        return false;
    }

    virtual bool visit(const GotoStatement *) override
    {
        b = true;
        return false;
    }

    virtual bool visit(const BranchStatement *) override
    {
        c = true;
        return false;
    }

    virtual bool visit(const CaseStatement *) override
    {
        d = true;
        return false;
    }

    virtual bool visit(const CallStatement *) override
    {
        e = true;
        return false;
    }

    virtual bool visit(const ReturnStatement *) override
    {
        f = true;
        return false;
    }

    virtual bool visit(const BoolAssign *) override
    {
        g = true;
        return false;
    }

    virtual bool visit(const Assign *) override
    {
        h = true;
        return false;
    }
};


void RtlTest::testVisitor()
{
    StmtVisitorStub *visitor = new StmtVisitorStub();

    /* jump stmt */
    GotoStatement *jump = new GotoStatement;

    jump->accept(visitor);
    QVERIFY(visitor->b);
    delete jump;

    /* branch stmt */
    BranchStatement *jcond = new BranchStatement;
    jcond->accept(visitor);
    QVERIFY(visitor->c);
    delete jcond;

    /* nway jump stmt */
    CaseStatement *nwayjump = new CaseStatement;
    nwayjump->accept(visitor);
    QVERIFY(visitor->d);
    delete nwayjump;

    /* call stmt */
    CallStatement *call = new CallStatement;
    call->accept(visitor);
    QVERIFY(visitor->e);
    delete call;

    /* return stmt */
    ReturnStatement *ret = new ReturnStatement;
    ret->accept(visitor);
    QVERIFY(visitor->f);
    delete ret;

    /* "bool" assgn */
    BoolAssign *scond = new BoolAssign(0);
    scond->accept(visitor);
    QVERIFY(visitor->g);
    delete scond;

    /* assignment stmt */
    Assign *as = new Assign;
    as->accept(visitor);
    QVERIFY(visitor->h);
    delete as;

    /* polymorphic */
    Statement *s = new CallStatement;
    s->accept(visitor);
    QVERIFY(visitor->e);
    delete s;

    /* cleanup */
    delete visitor;
}


// void RtlTest::testIsCompare() {
//    BinaryFileFactory bff;
//    BinaryFile *pBF = bff.Load(SWITCH_SPARC);
//    QVERIFY(pBF != 0);
//    QVERIFY(pBF->GetMachine() == MACHINE_SPARC);
//    Prog* prog = new Prog;
//    FrontEnd *pFE = new SparcFrontEnd(pBF, prog, &bff);
//    prog->setFrontEnd(pFE);

//    // Decode second instruction: "sub        %i0, 2, %o1"
//    int iReg;
//    Exp* eOperand = nullptr;
//    DecodeResult inst = pFE->decodeInstruction(Address(0x10910));
//    QVERIFY(inst.rtl != nullptr);
//    QVERIFY(inst.rtl->isCompare(iReg, eOperand) == false);

//    // Decode fifth instruction: "cmp        %o1, 5"
//    inst = pFE->decodeInstruction(0x1091c);
//    QVERIFY(inst.rtl != nullptr);
//    QVERIFY(inst.rtl->isCompare(iReg, eOperand) == true);
//    QCOMPARE(iReg,9);
//    QString expected("5");
//    std::ostringstream ost1;
//    eOperand->print(ost1);
//    QString actual(ost1.str());
//    QCOMPARE(actual,expected);

//    pBF->UnLoad();
//    delete pBF;
//    delete pFE;
//    pBF = bff.Load(SWITCH_PENT);
//    QVERIFY(pBF != 0);
//    QVERIFY(pBF->GetMachine() == MACHINE_PENTIUM);
//    pFE = new PentiumFrontEnd(pBF, prog, &bff);
//    prog->setFrontEnd(pFE);

//    // Decode fifth instruction: "cmp    $0x5,%eax"
//    inst = pFE->decodeInstruction(0x80488fb);
//    QVERIFY(inst.rtl != nullptr);
//    QVERIFY(inst.rtl->isCompare(iReg, eOperand) == true);
//    QCOMPARE(iReg,24);
//    std::ostringstream ost2;
//    eOperand->print(ost2);
//    actual = ost2.str();
//    QCOMPARE(actual,expected);

//    // Decode instruction: "add        $0x4,%esp"
//    inst = pFE->decodeInstruction(0x804890c);
//    QVERIFY(inst.rtl != nullptr);
//    QVERIFY(inst.rtl->isCompare(iReg, eOperand) == false);
//    pBF->UnLoad();
//    delete pFE;
// }


void RtlTest::testSetConscripts()
{
    Prog prog("fake_prog", &m_project);
    Module *module = prog.getOrInsertModule("test");
    Function *proc = module->createFunction("printf", Address(0x2000)); // Making it a true library function is problematic

    // m[1000] = m[1000] + 1000
    Statement *s1 = new Assign(Location::memOf(Const::get(1000), nullptr),
                               Binary::get(opPlus, Location::memOf(Const::get(1000), nullptr), Const::get(1000)));

    // "printf("max is %d", (local0 > 0) ? local0 : global1)

    CallStatement *s2   = new CallStatement();
    ReturnStatement calleeReturn;

    s2->setDestProc(proc);
    s2->setCalleeReturn(&calleeReturn); // So it's not a childless call
    SharedExp e1 = Const::get("max is %d");
    SharedExp e2 = std::make_shared<Ternary>(opTern, Binary::get(opGtr, Location::local("local0", nullptr), Const::get(0)),
                                             Location::local("local0", nullptr), Location::global("global1", nullptr));
    StatementList args;
    args.append(new Assign(Location::regOf(REG_SPARC_O0), e1));
    args.append(new Assign(Location::regOf(REG_SPARC_O1), e2));
    s2->setArguments(args);

    RTL rtl(Address(0x1000), { s1, s2 });
    StmtConscriptSetter sc(0, false);

    for (Statement *s : rtl) {
        s->accept(&sc);
    }

    QString     actual;
    QTextStream ost(&actual);
    rtl.print(ost);

    QString expected("0x00001000    0 *v* m[1000\\1\\] := m[1000\\2\\] + 1000\\3\\\n"
                    "              0 CALL printf(\n"
                    "                *v* r8 := \"max is %d\"\\4\\\n"
                    "                *v* r9 := (local0 > 0\\5\\) ? local0 : global1\n"
                    "              )\n"
                    "              Reaching definitions: \n"
                    "              Live variables: \n");

    compareStrings(actual, expected);
}


QTEST_GUILESS_MAIN(RtlTest)
