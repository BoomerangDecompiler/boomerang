/***************************************************************************//**
 * \file       ParserTest.cc
 * OVERVIEW:   Provides the implementation for the ParserTest class, which
 *                tests the sslparser.y etc
 *============================================================================*/
#include "ParserTest.h"
#include "sslparser.h"

#include <sstream>

#define SPARC_SSL   Boomerang::get()->getProgPath() + "frontend/machine/sparc/sparc.ssl"

/***************************************************************************//**
 * FUNCTION:        ParserTest::testRead
 * OVERVIEW:        Test reading the SSL file
 *============================================================================*/
TEST_F(ParserTest,testRead) {
    RTLInstDict d;
    ASSERT_TRUE(d.readSSLFile(SPARC_SSL));
}

/***************************************************************************//**
 * FUNCTION:        ParserTest::testExp
 * OVERVIEW:        Test parsing an expression
 *============================================================================*/
TEST_F(ParserTest,testExp) {
    std::string s("*i32* r0 := 5 + 6");
    Statement *a = SSLParser::parseExp(s.c_str());
    ASSERT_TRUE(a);
    std::ostringstream ost;
    a->print(ost);
    ASSERT_EQ("   0 "+s, std::string(ost.str()));
    std::string s2 = "*i32* r[0] := 5 + 6";
    a = SSLParser::parseExp(s2.c_str());
    ASSERT_TRUE(a);
    std::ostringstream ost2;
    a->print(ost2);
    // Still should print to string s, not s2
    ASSERT_EQ("   0 "+s, std::string(ost2.str()));
}

