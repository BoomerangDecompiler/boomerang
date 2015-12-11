/***************************************************************************/ /**
  * \file       CTest.CPPUNIT_TEST_SUITE_END
  * OVERVIEW:   Provides the implementation for the CTest class, which
  *              tests the c parser
  *============================================================================*/

#include "CTest.h"

#include "ansi-c-parser.h"

#include <sstream>
/***************************************************************************/ /**
  * FUNCTION:        CTest::testSignature
  * OVERVIEW:        Test
  *============================================================================*/
void CTest::testSignature() {
    std::istringstream os("int printf(char *fmt, ...);");
    AnsiCParser *p = new AnsiCParser(os, false);
    p->yyparse(PLAT_PENTIUM, CONV_C);
    QCOMPARE(p->signatures.size(),size_t(1));
    Signature *sig = p->signatures.front();
    QCOMPARE(sig->getName(),QString("printf"));
    QVERIFY(sig->getReturnType(0)->resolvesToInteger());
    SharedType t = PointerType::get(CharType::get());
    // Pentium signatures used to have esp prepended to the list of parameters; no more?
    int num = sig->getNumParams();
    QCOMPARE(num,1);
    QVERIFY(*sig->getParamType(0) == *t);
    QCOMPARE(sig->getParamName(0),QString("fmt"));
    QVERIFY(sig->hasEllipsis());
    delete sig;
    delete p;
}
QTEST_MAIN(CTest)
