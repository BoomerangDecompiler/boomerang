/*==============================================================================
 * FILE:       ExpTest.cc
 * OVERVIEW:   Provides the implementation for the ExpTest class, which
 *              tests the Exp and derived classes
 *============================================================================*/

#include "ExpTest.h"

/*==============================================================================
 * FUNCTION:        ExpTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<ExpTest> ("testExp", \
    &ExpTest::name, *this))

void ExpTest::registerTests(CppUnit::TestSuite* suite) {
    MYTEST(test99);
    MYTEST(testRegOf2);
    MYTEST(testPlus);
    MYTEST(testMinus);
    MYTEST(testMult);
    MYTEST(testDiv);
    MYTEST(testMults);
    MYTEST(testDivs);
    MYTEST(testMod);
    MYTEST(testMods);
}

int ExpTest::countTestCases () const
{ return 2; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        ExpTest::setUp
 * OVERVIEW:        Set up some expressions for use with all the tests
 * NOTE:            Called before any tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void ExpTest::setUp () {
    m_99 = new Const(99);
    m_rof2 = new Unary(idRegOf, new Const(2));
}

/*==============================================================================
 * FUNCTION:        ExpTest::tearDown
 * OVERVIEW:        Delete expressions created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void ExpTest::tearDown () {
    delete m_99;
    delete m_rof2;
}

/*==============================================================================
 * FUNCTION:        ExpTest::test99
 * OVERVIEW:        Test integer constant
 *============================================================================*/
void ExpTest::test99 () {
    ostrstream ost;
    m_99->print(ost);
    CPPUNIT_ASSERT (string("99") == string(ost.str()));
}

/*==============================================================================
 * FUNCTION:        ExpTest::testRegOf2
 * OVERVIEW:        Tests r[2], which is used in many tests. Also tests idRegOf
 *============================================================================*/
void ExpTest::testRegOf2 () {
    ostrstream ost;
    m_rof2->print(ost);
    CPPUNIT_ASSERT_EQUAL (string("r[2]"), string(ost.str()));
}

/*==============================================================================
 * FUNCTION:        ExpTest::testPlus
 * OVERVIEW:        Test idPlus
 *============================================================================*/
void ExpTest::testPlus () {
    ostrstream ost;
    Binary* b = new Binary(idPlus, m_99, m_rof2);
    b->print(ost);
    CPPUNIT_ASSERT_EQUAL (string("99 + r[2]"), string(ost.str()));
    delete b;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testMinus
 * OVERVIEW:        Test idMinus
 *============================================================================*/
void ExpTest::ExpTest::testMinus () {
    ostrstream ost;
    Binary* b = new Binary(idMinus, m_99, m_rof2);
    b->print(ost);
    CPPUNIT_ASSERT_EQUAL (string("99 - r[2]"), string(ost.str()));
    delete b;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testMult
 * OVERVIEW:        Test idMult
 *============================================================================*/
void ExpTest::testMult () {
    ostrstream ost;
    Binary* b = new Binary(idMult, m_99, m_rof2);
    b->print(ost);
    CPPUNIT_ASSERT_EQUAL (string("99 * r[2]"), string(ost.str()));
    delete b;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testDiv
 * OVERVIEW:        Test idDiv
 *============================================================================*/
void ExpTest::testDiv () {
    ostrstream ost;
    Binary* b = new Binary(idDiv, m_99, m_rof2);
    b->print(ost);
    CPPUNIT_ASSERT_EQUAL (string("99 / r[2]"), string(ost.str()));
    delete b;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testMults
 * OVERVIEW:        Test idMults (signed multiplication)
 *============================================================================*/
void ExpTest::testMults () {
    ostrstream ost;
    Binary* b = new Binary(idMults, m_99, m_rof2);
    b->print(ost);
    CPPUNIT_ASSERT_EQUAL (string("99 *! r[2]"), string(ost.str()));
    delete b;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testDivs
 * OVERVIEW:        Test idDivs (signed division)
 *============================================================================*/
void ExpTest::testDivs () {
    ostrstream ost;
    Binary* b = new Binary(idDivs, m_99, m_rof2);
    b->print(ost);
    CPPUNIT_ASSERT_EQUAL (string("99 /! r[2]"), string(ost.str()));
    delete b;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testMod
 * OVERVIEW:        Test idMod
 *============================================================================*/
void ExpTest::testMod () {
    ostrstream ost;
    Binary* b = new Binary(idMod, m_99, m_rof2);
    b->print(ost);
    CPPUNIT_ASSERT_EQUAL (string("99 % r[2]"), string(ost.str()));
    delete b;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testMods
 * OVERVIEW:        Test idMods
 *============================================================================*/
void ExpTest::testMods () {
    ostrstream ost;
    Binary* b = new Binary(idMods, m_99, m_rof2);
    b->print(ost);
    CPPUNIT_ASSERT_EQUAL (string("99 %! r[2]"), string(ost.str()));
    delete b;
}

