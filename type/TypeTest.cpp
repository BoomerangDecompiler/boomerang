/*==============================================================================
 * FILE:       TypeTest.cc
 * OVERVIEW:   Provides the implementation for the TypeTest class, which
 *              tests the Type class and some utility functions
 *============================================================================*/
/*
 * $Revision$
 *
 * 09 Apr 02 - Mike: Created
 * 22 Aug 03 - Mike: Extended for Constraint tests
 */

#ifndef BOOMDIR
#error Must define BOOMDIR
#endif

#define HELLO_WINDOWS       BOOMDIR "/test/windows/hello.exe"

#include <iostream>
#include "TypeTest.h"
#include "BinaryFile.h"         // Ugh - needed before frontend.h
#include "pentiumfrontend.h"
#include "signature.h"
#include "boomerang.h"
#include "log.h"

/*==============================================================================
 * FUNCTION:        TypeTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<TypeTest> ("testUtil", \
    &TypeTest::name, *this))

void TypeTest::registerTests(CppUnit::TestSuite* suite) {

//  Note: there is nothing left to test in Util (for now)
    MYTEST(testTypeLong);
    MYTEST(testNotEqual);
    MYTEST(testCompound);
}

int TypeTest::countTestCases () const
{ return 1; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        TypeTest::setUp
 * OVERVIEW:        Set up anything needed before all tests
 * NOTE:            Called before any tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void TypeTest::setUp () {
}

/*==============================================================================
 * FUNCTION:        TypeTest::tearDown
 * OVERVIEW:        Delete objects created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void TypeTest::tearDown () {
}

/*==============================================================================
 * FUNCTION:        TypeTest::testTypeLong
 * OVERVIEW:        Test type unsigned long
 *============================================================================*/
void TypeTest::testTypeLong () {

    std::string expected("unsigned long long");
    IntegerType t(64, false);
    std::string actual(t.getCtype());
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

/*==============================================================================
 * FUNCTION:        TypeTest::testNotEqual
 * OVERVIEW:        Test type inequality
 *============================================================================*/
void TypeTest::testNotEqual () {

    IntegerType t1(32, false);
    IntegerType t2(32, false);
    IntegerType t3(16, false);
    CPPUNIT_ASSERT(!(t1 != t2));
    CPPUNIT_ASSERT(t2 != t3);
}

/*==============================================================================
 * FUNCTION:        TypeTest::testNotEqual
 * OVERVIEW:        Test type inequality
 *============================================================================*/
class ErrLogger : public Log {
public:
    virtual Log &operator<<(const char *str) {
        std::cerr << str;
        return *this;
    }
    virtual ~ErrLogger() {};
};
void TypeTest::testCompound() {
    BinaryFile *pBF = BinaryFile::Load(HELLO_WINDOWS);
    FrontEnd *pFE = new PentiumFrontEnd(pBF);
    pFE->readLibraryCatalog();              // Read definitions

    Boomerang::get()->setLogger(new ErrLogger());
    Signature* paintSig = pFE->getLibSignature("BeginPaint");
    // Second argument should be an LPPAINTSTRUCT
    Type* ty = paintSig->getParamType(1);
    const char* p = ty->getCtype();
    std::string expected("LPPAINTSTRUCT");
    std::string actual(p);
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    // Get the type pointed to
    ty = ty->asPointer()->getPointsTo();
    p = ty->getCtype();
    expected = "PAINTSTRUCT";
    actual = p;
    CPPUNIT_ASSERT_EQUAL(expected, actual);


    // Offset 8 should have a RECT
    Type* subTy = ty->asCompound()->getTypeAtOffset(8*8);
    p = subTy->getCtype();
    expected = "RECT";
    actual = p;
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    // Name at offset C should be bottom
    p = subTy->asCompound()->getNameAtOffset(0x0C*8);
    expected = "bottom";
    actual = p;
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    // Now figure out the name at offset 8+C
    p = ty->asCompound()->getNameAtOffset((8 + 0x0C)*8);
    expected = "rcPaint";
    actual = p;
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    // Also at offset 8
    p = ty->asCompound()->getNameAtOffset((8 + 0)*8);
    actual = p;
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    // Also at offset 8+4
    p = ty->asCompound()->getNameAtOffset((8 + 4)*8);
    actual = p;
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    // And at offset 8+8
    p = ty->asCompound()->getNameAtOffset((8 + 8)*8);
    actual = p;
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}


