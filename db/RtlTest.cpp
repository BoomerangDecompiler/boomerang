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
    AssignExp* e = new AssignExp(32,
            new Unary(opRegOf, new Const(8)),
            new Binary(opPlus,
                new Unary(opRegOf, new Const(9)),
                new Const(99)));
    RTL r;
    r.appendExp(e);
    std::ostringstream ost;
    r.print(ost);
    std::string actual(ost.str());
    std::string expected("00000000    0 *32* r8 := r9 + 99\n");
    CPPUNIT_ASSERT_EQUAL(expected, actual);
    // No! appendExp does not copy the expression, so deleting the RTL will
    // delete the expression(s) in it.
    // Not sure if that's what we want...
    // delete e;
}

/*==============================================================================
 * FUNCTION:        RtlTest::testClone
 * OVERVIEW:        Test constructor from list of expressions; cloning of RTLs
 *============================================================================*/
void RtlTest::testClone () {
    AssignExp* e1 = new AssignExp(32,
            new Unary(opRegOf, new Const(8)),
            new Binary(opPlus,
                new Unary(opRegOf, new Const(9)),
                new Const(99)));
    AssignExp* e2 = new AssignExp(16,
            new Unary(opParam, new Const("x")),
            new Unary(opParam, new Const("y")));
    std::list<Exp*> le;
    le.push_back(e1);
    le.push_back(e2);
    RTL* r = new RTL(0x1234, &le);
    RTL* r2 = r->clone();
    std::ostringstream o1, o2;
    r->print(o1);
    delete r;           // And r2 should still stand!
    r2->print(o2);
    delete r2;
    std::string expected("00001234    0 *32* r8 := r9 + 99\n"
                         "            0 *16* x := y\n");

    std::string a1(o1.str());
    std::string a2(o2.str());
    CPPUNIT_ASSERT_EQUAL(expected, a1);
    CPPUNIT_ASSERT_EQUAL(expected, a2);
}

/*==============================================================================
 * FUNCTION:        RtlTest::testVisitor
 * OVERVIEW:        Test the accept function for correct visiting behaviour.
 * NOTES:           Stub class to test.
 *============================================================================*/

class RTLVisitorStub : public RTLVisitor {
public:
    bool a, b, c, d, e, f, g; 

    void clear() { a = b = c = d = e = f = false; }
    RTLVisitorStub() { clear(); }
    virtual ~RTLVisitorStub() { }
    virtual bool visit(RTL *rtl)        { a = true; return false; }
    virtual bool visit(HLJump *rtl)     { b = true; return false; }
    virtual bool visit(HLJcond *rtl)    { c = true; return false; }
    virtual bool visit(HLNwayJump *rtl) { d = true; return false; }
    virtual bool visit(HLCall *rtl)     { e = true; return false; }
    virtual bool visit(HLReturn *rtl)   { f = true; return false; }
    virtual bool visit(HLScond *rtl)    { g = true; return false; }
};

void RtlTest::testVisitor()
{
    RTLVisitorStub* visitor = new RTLVisitorStub();

    /* simple rtl */
    RTL *rtl = new RTL();
    rtl->accept(visitor);
    CPPUNIT_ASSERT(visitor->a);
    delete rtl;

    /* jump rtl */
    HLJump *jump = new HLJump(0);
    jump->accept(visitor);
    CPPUNIT_ASSERT(visitor->b);
    delete jump;

    /* jcond rtl */
    HLJcond *jcond = new HLJcond(0);
    jcond->accept(visitor);
    CPPUNIT_ASSERT(visitor->c);
    delete jcond;

    /* nway jump rtl */
    HLNwayJump *nwayjump = new HLNwayJump(0);
    nwayjump->accept(visitor);
    CPPUNIT_ASSERT(visitor->d);
    delete nwayjump;

    /* call rtl */
    HLCall *call = new HLCall(0);
    call->accept(visitor);
    CPPUNIT_ASSERT(visitor->e);
    delete call;

    /* return rtl */
    HLReturn *ret = new HLReturn(0);
    ret->accept(visitor);
    CPPUNIT_ASSERT(visitor->f);
    delete ret;

    /* scond rtl */
    HLScond *scond = new HLScond(0);
    scond->accept(visitor);
    CPPUNIT_ASSERT(visitor->g);
    delete scond;

    /* polymorphic */
    rtl = new HLCall(0);
    rtl->accept(visitor);
    CPPUNIT_ASSERT(visitor->e);
    delete rtl;

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
