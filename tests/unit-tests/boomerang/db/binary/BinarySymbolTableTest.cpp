#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BinarySymbolTableTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/binary/BinarySymbolTable.h"


void BinarySymbolTableTest::initTestCase()
{
}


void BinarySymbolTableTest::testSize()
{
    BinarySymbolTable tbl;

    QCOMPARE(tbl.size(), 0);
    tbl.createSymbol(Address(0x1000), "testSym");
    QCOMPARE(tbl.size(), 1);
}


void BinarySymbolTableTest::testEmpty()
{
    BinarySymbolTable tbl;
    QVERIFY(tbl.empty());

    tbl.createSymbol(Address(0x1000), "testSym");
    QVERIFY(!tbl.empty());
}


void BinarySymbolTableTest::testClear()
{
    BinarySymbolTable tbl;
    tbl.clear();
    QVERIFY(tbl.empty());
    tbl.createSymbol(Address(0x1000), "testSym");
    tbl.clear();
    QVERIFY(tbl.empty());
}


void BinarySymbolTableTest::testCreateSymbol()
{
    BinarySymbolTable tbl;

    BinarySymbol *sym1 = tbl.createSymbol(Address(0x1000), "testSym");
    QVERIFY(sym1 != nullptr);
    QCOMPARE(sym1->getName(), QString("testSym"));
    QCOMPARE(sym1->getLocation(), Address(0x1000));
    QCOMPARE(sym1->getSize(), 0);

    BinarySymbol *sym2 = tbl.createSymbol(Address(0x1000), "testSym");
    QVERIFY(sym2 == nullptr);

    // same name -> return existing symbol
    BinarySymbol *sym3 = tbl.createSymbol(Address(0x2000), "testSym");
    QVERIFY(sym3 == sym1);
    QVERIFY(tbl.size() == 1); // do not insert new symbol
}


void BinarySymbolTableTest::testFindSymbolByAddress()
{
    BinarySymbolTable tbl;
    QVERIFY(tbl.findSymbolByAddress(Address(0x1000)) == nullptr);

    BinarySymbol *sym = tbl.createSymbol(Address(0x1000), "testSym");
    QVERIFY(tbl.findSymbolByAddress(Address(0x1000)) == sym);
    QVERIFY(tbl.findSymbolByAddress(Address(0x2000)) == nullptr);
}


void BinarySymbolTableTest::testFindSymbolByName()
{
    BinarySymbolTable tbl;
    QVERIFY(tbl.findSymbolByName("test") == nullptr);

    BinarySymbol *sym = tbl.createSymbol(Address(0x1000), "testSym");
    QVERIFY(tbl.findSymbolByName("testSym") == sym);
    QVERIFY(tbl.findSymbolByName("testSym2") == nullptr);
}


void BinarySymbolTableTest::testRenameSymbol()
{
    BinarySymbolTable tbl;
    QVERIFY(!tbl.renameSymbol("test1", "test2"));
    QVERIFY(tbl.empty());

    BinarySymbol *test1 = tbl.createSymbol(Address(0x1000), "test1");
    QVERIFY(tbl.renameSymbol("test1", "test1"));
    QCOMPARE(tbl.size(), 1);

    QVERIFY(tbl.renameSymbol("test1", "foo1"));
    QCOMPARE(tbl.size(), 1);
    QCOMPARE(test1->getName(), QString("foo1"));
    QVERIFY(tbl.findSymbolByName("foo1") == test1);

    tbl.createSymbol(Address(0x2000), "test2");
    QVERIFY(!tbl.renameSymbol("foo1", "test2")); // name clash
}


QTEST_MAIN(BinarySymbolTableTest)
