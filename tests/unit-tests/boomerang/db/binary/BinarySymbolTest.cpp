#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BinarySymbolTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/binary/BinarySymbol.h"


void BinarySymbolTest::testCreate()
{
    BinarySymbol sym(Address(0x1000), "testSym");

    QCOMPARE(sym.getLocation(), Address(0x1000));
    QCOMPARE(sym.getName(), QString("testSym"));
    QCOMPARE(sym.getSize(), 0);
}


void BinarySymbolTest::testIsImportedFunction()
{
    BinarySymbol sym(Address(0x1000), "testSym");
    QVERIFY(!sym.isImportedFunction());

    sym.setAttribute("Function", true);
    sym.setAttribute("Imported", true);
    QVERIFY(sym.isImportedFunction());
}


void BinarySymbolTest::testIsStaticFunction()
{
    BinarySymbol sym(Address(0x1000), "testSym");
    QVERIFY(!sym.isStaticFunction());

    sym.setAttribute("StaticFunction", true);
    QVERIFY(sym.isStaticFunction());
}


void BinarySymbolTest::testIsFunction()
{
    BinarySymbol sym(Address(0x1000), "testSym");
    QVERIFY(!sym.isFunction());

    sym.setAttribute("Function", true);
    QVERIFY(sym.isFunction());
}


void BinarySymbolTest::testIsImported()
{
    BinarySymbol sym(Address(0x1000), "testSym");
    QVERIFY(!sym.isImported());

    sym.setAttribute("Imported", true);
    QVERIFY(sym.isImported());
}


void BinarySymbolTest::testBelongsToSourceFile()
{
    BinarySymbol sym(Address(0x1000), "testSym");
    QCOMPARE(sym.belongsToSourceFile(), QString(""));

    sym.setAttribute("SourceFile", "test.c");
    QCOMPARE(sym.belongsToSourceFile(), QString("test.c"));
}


QTEST_MAIN(BinarySymbolTest)
