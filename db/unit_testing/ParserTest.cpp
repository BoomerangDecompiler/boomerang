/***************************************************************************/ /**
  * \file       ParserTest.cc
  * OVERVIEW:   Provides the implementation for the ParserTest class, which
  *                tests the sslparser.y etc
  ******************************************************************************/
#include "ParserTest.h"
#include "sslparser.h"

#include <sstream>

#define SPARC_SSL Boomerang::get()->getProgPath() + "frontend/machine/sparc/sparc.ssl"

/***************************************************************************/ /**
  * \fn        ParserTest::testRead
  * OVERVIEW:        Test reading the SSL file
  ******************************************************************************/
void ParserTest::testRead() {
    RTLInstDict d;
    QVERIFY(d.readSSLFile(SPARC_SSL));
}

/***************************************************************************/ /**
  * \fn        ParserTest::testExp
  * OVERVIEW:        Test parsing an expression
  ******************************************************************************/
void ParserTest::testExp() {
    std::string s("*i32* r0 := 5 + 6");
    Instruction *a = SSLParser::parseExp(s.c_str());
    QVERIFY(a);
    std::ostringstream ost;
    a->print(ost);
    QCOMPARE("   0 " + s, std::string(ost.str()));
    std::string s2 = "*i32* r[0] := 5 + 6";
    a = SSLParser::parseExp(s2.c_str());
    QVERIFY(a);
    std::ostringstream ost2;
    a->print(ost2);
    // Still should print to string s, not s2
    QCOMPARE("   0 " + s, ost2.str());
}

QTEST_MAIN(ParserTest)
