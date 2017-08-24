/**
 * \file StatementTest.cpp
 * Provides the implementation for the StatementTest class, which
 * tests the dataflow subsystems
 */

/*
 * $Revision$
 *
 * 14 Jan 03 - Trent: Created
 * 17 Apr 03 - Mike: Added testRecursion to track down a nasty bug
 * 06 Jul 05 - Mike: Split testAddUsedLocs into six separate tests for Assign ... Bool
 */

#include "StatementTest.h"
#include "boomerang/db/CFG.h"
#include "boomerang/core/Boomerang.h"

#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/Ternary.h"

#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/CaseStatement.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/BoolAssign.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Project.h"
#include "boomerang/db/Managed.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Log.h"
#include "boomerang-frontend/pentium/pentiumfrontend.h"

#include <sstream>
#include <map>


#define HELLO_PENTIUM      (BOOMERANG_TEST_BASE "/tests/inputs/pentium/hello")
#define GLOBAL1_PENTIUM    (BOOMERANG_TEST_BASE "/tests/inputs/pentium/global1")


void StatementTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "/lib/boomerang/");
}


void StatementTest::testEmpty()
{
	// Force "verbose" flag (-v)
	SETTING(vFlag) = true;

  	Boomerang *boo = Boomerang::get();
	boo->getSettings()->setOutputDirectory("./unit_test/");
	IProject& project = *Boomerang::get()->getOrCreateProject();
    project.loadBinaryFile(HELLO_PENTIUM);
	IFileLoader* loader = project.getBestLoader(HELLO_PENTIUM);
	QVERIFY(loader != nullptr);


	// create Prog
	Prog* prog     = new Prog("testEmpty");
	IFrontEnd *pFE = new PentiumFrontEnd(loader, prog);
	prog->setFrontEnd(pFE);

	Module *m = *prog->getModuleList().begin();
	QVERIFY(m != nullptr);

	// create UserProc
	UserProc *proc = (UserProc *)m->getOrInsertFunction("test", Address(0x00000123));

	// create CFG
	Cfg                      *cfg   = proc->getCFG();
	std::list<RTL *>         *pRtls = new std::list<RTL *>();
	std::list<Statement *> *ls    = new std::list<Statement *>;
	ls->push_back(new ReturnStatement);
	pRtls->push_back(new RTL(Address(0x00000123)));
	BasicBlock* bb = cfg->newBB(pRtls, BBType::Ret);

    cfg->setEntryBB(bb);
	proc->setDecoded(); // We manually "decoded"

	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);
	// print cfg to a string
    QString actual;
	QTextStream st(&actual);
	cfg->print(st);

	QCOMPARE(
        actual,
        QString(
            "Control Flow Graph:\n"
            "Ret BB:\n"
            "  in edges: \n"
			"  out edges: \n"
			"0x00000123\n\n"
        )
    );

	// clean up
	delete prog;
}


void StatementTest::testFlow()
{
	// create Prog
	Prog              *prog = new Prog("testFlow");

    IProject& project = *Boomerang::get()->getOrCreateProject();
    project.loadBinaryFile(HELLO_PENTIUM);
    IFileLoader* loader = project.getBestLoader(HELLO_PENTIUM);
	QVERIFY(loader != nullptr);
	IFrontEnd         *pFE = new PentiumFrontEnd(loader, prog);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);

	// create UserProc
	std::string name  = "test";
	UserProc    *proc = (UserProc *)prog->createProc(Address(0x00000123));
	proc->setSignature(Signature::instantiate(Platform::PENTIUM, CallConv::C, name.c_str()));
	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	Assign           *a     = new Assign(Location::regOf(24), std::make_shared<Const>(5));

	a->setProc(proc);
	a->setNumber(1);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	BasicBlock* first = cfg->newBB(pRtls, BBType::Fall);
	pRtls = new std::list<RTL *>();
	rtl   = new RTL(Address(0x00000123));

	ReturnStatement *rs = new ReturnStatement;
	rs->setNumber(2);
	a = new Assign(Location::regOf(24), std::make_shared<Const>(5));
	a->setProc(proc);
	rs->addReturn(a);
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	BasicBlock* ret = cfg->newBB(pRtls, BBType::Ret);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first); // Also sets exitBB; important!
	proc->setDecoded();
	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);
	// print cfg to a string
	QString actual;
    QTextStream st(&actual);

	cfg->print(st);

	// The assignment to 5 gets propagated into the return, and the assignment
	// to r24 is removed
	QString expected = "Control Flow Graph:\n"
                       "Fall BB:\n"
                       "  in edges: \n"
			           "  out edges: 0x00000123 \n"
			           "0x00000000\n"
			           "Ret BB:\n"
			           "  in edges: 0x00000000(0x00000000) \n"
			           "  out edges: \n"
                       "0x00000123    2 RET *v* r24 := 5\n"
			           "              Modifieds: \n"
			           "              Reaching definitions: r24=5\n\n";

	QCOMPARE(actual, expected);
	// clean up
	delete prog;
}


void StatementTest::testKill()
{
	// create Prog
	Prog              *prog = new Prog("testKill");

    IProject& project = *Boomerang::get()->getOrCreateProject();
    project.loadBinaryFile(HELLO_PENTIUM);
    IFileLoader* loader = project.getBestLoader(HELLO_PENTIUM);
	QVERIFY(loader != nullptr);

	IFrontEnd         *pFE = new PentiumFrontEnd(loader, prog);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);
	// create UserProc
	QString name  = "test";
	UserProc    *proc = (UserProc *)prog->createProc(Address(0x00000123));
	proc->setSignature(Signature::instantiate(Platform::PENTIUM, CallConv::C, name));

	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	Assign           *e     = new Assign(Location::regOf(24), Const::get(5));

	e->setNumber(1);
	e->setProc(proc);
	rtl->appendStmt(e);

    e = new Assign(Location::regOf(24), Const::get(6));
	e->setNumber(2);
	e->setProc(proc);
	rtl->appendStmt(e);

    pRtls->push_back(rtl);
	BasicBlock* first = cfg->newBB(pRtls, BBType::Fall);
	pRtls = new std::list<RTL *>();
	rtl   = new RTL(Address(0x00000123));
	ReturnStatement *rs = new ReturnStatement;
	rs->setNumber(3);
	e = new Assign(Location::regOf(24), Const::get(0));
	e->setProc(proc);
	rs->addReturn(e);
	rtl->appendStmt(rs);

	pRtls->push_back(rtl);
	BasicBlock* ret = cfg->newBB(pRtls, BBType::Ret);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);

	// print cfg to a string
	QString actual;
    QTextStream st(&actual);

	cfg->print(st);
	QString expected =
               "Control Flow Graph:\n"
               "Fall BB:\n"
               "  in edges: \n"
			   "  out edges: 0x00000123 \n"
			   "0x00000000\n"
			   "Ret BB:\n"
			   "  in edges: 0x00000000(0x00000000) \n"
			   "  out edges: \n"
			   "0x00000123    3 RET *v* r24 := 0\n"
			   "              Modifieds: \n"
			   "              Reaching definitions: r24=6\n\n";

    QCOMPARE(actual, expected);

	// clean up
	delete prog;
}


void StatementTest::testUse()
{
	// create Prog
	Prog              *prog = new Prog("testUse");

    IProject& project = *Boomerang::get()->getOrCreateProject();
    project.loadBinaryFile(HELLO_PENTIUM);
    IFileLoader* loader = project.getBestLoader(HELLO_PENTIUM);
	QVERIFY(loader != nullptr);
	IFrontEnd         *pFE = new PentiumFrontEnd(loader, prog);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);
	// create UserProc
	std::string name  = "test";
	UserProc    *proc = (UserProc *)prog->createProc(Address(0x00000123));
	proc->setSignature(Signature::instantiate(Platform::PENTIUM, CallConv::C, name.c_str()));
	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	Assign           *a     = new Assign(Location::regOf(24), Const::get(5));
	a->setNumber(1);
	a->setProc(proc);
	rtl->appendStmt(a);

	a = new Assign(Location::regOf(28), Location::regOf(24));
	a->setNumber(2);
	a->setProc(proc);
	rtl->appendStmt(a);

	pRtls->push_back(rtl);
	BasicBlock* first = cfg->newBB(pRtls, BBType::Fall);
	pRtls = new std::list<RTL *>();
	rtl   = new RTL(Address(0x00000123));
	ReturnStatement *rs = new ReturnStatement;
	rs->setNumber(3);
	a = new Assign(Location::regOf(28), Const::get(1000));
	a->setProc(proc);
	rs->addReturn(a);
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	BasicBlock* ret = cfg->newBB(pRtls, BBType::Ret);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();

	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);
	// print cfg to a string
    QString actual;
	QTextStream st(&actual);
	cfg->print(st);

	QString expected =
               "Control Flow Graph:\n"
               "Fall BB:\n"
			   "  in edges: \n"
			   "  out edges: 0x00000123 \n"
			   "0x00000000\n"
			   "Ret BB:\n"
			   "  in edges: 0x00000000(0x00000000) \n"
			   "  out edges: \n"
			   "0x00000123    3 RET *v* r28 := 1000\n"
			   "              Modifieds: \n"
			   "              Reaching definitions: r24=5,   r28=5\n\n";

	QCOMPARE(actual, expected);
	// clean up
	delete prog;
}


void StatementTest::testUseOverKill()
{
	// create Prog
	Prog              *prog = new Prog("testUseOverKill");

    IProject& project = *Boomerang::get()->getOrCreateProject();
    project.loadBinaryFile(HELLO_PENTIUM);
    IFileLoader* loader = project.getBestLoader(HELLO_PENTIUM);
	QVERIFY(loader != nullptr);

	IFrontEnd         *pFE = new PentiumFrontEnd(loader, prog);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);
	// create UserProc
	std::string name  = "test";
	UserProc    *proc = (UserProc *)prog->createProc(Address(0x00000123));
	proc->setSignature(Signature::instantiate(Platform::PENTIUM, CallConv::C, name.c_str()));
	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	Assign           *e     = new Assign(Location::regOf(24), Const::get(5));
	e->setNumber(1);
	e->setProc(proc);
	rtl->appendStmt(e);
	e = new Assign(Location::regOf(24), Const::get(6));
	e->setNumber(2);
	e->setProc(proc);
	rtl->appendStmt(e);
	e = new Assign(Location::regOf(28), Location::regOf(24));
	e->setNumber(3);
	e->setProc(proc);
	rtl->appendStmt(e);
	pRtls->push_back(rtl);

	BasicBlock* first = cfg->newBB(pRtls, BBType::Fall);
	pRtls = new std::list<RTL *>();
	rtl   = new RTL(Address(0x00000123));
	ReturnStatement *rs = new ReturnStatement;
	rs->setNumber(4);
	e = new Assign(Location::regOf(24), Const::get(0));
	e->setProc(proc);
	rs->addReturn(e);
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);

	BasicBlock* ret = cfg->newBB(pRtls, BBType::Ret);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);

	// print cfg to a string
    QString actual;
	QTextStream st(&actual);
	cfg->print(st);

	// compare it to expected
	QString expected =
               "Control Flow Graph:\n"
               "Fall BB:\n"
			   "  in edges: \n"
			   "  out edges: 0x00000123 \n"
			   "0x00000000\n"
			   "Ret BB:\n"
			   "  in edges: 0x00000000(0x00000000) \n"
			   "  out edges: \n"
			   "0x00000123    4 RET *v* r24 := 0\n"
			   "              Modifieds: \n"
			   "              Reaching definitions: r24=6,   r28=6\n\n";

	QCOMPARE(actual, expected);
	// clean up
	delete prog;
}


void StatementTest::testUseOverBB()
{
	// create Prog
	Prog              *prog = new Prog("testUseOverBB");

    IProject& project = *Boomerang::get()->getOrCreateProject();
    project.loadBinaryFile(HELLO_PENTIUM);
    IFileLoader* pBF = project.getBestLoader(HELLO_PENTIUM);
	QVERIFY(pBF != nullptr);

	IFrontEnd *pFE = new PentiumFrontEnd(pBF, prog);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);
	// create UserProc
	std::string name  = "test";
	UserProc    *proc = (UserProc *)prog->createProc(Address(0x00000123));
	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	Assign           *a     = new Assign(Location::regOf(24), Const::get(5));
	a->setNumber(1);
	a->setProc(proc);
	rtl->appendStmt(a);
	a = new Assign(Location::regOf(24), Const::get(6));
	a->setNumber(2);
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	BasicBlock* first = cfg->newBB(pRtls, BBType::Fall);
	pRtls = new std::list<RTL *>();
	rtl   = new RTL();
	a     = new Assign(Location::regOf(28), Location::regOf(24));
	a->setNumber(3);
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	rtl = new RTL(Address(0x00000123));

	ReturnStatement *rs = new ReturnStatement;
	rs->setNumber(4);
	a = new Assign(Location::regOf(24), Const::get(0));
	a->setProc(proc);
	rs->addReturn(a);
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	BasicBlock* ret = cfg->newBB(pRtls, BBType::Ret);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);
	// print cfg to a string

    QString actual;
	QTextStream st(&actual);
	cfg->print(st);

	QString expected =
               "Control Flow Graph:\n"
               "Fall BB:\n"
			   "  in edges: \n"
			   "  out edges: 0x00000123 \n"
			   "0x00000000\n"
			   "Ret BB:\n"
			   "  in edges: 0x00000000(0x00000000) \n"
			   "  out edges: \n"
			   "0x00000000\n"
			   "0x00000123    4 RET *v* r24 := 0\n"
			   "              Modifieds: \n"
			   "              Reaching definitions: r24=6,   r28=6\n\n";

	QCOMPARE(actual, expected);
	// clean up
	delete prog;
}


void StatementTest::testUseKill()
{
	// create Prog
	Prog              *prog = new Prog("testUseKill");

    IProject& project = *Boomerang::get()->getOrCreateProject();
    project.loadBinaryFile(HELLO_PENTIUM);
    IFileLoader* loader = project.getBestLoader(HELLO_PENTIUM);
	QVERIFY(loader != nullptr);

	IFrontEnd         *pFE = new PentiumFrontEnd(loader, prog);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);
	// create UserProc
	std::string name  = "test";
	UserProc    *proc = (UserProc *)prog->createProc(Address(0x00000123));
	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	Assign           *a     = new Assign(Location::regOf(24), Const::get(5));
	a->setNumber(1);
	a->setProc(proc);
	rtl->appendStmt(a);
	a = new Assign(Location::regOf(24), Binary::get(opPlus, Location::regOf(24), Const::get(1)));
	a->setNumber(2);
	a->setProc(proc);
	rtl->appendStmt(a);

	pRtls->push_back(rtl);
	BasicBlock* first = cfg->newBB(pRtls, BBType::Fall);
	pRtls = new std::list<RTL *>();
	rtl   = new RTL(Address(0x00000123));
	ReturnStatement *rs = new ReturnStatement;
	rs->setNumber(3);
	a = new Assign(Location::regOf(24), Const::get(0));
	a->setProc(proc);
	rs->addReturn(a);
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	BasicBlock* ret = cfg->newBB(pRtls, BBType::Ret);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);
	// print cfg to a string

    QString actual;
    QTextStream st(&actual);
	cfg->print(st);

	// compare it to expected
	QString expected =
               "Control Flow Graph:\n"
               "Fall BB:\n"
			   "  in edges: \n"
			   "  out edges: 0x00000123 \n"
			   "0x00000000\n"
			   "Ret BB:\n"
			   "  in edges: 0x00000000(0x00000000) \n"
			   "  out edges: \n"
			   "0x00000123    3 RET *v* r24 := 0\n"
			   "              Modifieds: \n"
			   "              Reaching definitions: r24=6\n\n";

	QCOMPARE(actual, expected);
	// clean up
	delete prog;
}


void StatementTest::testEndlessLoop()
{
    QSKIP("Disabled.");

    // create Prog
	Prog              *prog = new Prog("testEndlessLoop");

    IProject& project = *Boomerang::get()->getOrCreateProject();
    project.loadBinaryFile(HELLO_PENTIUM);
    IFileLoader* loader = project.getBestLoader(HELLO_PENTIUM);
	QVERIFY(loader != nullptr);

	IFrontEnd         *pFE = new PentiumFrontEnd(loader, prog);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);
	// create UserProc
	std::string name  = "test";
	UserProc    *proc = (UserProc *)prog->createProc(Address(0x00000123));

	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	// r[24] := 5
	Assign *e = new Assign(Location::regOf(24), Const::get(5));
	e->setProc(proc);
	rtl->appendStmt(e);
	pRtls->push_back(rtl);
	BasicBlock* first = cfg->newBB(pRtls, BBType::Fall);
	pRtls = new std::list<RTL *>();
	rtl   = new RTL();
	// r[24] := r[24] + 1
	e = new Assign(Location::regOf(24), Binary::get(opPlus, Location::regOf(24), Const::get(1)));
	e->setProc(proc);
	rtl->appendStmt(e);
	pRtls->push_back(rtl);
	BasicBlock* body = cfg->newBB(pRtls, BBType::Oneway);
	first->setOutEdge(0, body);
	body->addInEdge(first);
	body->setOutEdge(0, body);
	body->addInEdge(body);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);

    QString actual;
    QTextStream st(&actual);

	// print cfg to a string
	cfg->print(st);

	QString expected = "Fall BB: reach in: \n"
			   "0x00000000 *v* r[24] := 5\n"
			   "Oneway BB:\n"
			   "0x00000000 *v* r[24] := r[24] + 1   uses: ** r[24] := 5, "
			   "*v* r[24] := r[24] + 1,    used by: ** r[24] := r[24] + 1, \n"
			   "cfg reachExit: \n";
	QCOMPARE(actual, expected);
	// clean up
	delete prog;
}


void StatementTest::testLocationSet()
{
	Location    rof(opRegOf, Const::get(12), nullptr); // r12
	Const&      theReg = *std::dynamic_pointer_cast<Const>(rof.getSubExp1());
	LocationSet ls;


	ls.insert(rof.clone()); // ls has r12
	theReg.setInt(8);
	ls.insert(rof.clone()); // ls has r8 r12
	theReg.setInt(31);
	ls.insert(rof.clone()); // ls has r8 r12 r31
	theReg.setInt(24);
	ls.insert(rof.clone()); // ls has r8 r12 r24 r31
	theReg.setInt(12);
	ls.insert(rof.clone()); // Note: r12 already inserted

	QCOMPARE(ls.size(), (size_t)4);
	theReg.setInt(8);
	auto ii = ls.begin();
	QVERIFY(rof == **ii); // First element should be r8

	theReg.setInt(12);
	SharedExp e = *(++ii);
	QVERIFY(rof == *e); // Second should be r12

	theReg.setInt(24);
	e = *(++ii);
	QVERIFY(rof == *e); // Next should be r24
	theReg.setInt(31);
	e = *(++ii);
	QVERIFY(rof == *e); // Last should be r31

	Location mof(opMemOf, Binary::get(opPlus, Location::regOf(14), Const::get(4)), nullptr); // m[r14 + 4]
	ls.insert(mof.clone());                                                                  // ls should be r8 r12 r24 r31 m[r14 + 4]
	ls.insert(mof.clone());

	QCOMPARE(ls.size(), (size_t)5); // Should have 5 elements

    ii = --ls.end();
	QVERIFY(mof == **ii);   // Last element should be m[r14 + 4] now
	LocationSet ls2 = ls;
	SharedExp e2 = *ls2.begin();
	QVERIFY(!(e2 == *ls.begin())); // Must be cloned
	QCOMPARE(ls2.size(), (size_t)5);

	theReg.setInt(8);
	QVERIFY(rof == **ls2.begin()); // First elements should compare equal

    theReg.setInt(12);
	e = *(++ls2.begin());                 // Second element
	QVERIFY(rof == *e);            // ... should be r12

    Assign s10(Const::get(0), Const::get(0));
    Assign s20(Const::get(0), Const::get(0));
	s10.setNumber(10);
	s20.setNumber(20);

    std::shared_ptr<RefExp> r1 = RefExp::get(Location::regOf(8), &s10);
	std::shared_ptr<RefExp> r2 = RefExp::get(Location::regOf(8), &s20);
	ls.insert(r1); // ls now m[r14 + 4] r8 r12 r24 r31 r8{10} (not sure where r8{10} appears)

	QCOMPARE(ls.size(), (size_t)6);
	SharedExp dummy;
	QVERIFY(!ls.findDifferentRef(r1, dummy));
	QVERIFY(ls.findDifferentRef(r2, dummy));

    SharedExp r8 = Location::regOf(8);
    QVERIFY(!ls.existsImplicit(r8));

	std::shared_ptr<RefExp> r3(new RefExp(Location::regOf(8), nullptr));
	ls.insert(r3);
	QVERIFY(ls.existsImplicit(r8));
	ls.remove(r3);

	ImplicitAssign zero(r8);
	std::shared_ptr<RefExp> r4(new RefExp(Location::regOf(8), &zero));
	ls.insert(r4);
	QVERIFY(ls.existsImplicit(r8));
}


void StatementTest::testWildLocationSet()
{
	Location rof12(opRegOf, Const::get(12), nullptr);
	Location rof13(opRegOf, Const::get(13), nullptr);
	Assign   a10, a20;

	a10.setNumber(10);
	a20.setNumber(20);
	std::shared_ptr<RefExp> r12_10(new RefExp(rof12.clone(), &a10));
	std::shared_ptr<RefExp> r12_20(new RefExp(rof12.clone(), &a20));
	std::shared_ptr<RefExp> r12_0(new RefExp(rof12.clone(), nullptr));
	std::shared_ptr<RefExp> r13_10(new RefExp(rof13.clone(), &a10));
	std::shared_ptr<RefExp> r13_20(new RefExp(rof13.clone(), &a20));
	std::shared_ptr<RefExp> r13_0(new RefExp(rof13.clone(), nullptr));
	std::shared_ptr<RefExp> r11_10(new RefExp(Location::regOf(11), &a10));
	std::shared_ptr<RefExp> r22_10(new RefExp(Location::regOf(22), &a10));

	LocationSet ls;
	ls.insert(r12_10);
	ls.insert(r12_20);
	ls.insert(r12_0);
	ls.insert(r13_10);
	ls.insert(r13_20);
	ls.insert(r13_0);
	std::shared_ptr<RefExp> wildr12(new RefExp(rof12.clone(), (Statement *)-1));
	QVERIFY(ls.exists(wildr12));
	std::shared_ptr<RefExp> wildr13(new RefExp(rof13.clone(), (Statement *)-1));
	QVERIFY(ls.exists(wildr13));
	std::shared_ptr<RefExp> wildr10(new RefExp(Location::regOf(10), (Statement *)-1));
	QVERIFY(!ls.exists(wildr10));

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
    QSKIP("Disabled.");

	// create Prog
	Prog              *prog = new Prog("testRecursion");

    IProject& project = *Boomerang::get()->getOrCreateProject();
    project.loadBinaryFile(HELLO_PENTIUM);
    IFileLoader* pBF = project.getBestLoader(HELLO_PENTIUM);
	QVERIFY(pBF != nullptr);

	IFrontEnd         *pFE = new PentiumFrontEnd(pBF, prog);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);

	// create UserProc;

	UserProc    *proc = new UserProc(Address::ZERO, "test", prog->getOrInsertModule("test"));

	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	// push bp
	// r28 := r28 + -4
	Assign *a = new Assign(Location::regOf(28), Binary::get(opPlus, Location::regOf(28), Const::get(-4)));
	rtl->appendStmt(a);
	// m[r28] := r29
	a = new Assign(Location::memOf(Location::regOf(28)), Location::regOf(29));
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	pRtls = new std::list<RTL *>();
	// push arg+1
	// r28 := r28 + -4
	a = new Assign(Location::regOf(28), Binary::get(opPlus, Location::regOf(28), Const::get(-4)));
	rtl->appendStmt(a);
	// Reference our parameter. At esp+0 is this arg; at esp+4 is old bp;
	// esp+8 is return address; esp+12 is our arg
	// m[r28] := m[r28+12] + 1
	a = new Assign(
		Location::memOf(Location::regOf(28)),
		Binary::get(opPlus, Location::memOf(
            Binary::get(opPlus, Location::regOf(28), Const::get(12))), Const::get(1)));

	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	BasicBlock* first = cfg->newBB(pRtls, BBType::Fall);

	// The call BB
	pRtls = new std::list<RTL *>();
	rtl   = new RTL(Address(0x0000001));
	// r28 := r28 + -4
	a = new Assign(Location::regOf(28), Binary::get(opPlus, Location::regOf(28), Const::get(-4)));
	rtl->appendStmt(a);
	// m[r28] := pc
	a = new Assign(Location::memOf(Location::regOf(28)), Terminal::get(opPC));
	rtl->appendStmt(a);
	// %pc := (%pc + 5) + 135893848
	a = new Assign(Terminal::get(opPC),
				   Binary::get(opPlus,
                       Binary::get(opPlus, Terminal::get(opPC), Const::get(5)),
                               Const::get(135893848)));
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	CallStatement *c = new CallStatement;
	rtl->appendStmt(c);

	c->setDestProc(proc); // Just call self
	BasicBlock* callbb = cfg->newBB(pRtls, BBType::Call);
	first->setOutEdge(0, callbb);
	callbb->addInEdge(first);
	callbb->setOutEdge(0, callbb);
	callbb->addInEdge(callbb);

	pRtls = new std::list<RTL *>();
	rtl   = new RTL(Address(0x00000123));
	rtl->appendStmt(new ReturnStatement);
	// This ReturnStatement requires the following two sets of semantics to pass the
	// tests for standard Pentium calling convention
	// pc = m[r28]
	a = new Assign(Terminal::get(opPC), Location::memOf(Location::regOf(28)));
	rtl->appendStmt(a);
	// r28 = r28 + 4
	a = new Assign(Location::regOf(28), Binary::get(opPlus, Location::regOf(28), Const::get(4)));
	rtl->appendStmt(a);
	pRtls->push_back(rtl);

	BasicBlock* ret = cfg->newBB(pRtls, BBType::Ret);
	callbb->setOutEdge(0, ret);
	ret->addInEdge(callbb);
	cfg->setEntryBB(first);

	// decompile the "proc"
	prog->decompile();

    // print cfg to a string
    QString actual;
    QTextStream st(&actual);
	cfg->print(st);

	QString expected = "Fall BB: reach in: \n"
			   "00000000 ** r[24] := 5   uses:    used by: ** r[24] := r[24] + 1, \n"
			   "00000000 ** r[24] := 5   uses:    used by: ** r[24] := r[24] + 1, \n"
			   "Call BB: reach in: ** r[24] := 5, ** r[24] := r[24] + 1, \n"
			   "00000001 ** r[24] := r[24] + 1   uses: ** r[24] := 5, "
			   "** r[24] := r[24] + 1,    used by: ** r[24] := r[24] + 1, \n"
			   "cfg reachExit: \n";
	QCOMPARE(actual, expected);
	// clean up
	delete prog;
}


void StatementTest::testClone()
{
	Assign *a1 = new Assign(Location::regOf(8), Binary::get(opPlus, Location::regOf(9), Const::get(99)));
	Assign *a2 = new Assign(IntegerType::get(16, 1), Location::get(opParam, Const::get("x"), nullptr),
							Location::get(opParam, Const::get("y"), nullptr));
	Assign *a3 = new Assign(IntegerType::get(16, -1), Location::get(opParam, Const::get("z"), nullptr),
							Location::get(opParam, Const::get("q"), nullptr));

	   Statement* c1 = a1->clone();
	   Statement* c2 = a2->clone();
	   Statement* c3 = a3->clone();

    QString original, clone;
    QTextStream original_st(&original);
    QTextStream clone_st(&clone);

	a1->print(original_st);
	delete a1; // And c1 should still stand!
	c1->print(clone_st);
	a2->print(original_st);
	c2->print(clone_st);
	a3->print(original_st);
	c3->print(clone_st);

	QString expected("   0 *v* r8 := r9 + 99   0 *i16* x := y"
                     "   0 *u16* z := q");

    QCOMPARE(original, expected);
    QCOMPARE(clone, expected);
}


void StatementTest::testIsAssign()
{
    QString actual;
	QTextStream st(&actual);
    // r2 := 99
	Assign a(Location::regOf(2), Const::get(99));
	a.print(st);
	QString expected("   0 *v* r2 := 99");

	QCOMPARE(expected, actual);
	QVERIFY(a.isAssign());

	CallStatement *c = new CallStatement;
	QVERIFY(!c->isAssign());
}


void StatementTest::testIsFlagAssgn()
{
    // FLAG addFlags(r2 , 99)
	Assign fc(Terminal::get(opFlags),
			  Binary::get(opFlagCall, Const::get("addFlags"),
                          Binary::get(opList, Location::regOf(2), Const::get(99))));
	CallStatement   *call = new CallStatement;
	BranchStatement *br   = new BranchStatement;
	Assign          *as   = new Assign(Location::regOf(9), Binary::get(opPlus, Location::regOf(10), Const::get(4)));

    QString actual;
    QString expected("   0 *v* %flags := addFlags( r2, 99 )");
    QTextStream ost(&actual);
    fc.print(ost);

    QCOMPARE(expected, actual);
	QVERIFY(fc.isFlagAssign());
	QVERIFY(!call->isFlagAssign());
	QVERIFY(!br->isFlagAssign());
	QVERIFY(!as->isFlagAssign());
	delete call;
	delete br;
}


void StatementTest::testAddUsedLocsAssign()
{
	// m[r28-4] := m[r28-8] * r26
	Assign *a = new Assign(Location::memOf(Binary::get(opMinus, Location::regOf(28), Const::get(4))),
						   Binary::get(opMult, Location::memOf(Binary::get(opMinus, Location::regOf(28), Const::get(8))),
									  Location::regOf(26)));

	a->setNumber(1);
	LocationSet l;
	a->addUsedLocs(l);

    QString actual;
    QTextStream ost(&actual);
	l.print(ost);
	QString expected = "r26,\tr28,\tm[r28 - 8]";
    QCOMPARE(expected, actual);

	l.clear();
	GotoStatement *g = new GotoStatement();
	g->setNumber(55);
	g->setDest(Location::memOf(Location::regOf(26)));
	g->addUsedLocs(l);

    actual = "";
    expected = "r26,\tm[r26]";
    l.print(ost);

	QCOMPARE(expected, actual);
}


void StatementTest::testAddUsedLocsBranch()
{
	// BranchStatement with dest m[r26{99}]{55}, condition %flags
	GotoStatement *g = new GotoStatement();

	g->setNumber(55);
	LocationSet     l;
	BranchStatement *b = new BranchStatement;
	b->setNumber(99);
	b->setDest(RefExp::get(Location::memOf(RefExp::get(Location::regOf(26), b)), g));
	b->setCondExpr(Terminal::get(opFlags));
	b->addUsedLocs(l);

    QString actual;
	QString expected("r26{99},\tm[r26{99}]{55},\t%flags");
    QTextStream ost(&actual);
    l.print(ost);
	QCOMPARE(expected, actual);
}


void StatementTest::testAddUsedLocsCase()
{
	// CaseStatement with pDest = m[r26], switchVar = m[r28 - 12]
	LocationSet   l;
	CaseStatement *c = new CaseStatement;

	c->setDest(Location::memOf(Location::regOf(26)));
	SWITCH_INFO si;
	si.pSwitchVar = Location::memOf(Binary::get(opMinus, Location::regOf(28), Const::get(12)));
	c->setSwitchInfo(&si);
	c->addUsedLocs(l);

    QString actual;
    QTextStream ost(&actual);
	l.print(ost);
	QString expected("r26,\tr28,\tm[r28 - 12],\tm[r26]");
	QCOMPARE(expected, actual);
}


void StatementTest::testAddUsedLocsCall()
{
	// CallStatement with pDest = m[r26], params = m[r27], r28{55}, defines r31, m[r24]
	LocationSet   l;
	GotoStatement *g = new GotoStatement();

	g->setNumber(55);
	CallStatement *ca = new CallStatement;
	ca->setDest(Location::memOf(Location::regOf(26)));
	StatementList argl;
	argl.append(new Assign(Location::regOf(8), Location::memOf(Location::regOf(27))));
	argl.append(new Assign(Location::regOf(9), RefExp::get(Location::regOf(28), g)));
	ca->setArguments(argl);
	ca->addDefine(new ImplicitAssign(Location::regOf(31)));
	ca->addDefine(new ImplicitAssign(Location::regOf(24)));
	ca->addUsedLocs(l);

    QString actual;
    QTextStream ost(&actual);
	l.print(ost);
	QString expected("r26,\tr27,\tm[r26],\tm[r27],\tr28{55}");
	QCOMPARE(expected, actual);

// Now with uses in collector

}


void StatementTest::testAddUsedLocsReturn()
{
	// ReturnStatement with returns r31, m[r24], m[r25]{55} + r[26]{99}]
	LocationSet   l;
	GotoStatement *g = new GotoStatement();

	g->setNumber(55);
	BranchStatement *b = new BranchStatement;
	b->setNumber(99);
	ReturnStatement *r = new ReturnStatement;
	r->addReturn(new Assign(Location::regOf(31), Const::get(100)));
	r->addReturn(new Assign(Location::memOf(Location::regOf(24)), Const::get(0)));
	r->addReturn(new Assign(
        Location::memOf(Binary::get(opPlus, RefExp::get(Location::regOf(25), g), RefExp::get(Location::regOf(26), b))),
                            Const::get(5)));
	r->addUsedLocs(l);

    QString actual;
    QTextStream ost(&actual);
	l.print(ost);

	QString expected("r24,\tr25{55},\tr26{99}");
	QCOMPARE(expected, actual);
}


void StatementTest::testAddUsedLocsBool()
{
    QSKIP("Disabled.");

	// Boolstatement with condition m[r24] = r25, dest m[r26]
	LocationSet l;
	BoolAssign  *bs = new BoolAssign(8);

	bs->setCondExpr(Binary::get(opEquals, Location::memOf(Location::regOf(24)), Location::regOf(25)));
	std::list<Statement *> stmts;
	Assign                 *a = new Assign(Location::memOf(Location::regOf(26)), Terminal::get(opNil));
	stmts.push_back(a);
	bs->setLeftFromList(&stmts);
	bs->addUsedLocs(l);

    QString actual;
    QTextStream ost(&actual);
    l.print(ost);

    QString expected("r24,\tr25,\tr26,\tm[r24]");
    QCOMPARE(actual, expected);

	// m[local21 + 16] := phi{0, 372}
	l.clear();
	SharedExp base = Location::memOf(Binary::get(opPlus, Location::local("local21", nullptr), Const::get(16)));
	Assign s372(base, Const::get(0));
	s372.setNumber(372);

	PhiAssign *pa = new PhiAssign(base);
	pa->putAt(nullptr, nullptr, base); // 0
	pa->putAt(nullptr, &s372, base);   // 1
	pa->addUsedLocs(l);
	// Note: phis were not considered to use blah if they ref m[blah], so local21 was not considered used

    actual = "";
	expected = "m[local21 + 16]{-},\tm[local21 + 16]{372},\tlocal21";
    l.print(ost);

    QCOMPARE(actual, expected);

	// m[r28{-} - 4] := -
	l.clear();
	ImplicitAssign *ia = new ImplicitAssign(
		Location::memOf(Binary::get(opMinus, RefExp::get(Location::regOf(28), nullptr), Const::get(4))));

    ia->addUsedLocs(l);
	actual = "";
	expected = "r28{-}";
    l.print(ost);
	QCOMPARE(actual, expected);
}


void StatementTest::testSubscriptVars()
{
	SharedExp srch = Location::regOf(28);
	Assign s9(Const::get(0), Const::get(0));
	s9.setNumber(9);

	// m[r28-4] := m[r28-8] * r26
	Assign *a = new Assign(Location::memOf(Binary::get(opMinus, Location::regOf(28), Const::get(4))),
						   Binary::get(opMult, Location::memOf(Binary::get(opMinus, Location::regOf(28), Const::get(8))),
									  Location::regOf(26)));
	a->setNumber(1);
    QString actual;
    QTextStream ost(&actual);

	a->subscriptVar(srch, &s9);
	ost << a;
	QString expected = "   1 *v* m[r28{9} - 4] := m[r28{9} - 8] * r26";
	QCOMPARE(expected, actual);

	// GotoStatement
	GotoStatement *g = new GotoStatement();
	g->setNumber(55);
	g->setDest(Location::regOf(28));
	g->subscriptVar(srch, &s9);

    actual = "";
    expected = "  55 GOTO r28{9}";
	ost << g;

    QCOMPARE(actual, expected);

	// BranchStatement with dest m[r26{99}]{55}, condition %flags
	BranchStatement *b = new BranchStatement;
	b->setNumber(99);
	SharedExp srchb = Location::memOf(RefExp::get(Location::regOf(26), b));
	b->setDest(RefExp::get(srchb, g));
	b->setCondExpr(Terminal::get(opFlags));

	b->subscriptVar(srchb, &s9); // Should be ignored now: new behaviour
	b->subscriptVar(Terminal::get(opFlags), g);

    actual = "";
    expected = "  99 BRANCH m[r26{99}]{55}, condition equals\n"
			   "High level: %flags{55}";
    ost << b;
    QCOMPARE(actual, expected);

	// CaseStatement with pDest = m[r26], switchVar = m[r28 - 12]
	CaseStatement *c = new CaseStatement;
	c->setDest(Location::memOf(Location::regOf(26)));
	SWITCH_INFO si;
	si.pSwitchVar = Location::memOf(Binary::get(opMinus, Location::regOf(28), Const::get(12)));
	c->setSwitchInfo(&si);

    c->subscriptVar(srch, &s9);

    actual = "";
	expected = "   0 SWITCH(m[r28{9} - 12])\n";
    ost << c;
    QCOMPARE(actual, expected);

	// CaseStatement (before recog) with pDest = r28, switchVar is nullptr
	c->setDest(Location::regOf(28));
	c->setSwitchInfo(nullptr);

	c->subscriptVar(srch, &s9);
	actual = "";
    expected = "   0 CASE [r28{9}]";
	ost << c;
    QCOMPARE(expected, actual);

	// CallStatement with pDest = m[r26], params = m[r27], r28, defines r28, m[r28]
	CallStatement *ca = new CallStatement;
	ca->setDest(Location::memOf(Location::regOf(26)));
	StatementList argl;

    Prog* prog = new Prog("testSubscriptVars");
    Module* mod = prog->getDefaultModule("test");

    argl.append(new Assign(Location::memOf(Location::regOf(27)), Const::get(1)));
	argl.append(new Assign(Location::regOf(28), Const::get(2)));
	ca->setArguments(argl);
	ca->addDefine(new ImplicitAssign(Location::regOf(28)));
	ca->addDefine(new ImplicitAssign(Location::memOf(Location::regOf(28))));

	ca->setDestProc(new UserProc(Address(0x00002000), "dest", mod)); // Must have a dest to be non-childless
	ca->setCalleeReturn(new ReturnStatement);                // So it's not a childless call, and we can see the defs and params
	ca->subscriptVar(srch, &s9);

    actual = "";
    expected = "   0 {*v* r28, *v* m[r28]} := CALL dest(\n"
			   "                *v* m[r27] := 1\n"
			   "                *v* r28 := 2\n"
			   "              )\n"
			   "              Reaching definitions: \n"
			   "              Live variables: "; // ? No newline?
	ost << ca;
	QCOMPARE(expected, actual);

	// CallStatement with pDest = r28, params = m[r27], r29, defines r31, m[r31]
	ca = new CallStatement;
	ca->setDest(Location::regOf(28));
	argl.clear();
	argl.append(new Assign(Location::memOf(Location::regOf(27)), Const::get(1)));
	argl.append(new Assign(Location::regOf(29), Const::get(2)));
	ca->setArguments(argl);
	ca->addDefine(new ImplicitAssign(Location::regOf(31)));
	ca->addDefine(new ImplicitAssign(Location::memOf(Location::regOf(31))));
	ca->setDestProc(new UserProc(Address(0x00002000), "dest", mod)); // Must have a dest to be non-childless
	ca->setCalleeReturn(new ReturnStatement);                // So it's not a childless call, and we can see the defs and params
	ca->subscriptVar(srch, &s9);

	actual = "";
    expected = "   0 {*v* r31, *v* m[r31]} := CALL dest(\n"
			   "                *v* m[r27] := 1\n"
			   "                *v* r29 := 2\n"
			   "              )\n"
			   "              Reaching definitions: \n"
			   "              Live variables: ";
    ost << ca;

    QCOMPARE(actual, expected);

	// ReturnStatement with returns r28, m[r28], m[r28]{55} + r[26]{99}]
	// FIXME: shouldn't this test have some propagation? Now, it seems it's just testing the print code!
	ReturnStatement *r = new ReturnStatement;
	r->addReturn(new Assign(Location::regOf(28), Const::get(1000)));
	r->addReturn(new Assign(Location::memOf(Location::regOf(28)), Const::get(2000)));
	r->addReturn(new Assign(
					 Location::memOf(Binary::get(opPlus, RefExp::get(Location::regOf(28), g),
                                                 RefExp::get(Location::regOf(26), b))),
					Const::get(100)));

	r->subscriptVar(srch, &s9); // New behaviour: gets ignored now

    actual = "";
    expected = "   0 RET *v* r28 := 1000,   *v* m[r28{9}] := 0x7d0,   *v* m[r28{55} + r26{99}] := 100\n"
			   "              Modifieds: \n"
			   "              Reaching definitions: ";
	ost << r;
	QCOMPARE(actual, expected);

	// Boolstatement with condition m[r28] = r28, dest m[r28]
	BoolAssign *bs = new BoolAssign(8);
	bs->setCondExpr(Binary::get(opEquals, Location::memOf(Location::regOf(28)), Location::regOf(28)));
	bs->setLeft(Location::memOf(Location::regOf(28)));

	bs->subscriptVar(srch, &s9);

	actual = "";
    expected = "   0 BOOL m[r28{9}] := CC(equals)\n"
			   "High level: m[r28{9}] = r28{9}\n";
	ost << bs;
    QCOMPARE(actual, expected);
}


void StatementTest::testBypass()
{
    QSKIP("Disabled.");

	Prog              *prog = new Prog("testBypass");

    IProject& project = *Boomerang::get()->getOrCreateProject();
    project.loadBinaryFile(GLOBAL1_PENTIUM);
    IFileLoader* pBF = project.getBestLoader(GLOBAL1_PENTIUM);
	QVERIFY(pBF != nullptr);

	IFrontEnd         *pFE = new PentiumFrontEnd(pBF, prog);

	Type::clearNamedTypes();
	prog->setFrontEnd(pFE);
	pFE->decode(prog, true);       // Decode main
	pFE->decode(prog, Address::INVALID); // Decode anything undecoded

    bool    gotMain;
	Address addr = pFE->getMainEntryPoint(gotMain);
	QVERIFY(addr != Address::INVALID);

    UserProc *proc = (UserProc *)prog->findProc("foo2");
	QVERIFY(proc);

    proc->promoteSignature(); // Make sure it's a PentiumSignature (needed for bypassing)
	Cfg *cfg = proc->getCFG();
	// Sort by address
	cfg->sortByAddress();
	// Initialise statements
	proc->initStatements();
	// Compute dominance frontier
	proc->getDataFlow()->dominators(cfg);
	// Number the statements
	// int stmtNumber = 0;
	proc->numberStatements();
	proc->getDataFlow()->renameBlockVars(proc, 0, 0); // Block 0, mem depth 0
	proc->getDataFlow()->renameBlockVars(proc, 0, 1); // Block 0, mem depth 1
	// Find various needed statements
	StatementList stmts;
	proc->getStatements(stmts);
	StatementList::iterator it = stmts.begin();

	while (!(*it)->isCall()) {
		it++;
	}

	CallStatement *call = (CallStatement *)*it; // Statement 18, a call to printf
	call->setDestProc(proc);                    // A recursive call
	// std::cerr << "Call is "; call->dump();
	advance(it, 2);

	   Statement *s20 = *it; // Statement 20
	// FIXME: Ugh. Somehow, statement 20 has already bypassed the call, and incorrectly from what I can see - MVE
	s20->bypass();        // r28 should bypass the call

    // Make sure it's what we expect!
	QString        expected("  20 *32* r28 := r28{-} - 16");
	QString        actual;
	QTextStream ost(&actual);
	ost << s20;

    QCOMPARE(actual, expected);
    delete pFE;
}


void StatementTest::testStripSizes()
{
	// *v* r24 := m[zfill(8,32,local5) + param6]*8**8* / 16
	// The double size casting happens as a result of substitution
	SharedExp lhs = Location::regOf(24);
	SharedExp rhs = Binary::get(
		opDiv,
		Binary::get(opSize, Const::get(8),
				   Binary::get(opSize, Const::get(8),
							  Location::memOf(Binary::get(opPlus,
                                                          std::make_shared<Ternary>(opZfill,
                                                                       Const::get(8),
                                                                       Const::get(32),
                                                                       Location::local("local5", nullptr)),
														  Location::local("param6", nullptr))))),
		Const::get(16));

    Statement *s = new Assign(lhs, rhs);

	s->stripSizes();
	QString expected("   0 *v* r24 := m[zfill(8,32,local5) + param6] / 16");
	QString actual;
	QTextStream ost(&actual);
	ost << s;
	QCOMPARE(actual, expected);
}


void StatementTest::testFindConstants()
{
	   Statement *a = new Assign(Location::regOf(24), Binary::get(opPlus, Const::get(3), Const::get(4)));

	std::list<std::shared_ptr<Const>> lc;
	a->findConstants(lc);

    QString actual;
    QTextStream ost(&actual);

	for (auto it = lc.begin(); it != lc.end(); ) {
		ost << *it;

		if (++it != lc.end()) {
			ost << ", ";
		}
	}

	QCOMPARE(actual, QString("3, 4"));
}

QTEST_MAIN(StatementTest)
