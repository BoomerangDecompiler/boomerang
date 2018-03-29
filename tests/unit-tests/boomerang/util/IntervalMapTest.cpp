#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "IntervalMapTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/util/IntervalMap.h"


#include <QTextStream>
#include <QDebug>


void IntervalMapTest::testIsEmpty()
{
    IntervalMap<Address, int> map;
    QVERIFY(map.isEmpty());
    map.insert(Interval<Address>(Address::ZERO, Address(0x1000)), 10);
    QVERIFY(!map.isEmpty());
}


void IntervalMapTest::testClear()
{
    IntervalMap<Address, int> map;
    map.clear();
    QVERIFY(map.isEmpty());

    map.insert(Address(0x1000), Address(0x1010), 10);
    map.clear();
    QVERIFY(map.isEmpty());
}


void IntervalMapTest::testInsert()
{
    IntervalMap<Address, int> map;

    QVERIFY(map.insert(Address(0x1000), Address(0x1000), 10) == map.end());
    QVERIFY(map.isEmpty());

    QVERIFY(map.insert(Address(0x1000), Address(0x1010), 10) != map.end());
    QVERIFY(!map.isEmpty());

    QVERIFY(map.insert(Address(0x1010), Address(0x2000), 20) != map.end());
    QVERIFY(std::distance(map.begin(), map.end()) == 2);

    // insertion fails: interval already occupied
    QVERIFY(map.insert(Address(0x1000), Address(0x2000), 30) == map.end());
    QVERIFY(std::distance(map.begin(), map.end()) == 2);
}


void IntervalMapTest::testErase()
{
    IntervalMap<Address, int> map;

    map.insert(Address(0x1000), Address(0x1010), 10);
    QVERIFY(map.erase(map.begin()) == map.end());

    map.insert(Address(0x1000), Address(0x1010), 10);
    map.insert(Address(0x2000), Address(0x2020), 20);
    auto it = map.erase(map.begin());
    QVERIFY(it == map.begin());
}


void IntervalMapTest::testEraseAll()
{
    IntervalMap<Address, int> map;

    map.eraseAll(Address(0x1000));
    QVERIFY(map.isEmpty());

    map.insert(Address(0x1000), Address(0x1000), 10);
    map.insert(Address(0x1000), Address(0x1010), 10);

    map.eraseAll(Address(0x1010));
    QVERIFY(!map.isEmpty());
    map.eraseAll(Address(0x800));
    QVERIFY(!map.isEmpty());

    map.eraseAll(Address(0x1000));
    QVERIFY(map.isEmpty());

    map.insert(Address(0x1000), Address(0x1010), 10);
    map.insert(Address(0x1008), Address(0x1010), 20);
    map.eraseAll(Address(0x100C));
    QVERIFY(map.isEmpty());
}


void IntervalMapTest::testFind()
{
    IntervalMap<Address, int>           map;
    IntervalMap<Address, int>::iterator it1 = map.insert(Address(0x1000), Address(0x2000), 10);
    IntervalMap<Address, int>::iterator it2 = map.insert(Address(0x2000), Address(0x3000), 20);

    QVERIFY(map.find(Address::ZERO) == map.end());
    QVERIFY(map.find(Address(0x1000)) == it1);       // start of interval
    QVERIFY(map.find(Address(0x1800)) == it1);       // middle of interval
    QVERIFY(map.find(Address(0x2000)) == it2);       // beginning of interval with preceding interval
    QVERIFY(map.find(Address(0x3000)) == map.end()); // end of interval
}


void IntervalMapTest::testEqualRange()
{
    IntervalMap<Address, int> map;

    auto p = map.equalRange(Address(0x1000), Address(0x1010));
    QVERIFY(p.first == map.end() && p.second == map.end());

    map.insert(Address(0x1000), Address(0x1010), 10);
    p = map.equalRange(Address(0x1000), Address(0x1010));
    QVERIFY(p.first == map.begin());
    QVERIFY(p.first != map.end());
    QVERIFY(p.second == map.end());

    p = map.equalRange(Address(0x800), Address(0x1000));
    QVERIFY(p.first == map.end());
    QVERIFY(p.second == map.end());

    p = map.equalRange(Address(0x800), Address(0x1008));
    QVERIFY(p.first == map.begin());
    QVERIFY(p.second == map.end());

    p = map.equalRange(Address(0x1000), Address(0x1008));
    QVERIFY(p.first == map.begin());
    QVERIFY(p.second == map.end());

    p = map.equalRange(Address(0x1008), Address(0x100C));
    QVERIFY(p.first == map.begin());
    QVERIFY(p.second == map.end());

    p = map.equalRange(Address(0x1008), Address(0x1010));
    QVERIFY(p.first == map.begin());
    QVERIFY(p.second == map.end());

    p = map.equalRange(Address(0x1008), Address(0x2000));
    QVERIFY(p.first == map.begin());
    QVERIFY(p.second == map.end());

    p = map.equalRange(Address(0x1010), Address(0x2000));
    QVERIFY(p.first == map.end());
    QVERIFY(p.second == map.end());

    IntervalMap<Address, int>::iterator itSecond = map.insert(Address(0x2000), Address(0x2020), 20);
    IntervalMap<Address, int>::iterator itFirst  = map.begin();

    p = map.equalRange(Address(0x1008), Address(0x1010));
    QVERIFY(p.first == itFirst);
    QVERIFY(p.second == itSecond);

    p = map.equalRange(Address(0x1008), Address(0x2010));
    QVERIFY(p.first == itFirst);
    QVERIFY(p.second == map.end());

    p = map.equalRange(Address(0x1080), Address(0x2010));
    QVERIFY(p.first == itSecond);
    QVERIFY(p.second == map.end());

    p = map.equalRange(Address(0x1080), Address(0x1080));
    QVERIFY(p.first == map.end());
    QVERIFY(p.second == map.end());

    p = map.equalRange(Address(0x2000), Address(0x2000));
    QVERIFY(p.first == map.end());
    QVERIFY(p.second == map.end());

    p = map.equalRange(Address(0x2000), Address(0x2020));
    QVERIFY(p.first == itSecond);
    QVERIFY(p.second == map.end());
}


QTEST_GUILESS_MAIN(IntervalMapTest)
