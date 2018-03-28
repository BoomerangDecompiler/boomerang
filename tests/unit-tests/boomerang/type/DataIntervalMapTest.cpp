#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DataIntervalMapTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/type/DataIntervalMap.h"
#include "boomerang/type/type/IntegerType.h"


void DataIntervalMapTest::testIsClear()
{
    DataIntervalMap dim;

    QVERIFY(dim.isClear(Address::ZERO, Address(0x1000)));

    dim.insertItem(Address(0x1000), "first", IntegerType::get(32, 1));

    QVERIFY(dim.isClear(Address::ZERO, Address(0x1000)));
    QVERIFY(!dim.isClear(Address(0x0800), Address(0x1004)));
    QVERIFY(!dim.isClear(Address(0x1000), Address(0x1004)));
    QVERIFY(!dim.isClear(Address(0x1002), Address(0x1010)));
    QVERIFY(dim.isClear(Address(0x1004), Address(0x1010)));
}


void DataIntervalMapTest::testFind()
{
    DataIntervalMap dim;

    QVERIFY(dim.find(Address(0x1000)) == nullptr);
    QVERIFY(dim.find_it(Address(0x1000)) == dim.end());

    dim.insertItem(Address(0x1000), "first", IntegerType::get(32, 1));

    QVERIFY(dim.find(Address(0x800)) == nullptr);
    QVERIFY(dim.find(Address(0x1000)) != nullptr);
    QVERIFY(dim.find(Address(0x1002)) != nullptr);
    QVERIFY(dim.find(Address(0x1004)) == nullptr);

    QVERIFY(dim.find_it(Address(0x800)) == dim.end());
    QVERIFY(dim.find_it(Address(0x1000)) != dim.end());
    QVERIFY(dim.find_it(Address(0x1002)) != dim.end());
    QVERIFY(dim.find_it(Address(0x1004)) == dim.end());
}


void DataIntervalMapTest::testInsert()
{
    DataIntervalMap dim;

    DataIntervalMap::iterator it = dim.insertItem(Address(0x1000), "first", IntegerType::get(32, 1));

    const TypedVariable& var = it->second;

    QVERIFY(var.baseAddr == Address(0x1000));
    QVERIFY(var.size == 32);
    QVERIFY(var.name == "first");
    QCOMPARE(var.type->toString(), IntegerType::get(32, 1)->toString());

    // overlapped non-forced
    QVERIFY(dim.insertItem(Address(0x1002), "second", IntegerType::get(32, 1)) == dim.end());
    QCOMPARE(dim.prints(), "0x00001000-0x00001004 first int\n");

    // overlapped forced
    DataIntervalMap::iterator it2 = dim.insertItem(Address(0x1002), "second", IntegerType::get(32, 1), true);
    QVERIFY(it2 != dim.end());
    QCOMPARE(dim.prints(), "0x00001002-0x00001006 second int\n");
}


QTEST_MAIN(DataIntervalMapTest)
