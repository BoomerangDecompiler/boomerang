/*==============================================================================
 * FILE:       DataflowTest.cc
 * OVERVIEW:   Provides the implementation for the DataflowTest class, which
 *              tests the dataflow subsystems
 *============================================================================*/
/*
 * $Revision$
 *
 * 14 Jan 03 - Trent: Created
 */

#include "DataflowTest.h"
#include "cfg.h"
#include "rtl.h"

#include <sstream>
#include <map>

/*==============================================================================
 * FUNCTION:        DataflowTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<DataflowTest> ("testDataflow", \
    &DataflowTest::name, *this))

void DataflowTest::registerTests(CppUnit::TestSuite* suite) {

    MYTEST(testEmpty);
    MYTEST(testFlow);
    MYTEST(testKill);
    MYTEST(testUse);
    MYTEST(testUseOverKill);
    MYTEST(testUseOverBB);
    MYTEST(testUseKill);
    MYTEST(testEndlessLoop);
}

int DataflowTest::countTestCases () const
{ return 2; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        DataflowTest::setUp
 * OVERVIEW:        Set up some expressions for use with all the tests
 * NOTE:            Called before any tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void DataflowTest::setUp () {
}

/*==============================================================================
 * FUNCTION:        DataflowTest::tearDown
 * OVERVIEW:        Delete expressions created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void DataflowTest::tearDown () {
}

/*==============================================================================
 * FUNCTION:        DataflowTest::testEmpty
 * OVERVIEW:        
 *============================================================================*/
void DataflowTest::testEmpty () {
    // create Prog
    Prog *prog = new Prog();
    // create UserProc
    std::string name = "test";
    UserProc *proc = new UserProc(prog, name, 0x123);
    // create CFG
    Cfg *cfg = proc->getCFG();
    std::list<RTL*>* pRtls = new std::list<RTL*>();
    pRtls->push_back(new HLReturn(0x123));
    cfg->newBB(pRtls, RET, 0);
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected = "Ret BB: live in: \n00000123 RET\n";
    CPPUNIT_ASSERT_EQUAL(expected, s);
    // clean up
    delete prog;
}

/*==============================================================================
 * FUNCTION:        DataflowTest::testFlow
 * OVERVIEW:        
 *============================================================================*/
void DataflowTest::testFlow () {
    // create Prog
    Prog *prog = new Prog();
    // create UserProc
    std::string name = "test";
    UserProc *proc = new UserProc(prog, name, 0x123);
    // create CFG
    Cfg *cfg = proc->getCFG();
    std::list<RTL*>* pRtls = new std::list<RTL*>();
    RTL *rtl = new RTL();
    AssignExp *e = new AssignExp(new Unary(opRegOf, new Const(24)),
			         new Const(5));
    e->setProc(proc);
    rtl->appendExp(e);
    pRtls->push_back(rtl);
    PBB first = cfg->newBB(pRtls, FALL, 1);
    pRtls = new std::list<RTL*>();
    pRtls->push_back(new HLReturn(0x123));
    PBB ret = cfg->newBB(pRtls, RET, 0);
    first->setOutEdge(0, ret);
    ret->addInEdge(first);
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected;
    expected += "Fall BB: live in: \n";
    expected += "00000000 *32* r[24] := 5   uses:    used by: \n";
    expected += "Ret BB: live in: *32* r[24] := 5, \n";
    expected += "00000123 RET\n";
    CPPUNIT_ASSERT_EQUAL(expected, s);
    // clean up
    delete prog;
}

/*==============================================================================
 * FUNCTION:        DataflowTest::testKill
 * OVERVIEW:        
 *============================================================================*/
void DataflowTest::testKill () {
    // create Prog
    Prog *prog = new Prog();
    // create UserProc
    std::string name = "test";
    UserProc *proc = new UserProc(prog, name, 0x123);
    // create CFG
    Cfg *cfg = proc->getCFG();
    std::list<RTL*>* pRtls = new std::list<RTL*>();
    RTL *rtl = new RTL();
    AssignExp *e = new AssignExp(new Unary(opRegOf, new Const(24)),
			         new Const(5));
    e->setProc(proc);
    rtl->appendExp(e);
    e = new AssignExp(new Unary(opRegOf, new Const(24)),
	              new Const(6));
    e->setProc(proc);
    rtl->appendExp(e);
    pRtls->push_back(rtl);
    PBB first = cfg->newBB(pRtls, FALL, 1);
    pRtls = new std::list<RTL*>();
    pRtls->push_back(new HLReturn(0x123));
    PBB ret = cfg->newBB(pRtls, RET, 0);
    first->setOutEdge(0, ret);
    ret->addInEdge(first);
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected;
    expected += "Fall BB: live in: \n";
    expected += "00000000 *32* r[24] := 5   uses:    used by: \n";
    expected += "         *32* r[24] := 6   uses:    used by: \n";
    expected += "Ret BB: live in: *32* r[24] := 6, \n";
    expected += "00000123 RET\n";
    CPPUNIT_ASSERT_EQUAL(expected, s);
    // clean up
    delete prog;
}

/*==============================================================================
 * FUNCTION:        DataflowTest::testUse
 * OVERVIEW:        
 *============================================================================*/
void DataflowTest::testUse () {
    // create Prog
    Prog *prog = new Prog();
    // create UserProc
    std::string name = "test";
    UserProc *proc = new UserProc(prog, name, 0);
    // create CFG
    Cfg *cfg = proc->getCFG();
    std::list<RTL*>* pRtls = new std::list<RTL*>();
    RTL *rtl = new RTL();
    AssignExp *e = new AssignExp(new Unary(opRegOf, new Const(24)),
			         new Const(5));
    e->setProc(proc);
    rtl->appendExp(e);
    e = new AssignExp(new Unary(opRegOf, new Const(28)),
	              new Unary(opRegOf, new Const(24)));
    e->setProc(proc);
    rtl->appendExp(e);
    pRtls->push_back(rtl);
    PBB first = cfg->newBB(pRtls, FALL, 1);
    pRtls = new std::list<RTL*>();
    pRtls->push_back(new HLReturn(0x123));
    PBB ret = cfg->newBB(pRtls, RET, 0);
    first->setOutEdge(0, ret);
    ret->addInEdge(first);
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected;
    expected += "Fall BB: live in: \n";
    expected += "00000000 *32* r[24] := 5   uses:    used by: *32* r[28] := r[24], \n";
    expected += "         *32* r[28] := r[24]   uses: *32* r[24] := 5,    used by: \n";
    expected += "Ret BB: live in: *32* r[24] := 5, *32* r[28] := r[24], \n";
    expected += "00000123 RET\n";
    CPPUNIT_ASSERT_EQUAL(expected, s);
    // clean up
    delete prog;
}

/*==============================================================================
 * FUNCTION:        DataflowTest::testUseOverKill
 * OVERVIEW:        
 *============================================================================*/
void DataflowTest::testUseOverKill () {
    // create Prog
    Prog *prog = new Prog();
    // create UserProc
    std::string name = "test";
    UserProc *proc = new UserProc(prog, name, 0);
    // create CFG
    Cfg *cfg = proc->getCFG();
    std::list<RTL*>* pRtls = new std::list<RTL*>();
    RTL *rtl = new RTL();
    AssignExp *e = new AssignExp(new Unary(opRegOf, new Const(24)),
			         new Const(5));
    e->setProc(proc);
    rtl->appendExp(e);
    e = new AssignExp(new Unary(opRegOf, new Const(24)),
			         new Const(6));
    e->setProc(proc);
    rtl->appendExp(e);
    e = new AssignExp(new Unary(opRegOf, new Const(28)),
	              new Unary(opRegOf, new Const(24)));
    e->setProc(proc);
    rtl->appendExp(e);
    pRtls->push_back(rtl);
    PBB first = cfg->newBB(pRtls, FALL, 1);
    pRtls = new std::list<RTL*>();
    pRtls->push_back(new HLReturn(0x123));
    PBB ret = cfg->newBB(pRtls, RET, 0);
    first->setOutEdge(0, ret);
    ret->addInEdge(first);
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected;
    expected += "Fall BB: live in: \n";
    expected += "00000000 *32* r[24] := 5   uses:    used by: \n";
    expected += "         *32* r[24] := 6   uses:    used by: *32* r[28] := r[24], \n";
    expected += "         *32* r[28] := r[24]   uses: *32* r[24] := 6,    used by: \n";
    expected += "Ret BB: live in: *32* r[24] := 6, *32* r[28] := r[24], \n";
    expected += "00000123 RET\n";
    CPPUNIT_ASSERT_EQUAL(expected, s);
    // clean up
    delete prog;
}

/*==============================================================================
 * FUNCTION:        DataflowTest::testUseOverBB
 * OVERVIEW:        
 *============================================================================*/
void DataflowTest::testUseOverBB () {
    // create Prog
    Prog *prog = new Prog();
    // create UserProc
    std::string name = "test";
    UserProc *proc = new UserProc(prog, name, 0);
    // create CFG
    Cfg *cfg = proc->getCFG();
    std::list<RTL*>* pRtls = new std::list<RTL*>();
    RTL *rtl = new RTL();
    AssignExp *e = new AssignExp(new Unary(opRegOf, new Const(24)),
			         new Const(5));
    e->setProc(proc);
    rtl->appendExp(e);
    e = new AssignExp(new Unary(opRegOf, new Const(24)),
			         new Const(6));
    e->setProc(proc);
    rtl->appendExp(e);
    pRtls->push_back(rtl);
    PBB first = cfg->newBB(pRtls, FALL, 1);
    pRtls = new std::list<RTL*>();
    rtl = new RTL();
    e = new AssignExp(new Unary(opRegOf, new Const(28)),
	              new Unary(opRegOf, new Const(24)));
    e->setProc(proc);
    rtl->appendExp(e);
    pRtls->push_back(rtl);
    pRtls->push_back(new HLReturn(0x123));
    PBB ret = cfg->newBB(pRtls, RET, 0);
    first->setOutEdge(0, ret);
    ret->addInEdge(first);
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected;
    expected += "Fall BB: live in: \n";
    expected += "00000000 *32* r[24] := 5   uses:    used by: \n";
    expected += "         *32* r[24] := 6   uses:    used by: *32* r[28] := r[24], \n";
    expected += "Ret BB: live in: *32* r[24] := 6, \n";
    expected += "00000000 *32* r[28] := r[24]   uses: *32* r[24] := 6,    used by: \n";
    expected += "00000123 RET\n";
    CPPUNIT_ASSERT_EQUAL(expected, s);
    // clean up
    delete prog;
}

/*==============================================================================
 * FUNCTION:        DataflowTest::testUseKill
 * OVERVIEW:        
 *============================================================================*/
void DataflowTest::testUseKill () {
    // create Prog
    Prog *prog = new Prog();
    // create UserProc
    std::string name = "test";
    UserProc *proc = new UserProc(prog, name, 0);
    // create CFG
    Cfg *cfg = proc->getCFG();
    std::list<RTL*>* pRtls = new std::list<RTL*>();
    RTL *rtl = new RTL();
    AssignExp *e = new AssignExp(new Unary(opRegOf, new Const(24)),
			         new Const(5));
    e->setProc(proc);
    rtl->appendExp(e);
    e = new AssignExp(new Unary(opRegOf, new Const(24)),
		      new Binary(opPlus, new Unary(opRegOf, new Const(24)),
			                 new Const(1)));
    e->setProc(proc);
    rtl->appendExp(e);
    pRtls->push_back(rtl);
    PBB first = cfg->newBB(pRtls, FALL, 1);
    pRtls = new std::list<RTL*>();
    pRtls->push_back(new HLReturn(0x123));
    PBB ret = cfg->newBB(pRtls, RET, 0);
    first->setOutEdge(0, ret);
    ret->addInEdge(first);
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected;
    expected += "Fall BB: live in: \n";
    expected += "00000000 *32* r[24] := 5   uses:    used by: *32* r[24] := r[24] + 1, \n";
    expected += "         *32* r[24] := r[24] + 1   uses: *32* r[24] := 5,    used by: \n";
    expected += "Ret BB: live in: *32* r[24] := r[24] + 1, \n";
    expected += "00000123 RET\n";
    CPPUNIT_ASSERT_EQUAL(expected, s);
    // clean up
    delete prog;
}

/*==============================================================================
 * FUNCTION:        DataflowTest::testEndlessLoop
 * OVERVIEW:        
 *============================================================================*/
void DataflowTest::testEndlessLoop () {
    // create Prog
    Prog *prog = new Prog();
    // create UserProc
    std::string name = "test";
    UserProc *proc = new UserProc(prog, name, 0);
    // create CFG
    Cfg *cfg = proc->getCFG();
    std::list<RTL*>* pRtls = new std::list<RTL*>();
    RTL *rtl = new RTL();
    AssignExp *e = new AssignExp(new Unary(opRegOf, new Const(24)),
			         new Const(5));
    e->setProc(proc);
    rtl->appendExp(e);
    pRtls->push_back(rtl);
    PBB first = cfg->newBB(pRtls, FALL, 1);
    pRtls = new std::list<RTL*>();
    rtl = new RTL();
    e = new AssignExp(new Unary(opRegOf, new Const(24)),
		      new Binary(opPlus, new Unary(opRegOf, new Const(24)),
			                 new Const(1)));
    e->setProc(proc);
    rtl->appendExp(e);
    pRtls->push_back(rtl);
    PBB body = cfg->newBB(pRtls, ONEWAY, 1);
    first->setOutEdge(0, body);
    body->addInEdge(first);
    body->setOutEdge(0, body);
    body->addInEdge(body);
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected;
    expected += "Fall BB: live in: \n";
    expected += "00000000 *32* r[24] := 5   uses:    used by: *32* r[24] := r[24] + 1, \n";
    expected += "Oneway BB: live in: *32* r[24] := 5, *32* r[24] := r[24] + 1, \n";
    expected += "00000000 *32* r[24] := r[24] + 1   uses: *32* r[24] := 5, *32* r[24] := r[24] + 1,    used by: \n";
    CPPUNIT_ASSERT_EQUAL(expected, s);
    // clean up
    delete prog;
}
