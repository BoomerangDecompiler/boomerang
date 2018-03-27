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


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/ssl/sslparser.h"
#include "boomerang/db/statements/Statement.h"
#include "boomerang/util/Log.h"

#include <QDebug>


#define SPARC_SSL    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("ssl/sparc.ssl"))


void ParserTest::cleanupTestCase()
{
    Boomerang::destroy();
}


void ParserTest::testRead()
{
    RTLInstDict d;

    QVERIFY(d.readSSLFile(SPARC_SSL));
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


QTEST_MAIN(ParserTest)
