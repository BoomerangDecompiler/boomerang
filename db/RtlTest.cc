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

char* str(std::ostringstream& os);      // In testDbase.cc

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
    TypedExp* e = new TypedExp(Type(),
        new Binary(opAssign,
            new Unary(opRegOf, new Const(8)),
            new Binary(opPlus,
                new Unary(opRegOf, new Const(9)),
                new Const(99))));
    RTL r;
    r.appendExp(e);
    std::ostringstream ost;
    r.print(ost);
    std::string actual(str(ost));
    std::string expected("00000000 *32* r[8] := r[9] + 99\n");
    CPPUNIT_ASSERT_EQUAL(expected, actual);
    delete e;
}

/*==============================================================================
 * FUNCTION:        RtlTest::testClone
 * OVERVIEW:        Test constructor from list of expressions; cloning of RTLs
 *============================================================================*/
void RtlTest::testClone () {
    TypedExp* e1 = new TypedExp(Type(),
        new Binary(opAssign,
            new Unary(opRegOf, new Const(8)),
            new Binary(opPlus,
                new Unary(opRegOf, new Const(9)),
                new Const(99))));
    TypedExp* e2 = new TypedExp(Type(INTEGER, 16),
        new Binary(opAssign,
            new Unary(opParam, new Const("x")),
            new Unary(opParam, new Const("y"))));
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
    std::string expected("00001234 *32* r[8] := r[9] + 99\n         *16* x := y\n");
    std::string a1(str(o1));
    std::string a2(str(o2));
    CPPUNIT_ASSERT_EQUAL(expected, a1);
    CPPUNIT_ASSERT_EQUAL(expected, a2);
}
