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

    MYTEST(testLocationSet);
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
    std::string expected = "Ret BB: reach in: \n00000123 RET\n"
        "cfg reachExit: \n";
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
    cfg->setEntryBB(first);     // Also sets exitBB; important!
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected;
    expected =
      "Fall BB: reach in: \n"
      "00000000 *32* r[24] := 5   uses:    used by: \n"
      "Ret BB: reach in: *32* r[24] := 5, \n"
      "00000123 RET\n"
      "cfg reachExit: *32* r[24] := 5, \n";
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
    cfg->setEntryBB(first);
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected;
    expected =
      "Fall BB: reach in: \n"
      "00000000 *32* r[24] := 5   uses:    used by: \n"
      "         *32* r[24] := 6   uses:    used by: \n"
      "Ret BB: reach in: *32* r[24] := 6, \n"
      "00000123 RET\n"
      "cfg reachExit: *32* r[24] := 6, \n";
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
    cfg->setEntryBB(first);
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected;
    expected =
      "Fall BB: reach in: \n"
      "00000000 *32* r[24] := 5   uses:    used by: *32* r[28] := r[24], \n"
      "         *32* r[28] := r[24]   uses: *32* r[24] := 5,    used by: \n"
      "Ret BB: reach in: *32* r[24] := 5, *32* r[28] := r[24], \n"
      "00000123 RET\n"
      "cfg reachExit: *32* r[24] := 5, *32* r[28] := r[24], \n";
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
    cfg->setEntryBB(first);
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected;
    expected = 
      "Fall BB: reach in: \n"
      "00000000 *32* r[24] := 5   uses:    used by: \n"
      "         *32* r[24] := 6   uses:    used by: *32* r[28] := r[24], \n"
      "         *32* r[28] := r[24]   uses: *32* r[24] := 6,    used by: \n"
      "Ret BB: reach in: *32* r[24] := 6, *32* r[28] := r[24], \n"
      "00000123 RET\n"
      "cfg reachExit: *32* r[24] := 6, *32* r[28] := r[24], \n";
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
    cfg->setEntryBB(first);
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected;
    expected =
      "Fall BB: reach in: \n"
      "00000000 *32* r[24] := 5   uses:    used by: \n"
      "         *32* r[24] := 6   uses:    used by: *32* r[28] := r[24], \n"
      "Ret BB: reach in: *32* r[24] := 6, \n"
      "00000000 *32* r[28] := r[24]   uses: *32* r[24] := 6,    used by: \n"
      "00000123 RET\n"
      "cfg reachExit: *32* r[24] := 6, *32* r[28] := r[24], \n";
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
    cfg->setEntryBB(first);
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected;
    expected  = 
      "Fall BB: reach in: \n"
      "00000000 *32* r[24] := 5   uses:    used by: *32* r[24] := r[24] + 1, \n"
      "         *32* r[24] := r[24] + 1   uses: *32* r[24] := 5,    used by: \n"
      "Ret BB: reach in: *32* r[24] := r[24] + 1, \n"
      "00000123 RET\n"
      "cfg reachExit: *32* r[24] := r[24] + 1, \n";
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
    // r[24] := 5
    AssignExp *e = new AssignExp(new Unary(opRegOf, new Const(24)),
                     new Const(5));
    e->setProc(proc);
    rtl->appendExp(e);
    pRtls->push_back(rtl);
    PBB first = cfg->newBB(pRtls, FALL, 1);
    pRtls = new std::list<RTL*>();
    rtl = new RTL();
    // r[24] := r[24] + 1
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
    cfg->setEntryBB(first);
    // compute dataflow
    cfg->computeDataflow();
    // print cfg to a string
    std::ostringstream st;
    cfg->print(st, true);
    std::string s = st.str();
    // compare it to expected
    std::string expected;
    expected =
      "Fall BB: reach in: \n"
      "00000000 *32* r[24] := 5   uses:    used by: *32* r[24] := r[24] + 1, \n"
      "Oneway BB: reach in: *32* r[24] := 5, *32* r[24] := r[24] + 1, \n"
      "00000000 *32* r[24] := r[24] + 1   uses: *32* r[24] := 5, "
      "*32* r[24] := r[24] + 1,    used by: *32* r[24] := r[24] + 1, \n"
      "cfg reachExit: \n";
    CPPUNIT_ASSERT_EQUAL(expected, s);
    // clean up
    delete prog;
}

/*==============================================================================
 * FUNCTION:        DataflowTest::testLocationSet
 * OVERVIEW:        
 *============================================================================*/
void DataflowTest::testLocationSet () {
    Unary rof(opRegOf, new Const(12));
    Const& theReg = *(Const*)rof.getSubExp1();
    LocationSet ls;
    LocSetIter ii;
    ls.insert(rof.clone());
    theReg.setInt(8);
    ls.insert(rof.clone());
    theReg.setInt(31);
    ls.insert(rof.clone());
    theReg.setInt(24);
    ls.insert(rof.clone());
    theReg.setInt(12);
    ls.insert(rof.clone());     // Note: r[12] already inserted
    CPPUNIT_ASSERT_EQUAL(4, ls.size());
    theReg.setInt(8);
    CPPUNIT_ASSERT(rof == *ls.getFirst(ii));
    theReg.setInt(12);
    Exp* e;
    e = ls.getNext(ii); CPPUNIT_ASSERT(rof == *e);
    theReg.setInt(24);
    e = ls.getNext(ii); CPPUNIT_ASSERT(rof == *e);
    theReg.setInt(31);
    e = ls.getNext(ii); CPPUNIT_ASSERT(rof == *e);
    Unary mof(opMemOf,
        new Binary(opPlus,
            new Unary(opRegOf, new Const(14)),
            new Const(4)));
    ls.insert(mof.clone());
    ls.insert(mof.clone());
    CPPUNIT_ASSERT_EQUAL(5, ls.size());
    CPPUNIT_ASSERT(mof == *ls.getFirst(ii));
    LocationSet ls2 = ls;
    Exp* e2 = ls2.getFirst(ii);
    CPPUNIT_ASSERT(e2 != ls.getFirst(ii));      // Must be cloned
    CPPUNIT_ASSERT_EQUAL(5, ls2.size());
    CPPUNIT_ASSERT(mof == *ls2.getFirst(ii));
    theReg.setInt(8);
    e = ls2.getNext(ii); CPPUNIT_ASSERT(rof == *e);
}
