/***************************************************************************/ /**
  * \file       CTest.cc
  * OVERVIEW:   Provides the implementation for the CTest class, which
  *              tests the c parser
  *============================================================================*/
/*
 * $Revision$
 *
 * 03 Dec 02 - Trent: Created
 */

#include "CTest.h"
#include "ansi-c-parser.h"

CPPUNIT_TEST_SUITE_REGISTRATION(CTest);

/***************************************************************************/ /**
  * FUNCTION:        CTest::setUp
  * OVERVIEW:        Set up anything needed before all tests
  * NOTE:            Called before any tests
  * PARAMETERS:      <none>
  *
  *============================================================================*/
void CTest::setUp() {}

/***************************************************************************/ /**
  * FUNCTION:        CTest::tearDown
  * OVERVIEW:        Delete objects created in setUp
  * NOTE:            Called after all tests
  * PARAMETERS:      <none>
  *
  *============================================================================*/
void CTest::tearDown() {}

/***************************************************************************/ /**
  * FUNCTION:        CTest::testSignature
  * OVERVIEW:        Test
  * PARAMETERS:      <none>
  *
  *============================================================================*/
void CTest::testSignature() {
    std::istringstream os("int printf(char *fmt, ...);");
    AnsiCParser *p = new AnsiCParser(os, false);
    p->yyparse(PLAT_PENTIUM, CONV_C);
    CPPUNIT_ASSERT_EQUAL(1, (int)p->signatures.size());
    Signature *sig = p->signatures.front();
    CPPUNIT_ASSERT_EQUAL(std::string("printf"), std::string(sig->getName()));
    CPPUNIT_ASSERT(sig->getReturnType(0)->resolvesToInteger());
    Type *t = new PointerType(new CharType());
    // Pentium signatures used to have esp prepended to the list of parameters; no more?
    int num = sig->getNumParams();
    CPPUNIT_ASSERT_EQUAL(1, num);
    CPPUNIT_ASSERT(*sig->getParamType(0) == *t);
    CPPUNIT_ASSERT_EQUAL(std::string("fmt"), std::string(sig->getParamName(0)));
    CPPUNIT_ASSERT(sig->hasEllipsis());
    delete t;
}
