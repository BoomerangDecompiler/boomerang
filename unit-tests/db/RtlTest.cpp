/**
 * \file RtlTest.cpp
 * Provides the implementation for the RtlTest class, which
 * tests the RTL and derived classes
 */
#include "RtlTest.h"

#include "boomerang/core/Boomerang.h"

#include "boomerang/db/exp/Exp.h"
#include "boomerang/core/BinaryFileFactory.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/Proc.h"
#include "boomerang/db/Prog.h"

#include "boomerang/db/statements/BoolAssign.h"
#include "boomerang/db/statements/GotoStatement.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/statements/CaseStatement.h"
#include "boomerang/db/statements/CallStatement.h"

#include "boomerang/db/Visitor.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Log.h"

#include "boomerang-frontend/pentium/pentiumfrontend.h"
#include "boomerang-frontend/sparc/sparcfrontend.h"

#include <sstream>

#define SWITCH_SPARC    (BOOMERANG_TEST_BASE "/tests/inputs/sparc/switch_cc")
#define SWITCH_PENT     (BOOMERANG_TEST_BASE "/tests/inputs/pentium/switch_cc")


void RtlTest::initTestCase()
{
    Boomerang::get()->setDataDirectory(BOOMERANG_TEST_BASE);
}


void RtlTest::testAppend()
{
	Assign *a = new Assign(Location::regOf(8), Binary::get(opPlus, Location::regOf(9), Const::get(99)));
	RTL    r;

	r.appendStmt(a);
	QString     res;
	QTextStream ost(&res);
	r.print(ost);
	QString expected("00000000    0 *v* r8 := r9 + 99\n");
	QCOMPARE(res, expected);
	// No! appendExp does not copy the expression, so deleting the RTL will
	// delete the expression(s) in it.
	// Not sure if that's what we want...
	// delete a;
}


void RtlTest::testClone()
{
	Assign *a1 = new Assign(Location::regOf(8), Binary::get(opPlus, Location::regOf(9), Const::get(99)));
	Assign *a2 = new Assign(IntegerType::get(16), Location::get(opParam, Const::get("x"), nullptr),
							Location::get(opParam, Const::get("y"), nullptr));

	std::list<Instruction *> ls;
	ls.push_back(a1);
	ls.push_back(a2);
	RTL         *r = new RTL(Address(0x1234), &ls);
	RTL         *r2 = r->clone();
	QString     act1, act2;
	QTextStream o1(&act1), o2(&act2);
	r->print(o1);
	delete r; // And r2 should still stand!
	r2->print(o2);
	delete r2;
	QString expected("00001234    0 *v* r8 := r9 + 99\n"
					 "            0 *j16* x := y\n");

	QCOMPARE(act1, expected);
	QCOMPARE(act2, expected);
}


class StmtVisitorStub : public StmtVisitor
{
public:
	bool a, b, c, d, e, f, g, h;

	void clear() { a = b = c = d = e = f = g = h = false; }
	StmtVisitorStub() { clear(); }
	virtual ~StmtVisitorStub() {}
	virtual bool visit(RTL *) override
	{
		a = true;
		return false;
	}

	virtual bool visit(GotoStatement *) override
	{
		b = true;
		return false;
	}

	virtual bool visit(BranchStatement *) override
	{
		c = true;
		return false;
	}

	virtual bool visit(CaseStatement *) override
	{
		d = true;
		return false;
	}

	virtual bool visit(CallStatement *) override
	{
		e = true;
		return false;
	}

	virtual bool visit(ReturnStatement *) override
	{
		f = true;
		return false;
	}

	virtual bool visit(BoolAssign *) override
	{
		g = true;
		return false;
	}

	virtual bool visit(Assign *) override
	{
		h = true;
		return false;
	}
};

void RtlTest::testVisitor()
{
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
	// m[1000] = m[1000] + 1000
	Instruction *s1 = new Assign(Location::memOf(Const::get(1000), 0),
								 Binary::get(opPlus, Location::memOf(Const::get(1000), nullptr), Const::get(1000)));

	// "printf("max is %d", (local0 > 0) ? local0 : global1)
	CallStatement *s2   = new CallStatement();
	Prog          *p    = new Prog("fake_prog");
	Module        *m    = p->getOrInsertModule("test");
	Function      *proc = new UserProc(m, "printf", Address(0x2000)); // Making a true LibProc is problematic

	s2->setDestProc(proc);
	s2->setCalleeReturn(new ReturnStatement); // So it's not a childless call
	SharedExp e1 = Const::get("max is %d");
	SharedExp e2 = std::make_shared<Ternary>(opTern, Binary::get(opGtr, Location::local("local0", nullptr), Const::get(0)),
											 Location::local("local0", nullptr), Location::global("global1", nullptr));
	StatementList args;
	args.append(new Assign(Location::regOf(8), e1));
	args.append(new Assign(Location::regOf(9), e2));
	s2->setArguments(args);

	std::list<Instruction *> list;
	list.push_back(s1);
	list.push_back(s2);
	RTL                 *rtl = new RTL(Address(0x1000), &list);
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

	QString     actual;
	QTextStream ost(&actual);
	rtl->print(ost);
	QCOMPARE(actual, expected);
}


QTEST_MAIN(RtlTest)
