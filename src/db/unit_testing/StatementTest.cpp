/***************************************************************************/ /**
 * \file       StatementTest.cc
 * OVERVIEW:   Provides the implementation for the StatementTest class, which
 *                tests the dataflow subsystems
 *============================================================================*/

/*
 * $Revision$
 *
 * 14 Jan 03 - Trent: Created
 * 17 Apr 03 - Mike: Added testRecursion to track down a nasty bug
 * 06 Jul 05 - Mike: Split testAddUsedLocs into six separate tests for Assign ... Bool
 */

#include <sstream>
#include <map>
#include "StatementTest.h"
#include "cfg.h"
#include "rtl.h"
#include "pentiumfrontend.h"
#include "boomerang.h"
#include "exp.h"
#include "managed.h"
#include "log.h"
#include "signature.h"

// class NullLogger : public Log {
// public:
//    virtual Log &operator<<(const char *str) {
//        // std::cerr << str;
//        return *this;
//    }
//    virtual ~NullLogger() {};
//};
#include "prog.h"
#include "proc.h"

#define HELLO_PENTIUM      baseDir.absoluteFilePath("tests/inputs/pentium/hello")
#define GLOBAL1_PENTIUM    baseDir.absoluteFilePath("tests/inputs/pentium/global1")

static bool    logset = false;
static QString TEST_BASE;
static QDir    baseDir;

/***************************************************************************/ /**
 * FUNCTION:        StatementTest::initTestCase
 * OVERVIEW:        Set up some expressions for use with all the tests
 * NOTE:            Called before any tests
 * PARAMETERS:        <none>
 *
 *============================================================================*/
void StatementTest::initTestCase()
{
	if (!logset) {
		TEST_BASE = QProcessEnvironment::systemEnvironment().value("BOOMERANG_TEST_BASE", "");
		baseDir   = QDir(TEST_BASE);

		if (TEST_BASE.isEmpty()) {
			qWarning() << "BOOMERANG_TEST_BASE environment variable not set, will assume '..', many test may fail";
			TEST_BASE = "..";
			baseDir   = QDir("..");
		}

		logset = true;
		Boomerang::get()->setProgPath(TEST_BASE);
		Boomerang::get()->setPluginPath(TEST_BASE + "/out");
		Boomerang::get()->setLogger(new NullLogger());
	}
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testEmpty
 * OVERVIEW:
 *============================================================================*/
void StatementTest::testEmpty()
{
	// Force "verbose" flag (-v)
	Boomerang *boo = Boomerang::get();

	boo->vFlag = true;
	boo->setOutputDirectory("./unit_test/");
	boo->setLogger(new FileLogger());

	// create Prog
	Prog              *prog = new Prog("testEmpty");
	BinaryFileFactory bff;
	QObject           *pBF = bff.Load(HELLO_PENTIUM);
	QVERIFY(pBF != 0);
	FrontEnd *pFE = new PentiumFrontEnd(pBF, prog, &bff);
	prog->setFrontEnd(pFE);

	Module *m = *prog->begin();
	QVERIFY(m != nullptr);
	// create UserProc
	UserProc *proc = (UserProc *)m->getOrInsertFunction("test", 0x123);
	// create CFG
	Cfg                      *cfg   = proc->getCFG();
	std::list<RTL *>         *pRtls = new std::list<RTL *>();
	std::list<Instruction *> *ls    = new std::list<Instruction *>;
	ls->push_back(new ReturnStatement);
	pRtls->push_back(new RTL(0x123));
	PBB bb = cfg->newBB(pRtls, RET, 0);
	cfg->setEntryBB(bb);
	proc->setDecoded(); // We manually "decoded"
	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected = "Ret BB:\n"
						   "in edges: \n"
						   "out edges: \n"
						   "00000123\n\n";
	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testFlow
 * OVERVIEW:
 *============================================================================*/
void StatementTest::testFlow()
{
	// create Prog
	Prog              *prog = new Prog;
	BinaryFileFactory bff;
	BinaryFile        *pBF = bff.Load(HELLO_PENTIUM); // Don't actually use it
	FrontEnd          *pFE = new PentiumFrontEnd(pBF, prog, &bff);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);

	// create UserProc
	std::string name  = "test";
	UserProc    *proc = (UserProc *)prog->newProc("test", 0x123);
	proc->setSignature(Signature::instantiate(PLAT_PENTIUM, CONV_C, name.c_str()));
	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	Assign           *a     = new Assign(Location::regOf(24), new Const(5));
	a->setProc(proc);
	a->setNumber(1);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);
	pRtls = new std::list<RTL *>();
	rtl   = new RTL(0x123);
	ReturnStatement *rs = new ReturnStatement;
	rs->setNumber(2);
	a = new Assign(Location::regOf(24), new Const(5));
	a->setProc(proc);
	rs->addReturn(a);
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	PBB ret = cfg->newBB(pRtls, RET, 0);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first); // Also sets exitBB; important!
	proc->setDecoded();
	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	// The assignment to 5 gets propagated into the return, and the assignment
	// to r24 is removed
	expected = "Fall BB:\n"
			   "in edges: \n"
			   "out edges: 123 \n"
			   "00000000\n"
			   "Ret BB:\n"
			   "in edges: 0 \n"
			   "out edges: \n"
			   "00000123    2 RET *v* r24 := 5\n"
			   "              Modifieds: \n"
			   "              Reaching definitions: r24=5\n\n";

	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testKill
 * OVERVIEW:
 *============================================================================*/
void StatementTest::testKill()
{
	// create Prog
	Prog              *prog = new Prog;
	BinaryFileFactory bff;
	BinaryFile        *pBF = bff.Load(HELLO_PENTIUM); // Don't actually use it
	FrontEnd          *pFE = new PentiumFrontEnd(pBF, prog, &bff);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);
	// create UserProc
	std::string name  = "test";
	UserProc    *proc = (UserProc *)prog->newProc("test", 0x123);
	proc->setSignature(Signature::instantiate(PLAT_PENTIUM, CONV_C, name.c_str()));
	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	Assign           *e     = new Assign(Location::regOf(24), new Const(5));
	e->setNumber(1);
	e->setProc(proc);
	rtl->appendStmt(e);
	e = new Assign(Location::regOf(24), new Const(6));
	e->setNumber(2);
	e->setProc(proc);
	rtl->appendStmt(e);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);
	pRtls = new std::list<RTL *>();
	rtl   = new RTL(0x123);
	ReturnStatement *rs = new ReturnStatement;
	rs->setNumber(3);
	e = new Assign(Location::regOf(24), new Const(0));
	e->setProc(proc);
	rs->addReturn(e);
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	PBB ret = cfg->newBB(pRtls, RET, 0);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	expected = "Fall BB:\n"
			   "in edges: \n"
			   "out edges: 123 \n"
			   "00000000\n"
			   "Ret BB:\n"
			   "in edges: 0 \n"
			   "out edges: \n"
			   "00000123    3 RET *v* r24 := 0\n"
			   "              Modifieds: \n"
			   "              Reaching definitions: r24=6\n\n";

	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testUse
 * OVERVIEW:
 *============================================================================*/
void StatementTest::testUse()
{
	// create Prog
	Prog              *prog = new Prog;
	BinaryFileFactory bff;
	BinaryFile        *pBF = bff.Load(HELLO_PENTIUM); // Don't actually use it
	FrontEnd          *pFE = new PentiumFrontEnd(pBF, prog, &bff);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);
	// create UserProc
	std::string name  = "test";
	UserProc    *proc = (UserProc *)prog->newProc("test", 0x123);
	proc->setSignature(Signature::instantiate(PLAT_PENTIUM, CONV_C, name.c_str()));
	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	Assign           *a     = new Assign(Location::regOf(24), new Const(5));
	a->setNumber(1);
	a->setProc(proc);
	rtl->appendStmt(a);
	a = new Assign(Location::regOf(28), Location::regOf(24));
	a->setNumber(2);
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);
	pRtls = new std::list<RTL *>();
	rtl   = new RTL(0x123);
	ReturnStatement *rs = new ReturnStatement;
	rs->setNumber(3);
	a = new Assign(Location::regOf(28), new Const(1000));
	a->setProc(proc);
	rs->addReturn(a);
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	PBB ret = cfg->newBB(pRtls, RET, 0);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	expected = "Fall BB:\n"
			   "in edges: \n"
			   "out edges: 123 \n"
			   "00000000\n"
			   "Ret BB:\n"
			   "in edges: 0 \n"
			   "out edges: \n"
			   "00000123    3 RET *v* r28 := 1000\n"
			   "              Modifieds: \n"
			   "              Reaching definitions: r24=5,   r28=5\n\n";

	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testUseOverKill
 * OVERVIEW:
 *============================================================================*/
void StatementTest::testUseOverKill()
{
	// create Prog
	Prog              *prog = new Prog;
	BinaryFileFactory bff;
	BinaryFile        *pBF = bff.Load(HELLO_PENTIUM); // Don't actually use it
	FrontEnd          *pFE = new PentiumFrontEnd(pBF, prog, &bff);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);
	// create UserProc
	std::string name  = "test";
	UserProc    *proc = (UserProc *)prog->newProc("test", 0x123);
	proc->setSignature(Signature::instantiate(PLAT_PENTIUM, CONV_C, name.c_str()));
	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	Assign           *e     = new Assign(Location::regOf(24), new Const(5));
	e->setNumber(1);
	e->setProc(proc);
	rtl->appendStmt(e);
	e = new Assign(Location::regOf(24), new Const(6));
	e->setNumber(2);
	e->setProc(proc);
	rtl->appendStmt(e);
	e = new Assign(Location::regOf(28), Location::regOf(24));
	e->setNumber(3);
	e->setProc(proc);
	rtl->appendStmt(e);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);
	pRtls = new std::list<RTL *>();
	rtl   = new RTL(0x123);
	ReturnStatement *rs = new ReturnStatement;
	rs->setNumber(4);
	e = new Assign(Location::regOf(24), new Const(0));
	e->setProc(proc);
	rs->addReturn(e);
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	PBB ret = cfg->newBB(pRtls, RET, 0);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	expected = "Fall BB:\n"
			   "in edges: \n"
			   "out edges: 123 \n"
			   "00000000\n"
			   "Ret BB:\n"
			   "in edges: 0 \n"
			   "out edges: \n"
			   "00000123    4 RET *v* r24 := 0\n"
			   "              Modifieds: \n"
			   "              Reaching definitions: r24=6,   r28=6\n\n";

	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testUseOverBB
 * OVERVIEW:
 *============================================================================*/
void StatementTest::testUseOverBB()
{
	// create Prog
	Prog              *prog = new Prog;
	BinaryFileFactory bff;
	BinaryFile        *pBF = bff.Load(HELLO_PENTIUM); // Don't actually use it
	FrontEnd          *pFE = new PentiumFrontEnd(pBF, prog, &bff);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);
	// create UserProc
	std::string name  = "test";
	UserProc    *proc = (UserProc *)prog->newProc("test", 0x123);
	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	Assign           *a     = new Assign(Location::regOf(24), new Const(5));
	a->setNumber(1);
	a->setProc(proc);
	rtl->appendStmt(a);
	a = new Assign(Location::regOf(24), new Const(6));
	a->setNumber(2);
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);
	pRtls = new std::list<RTL *>();
	rtl   = new RTL();
	a     = new Assign(Location::regOf(28), Location::regOf(24));
	a->setNumber(3);
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	rtl = new RTL(0x123);
	ReturnStatement *rs = new ReturnStatement;
	rs->setNumber(4);
	a = new Assign(Location::regOf(24), new Const(0));
	a->setProc(proc);
	rs->addReturn(a);
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	PBB ret = cfg->newBB(pRtls, RET, 0);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	expected = "Fall BB:\n"
			   "in edges: \n"
			   "out edges: 123 \n"
			   "00000000\n"
			   "Ret BB:\n"
			   "in edges: 0 \n"
			   "out edges: \n"
			   "00000000\n"
			   "00000123    4 RET *v* r24 := 0\n"
			   "              Modifieds: \n"
			   "              Reaching definitions: r24=6,   r28=6\n\n";

	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testUseKill
 * OVERVIEW:
 *============================================================================*/
void StatementTest::testUseKill()
{
	// create Prog
	Prog              *prog = new Prog;
	BinaryFileFactory bff;
	BinaryFile        *pBF = bff.Load(HELLO_PENTIUM); // Don't actually use it
	FrontEnd          *pFE = new PentiumFrontEnd(pBF, prog, &bff);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);
	// create UserProc
	std::string name  = "test";
	UserProc    *proc = (UserProc *)prog->newProc("test", 0x123);
	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	Assign           *a     = new Assign(Location::regOf(24), new Const(5));
	a->setNumber(1);
	a->setProc(proc);
	rtl->appendStmt(a);
	a = new Assign(Location::regOf(24), new Binary(opPlus, Location::regOf(24), new Const(1)));
	a->setNumber(2);
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);
	pRtls = new std::list<RTL *>();
	rtl   = new RTL(0x123);
	ReturnStatement *rs = new ReturnStatement;
	rs->setNumber(3);
	a = new Assign(Location::regOf(24), new Const(0));
	a->setProc(proc);
	rs->addReturn(a);
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	PBB ret = cfg->newBB(pRtls, RET, 0);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	expected = "Fall BB:\n"
			   "in edges: \n"
			   "out edges: 123 \n"
			   "00000000\n"
			   "Ret BB:\n"
			   "in edges: 0 \n"
			   "out edges: \n"
			   "00000123    3 RET *v* r24 := 0\n"
			   "              Modifieds: \n"
			   "              Reaching definitions: r24=6\n\n";

	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testEndlessLoop
 * OVERVIEW:
 *============================================================================*/
void StatementTest::testEndlessLoop()
{
	// create Prog
	Prog              *prog = new Prog;
	BinaryFileFactory bff;
	BinaryFile        *pBF = bff.Load(HELLO_PENTIUM); // Don't actually use it
	FrontEnd          *pFE = new PentiumFrontEnd(pBF, prog, &bff);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);
	// create UserProc
	std::string name  = "test";
	UserProc    *proc = (UserProc *)prog->newProc("test", 0x123);
	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	// r[24] := 5
	Assign *e = new Assign(Location::regOf(24), new Const(5));
	e->setProc(proc);
	rtl->appendStmt(e);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);
	pRtls = new std::list<RTL *>();
	rtl   = new RTL();
	// r[24] := r[24] + 1
	e = new Assign(Location::regOf(24), new Binary(opPlus, Location::regOf(24), new Const(1)));
	e->setProc(proc);
	rtl->appendStmt(e);
	pRtls->push_back(rtl);
	PBB body = cfg->newBB(pRtls, ONEWAY, 1);
	first->setOutEdge(0, body);
	body->addInEdge(first);
	body->setOutEdge(0, body);
	body->addInEdge(body);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	int indent = 0;
	proc->decompile(new ProcList, indent);
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	expected = "Fall BB: reach in: \n"
			   "00000000 *v* r[24] := 5\n"
			   "Oneway BB:\n"
			   "00000000 *v* r[24] := r[24] + 1   uses: ** r[24] := 5, "
			   "*v* r[24] := r[24] + 1,    used by: ** r[24] := r[24] + 1, \n"
			   "cfg reachExit: \n";
	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testLocationSet
 * OVERVIEW:
 *============================================================================*/
void StatementTest::testLocationSet()
{
	Location    rof(opRegOf, new Const(12), nullptr); // r12
	Const&      theReg = *(Const *)rof.getSubExp1();
	LocationSet ls;

	LocationSet::iterator ii;
	ls.insert(rof.clone()); // ls has r12
	theReg.setInt(8);
	ls.insert(rof.clone()); // ls has r8 r12
	theReg.setInt(31);
	ls.insert(rof.clone()); // ls has r8 r12 r31
	theReg.setInt(24);
	ls.insert(rof.clone()); // ls has r8 r12 r24 r31
	theReg.setInt(12);
	ls.insert(rof.clone()); // Note: r12 already inserted
	int size = (int)ls.size();
	CPPUNIT_ASSERT_EQUAL(4, size);
	theReg.setInt(8);
	ii = ls.begin();
	CPPUNIT_ASSERT(rof == **ii); // First element should be r8
	theReg.setInt(12);
	Exp *e;
	e = *(++ii);
	CPPUNIT_ASSERT(rof == *e); // Second should be r12
	theReg.setInt(24);
	e = *(++ii);
	CPPUNIT_ASSERT(rof == *e); // Next should be r24
	theReg.setInt(31);
	e = *(++ii);
	CPPUNIT_ASSERT(rof == *e);                                                             // Last should be r31
	Location mof(opMemOf, new Binary(opPlus, Location::regOf(14), new Const(4)), nullptr); // m[r14 + 4]
	ls.insert(mof.clone());                                                                // ls should be r8 r12 r24 r31 m[r14 + 4]
	ls.insert(mof.clone());
	size = (int)ls.size();
	CPPUNIT_ASSERT_EQUAL(5, size); // Should have 5 elements
	ii = --ls.end();
	CPPUNIT_ASSERT(mof == **ii);   // Last element should be m[r14 + 4] now
	LocationSet ls2 = ls;
	Exp         *e2 = *ls2.begin();
	CPPUNIT_ASSERT(!(e2 == *ls.begin())); // Must be cloned
	size = (int)ls2.size();
	CPPUNIT_ASSERT_EQUAL(5, size);
	theReg.setInt(8);
	CPPUNIT_ASSERT(rof == **ls2.begin()); // First elements should compare equal
	theReg.setInt(12);
	e = *(++ls2.begin());                 // Second element
	CPPUNIT_ASSERT(rof == *e);            // ... should be r12
	Assign s10(new Const(0), new Const(0)), s20(new Const(0), new Const(0));
	s10.setNumber(10);
	s20.setNumber(20);
	RefExp *r1 = new RefExp(Location::regOf(8), &s10);
	RefExp *r2 = new RefExp(Location::regOf(8), &s20);
	ls.insert(r1); // ls now m[r14 + 4] r8 r12 r24 r31 r8{10} (not sure where r8{10} appears)
	size = (int)ls.size();
	CPPUNIT_ASSERT_EQUAL(6, size);
	Exp *dummy;
	CPPUNIT_ASSERT(!ls.findDifferentRef(r1, dummy));
	CPPUNIT_ASSERT(ls.findDifferentRef(r2, dummy));

	Exp *r8 = Location::regOf(8);
	CPPUNIT_ASSERT(!ls.existsImplicit(r8));

	RefExp r3(Location::regOf(8), nullptr);
	ls.insert(&r3);
	std::cerr << ls.prints() << "\n";
	CPPUNIT_ASSERT(ls.existsImplicit(r8));

	ls.remove(&r3);

	ImplicitAssign zero(r8);
	RefExp         r4(Location::regOf(8), &zero);
	ls.insert(&r4);
	std::cerr << ls.prints() << "\n";
	CPPUNIT_ASSERT(ls.existsImplicit(r8));
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testWildLocationSet
 * OVERVIEW:
 *============================================================================*/
void StatementTest::testWildLocationSet()
{
	Location rof12(opRegOf, new Const(12), nullptr);
	Location rof13(opRegOf, new Const(13), nullptr);
	Assign   a10, a20;

	a10.setNumber(10);
	a20.setNumber(20);
	RefExp      r12_10(rof12.clone(), &a10);
	RefExp      r12_20(rof12.clone(), &a20);
	RefExp      r12_0(rof12.clone(), nullptr);
	RefExp      r13_10(rof13.clone(), &a10);
	RefExp      r13_20(rof13.clone(), &a20);
	RefExp      r13_0(rof13.clone(), nullptr);
	RefExp      r11_10(Location::regOf(11), &a10);
	RefExp      r22_10(Location::regOf(22), &a10);
	LocationSet ls;
	ls.insert(&r12_10);
	ls.insert(&r12_20);
	ls.insert(&r12_0);
	ls.insert(&r13_10);
	ls.insert(&r13_20);
	ls.insert(&r13_0);
	RefExp wildr12(rof12.clone(), (Statement *)-1);
	CPPUNIT_ASSERT(ls.exists(&wildr12));
	RefExp wildr13(rof13.clone(), (Statement *)-1);
	CPPUNIT_ASSERT(ls.exists(&wildr13));
	RefExp wildr10(Location::regOf(10), (Statement *)-1);
	CPPUNIT_ASSERT(!ls.exists(&wildr10));
	// Test findDifferentRef
	Exp *x;
	CPPUNIT_ASSERT(ls.findDifferentRef(&r13_10, x));
	CPPUNIT_ASSERT(ls.findDifferentRef(&r13_20, x));
	CPPUNIT_ASSERT(ls.findDifferentRef(&r13_0, x));
	CPPUNIT_ASSERT(ls.findDifferentRef(&r12_10, x));
	CPPUNIT_ASSERT(ls.findDifferentRef(&r12_20, x));
	CPPUNIT_ASSERT(ls.findDifferentRef(&r12_0, x));
	// Next 4 should fail
	CPPUNIT_ASSERT(!ls.findDifferentRef(&r11_10, x));
	CPPUNIT_ASSERT(!ls.findDifferentRef(&r22_10, x));
	ls.insert(&r11_10);
	ls.insert(&r22_10);
	CPPUNIT_ASSERT(!ls.findDifferentRef(&r11_10, x));
	CPPUNIT_ASSERT(!ls.findDifferentRef(&r22_10, x));
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testRecursion
 * OVERVIEW:        Test push of argument (X86 style), then call self
 *============================================================================*/
void StatementTest::testRecursion()
{
	// create Prog
	Prog              *prog = new Prog;
	BinaryFileFactory bff;
	BinaryFile        *pBF = bff.Load(HELLO_PENTIUM); // Don't actually use it
	FrontEnd          *pFE = new PentiumFrontEnd(pBF, prog, &bff);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);
	// create UserProc
	std::string name  = "test";
	UserProc    *proc = new UserProc(prog, name, 0);
	// create CFG
	Cfg              *cfg   = proc->getCFG();
	std::list<RTL *> *pRtls = new std::list<RTL *>();
	RTL              *rtl   = new RTL();
	// push bp
	// r28 := r28 + -4
	Assign *a = new Assign(Location::regOf(28), new Binary(opPlus, Location::regOf(28), new Const(-4)));
	rtl->appendStmt(a);
	// m[r28] := r29
	a = new Assign(Location::memOf(Location::regOf(28)), Location::regOf(29));
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	pRtls = new std::list<RTL *>();
	// push arg+1
	// r28 := r28 + -4
	a = new Assign(Location::regOf(28), new Binary(opPlus, Location::regOf(28), new Const(-4)));
	rtl->appendStmt(a);
	// Reference our parameter. At esp+0 is this arg; at esp+4 is old bp;
	// esp+8 is return address; esp+12 is our arg
	// m[r28] := m[r28+12] + 1
	a = new Assign(
		Location::memOf(Location::regOf(28)),
		new Binary(opPlus, Location::memOf(new Binary(opPlus, Location::regOf(28), new Const(12))), new Const(1)));
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);

	// The call BB
	pRtls = new std::list<RTL *>();
	rtl   = new RTL(1);
	// r28 := r28 + -4
	a = new Assign(Location::regOf(28), new Binary(opPlus, Location::regOf(28), new Const(-4)));
	rtl->appendStmt(a);
	// m[r28] := pc
	a = new Assign(Location::memOf(Location::regOf(28)), new Terminal(opPC));
	rtl->appendStmt(a);
	// %pc := (%pc + 5) + 135893848
	a = new Assign(new Terminal(opPC),
				   new Binary(opPlus, new Binary(opPlus, new Terminal(opPC), new Const(5)), new Const(135893848)));
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	CallStatement *c = new CallStatement;
	rtl->appendStmt(c);
#if 0
	// Vector of 1 arg
	std::vector<Exp *> args;
	// m[r[28]+8]
	Exp *a = Location::memOf(new Binary(opPlus,
										Location::regOf(28), new Const(8)));
	args.push_back(a);
	crtl->setArguments(args);
#endif
	c->setDestProc(proc); // Just call self
	PBB callbb = cfg->newBB(pRtls, CALL, 1);
	first->setOutEdge(0, callbb);
	callbb->addInEdge(first);
	callbb->setOutEdge(0, callbb);
	callbb->addInEdge(callbb);

	pRtls = new std::list<RTL *>();
	rtl   = new RTL(0x123);
	rtl->appendStmt(new ReturnStatement);
	// This ReturnStatement requires the following two sets of semantics to pass the
	// tests for standard Pentium calling convention
	// pc = m[r28]
	a = new Assign(new Terminal(opPC), Location::memOf(Location::regOf(28)));
	rtl->appendStmt(a);
	// r28 = r28 + 4
	a = new Assign(Location::regOf(28), new Binary(opPlus, Location::regOf(28), new Const(4)));
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	PBB ret = cfg->newBB(pRtls, RET, 0);
	callbb->setOutEdge(0, ret);
	ret->addInEdge(callbb);
	cfg->setEntryBB(first);

	// decompile the "proc"
	prog->decompile();
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	expected = "Fall BB: reach in: \n"
			   "00000000 ** r[24] := 5   uses:    used by: ** r[24] := r[24] + 1, \n"
			   "00000000 ** r[24] := 5   uses:    used by: ** r[24] := r[24] + 1, \n"
			   "Call BB: reach in: ** r[24] := 5, ** r[24] := r[24] + 1, \n"
			   "00000001 ** r[24] := r[24] + 1   uses: ** r[24] := 5, "
			   "** r[24] := r[24] + 1,    used by: ** r[24] := r[24] + 1, \n"
			   "cfg reachExit: \n";
	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}


/***************************************************************************/ /**
 * FUNCTION:        StatamentTest::testClone
 * OVERVIEW:        Test cloning of Assigns (and exps)
 *============================================================================*/
void StatementTest::testClone()
{
	Assign *a1 = new Assign(Location::regOf(8), new Binary(opPlus, Location::regOf(9), new Const(99)));
	Assign *a2 = new Assign(new IntegerType(16, 1), new Location(opParam, new Const("x"), nullptr),
							new Location(opParam, new Const("y"), nullptr));
	Assign *a3 = new Assign(new IntegerType(16, -1), new Location(opParam, new Const("z"), nullptr),
							new Location(opParam, new Const("q"), nullptr));
	Statement *c1 = a1->clone();
	Statement *c2 = a2->clone();
	Statement *c3 = a3->clone();

	std::ostringstream o1, o2;
	a1->print(o1);
	delete a1; // And c1 should still stand!
	c1->print(o2);
	a2->print(o1);
	c2->print(o2);
	a3->print(o1);
	c3->print(o2);
	std::string expected("   0 *v* r8 := r9 + 99   0 *i16* x := y"
						 "   0 *u16* z := q");
	std::string act1(o1.str());
	std::string act2(o2.str());
	CPPUNIT_ASSERT_EQUAL(expected, act1); // Originals
	CPPUNIT_ASSERT_EQUAL(expected, act2); // Clones
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testIsAssign
 * OVERVIEW:        Test assignment test
 *============================================================================*/
void StatementTest::testIsAssign()
{
	std::ostringstream ost;
	// r2 := 99
	Assign a(Location::regOf(2), new Const(99));
	a.print(ost);
	std::string expected("   0 *v* r2 := 99");
	std::string actual(ost.str());
	CPPUNIT_ASSERT_EQUAL(expected, actual);
	//      CPPUNIT_ASSERT_EQUAL (std::string("*v* r2 := 99"), std::string(ost.str()));
	CPPUNIT_ASSERT(a.isAssign());

	CallStatement *c = new CallStatement;
	CPPUNIT_ASSERT(!c->isAssign());
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testIsFlagCall
 * OVERVIEW:        Test the isFlagAssgn function, and opFlagCall
 *============================================================================*/
void StatementTest::testIsFlagAssgn()
{
	std::ostringstream ost;
	// FLAG addFlags(r2 , 99)
	Assign fc(new Terminal(opFlags),
			  new Binary(opFlagCall, new Const("addFlags"), new Binary(opList, Location::regOf(2), new Const(99))));
	CallStatement   *call = new CallStatement;
	BranchStatement *br   = new BranchStatement;
	Assign          *as   = new Assign(Location::regOf(9), new Binary(opPlus, Location::regOf(10), new Const(4)));
	fc.print(ost);
	std::string expected("   0 *v* %flags := addFlags( r2, 99 )");
	std::string actual(ost.str());
	CPPUNIT_ASSERT_EQUAL(expected, actual);
	CPPUNIT_ASSERT(fc.isFlagAssgn());
	CPPUNIT_ASSERT(!call->isFlagAssgn());
	CPPUNIT_ASSERT(!br->isFlagAssgn());
	CPPUNIT_ASSERT(!as->isFlagAssgn());
	delete call;
	delete br;
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testAddUsedLocsAssign .. testAddUsedLocsBool
 * OVERVIEW:        Test the finding of locations used by this statement
 *============================================================================*/
void StatementTest::testAddUsedLocsAssign()
{
	// m[r28-4] := m[r28-8] * r26
	Assign *a = new Assign(Location::memOf(new Binary(opMinus, Location::regOf(28), new Const(4))),
						   new Binary(opMult, Location::memOf(new Binary(opMinus, Location::regOf(28), new Const(8))),
									  Location::regOf(26)));

	a->setNumber(1);
	LocationSet l;
	a->addUsedLocs(l);
	std::ostringstream ost1;
	l.print(ost1);
	std::string expected = "r26,\tr28,\tm[r28 - 8]";
	std::string actual   = ost1.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	l.clear();
	GotoStatement *g = new GotoStatement();
	g->setNumber(55);
	g->setDest(Location::memOf(Location::regOf(26)));
	g->addUsedLocs(l);
	std::ostringstream ost2;
	l.print(ost2);
	expected = "r26,\tm[r26]";
	actual   = ost2.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);
}


void StatementTest::testAddUsedLocsBranch()
{
	// BranchStatement with dest m[r26{99}]{55}, condition %flags
	GotoStatement *g = new GotoStatement();

	g->setNumber(55);
	LocationSet     l;
	BranchStatement *b = new BranchStatement;
	b->setNumber(99);
	b->setDest(new RefExp(Location::memOf(new RefExp(Location::regOf(26), b)), g));
	b->setCondExpr(new Terminal(opFlags));
	b->addUsedLocs(l);
	std::ostringstream ost3;
	l.print(ost3);
	std::string expected("r26{99},\tm[r26{99}]{55},\t%flags");
	std::string actual(ost3.str());
	CPPUNIT_ASSERT_EQUAL(expected, actual);
}


void StatementTest::testAddUsedLocsCase()
{
	// CaseStatement with pDest = m[r26], switchVar = m[r28 - 12]
	LocationSet   l;
	CaseStatement *c = new CaseStatement;

	c->setDest(Location::memOf(Location::regOf(26)));
	SWITCH_INFO si;
	si.pSwitchVar = Location::memOf(new Binary(opMinus, Location::regOf(28), new Const(12)));
	c->setSwitchInfo(&si);
	c->addUsedLocs(l);
	std::ostringstream ost4;
	l.print(ost4);
	std::string expected("r26,\tr28,\tm[r28 - 12],\tm[r26]");
	std::string actual(ost4.str());
	CPPUNIT_ASSERT_EQUAL(expected, actual);
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
	argl.append(new Assign(Location::regOf(9), new RefExp(Location::regOf(28), g)));
	ca->setArguments(argl);
	ca->addDefine(new ImplicitAssign(Location::regOf(31)));
	ca->addDefine(new ImplicitAssign(Location::regOf(24)));
	ca->addUsedLocs(l);
	std::ostringstream ost5;
	l.print(ost5);
	std::string expected("r26,\tr27,\tm[r26],\tm[r27],\tr28{55}");
	std::string actual(ost5.str());
	CPPUNIT_ASSERT_EQUAL(expected, actual);

// Now with uses in collector
#if 0 // FIXME: to be completed
	l.clear();
	ca->addUsedLocs(l, true);
	std::ostringstream ost5f;
	l.print(ost5f);
	expected = "m[r26],\tm[r27],\tr26,\tr27,\tr28{55}";
	actual   = ost5f.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);
#endif
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
	r->addReturn(new Assign(Location::regOf(31), new Const(100)));
	r->addReturn(new Assign(Location::memOf(Location::regOf(24)), new Const(0)));
	r->addReturn(new Assign(
					 Location::memOf(new Binary(opPlus, new RefExp(Location::regOf(25), g), new RefExp(Location::regOf(26), b))),
					 new Const(5)));
	r->addUsedLocs(l);
	std::ostringstream ost6;
	l.print(ost6);
	std::string expected("r24,\tr25{55},\tr26{99}");
	std::string actual(ost6.str());
	CPPUNIT_ASSERT_EQUAL(expected, actual);
}


void StatementTest::testAddUsedLocsBool()
{
	// Boolstatement with condition m[r24] = r25, dest m[r26]
	LocationSet l;
	BoolAssign  *bs = new BoolAssign(8);

	bs->setCondExpr(new Binary(opEquals, Location::memOf(Location::regOf(24)), Location::regOf(25)));
	std::list<Statement *> stmts;
	Assign                 *a = new Assign(Location::memOf(Location::regOf(26)), new Terminal(opNil));
	stmts.push_back(a);
	bs->setLeftFromList(&stmts);
	bs->addUsedLocs(l);
	std::ostringstream ost7;
	l.print(ost7);
	std::string expected("r24,\tr25,\tr26,\tm[r24]");
	std::string actual(ost7.str());
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// m[local21 + 16] := phi{0, 372}
	l.clear();
	Exp    *base = Location::memOf(new Binary(opPlus, Location::local("local21", nullptr), new Const(16)));
	Assign s372(base, new Const(0));
	s372.setNumber(372);
	PhiAssign *pa = new PhiAssign(base);
	pa->putAt(0, nullptr, base);
	pa->putAt(1, &s372, base);
	pa->addUsedLocs(l);
	// Note: phis were not considered to use blah if they ref m[blah], so local21 was not considered used
	expected = "m[local21 + 16]{-},\tm[local21 + 16]{372},\tlocal21";
	std::ostringstream ost8;
	l.print(ost8);
	actual = ost8.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// m[r28{-} - 4] := -
	l.clear();
	ImplicitAssign *ia = new ImplicitAssign(
		Location::memOf(new Binary(opMinus, new RefExp(Location::regOf(28), nullptr), new Const(4))));
	std::ostringstream ost9;
	ia->addUsedLocs(l);
	l.print(ost9);
	actual   = ost9.str();
	expected = "r28{-}";
	CPPUNIT_ASSERT_EQUAL(expected, actual);
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testSubscriptVars
 * OVERVIEW:        Test the subscripting of locations in Statements
 *============================================================================*/
void StatementTest::testSubscriptVars()
{
	Exp    *srch = Location::regOf(28);
	Assign s9(new Const(0), new Const(0));

	s9.setNumber(9);

	// m[r28-4] := m[r28-8] * r26
	Assign *a = new Assign(Location::memOf(new Binary(opMinus, Location::regOf(28), new Const(4))),
						   new Binary(opMult, Location::memOf(new Binary(opMinus, Location::regOf(28), new Const(8))),
									  Location::regOf(26)));
	a->setNumber(1);
	std::ostringstream ost1;
	a->subscriptVar(srch, &s9);
	ost1 << a;
	std::string expected = "   1 *v* m[r28{9} - 4] := m[r28{9} - 8] * r26";
	std::string actual(ost1.str());
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// GotoStatement
	GotoStatement *g = new GotoStatement();
	g->setNumber(55);
	g->setDest(Location::regOf(28));
	std::ostringstream ost2;
	g->subscriptVar(srch, &s9);
	ost2 << g;
	expected = "  55 GOTO r28{9}";
	actual   = ost2.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// BranchStatement with dest m[r26{99}]{55}, condition %flags
	BranchStatement *b = new BranchStatement;
	b->setNumber(99);
	Exp *srchb = Location::memOf(new RefExp(Location::regOf(26), b));
	b->setDest(new RefExp(srchb, g));
	b->setCondExpr(new Terminal(opFlags));
	std::ostringstream ost3;
	b->subscriptVar(srchb, &s9); // Should be ignored now: new behaviour
	b->subscriptVar(new Terminal(opFlags), g);
	ost3 << b;
	expected = "  99 BRANCH m[r26{99}]{55}, condition equals\n"
			   "High level: %flags{55}";
	actual = ost3.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// CaseStatement with pDest = m[r26], switchVar = m[r28 - 12]
	CaseStatement *c = new CaseStatement;
	c->setDest(Location::memOf(Location::regOf(26)));
	SWITCH_INFO si;
	si.pSwitchVar = Location::memOf(new Binary(opMinus, Location::regOf(28), new Const(12)));
	c->setSwitchInfo(&si);
	std::ostringstream ost4;
	c->subscriptVar(srch, &s9);
	ost4 << c;
	expected = "   0 SWITCH(m[r28{9} - 12])\n";
	actual   = ost4.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// CaseStatement (before recog) with pDest = r28, switchVar is nullptr
	c->setDest(Location::regOf(28));
	c->setSwitchInfo(nullptr);
	std::ostringstream ost4a;
	c->subscriptVar(srch, &s9);
	ost4a << c;
	expected = "   0 CASE [r28{9}]";
	actual   = ost4a.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// CallStatement with pDest = m[r26], params = m[r27], r28, defines r28, m[r28]
	CallStatement *ca = new CallStatement;
	ca->setDest(Location::memOf(Location::regOf(26)));
	StatementList argl;
	argl.append(new Assign(Location::memOf(Location::regOf(27)), new Const(1)));
	argl.append(new Assign(Location::regOf(28), new Const(2)));
	ca->setArguments(argl);
	ca->addDefine(new ImplicitAssign(Location::regOf(28)));
	ca->addDefine(new ImplicitAssign(Location::memOf(Location::regOf(28))));
	std::string name("dest");
	ca->setDestProc(new UserProc(new Prog(), name, 0x2000)); // Must have a dest to be non-childless
	ca->setCalleeReturn(new ReturnStatement);                // So it's not a childless call, and we can see the defs and params
	std::ostringstream ost5;
	ca->subscriptVar(srch, &s9);
	ost5 << ca;
	expected = "   0 {*v* r28, *v* m[r28]} := CALL dest(\n"
			   "                *v* m[r27] := 1\n"
			   "                *v* r28 := 2\n"
			   "              )\n"
			   "              Reaching definitions: \n"
			   "              Live variables: "; // ? No newline?
	actual = ost5.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// CallStatement with pDest = r28, params = m[r27], r29, defines r31, m[r31]
	ca = new CallStatement;
	ca->setDest(Location::regOf(28));
	argl.clear();
	argl.append(new Assign(Location::memOf(Location::regOf(27)), new Const(1)));
	argl.append(new Assign(Location::regOf(29), new Const(2)));
	ca->setArguments(argl);
	ca->addDefine(new ImplicitAssign(Location::regOf(31)));
	ca->addDefine(new ImplicitAssign(Location::memOf(Location::regOf(31))));
	ca->setDestProc(new UserProc(new Prog(), name, 0x2000)); // Must have a dest to be non-childless
	ca->setCalleeReturn(new ReturnStatement);                // So it's not a childless call, and we can see the defs and params
	std::ostringstream ost5a;
	ca->subscriptVar(srch, &s9);
	ost5a << ca;
	expected = "   0 {*v* r31, *v* m[r31]} := CALL dest(\n"
			   "                *v* m[r27] := 1\n"
			   "                *v* r29 := 2\n"
			   "              )\n"
			   "              Reaching definitions: \n"
			   "              Live variables: ";
	actual = ost5a.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// ReturnStatement with returns r28, m[r28], m[r28]{55} + r[26]{99}]
	// FIXME: shouldn't this test have some propagation? Now, it seems it's just testing the print code!
	ReturnStatement *r = new ReturnStatement;
	r->addReturn(new Assign(Location::regOf(28), new Const(1000)));
	r->addReturn(new Assign(Location::memOf(Location::regOf(28)), new Const(2000)));
	r->addReturn(new Assign(
					 Location::memOf(new Binary(opPlus, new RefExp(Location::regOf(28), g), new RefExp(Location::regOf(26), b))),
					 new Const(100)));
	std::ostringstream ost6;
	r->subscriptVar(srch, &s9); // New behaviour: gets ignored now
	ost6 << r;
	expected = "   0 RET *v* r28 := 1000,   *v* m[r28{9}] := 0x7d0,   *v* m[r28{55} + r26{99}] := 100\n"
			   "              Modifieds: \n"
			   "              Reaching definitions: ";
	actual = ost6.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// Boolstatement with condition m[r28] = r28, dest m[r28]
	BoolAssign *bs = new BoolAssign(8);
	bs->setCondExpr(new Binary(opEquals, Location::memOf(Location::regOf(28)), Location::regOf(28)));
	bs->setLeft(Location::memOf(Location::regOf(28)));
	std::ostringstream ost7;
	bs->subscriptVar(srch, &s9);
	ost7 << bs;
	expected = "   0 BOOL m[r28{9}] := CC(equals)\n"
			   "High level: m[r28{9}] = r28{9}\n";
	actual = ost7.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testBypass
 * OVERVIEW:        Test the visitor code that fixes references that were to locations defined by calls
 *============================================================================*/
void StatementTest::testBypass()
{
	Prog              *prog = new Prog;
	BinaryFileFactory bff;
	BinaryFile        *pBF = bff.Load(GLOBAL1_PENTIUM);
	FrontEnd          *pFE = new PentiumFrontEnd(pBF, prog, &bff);

	Type::clearNamedTypes();
	prog->setFrontEnd(pFE);
	pFE->decode(prog, true);       // Decode main
	pFE->decode(prog, NO_ADDRESS); // Decode anything undecoded
	bool    gotMain;
	ADDRESS addr = pFE->getMainEntryPoint(gotMain);
	CPPUNIT_ASSERT(addr != NO_ADDRESS);
	UserProc *proc = (UserProc *)prog->findProc("foo2");
	assert(proc);
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
	std::string        expected("  20 *32* r28 := r28{-} - 16");
	std::string        actual;
	std::ostringstream ost1;
	ost1 << s20;
	actual = ost1.str();
// CPPUNIT_ASSERT_EQUAL(expected, actual);
#if 0 // No longer needed, but could maybe expand the test one day
	// Fake it to be known that r29 is preserved
	Exp *r29 = Location::regOf(29);
	proc->setProven(new Binary(opEquals, r29, r29->clone()));
	(*it)->bypass();
	// Now expect r29{30} to be r29{3}
	expected = "  22 *32* r24 := m[r29{3} + 8]{-}";
	std::ostringstream ost2;
	ost2 << *it;
	actual = ost2.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);
#endif
	delete pFE;
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testStripSizes
 * OVERVIEW:        Test the visitor code that strips out size casts
 *============================================================================*/
void StatementTest::testStripSizes()
{
	// *v* r24 := m[zfill(8,32,local5) + param6]*8**8* / 16
	// The double size casting happens as a result of substitution
	Exp *lhs = Location::regOf(24);
	Exp *rhs = new Binary(
		opDiv,
		new Binary(opSize, new Const(8),
				   new Binary(opSize, new Const(8),
							  Location::memOf(new Binary(opPlus, new Ternary(opZfill, new Const(8), new Const(32),
																			 Location::local("local5", nullptr)),
														 Location::local("param6", nullptr))))),
		new Const(16));
	Statement *s = new Assign(lhs, rhs);

	s->stripSizes();
	std::string        expected("   0 *v* r24 := m[zfill(8,32,local5) + param6] / 16");
	std::string        actual;
	std::ostringstream ost;
	ost << s;
	actual = ost.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);
}


/***************************************************************************/ /**
 * FUNCTION:        StatementTest::testFindConstants
 * OVERVIEW:        Test the visitor code that finds constants
 *============================================================================*/
void StatementTest::testFindConstants()
{
	Statement *a = new Assign(Location::regOf(24), new Binary(opPlus, new Const(3), new Const(4)));

	std::list<Const *> lc;
	a->findConstants(lc);
	std::list<Const *>::iterator it;
	std::ostringstream           ost1;

	for (it = lc.begin(); it != lc.end(); ) {
		ost1 << *it;

		if (++it != lc.end()) {
			ost1 << ", ";
		}
	}

	std::string actual = ost1.str();
	std::string expected("3, 4");
	CPPUNIT_ASSERT_EQUAL(expected, actual);
}
