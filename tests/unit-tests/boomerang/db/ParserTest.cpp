#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ParserTest.h"


#include "boomerang/ssl/parser/SSLParser.h"
#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/util/Log.h"

#include <QDebug>


void ParserTest::testRead()
{
    RTLInstDict d(false);

    QVERIFY(d.readSSLFile(BOOMERANG_TEST_BASE "share/boomerang/ssl/sparc.ssl"));
}


void ParserTest::testExp()
{
    QString   s("*i32* r0 := 5 + 6");
    Statement *a = SSLParser::parseExp(qPrintable(s));

    QVERIFY(a);
    QString     res;
    QTextStream ost(&res);
    a->print(ost);
    QCOMPARE(res, "   0 " + s);
    QString s2 = "*i32* r[0] := 5 + 6";
    a = SSLParser::parseExp(qPrintable(s2));
    QVERIFY(a);
    res.clear();
    a->print(ost);
    // Still should print to string s, not s2
    QCOMPARE(res, "   0 " + s);
}


QTEST_GUILESS_MAIN(ParserTest)
