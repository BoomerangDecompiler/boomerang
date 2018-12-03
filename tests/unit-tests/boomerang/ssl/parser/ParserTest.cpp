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


#include "boomerang/ssl/RTLInstDict.h"
#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/util/log/Log.h"

#include <QDebug>


void ParserTest::testRead()
{
    RTLInstDict d(false);

    QVERIFY(d.readSSLFile(BOOMERANG_TEST_BASE "share/boomerang/ssl/x86.ssl"));
    QVERIFY(d.readSSLFile(BOOMERANG_TEST_BASE "share/boomerang/ssl/sparc.ssl"));
    QVERIFY(d.readSSLFile(BOOMERANG_TEST_BASE "share/boomerang/ssl/ppc.ssl"));
    QVERIFY(d.readSSLFile(BOOMERANG_TEST_BASE "share/boomerang/ssl/st20.ssl"));
}


QTEST_GUILESS_MAIN(ParserTest)
