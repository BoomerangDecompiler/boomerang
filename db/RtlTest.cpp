/*==============================================================================
 * FILE:       RtlTest.cc
 * OVERVIEW:   Provides the implementation for the RtlTest class, which
 *              tests the RTL and derived classes
 *============================================================================*/
/*
 * $Revision$
 *
 * 13 May 02 - Mike: Created
 */

#include "RtlTest.h"
#include "statement.h"
#include "exp.h"
#include <sstream>
#include "BinaryFile.h"
#include "frontend.h"
#include "sparcfrontend.h"
#include "pentiumfrontend.h"
#include "decoder.h"


#ifndef BOOMDIR
#error Must define BOOMDIR
#endif

#define SWITCH_SPARC        BOOMDIR "/test/sparc/switch_cc"
#define SWITCH_PENT         BOOMDIR "/test/pentium/switch_cc"

/*==============================================================================
 * FUNCTION:        RtlTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<RtlTest> ("RtlTest", \
    &RtlTest::name, *this))

void RtlTest::registerTests(CppUnit::TestSuite* suite) {
    MYTEST(testAppend);
    MYTEST(testClone);
    MYTEST(testVisitor);
    MYTEST(testIsCompare);
}

int RtlTest::countTestCases () const
{ return 2; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        RtlTest::setUp
 * OVERVIEW:        Set up some expressions for use with all the tests
 * NOTE:            Called before any tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void RtlTest::setUp () {
}

/*==============================================================================
 * FUNCTION:        RtlTest::tearDown
 * OVERVIEW:        Delete expressions created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void RtlTest::tearDown () {
}

/*==============================================================================
 * FUNCTION:        RtlTest::testAppend
 * OVERVIEW:        Test appendExp and printing of RTLs
 *============================================================================*/
void RtlTest::testAppend () {
    Assign* a = new Assign(32,
            Location::regOf(8),
            new Binary(opPlus,
                Location::regOf(9),
                new Const(99)));
    RTL r;
    r.appendStmt(a);
    std::ostringstream ost;
    r.print(ost);
    std::string actual(ost.str());
    std::string expected("00000000    0 *32* r8 := r9 + 99\n");
    CPPUNIT_ASSERT_EQUAL(expected, actual);
    // No! appendExp does not copy the expression, so deleting the RTL will
    // delete the expression(s) in it.
    // Not sure if that's what we want...
    // delete a;
}

/*==============================================================================
 * FUNCTION:        RtlTest::testClone
 * OVERVIEW:        Test constructor from list of expressions; cloning of RTLs
 *============================================================================*/
void RtlTest::testClone () {
    Assign* a1 = new Assign(32,
            Location::regOf(8),
            new Binary(opPlus,
                Location::regOf(9),
                new Const(99)));
    Assign* a2 = new Assign(16,
            new Unary(opParam, new Const("x")),
            new Unary(opParam, new Const("y")));
    std::list<Statement*> ls;
    ls.push_back(a1);
    ls.push_back(a2);
    RTL* r = new RTL(0x1234, &ls);
    RTL* r2 = r->clone();
    std::ostringstream o1, o2;
    r->print(o1);
    delete r;           // And r2 should still stand!
    r2->print(o2);
    delete r2;
    std::string expected("00001234    0 *32* r8 := r9 + 99\n"
                         "            0 *16* x := y\n");

    std::string act1(o1.str());
    std::string act2(o2.str());
    CPPUNIT_ASSERT_EQUAL(expected, act1);
    CPPUNIT_ASSERT_EQUAL(expected, act2);
}

/*==============================================================================
 * FUNCTION:        RtlTest::testVisitor
 * OVERVIEW:        Test the accept function for correct visiting behaviour.
 * NOTES:           Stub class to test.
 *============================================================================*/

class StmtVisitorStub : public StmtVisitor {
public:
    bool a, b, c, d, e, f, g, h; 

    void clear() { a = b = c = d = e = f = g = h = false; }
    StmtVisitorStub() { clear(); }
    virtual ~StmtVisitorStub() { }
    virtual bool visit(            RTL *s) { a = true; return false; }
    virtual bool visit(  GotoStatement *s) { b = true; return false; }
    virtual bool visit(BranchStatement *s) { c = true; return false; }
    virtual bool visit(  CaseStatement *s) { d = true; return false; }
    virtual bool visit(  CallStatement *s) { e = true; return false; }
    virtual bool visit(ReturnStatement *s) { f = true; return false; }
    virtual bool visit(   BoolStatement *s) { g = true; return false; }
    virtual bool visit(         Assign *s) { h = true; return false; }
};

void RtlTest::testVisitor()
{
    StmtVisitorStub* visitor = new StmtVisitorStub();

    /* rtl */
    RTL *rtl = new RTL();
    rtl->accept(visitor);
    CPPUNIT_ASSERT(visitor->a);
    delete rtl;

    /* jump stmt */
    GotoStatement *jump = new GotoStatement;
    jump->accept(visitor);
    CPPUNIT_ASSERT(visitor->b);
    delete jump;

    /* branch stmt */
    BranchStatement *jcond = new BranchStatement;
    jcond->accept(visitor);
    CPPUNIT_ASSERT(visitor->c);
    delete jcond;

    /* nway jump stmt */
    CaseStatement *nwayjump = new CaseStatement;
    nwayjump->accept(visitor);
    CPPUNIT_ASSERT(visitor->d);
    delete nwayjump;

    /* call stmt */
    CallStatement *call = new CallStatement;
    call->accept(visitor);
    CPPUNIT_ASSERT(visitor->e);
    delete call;

    /* return stmt */
    ReturnStatement *ret = new ReturnStatement;
    ret->accept(visitor);
    CPPUNIT_ASSERT(visitor->f);
    delete ret;

    /* "bool" stmt */
    BoolStatement *scond = new BoolStatement(0);
    scond->accept(visitor);
    CPPUNIT_ASSERT(visitor->g);
    delete scond;

    /* assignment stmt */
    Assign *as = new Assign;
    as->accept(visitor);
    CPPUNIT_ASSERT(visitor->h);
    delete as;

    /* polymorphic */
    Statement* s = new CallStatement;
    s->accept(visitor);
    CPPUNIT_ASSERT(visitor->e);
    delete s;

    /* cleanup */
    delete visitor;
}

/*==============================================================================
 * FUNCTION:        RtlTest::testIsCompare
 * OVERVIEW:        Test the isCompare function
 *============================================================================*/
void RtlTest::testIsCompare () {
    BinaryFile *pBF = BinaryFile::Load(SWITCH_SPARC);
    CPPUNIT_ASSERT(pBF != 0);
    CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_SPARC);
    FrontEnd *pFE = new SparcFrontEnd(pBF);

    // Decode second instruction: "sub          %i0, 2, %o1"
    int iReg;
    Exp* eOperand = NULL;
    DecodeResult inst = pFE->decodeInstruction(0x10910);
    CPPUNIT_ASSERT(inst.rtl != NULL);
    CPPUNIT_ASSERT(inst.rtl->isCompare(iReg, eOperand) == false);
    
    // Decode fifth instruction: "cmp          %o1, 5"
    inst = pFE->decodeInstruction(0x1091c);
    CPPUNIT_ASSERT(inst.rtl != NULL);
    CPPUNIT_ASSERT(inst.rtl->isCompare(iReg, eOperand) == true);
    CPPUNIT_ASSERT_EQUAL(9, iReg);
    std::string expected("5");
    std::ostringstream ost1;
    eOperand->print(ost1);
    std::string actual(ost1.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    pBF->UnLoad();
    delete pBF;
    delete pFE;
    pBF = BinaryFile::Load(SWITCH_PENT);
    CPPUNIT_ASSERT(pBF != 0);
    CPPUNIT_ASSERT(pBF->GetMachine() == MACHINE_PENTIUM);
    pFE = new PentiumFrontEnd(pBF);

    // Decode fifth instruction: "cmp    $0x5,%eax"
    inst = pFE->decodeInstruction(0x80488fb);
    CPPUNIT_ASSERT(inst.rtl != NULL);
    CPPUNIT_ASSERT(inst.rtl->isCompare(iReg, eOperand) == true);
    CPPUNIT_ASSERT_EQUAL(24, iReg);
    std::ostringstream ost2;
    eOperand->print(ost2);
    actual = ost2.str();
    CPPUNIT_ASSERT_EQUAL(expected, actual);
    
    // Decode instruction: "add    $0x4,%esp"
    inst = pFE->decodeInstruction(0x804890c);
    CPPUNIT_ASSERT(inst.rtl != NULL);
    CPPUNIT_ASSERT(inst.rtl->isCompare(iReg, eOperand) == false);
    
}
