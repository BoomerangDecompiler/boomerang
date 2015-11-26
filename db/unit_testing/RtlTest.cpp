/***************************************************************************/ /**
  * \file       RtlTest.cc
  * OVERVIEW:   Provides the implementation for the RtlTest class, which
  *                tests the RTL and derived classes
  ******************************************************************************/
#include "RtlTest.h"
#include "statement.h"
#include "exp.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "sparcfrontend.h"
#include "pentiumfrontend.h"
#include "decoder.h"
#include "proc.h"
#include "prog.h"
#include "visitor.h"
#include "log.h"
#include "boomerang.h"

#include <sstream>

#define SWITCH_SPARC "tests/inputs/sparc/switch_cc"
#define SWITCH_PENT "tests/inputs/pentium/switch_cc"
static bool logset = false;
QString TEST_BASE;
QDir baseDir;
void RtlTest::initTestCase() {
    if (!logset) {
        TEST_BASE = QProcessEnvironment::systemEnvironment().value("BOOMERANG_TEST_BASE", "");
        baseDir = QDir(TEST_BASE);
        if (TEST_BASE.isEmpty()) {
            qWarning() << "BOOMERANG_TEST_BASE environment variable not set, will assume '..', many test may fail";
            TEST_BASE = "..";
            baseDir = QDir("..");
        }
        logset = true;
        Boomerang::get()->setProgPath(TEST_BASE);
        Boomerang::get()->setPluginPath(TEST_BASE + "/out");
        Boomerang::get()->setLogger(new NullLogger());
    }
}

/***************************************************************************/ /**
  * \fn        RtlTest::testAppend
  * OVERVIEW:        Test appendExp and printing of RTLs
  ******************************************************************************/
void RtlTest::testAppend() {
    Assign *a = new Assign(Location::regOf(8), new Binary(opPlus, Location::regOf(9), new Const(99)));
    RTL r;
    r.appendStmt(a);
    QString res;
    QTextStream ost(&res);
    r.print(ost);
    QString expected("00000000    0 *v* r8 := r9 + 99\n");
    QCOMPARE(res,expected);
    // No! appendExp does not copy the expression, so deleting the RTL will
    // delete the expression(s) in it.
    // Not sure if that's what we want...
    // delete a;
}

/***************************************************************************/ /**
  * \fn        RtlTest::testClone
  * OVERVIEW:        Test constructor from list of expressions; cloning of RTLs
  ******************************************************************************/
void RtlTest::testClone() {
    Assign *a1 = new Assign(Location::regOf(8), new Binary(opPlus, Location::regOf(9), new Const(99)));
    Assign *a2 = new Assign(IntegerType::get(16), new Location(opParam, new Const("x"), nullptr),
                            new Location(opParam, new Const("y"), nullptr));
    std::list<Instruction *> ls;
    ls.push_back(a1);
    ls.push_back(a2);
    RTL *r = new RTL(ADDRESS::g(0x1234), &ls);
    RTL *r2 = r->clone();
    QString act1,act2;
    QTextStream o1(&act1),o2(&act2);
    r->print(o1);
    delete r; // And r2 should still stand!
    r2->print(o2);
    delete r2;
    QString expected("00001234    0 *v* r8 := r9 + 99\n"
                         "            0 *j16* x := y\n");

    QCOMPARE(act1,expected);
    QCOMPARE(act2,expected);
}

/***************************************************************************/ /**
  * \fn        RtlTest::testVisitor
  * OVERVIEW:        Test the accept function for correct visiting behaviour.
  * NOTES:            Stub class to test.
  ******************************************************************************/

class StmtVisitorStub : public StmtVisitor {
  public:
    bool a, b, c, d, e, f, g, h;

    void clear() { a = b = c = d = e = f = g = h = false; }
    StmtVisitorStub() { clear(); }
    virtual ~StmtVisitorStub() {}
    virtual bool visit(RTL */*s*/) {
        a = true;
        return false;
    }
    virtual bool visit(GotoStatement */*s*/) {
        b = true;
        return false;
    }
    virtual bool visit(BranchStatement */*s*/) {
        c = true;
        return false;
    }
    virtual bool visit(CaseStatement */*s*/) {
        d = true;
        return false;
    }
    virtual bool visit(CallStatement */*s*/) {
        e = true;
        return false;
    }
    virtual bool visit(ReturnStatement */*s*/) {
        f = true;
        return false;
    }
    virtual bool visit(BoolAssign */*s*/) {
        g = true;
        return false;
    }
    virtual bool visit(Assign */*s*/) {
        h = true;
        return false;
    }
};

void RtlTest::testVisitor() {
    StmtVisitorStub *visitor = new StmtVisitorStub();

    //    /* rtl */
    //    RTL *rtl = new RTL();
    //    rtl->accept(visitor);
    //    QVERIFY(visitor->a);
    //    delete rtl;

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
    Instruction *s = new CallStatement;
    s->accept(visitor);
    QVERIFY(visitor->e);
    delete s;

    /* cleanup */
    delete visitor;
}

/***************************************************************************/ /**
  * \fn        RtlTest::testIsCompare
  * OVERVIEW:        Test the isCompare function
  ******************************************************************************/
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
//    DecodeResult inst = pFE->decodeInstruction(ADDRESS::g(0x10910));
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
//}

void RtlTest::testSetConscripts() {
    // m[1000] = m[1000] + 1000
    Instruction *s1 = new Assign(Location::memOf(new Const(1000), 0),
                               new Binary(opPlus, Location::memOf(new Const(1000), nullptr), new Const(1000)));

    // "printf("max is %d", (local0 > 0) ? local0 : global1)
    CallStatement *s2 = new CallStatement();
    Prog *p = new Prog("fake_prog");
    Module *m = p->getOrInsertModule("test");
    Function *proc = new UserProc(m, "printf", ADDRESS::g(0x2000)); // Making a true LibProc is problematic
    s2->setDestProc(proc);
    s2->setCalleeReturn(new ReturnStatement); // So it's not a childless call
    Exp *e1 = new Const("max is %d");
    Exp *e2 = new Ternary(opTern, new Binary(opGtr, Location::local("local0", nullptr), new Const(0)),
                          Location::local("local0", nullptr), Location::global("global1", nullptr));
    StatementList args;
    args.append(new Assign(Location::regOf(8), e1));
    args.append(new Assign(Location::regOf(9), e2));
    s2->setArguments(args);

    std::list<Instruction *> list;
    list.push_back(s1);
    list.push_back(s2);
    RTL *rtl = new RTL(ADDRESS::g(0x1000), &list);
    StmtConscriptSetter sc(0, false);
    for (Instruction *s : *rtl) {
        s->accept(&sc);
    }
    QString expected("00001000    0 *v* m[1000\\1\\] := m[1000\\2\\] + 1000\\3\\\n"
                         "            0 CALL printf(\n"
                         "                *v* r8 := \"max is %d\"\\4\\\n"
                         "                *v* r9 := (local0 > 0\\5\\) ? local0 : global1\n"
                         "              )\n"
                         "              Reaching definitions: \n"
                         "              Live variables: \n");

    QString actual;
    QTextStream ost(&actual);
    rtl->print(ost);
    QCOMPARE(actual,expected);
}

QTEST_MAIN(RtlTest)
