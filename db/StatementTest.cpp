/*==============================================================================
 * FILE:	   StatementTest.cc
 * OVERVIEW:   Provides the implementation for the StatementTest class, which
 *				tests the dataflow subsystems
 *============================================================================*/
/*
 * $Revision$
 *
 * 14 Jan 03 - Trent: Created
 * 17 Apr 03 - Mike: Added testRecursion to track down a nasty bug
 */

#define HELLO_PENTIUM	   "test/pentium/hello"
#define FIB_PENTIUM		   "test/pentium/fib"

#include "StatementTest.h"
#include "cfg.h"
#include "rtl.h"
#include "pentiumfrontend.h"
#include "boomerang.h"
#include "exp.h"
#include "managed.h"

#include <sstream>
#include <map>

class NullLogger : public Log {
public:
	virtual Log &operator<<(const char *str) {
	 //std::cerr << str;
		return *this;
	}
	virtual ~NullLogger() {};
};
/*==============================================================================
 * FUNCTION:		StatementTest::registerTests
 * OVERVIEW:		Register the test functions in the given suite
 * PARAMETERS:		Pointer to the test suite
 * RETURNS:			<nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<StatementTest> ("Statements", \
	&StatementTest::name, *this))

void StatementTest::registerTests(CppUnit::TestSuite* suite) {

	MYTEST(testLocationSet);
	MYTEST(testWildLocationSet);
	MYTEST(testEmpty);
	MYTEST(testFlow);
	MYTEST(testKill);
	MYTEST(testUse);
	MYTEST(testUseOverKill);
	MYTEST(testUseOverBB);
	MYTEST(testUseKill);
	//MYTEST(testEndlessLoop);
	//MYTEST(testRecursion);
	//MYTEST(testExpand);
	MYTEST(testClone);
	MYTEST(testIsAssign);
	MYTEST(testIsFlagAssgn);
	MYTEST(testAddUsedLocs);
	MYTEST(testSubscriptVars);
	MYTEST(testCallRefsFixer);
	MYTEST(testStripSizes);
}

int StatementTest::countTestCases () const
{ return 2; }	// ? What's this for?

/*==============================================================================
 * FUNCTION:		StatementTest::setUp
 * OVERVIEW:		Set up some expressions for use with all the tests
 * NOTE:			Called before any tests
 * PARAMETERS:		<none>
 * RETURNS:			<nothing>
 *============================================================================*/
static bool logset = false;
void StatementTest::setUp () {
	if (!logset) {
		logset = true;
		Boomerang::get()->setLogger(new NullLogger());
	}
}

/*==============================================================================
 * FUNCTION:		StatementTest::tearDown
 * OVERVIEW:		Delete expressions created in setUp
 * NOTE:			Called after all tests
 * PARAMETERS:		<none>
 * RETURNS:			<nothing>
 *============================================================================*/
void StatementTest::tearDown () {
}

/*==============================================================================
 * FUNCTION:		StatementTest::testEmpty
 * OVERVIEW:		
 *============================================================================*/
void StatementTest::testEmpty () {
	// Force "verbose" flag (-v)
	Boomerang* boo = Boomerang::get();
	boo->vFlag = true;
	boo->setOutputDirectory("./unit_test/");
	boo->setLogger(new FileLogger());

	// create Prog
	BinaryFile *pBF = BinaryFile::Load(HELLO_PENTIUM);	// Don't actually use it
	FrontEnd *pFE = new PentiumFrontEnd(pBF);
	Prog* prog = new Prog(pBF, pFE);
	// create UserProc
	std::string name = "test";
	UserProc* proc = (UserProc*) prog->newProc("test", 0x123);
	// create CFG
	Cfg *cfg = proc->getCFG();
	std::list<RTL*>* pRtls = new std::list<RTL*>();
	std::list<Statement*>* ls = new std::list<Statement*>;
	ls->push_back(new ReturnStatement);
	pRtls->push_back(new RTL(0x123));
	PBB bb = cfg->newBB(pRtls, RET, 0);
	cfg->setEntryBB(bb);
	proc->setDecoded();		// We manually "decoded"
	// compute dataflow
	proc->decompile();
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected = 
		"Ret BB:\n"
		"00000123\n\n";
	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}

/*==============================================================================
 * FUNCTION:		StatementTest::testFlow
 * OVERVIEW:		
 *============================================================================*/
void StatementTest::testFlow () {
	// create Prog
	BinaryFile *pBF = BinaryFile::Load(HELLO_PENTIUM);	// Don't actually use it
	FrontEnd *pFE = new PentiumFrontEnd(pBF);
	// We need a Prog object with a pBF (for getEarlyParamExp())
 	Prog* prog = new Prog(pBF, pFE);
	// create UserProc
	std::string name = "test";
	UserProc* proc = (UserProc*) prog->newProc("test", 0x123);
	// create CFG
	Cfg *cfg = proc->getCFG();
	std::list<RTL*>* pRtls = new std::list<RTL*>();
	RTL *rtl = new RTL();
	Assign *a = new Assign(Location::regOf(24),
		new Const(5));
	a->setProc(proc);
	a->setNumber(1);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);
	pRtls = new std::list<RTL*>();
	rtl = new RTL(0x123);
	ReturnStatement* rs = new ReturnStatement;
	rs->setNumber(2);
	rs->addReturn(Location::regOf(24));
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	PBB ret = cfg->newBB(pRtls, RET, 0);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);		// Also sets exitBB; important!
	proc->setDecoded();
	// compute dataflow
	proc->decompile();
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	// The assignment to 5 gets propagated into the return, and the assignment
	// to r24 is removed
	expected =
		"Fall BB:\n"
		"00000000\n"
		"Ret BB:\n"
		"00000123    2 RET 5\n\n";
	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}

/*==============================================================================
 * FUNCTION:		StatementTest::testKill
 * OVERVIEW:		
 *============================================================================*/
void StatementTest::testKill () {
	// create Prog
	BinaryFile *pBF = BinaryFile::Load(HELLO_PENTIUM);	// Don't actually use it
	FrontEnd *pFE = new PentiumFrontEnd(pBF);
	// We need a Prog object with a pBF (for getEarlyParamExp())
	Prog* prog = new Prog(pBF, pFE);
	// create UserProc
	std::string name = "test";
	UserProc* proc = (UserProc*) prog->newProc("test", 0x123);
	// create CFG
	Cfg *cfg = proc->getCFG();
	std::list<RTL*>* pRtls = new std::list<RTL*>();
	RTL *rtl = new RTL();
	Assign *e = new Assign(Location::regOf(24),
					 new Const(5));
	e->setNumber(1);
	e->setProc(proc);
	rtl->appendStmt(e);
	e = new Assign(Location::regOf(24),
				  new Const(6));
	e->setNumber(2);
	e->setProc(proc);
	rtl->appendStmt(e);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);
	pRtls = new std::list<RTL*>();
	rtl = new RTL(0x123);
	ReturnStatement* rs = new ReturnStatement;
	rs->setNumber(3);
	rs->addReturn(Location::regOf(24));
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	PBB ret = cfg->newBB(pRtls, RET, 0);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	proc->decompile();
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	expected =
	  "Fall BB:\n"
	  "00000000\n"
	  "Ret BB:\n"
	  "00000123    3 RET 6\n\n";
	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}

/*==============================================================================
 * FUNCTION:		StatementTest::testUse
 * OVERVIEW:		
 *============================================================================*/
void StatementTest::testUse () {
	// create Prog
	BinaryFile *pBF = BinaryFile::Load(HELLO_PENTIUM);	// Don't actually use it
	FrontEnd *pFE = new PentiumFrontEnd(pBF);
	// We need a Prog object with a pBF (for getEarlyParamExp())
	Prog* prog = new Prog(pBF, pFE);
	// create UserProc
	std::string name = "test";
	UserProc* proc = (UserProc*) prog->newProc("test", 0x123);
	// create CFG
	Cfg *cfg = proc->getCFG();
	std::list<RTL*>* pRtls = new std::list<RTL*>();
	RTL *rtl = new RTL();
	Assign *a = new Assign(Location::regOf(24),
					 new Const(5));
	a->setNumber(1);
	a->setProc(proc);
	rtl->appendStmt(a);
	a = new Assign(Location::regOf(28),
				  Location::regOf(24));
	a->setNumber(2);
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);
	pRtls = new std::list<RTL*>();
	rtl = new RTL(0x123);
	ReturnStatement* rs = new ReturnStatement;
	rs->setNumber(3);
	rs->addReturn(Location::regOf(28));
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	PBB ret = cfg->newBB(pRtls, RET, 0);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	proc->decompile();
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	expected =
	  "Fall BB:\n"
	  "00000000\n"
	  "Ret BB:\n"
	  "00000123    3 RET 5\n\n";
	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}

/*==============================================================================
 * FUNCTION:		StatementTest::testUseOverKill
 * OVERVIEW:		
 *============================================================================*/
void StatementTest::testUseOverKill () {
	// create Prog
	BinaryFile *pBF = BinaryFile::Load(HELLO_PENTIUM);	// Don't actually use it
	FrontEnd *pFE = new PentiumFrontEnd(pBF);
	// We need a Prog object with a pBF (for getEarlyParamExp())
	Prog* prog = new Prog(pBF, pFE);
	// create UserProc
	std::string name = "test";
	UserProc* proc = (UserProc*) prog->newProc("test", 0x123);
	// create CFG
	Cfg *cfg = proc->getCFG();
	std::list<RTL*>* pRtls = new std::list<RTL*>();
	RTL *rtl = new RTL();
	Assign *e = new Assign(Location::regOf(24),
					 new Const(5));
	e->setNumber(1);
	e->setProc(proc);
	rtl->appendStmt(e);
	e = new Assign(Location::regOf(24),
					 new Const(6));
	e->setNumber(2);
	e->setProc(proc);
	rtl->appendStmt(e);
	e = new Assign(Location::regOf(28),
				  Location::regOf(24));
	e->setNumber(3);
	e->setProc(proc);
	rtl->appendStmt(e);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);
	pRtls = new std::list<RTL*>();
	rtl = new RTL(0x123);
	ReturnStatement* rs = new ReturnStatement;
	rs->setNumber(2);
	rs->addReturn(Location::regOf(24));
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	PBB ret = cfg->newBB(pRtls, RET, 0);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	proc->decompile();
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	expected = 
	  "Fall BB:\n"
	  "00000000\n"
	  "Ret BB:\n"
	  "00000123    4 RET 6\n\n";
	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}

/*==============================================================================
 * FUNCTION:		StatementTest::testUseOverBB
 * OVERVIEW:		
 *============================================================================*/
void StatementTest::testUseOverBB () {
	// create Prog
	BinaryFile *pBF = BinaryFile::Load(HELLO_PENTIUM);	// Don't actually use it
	FrontEnd *pFE = new PentiumFrontEnd(pBF);
	// We need a Prog object with a pBF (for getEarlyParamExp())
	Prog* prog = new Prog(pBF, pFE);
	// create UserProc
	std::string name = "test";
	UserProc* proc = (UserProc*) prog->newProc("test", 0x123);
	// create CFG
	Cfg *cfg = proc->getCFG();
	std::list<RTL*>* pRtls = new std::list<RTL*>();
	RTL *rtl = new RTL();
	Assign *a = new Assign(Location::regOf(24),
					 new Const(5));
	a->setNumber(1);
	a->setProc(proc);
	rtl->appendStmt(a);
	a = new Assign(Location::regOf(24),
					 new Const(6));
	a->setNumber(2);
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);
	pRtls = new std::list<RTL*>();
	rtl = new RTL();
	a = new Assign(Location::regOf(28),
				  Location::regOf(24));
	a->setNumber(3);
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	rtl = new RTL(0x123);
	ReturnStatement* rs = new ReturnStatement;
	rs->setNumber(4);
	rs->addReturn(Location::regOf(24));
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	PBB ret = cfg->newBB(pRtls, RET, 0);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	proc->decompile();
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	expected =
	  "Fall BB:\n"
	  "00000000\n"
	  "Ret BB:\n"
	  "00000000\n"
	  "00000123    4 RET 6\n\n";
	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}

/*==============================================================================
 * FUNCTION:		StatementTest::testUseKill
 * OVERVIEW:		
 *============================================================================*/
void StatementTest::testUseKill () {
	// create Prog
	BinaryFile *pBF = BinaryFile::Load(HELLO_PENTIUM);	// Don't actually use it
	FrontEnd *pFE = new PentiumFrontEnd(pBF);
	// We need a Prog object with a pBF (for getEarlyParamExp())
	Prog* prog = new Prog(pBF, pFE);
	// create UserProc
	std::string name = "test";
	UserProc* proc = (UserProc*) prog->newProc("test", 0x123);
	// create CFG
	Cfg *cfg = proc->getCFG();
	std::list<RTL*>* pRtls = new std::list<RTL*>();
	RTL *rtl = new RTL();
	Assign *a = new Assign(Location::regOf(24),
					 new Const(5));
	a->setNumber(1);
	a->setProc(proc);
	rtl->appendStmt(a);
	a = new Assign(Location::regOf(24),
			  new Binary(opPlus, Location::regOf(24),
							 new Const(1)));
	a->setNumber(2);
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);
	pRtls = new std::list<RTL*>();
	rtl = new RTL(0x123);
	ReturnStatement* rs = new ReturnStatement;
	rs->setNumber(3);
	rs->addReturn(Location::regOf(24));
	rtl->appendStmt(rs);
	pRtls->push_back(rtl);
	PBB ret = cfg->newBB(pRtls, RET, 0);
	first->setOutEdge(0, ret);
	ret->addInEdge(first);
	cfg->setEntryBB(first);
	proc->setDecoded();
	// compute dataflow
	proc->decompile();
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	expected  = 
	  "Fall BB:\n"
	  "00000000\n"
	  "Ret BB:\n"
	  "00000123    3 RET 6\n\n";
	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}

/*==============================================================================
 * FUNCTION:		StatementTest::testEndlessLoop
 * OVERVIEW:		
 *============================================================================*/
void StatementTest::testEndlessLoop () {
	// create Prog
	BinaryFile *pBF = BinaryFile::Load(HELLO_PENTIUM);	// Don't actually use it
	FrontEnd *pFE = new PentiumFrontEnd(pBF);
	// We need a Prog object with a pBF (for getEarlyParamExp())
	Prog* prog = new Prog(pBF, pFE);
	// create UserProc
	std::string name = "test";
	UserProc* proc = (UserProc*) prog->newProc("test", 0x123);
	// create CFG
	Cfg *cfg = proc->getCFG();
	std::list<RTL*>* pRtls = new std::list<RTL*>();
	RTL *rtl = new RTL();
	// r[24] := 5
	Assign *e = new Assign(Location::regOf(24),
					 new Const(5));
	e->setProc(proc);
	rtl->appendStmt(e);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);
	pRtls = new std::list<RTL*>();
	rtl = new RTL();
	// r[24] := r[24] + 1
	e = new Assign(Location::regOf(24),
			  new Binary(opPlus, Location::regOf(24),
							 new Const(1)));
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
	proc->decompile();
	// print cfg to a string
	std::ostringstream st;
	cfg->print(st);
	std::string s = st.str();
	// compare it to expected
	std::string expected;
	expected =
	  "Fall BB: reach in: \n"
	  "00000000 *v* r[24] := 5\n"
	  "Oneway BB:\n"
	  "00000000 *v* r[24] := r[24] + 1   uses: ** r[24] := 5, "
	  "*v* r[24] := r[24] + 1,    used by: ** r[24] := r[24] + 1, \n"
	  "cfg reachExit: \n";
	CPPUNIT_ASSERT_EQUAL(expected, s);
	// clean up
	delete prog;
}

/*==============================================================================
 * FUNCTION:		StatementTest::testLocationSet
 * OVERVIEW:		
 *============================================================================*/
void StatementTest::testLocationSet () {
	Location rof(opRegOf, new Const(12), NULL);
	Const& theReg = *(Const*)rof.getSubExp1();
	LocationSet ls;
	LocationSet::iterator ii;
	ls.insert(rof.clone());
	theReg.setInt(8);
	ls.insert(rof.clone());
	theReg.setInt(31);
	ls.insert(rof.clone());
	theReg.setInt(24);
	ls.insert(rof.clone());
	theReg.setInt(12);
	ls.insert(rof.clone());		// Note: r[12] already inserted
	CPPUNIT_ASSERT_EQUAL(4, ls.size());
	theReg.setInt(8);
	ii = ls.begin();
	CPPUNIT_ASSERT(rof == **ii);
	theReg.setInt(12);
	Exp* e;
	e = *(++ii); CPPUNIT_ASSERT(rof == *e);
	theReg.setInt(24);
	e = *(++ii); CPPUNIT_ASSERT(rof == *e);
	theReg.setInt(31);
	e = *(++ii); CPPUNIT_ASSERT(rof == *e);
	Location mof(opMemOf,
		new Binary(opPlus,
			Location::regOf(14),
			new Const(4)), NULL);
	ls.insert(mof.clone());
	ls.insert(mof.clone());
	CPPUNIT_ASSERT_EQUAL(5, ls.size());
	ii = ls.begin();
	CPPUNIT_ASSERT(mof == **ii);
	LocationSet ls2 = ls;
	Exp* e2 = *ls2.begin();
	CPPUNIT_ASSERT(e2 != *ls.begin());		// Must be cloned
	CPPUNIT_ASSERT_EQUAL(5, ls2.size());
	CPPUNIT_ASSERT(mof == **ls2.begin());
	theReg.setInt(8);
	e = *(++ls2.begin()); CPPUNIT_ASSERT(rof == *e);
}

/*==============================================================================
 * FUNCTION:		StatementTest::testWildLocationSet
 * OVERVIEW:		
 *============================================================================*/
void StatementTest::testWildLocationSet () {
	Location rof12(opRegOf, new Const(12), NULL);
	Location rof13(opRegOf, new Const(13), NULL);
	Assign a10, a20;
	a10.setNumber(10);
	a20.setNumber(20);
	RefExp r12_10(rof12.clone(), &a10);
	RefExp r12_20(rof12.clone(), &a20);
	RefExp r12_0 (rof12.clone(), NULL);
	RefExp r13_10(rof13.clone(), &a10);
	RefExp r13_20(rof13.clone(), &a20);
	RefExp r13_0 (rof13.clone(), NULL);
	RefExp r11_10(Location::regOf(11), &a10);
	RefExp r22_10(Location::regOf(22), &a10);
	LocationSet ls;
	ls.insert(&r12_10);
	ls.insert(&r12_20);
	ls.insert(&r12_0);
	ls.insert(&r13_10);
	ls.insert(&r13_20);
	ls.insert(&r13_0);
	RefExp wildr12(rof12.clone(), (Statement*)-1);
	CPPUNIT_ASSERT(ls.find(&wildr12));
	RefExp wildr13(rof13.clone(), (Statement*)-1);
	CPPUNIT_ASSERT(ls.find(&wildr13));
	RefExp wildr10(Location::regOf(10), (Statement*)-1);
	CPPUNIT_ASSERT(!ls.find(&wildr10));
	// Test findDifferentRef
	Exp* x;
	CPPUNIT_ASSERT( ls.findDifferentRef(&r13_10, x));
	CPPUNIT_ASSERT( ls.findDifferentRef(&r13_20, x));
	CPPUNIT_ASSERT( ls.findDifferentRef(&r13_0 , x));
	CPPUNIT_ASSERT( ls.findDifferentRef(&r12_10, x));
	CPPUNIT_ASSERT( ls.findDifferentRef(&r12_20, x));
	CPPUNIT_ASSERT( ls.findDifferentRef(&r12_0 , x));
	// Next 4 should fail
	CPPUNIT_ASSERT(!ls.findDifferentRef(&r11_10, x));
	CPPUNIT_ASSERT(!ls.findDifferentRef(&r22_10, x));
	ls.insert(&r11_10);
	ls.insert(&r22_10);
	CPPUNIT_ASSERT(!ls.findDifferentRef(&r11_10, x));
	CPPUNIT_ASSERT(!ls.findDifferentRef(&r22_10, x));
}

/*==============================================================================
 * FUNCTION:		StatementTest::testRecursion
 * OVERVIEW:		Test push of argument (X86 style), then call self
 *============================================================================*/
void StatementTest::testRecursion () {
	// create Prog
	BinaryFile *pBF = BinaryFile::Load(HELLO_PENTIUM);	// Don't actually use it
	FrontEnd *pFE = new PentiumFrontEnd(pBF);
	// We need a Prog object with a pBF (for getEarlyParamExp())
	Prog* prog = new Prog(pBF, pFE);
	// create UserProc
	std::string name = "test";
	UserProc *proc = new UserProc(prog, name, 0);
	// create CFG
	Cfg *cfg = proc->getCFG();
	std::list<RTL*>* pRtls = new std::list<RTL*>();
	RTL *rtl = new RTL();
	// push bp
	// r28 := r28 + -4
	Assign *a = new Assign(Location::regOf(28),
		new Binary(opPlus,
			Location::regOf(28),
			new Const(-4)));
	rtl->appendStmt(a);
	// m[r28] := r29
	a = new Assign(
		Location::memOf(
			Location::regOf(28)),
		Location::regOf(29));
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	pRtls = new std::list<RTL*>();
	// push arg+1
	// r28 := r28 + -4
	a = new Assign(Location::regOf(28),
			new Binary(opPlus,
				Location::regOf(28),
				new Const(-4)));
	rtl->appendStmt(a);
	// Reference our parameter. At esp+0 is this arg; at esp+4 is old bp;
	// esp+8 is return address; esp+12 is our arg
	// m[r28] := m[r28+12] + 1
	a = new Assign(Location::memOf(Location::regOf(28)),
					 new Binary(opPlus,
						Location::memOf(
							new Binary(opPlus,
								Location::regOf(28),
								new Const(12))),
						new Const(1)));
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	PBB first = cfg->newBB(pRtls, FALL, 1);

	// The call BB
	pRtls = new std::list<RTL*>();
	rtl = new RTL(1);
	// r28 := r28 + -4
	a = new Assign(Location::regOf(28),
		new Binary(opPlus, Location::regOf(28), new Const(-4)));
	rtl->appendStmt(a);
	// m[r28] := pc
	a = new Assign(Location::memOf(Location::regOf(28)),
		new Terminal(opPC));
	rtl->appendStmt(a);
	// %pc := (%pc + 5) + 135893848
	a = new Assign(new Terminal(opPC),
		new Binary(opPlus,
			new Binary(opPlus,
				new Terminal(opPC),
				new Const(5)),
			new Const(135893848)));
	a->setProc(proc);
	rtl->appendStmt(a);
	pRtls->push_back(rtl);
	CallStatement* c = new CallStatement;
	rtl->appendStmt(c);
#if 0
	// Vector of 1 arg
	std::vector<Exp*> args;
	// m[r[28]+8]
	Exp* a = Location::memOf( new Binary(opPlus,
	  Location::regOf(28), new Const(8)));
	args.push_back(a);
	crtl->setArguments(args);
#endif
	c->setDestProc(proc);		 // Just call self
	PBB callbb = cfg->newBB(pRtls, CALL, 1);
	first->setOutEdge(0, callbb);
	callbb->addInEdge(first);
	callbb->setOutEdge(0, callbb);
	callbb->addInEdge(callbb);

	pRtls = new std::list<RTL*>();
	rtl = new RTL(0x123);
	rtl->appendStmt(new ReturnStatement);
	// This ReturnStatement requires the following two sets of semantics to pass the
	// tests for standard Pentium calling convention
	// pc = m[r28]
	a = new Assign(new Terminal(opPC),
		Location::memOf(
			Location::regOf(28)));
	rtl->appendStmt(a);
	// r28 = r28 + 4
	a = new Assign(Location::regOf(28),
		new Binary(opPlus,
			Location::regOf(28),
			new Const(4)));
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
	expected =
	  "Fall BB: reach in: \n"
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

/*==============================================================================
 * FUNCTION:		StatamentTest::testClone
 * OVERVIEW:		Test cloning of Assigns (and exps)
 *============================================================================*/
void StatementTest::testClone () {
	Assign* a1 = new Assign(
			Location::regOf(8),
			new Binary(opPlus,
				Location::regOf(9),
				new Const(99)));
	Assign* a2 = new Assign(new IntegerType(16, 1),
			new Location(opParam, new Const("x"), NULL),
			new Location(opParam, new Const("y"), NULL));
	Assign* a3 = new Assign(new IntegerType(16, -1),
			new Location(opParam, new Const("z"), NULL),
			new Location(opParam, new Const("q"), NULL));
	Statement* c1 = a1->clone();
	Statement* c2 = a2->clone();
	Statement* c3 = a3->clone();
	std::ostringstream o1, o2;
	a1->print(o1);
	delete a1;			 // And c1 should still stand!
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
 
/*==============================================================================
 * FUNCTION:		StatementTest::testIsAssign
 * OVERVIEW:		Test assignment test
 *============================================================================*/
void StatementTest::testIsAssign () {
	std::ostringstream ost;
	// r2 := 99
	Assign a(
		Location::regOf(2),
		new Const(99));
	a.print(ost);
	std::string expected("   0 *v* r2 := 99");
	std::string actual (ost.str());
	CPPUNIT_ASSERT_EQUAL(expected, actual);
//	  CPPUNIT_ASSERT_EQUAL (std::string("*v* r2 := 99"), std::string(ost.str()));
	CPPUNIT_ASSERT(a.isAssign());

	CallStatement* c = new CallStatement;
	CPPUNIT_ASSERT(!c->isAssign());
}

/*==============================================================================
 * FUNCTION:		StatementTest::testIsFlagCall
 * OVERVIEW:		Test the isFlagAssgn function, and opFlagCall
 *============================================================================*/
void StatementTest::testIsFlagAssgn () {
	std::ostringstream ost;
	// FLAG addFlags(r2 , 99)
	Assign fc(
		new Terminal(opFlags),
		new Binary (opFlagCall,
			new Const("addFlags"),
			new Binary(opList,
				Location::regOf(2),
				new Const(99))));
	CallStatement* call = new CallStatement;
	BranchStatement* br = new BranchStatement;
	Assign* as = new Assign(
		Location::regOf(9),
		new Binary(opPlus,
			Location::regOf(10),
			new Const(4)));
	fc.print(ost);
	std::string expected("   0 *v* %flags := addFlags( r2, 99 )");
	std::string actual(ost.str());
	CPPUNIT_ASSERT_EQUAL(expected, actual);
	CPPUNIT_ASSERT (	fc.isFlagAssgn());
	CPPUNIT_ASSERT (!call->isFlagAssgn());
	CPPUNIT_ASSERT (!  br->isFlagAssgn());
	CPPUNIT_ASSERT (!  as->isFlagAssgn());
	delete call; delete br;
}

/*==============================================================================
 * FUNCTION:		StatementTest::testAddUsedLocs
 * OVERVIEW:		Test the finding of locations used by this statement
 *============================================================================*/
void StatementTest::testAddUsedLocs () {
	// m[r28-4] := m[r28-8] * r26
	Assign* a = new Assign(
		Location::memOf(
			new Binary(opMinus,
				Location::regOf(28),
				new Const(4))),
		new Binary(opMult,
			Location::memOf(
				new Binary(opMinus,
					Location::regOf(28),
					new Const(8))),
				Location::regOf(26)));
	a->setNumber(1);
	LocationSet l;
	a->addUsedLocs(l);
	std::ostringstream ost1;
	l.print(ost1);
	std::string expected = "m[r28 - 8],\tr26,\tr28\n";
	std::string actual = ost1.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	l.clear();
	GotoStatement* g = new GotoStatement();
	g->setNumber(55);
	g->setDest(Location::memOf(Location::regOf(26)));
	g->addUsedLocs(l);
	std::ostringstream ost2;
	l.print(ost2);
	expected = "m[r26],\tr26\n";
	actual = ost2.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// BranchStatement with dest m[r26{99}]{55}, condition %flags
	l.clear();
	BranchStatement* b = new BranchStatement;
	b->setNumber(99);
	b->setDest(
		new RefExp(
			Location::memOf(
				new RefExp(
					Location::regOf(26),
					b)),
			g));
	b->setCondExpr(new Terminal(opFlags));
	b->addUsedLocs(l);
	std::ostringstream ost3;
	l.print(ost3);
	expected = "m[r26{99}]{55},\tr26{99},\t%flags\n";
	actual = ost3.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// CaseStatement with pDest = m[r26], switchVar = m[r28 - 12]
	l.clear();
	CaseStatement* c = new CaseStatement;
	c->setDest(Location::memOf(Location::regOf(26)));
	SWITCH_INFO si;
	si.pSwitchVar = Location::memOf(
		new Binary(opMinus,
			Location::regOf(28),
			new Const(12)));
	c->setSwitchInfo(&si);
	c->addUsedLocs(l);
	std::ostringstream ost4;
	l.print(ost4);
	expected = "m[r28 - 12],\tm[r26],\tr26,\tr28\n";
	actual = ost4.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);
	
	// CallStatement with pDest = m[r26], params = m[r27], r28{55},
	//	 implicit params m[r29], r30, returns r31, m[r24]
	l.clear();
	CallStatement* ca = new CallStatement;
	ca->setDest(Location::memOf(Location::regOf(26)));
	std::vector<Exp*> argl;
	argl.push_back(Location::memOf(Location::regOf(27)));
	argl.push_back(new RefExp(Location::regOf(28), g));
	ca->setArguments(argl);
	argl.clear();
	argl.push_back(Location::memOf(Location::regOf(29)));
	argl.push_back(Location::regOf(30));
	ca->setImpArguments(argl);
	ca->addReturn(Location::regOf(31));
	ca->addReturn(Location::memOf(Location::regOf(24)));
	ca->addUsedLocs(l);
	std::ostringstream ost5;
	l.print(ost5);
	expected =
	  "m[r26],\tm[r27],\tm[r29],\tr24,\tr26,\tr27,\tr29,\tr30,\tr28{55}\n";
	actual = ost5.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// Now with final
	l.clear();
	ca->addUsedLocs(l, true);
	std::ostringstream ost5f;
	l.print(ost5f);
	expected = "m[r26],\tm[r27],\tr26,\tr27,\tr28{55}\n";
	actual = ost5f.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);
	
	// ReturnStatement with returns r31, m[r24], m[r25]{55} + r[26]{99}]
	l.clear();
	ReturnStatement* r = new ReturnStatement;
	r->setDest(Location::memOf(Location::regOf(26)));
	r->addReturn(Location::regOf(31));
	r->addReturn(Location::memOf(Location::regOf(24)));
	r->addReturn(Location::memOf(
		new Binary(opPlus,
			new RefExp(Location::regOf(25), g),
			new RefExp(Location::regOf(26), b))));
	r->addUsedLocs(l);
	std::ostringstream ost6;
	l.print(ost6);
	expected="m[r25{55} + r26{99}],\tm[r24],\tr24,\tr31,\tr25{55},\tr26{99}\n";
	actual = ost6.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// Boolstatement with condition m[r24] = r25, dest m[r26]
	l.clear();
	BoolAssign* bs = new BoolAssign(8);
	bs->setCondExpr(new Binary(opEquals,
		Location::memOf(Location::regOf(24)),
		Location::regOf(25)));
	std::list<Statement*> stmts;
	a = new Assign(Location::memOf(Location::regOf(26)), new Terminal(opNil));
	stmts.push_back(a);
	bs->setLeftFromList(&stmts);
	bs->addUsedLocs(l);
	std::ostringstream ost7;
	l.print(ost7);
	expected="m[r24],\tr24,\tr25,\tr26\n";
	actual = ost7.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// m[local21 + 16] := phi{0, 372}
	l.clear();
	Exp* base = Location::memOf(
    	new Binary(opPlus,
			Location::local("local21", NULL),
			new Const(16)));
	Assign s372(base, new Const(0));
	s372.setNumber(372);
	PhiAssign* pa = new PhiAssign(base);
	pa->putAt(0, NULL);
	pa->putAt(1, &s372);
	pa->addUsedLocs(l);
	// Note: phis were not considered to use blah if they ref m[blah],
	// so local21 was not considered used
	expected = "m[local21 + 16]{0},\tm[local21 + 16]{372},\tlocal21\n";
	std::ostringstream ost8;
	l.print(ost8);
	actual = ost8.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

}

/*==============================================================================
 * FUNCTION:		StatementTest::testSubscriptVars
 * OVERVIEW:		Test the subscripting of locations in Statements
 *============================================================================*/
void StatementTest::testSubscriptVars () {
	Exp* srch = Location::regOf(28);
	Assign s9(new Const(0), new Const(0));
	s9.setNumber(9);

	// m[r28-4] := m[r28-8] * r26
	Assign* a = new Assign(
		Location::memOf(
			new Binary(opMinus,
				Location::regOf(28),
				new Const(4))),
		new Binary(opMult,
			Location::memOf(
				new Binary(opMinus,
					Location::regOf(28),
					new Const(8))),
				Location::regOf(26)));
	a->setNumber(1);
	std::ostringstream ost1;
	a->subscriptVar(srch, &s9);
	ost1 << a;
	std::string expected = "   1 *v* m[r28{9} - 4] := m[r28{9} - 8] * r26";
	std::string actual = ost1.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// GotoStatement
	GotoStatement* g = new GotoStatement();
	g->setNumber(55);
	g->setDest(Location::regOf(28));
	std::ostringstream ost2;
	g->subscriptVar(srch, &s9);
	ost2 << g;
	expected = "  55 GOTO r28{9}";
	actual = ost2.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// BranchStatement with dest m[r26{99}]{55}, condition %flags
	BranchStatement* b = new BranchStatement;
	b->setNumber(99);
	Exp* srchb = Location::memOf(
				new RefExp(
					Location::regOf(26),
					b));
	b->setDest(new RefExp(srchb, g));
	b->setCondExpr(new Terminal(opFlags));
	std::ostringstream ost3;
	b->subscriptVar(srchb, &s9);
	b->subscriptVar(new Terminal(opFlags), g);
	ost3 << b;
	expected = "  99 BRANCH m[r26{99}]{9}, condition equals\n"
		"High level: %flags{55}";
	actual = ost3.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// CaseStatement with pDest = m[r26], switchVar = m[r28 - 12]
	CaseStatement* c = new CaseStatement;
	c->setDest(Location::memOf(Location::regOf(26)));
	SWITCH_INFO si;
	si.pSwitchVar = Location::memOf(
		new Binary(opMinus,
			Location::regOf(28),
			new Const(12)));
	c->setSwitchInfo(&si);
	std::ostringstream ost4;
	c->subscriptVar(srch, &s9);
	ost4 << c;
	expected = "   0 SWITCH(m[r28{9} - 12])\n";
	actual = ost4.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// CaseStatement (before recog) with pDest = r28, switchVar is NULL
	c->setDest(Location::regOf(28));
	c->setSwitchInfo(NULL);
	std::ostringstream ost4a;
	c->subscriptVar(srch, &s9);
	ost4a << c;
	expected = "   0 CASE [r28{9}]";
	actual = ost4a.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);
	
	// CallStatement with pDest = m[r26], params = m[r27], r28,
	//	 implicit params m[r29], r30, returns r28, m[r28]
	CallStatement* ca = new CallStatement;
	ca->setDest(Location::memOf(Location::regOf(26)));
	std::vector<Exp*> argl;
	argl.push_back(Location::memOf(Location::regOf(27)));
	argl.push_back(Location::regOf(28));
	ca->setArguments(argl);
	argl.clear();
	argl.push_back(Location::memOf(Location::regOf(29)));
	argl.push_back(Location::regOf(30));
	ca->setImpArguments(argl);
	ca->addReturn(Location::regOf(28));
	ca->addReturn(Location::memOf(Location::regOf(28)));
	std::ostringstream ost5;
	ca->subscriptVar(srch, &s9);
	ost5 << ca;
	expected =
	"   0 CALL m[r26](m[r27], r28{9} implicit: m[r29], r30) { r28, m[r28{9}] }";
	actual = ost5.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// CallStatement with pDest = r28, params = m[r27], r29,
	//	 implicit params m[r29], r28, returns r31, m[r31]
	ca = new CallStatement;
	ca->setDest(Location::regOf(28));
	argl.clear();
	argl.push_back(Location::memOf(Location::regOf(27)));
	argl.push_back(Location::regOf(29));
	ca->setArguments(argl);
	argl.clear();
	argl.push_back(Location::memOf(Location::regOf(29)));
	argl.push_back(Location::regOf(28));
	ca->setImpArguments(argl);
	ca->addReturn(Location::regOf(31));
	ca->addReturn(Location::memOf(Location::regOf(31)));
	std::ostringstream ost5a;
	ca->subscriptVar(srch, &s9);
	ost5a << ca;
	expected =
	"   0 CALL r28{9}(m[r27], r29 implicit: m[r29], r28{9}) { r31, m[r31] }";
	actual = ost5a.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);


	// ReturnStatement with returns r28, m[r28], m[r28]{55} + r[26]{99}]
	// The {55} one is a bit dodgy to me, but that's how the old subscriptVar
	// code worked
	ReturnStatement* r = new ReturnStatement;
	r->setDest(Location::memOf(Location::regOf(28)));	// Not used?
	r->addReturn(Location::regOf(28));
	r->addReturn(Location::memOf(Location::regOf(28)));
	r->addReturn(Location::memOf(
		new Binary(opPlus,
			new RefExp(Location::regOf(28), g),
			new RefExp(Location::regOf(26), b))));
	std::ostringstream ost6;
	r->subscriptVar(srch, &s9);
	ost6 << r;
	expected="   0 RET r28{9}, m[r28{9}], m[r28{9} + r26{99}]";
	actual = ost6.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// Boolstatement with condition m[r28] = r28, dest m[r28]
	BoolAssign* bs = new BoolAssign(8);
	bs->setCondExpr(new Binary(opEquals,
		Location::memOf(Location::regOf(28)),
		Location::regOf(28)));
	bs->setLeft(Location::memOf(Location::regOf(28)));
	std::ostringstream ost7;
	bs->subscriptVar(srch, &s9);
	ost7 << bs;
	expected="   0 BOOL m[r28{9}] := CC(equals)\n"
		"High level: m[r28{9}] = r28{9}\n";
	actual = ost7.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);
}

/*==============================================================================
 * FUNCTION:		StatementTest::testCallRefsFixer
 * OVERVIEW:		Test the visitor code that fixes references that were to
 *					  return locations from calls
 *============================================================================*/
void StatementTest::testCallRefsFixer () {
	BinaryFile *pBF = BinaryFile::Load(FIB_PENTIUM);
	FrontEnd *pFE = new PentiumFrontEnd(pBF);
	Type::clearNamedTypes();
	Prog *prog = pFE->decode();
	bool gotMain;
	ADDRESS addr = pFE->getMainEntryPoint(gotMain);
	CPPUNIT_ASSERT (addr != NO_ADDRESS);
	UserProc* proc = (UserProc*) prog->findProc("fib");
	assert(proc);
	Cfg* cfg = proc->getCFG();
	// Sort by address
	cfg->sortByAddress();
	// Initialise statements
	proc->initStatements();
	// Compute dominance frontier
	cfg->dominators();
	// Number the statements
	int stmtNumber = 0;
	proc->numberStatements(stmtNumber);
	cfg->renameBlockVars(0, 0);		 // Block 0, mem depth 0
	cfg->renameBlockVars(0, 1);		 // Block 0, mem depth 1
	// Find various needed statements
	StatementList stmts;
	proc->getStatements(stmts);
	StatementList::iterator it;
	it = stmts.begin();							// Statement 1
	advance(it, 20-1);
	CallStatement* call = (CallStatement*)*it;	// Statement 20
	call->setDestProc(proc);					// A recursive call
	// std::cerr << "Call is " << call << "\n";
	advance(it, 2);
	Statement* s22 = *it;						// Statement 22
	// Make sure it's what we expect!
	std::string expected("  22 *32* r24 := m[r29{20} + 8]{0}");
	std::string actual;
	std::ostringstream ost1;
	ost1 << s22;
	actual = ost1.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);
	// Fake it to be known that r29 is preserved
	Exp* r29 = Location::regOf(29);
	proc->setProven(new Binary(opEquals, r29, r29->clone()));
	(*it)->fixCallRefs();
	// Now expect r29{30} to be r29{3}
	expected = "  22 *32* r24 := m[r29{3} + 8]{0}";
	std::ostringstream ost2;
	ost2 << *it;
	actual = ost2.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);
}

/*==============================================================================
 * FUNCTION:		StatementTest::testStripSizes
 * OVERVIEW:		Test the visitor code that strips out size casts
 *============================================================================*/
void StatementTest::testStripSizes () {
	// *v* r24 := m[zfill(8,32,local5) + param6]*8**8* / 16
	// The double size casting happens as a result of substitution
	Exp* lhs = Location::regOf(24);
	Exp* rhs = new Binary(opDiv,
		new Binary(opSize,
			new Const(8),
			new Binary(opSize,
				new Const(8),
				Location::memOf(
					new Binary(opPlus,
						new Ternary(opZfill,
							new Const(8),
							new Const(32),
							Location::local("local5", NULL)),
						Location::local("param6", NULL))))),
		new Const(16));
	Statement* s = new Assign(lhs, rhs);
	s->stripSizes();
	std::string expected(
	  "   0 *v* r24 := m[zfill(8,32,local5) + param6] / 16");
	std::string actual;
	std::ostringstream ost;
	ost << s;
	actual = ost.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);
}
