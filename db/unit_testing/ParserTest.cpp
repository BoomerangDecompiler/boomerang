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
    QString s("*i32* r0 := 5 + 6");
    Instruction *a = SSLParser::parseExp(qPrintable(s));
    QVERIFY(a);
    QString res;
    QTextStream ost(&res);
    a->print(ost);
    QCOMPARE("   0 " + s, res);
    std::string s2 = "*i32* r[0] := 5 + 6";
    a = SSLParser::parseExp(s2.c_str());
    QVERIFY(a);
    res.clear();
    a->print(ost);
    // Still should print to string s, not s2
    QCOMPARE("   0 " + s, res);
}

QTEST_MAIN(ParserTest)
