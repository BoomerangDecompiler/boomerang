#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "IntervalSetTest.h"

#include "boomerang/util/IntervalSet.h"
#include "boomerang/util/Address.h"


void IntervalSetTest::initTestCase()
{
}


void IntervalSetTest::testIsEmpty()
{
    IntervalSet<Address> set;

    QVERIFY(set.isEmpty());
    set.insert(Address(0x1000), Address(0x1010));
    QVERIFY(!set.isEmpty());
}


void IntervalSetTest::testClear()
{
    IntervalSet<Address> set;
    set.clear();
    QVERIFY(set.isEmpty());

    set.insert(Address(0x1000), Address(0x1010));
    set.insert(Address(0x2000), Address(0x2020));
    set.clear();
    QVERIFY(set.isEmpty());
}


void IntervalSetTest::testInsert()
{
    IntervalSet<Address> set;

    set.insert(Address(0x1000), Address(0x1000));
    QVERIFY(std::distance(set.begin(), set.end()) == 0);

    set.insert(Address(0x1000), Address(0x1010));
    QVERIFY(std::distance(set.begin(), set.end()) == 1);

    // insert (non-blocking)
    set.insert(Address(0x2000), Address(0x2020));
    QVERIFY(std::distance(set.begin(), set.end()) == 2);

    // Insert (adjacent)
    set.insert(Address(0x1010), Address(0x2000));
    QVERIFY(std::distance(set.begin(), set.end()) == 3);

    // insert (blocking)
    set.insert(Address(0x2000), Address(0x2020));
    QVERIFY(std::distance(set.begin(), set.end()) == 3);
}


void IntervalSetTest::testEqualRange()
{
    IntervalSet<Address> set;

    auto p = set.equalRange(Address(0x1000), Address(0x1010));
    QVERIFY(p.first == set.end() && p.second == set.end());

    set.insert(Address(0x1000), Address(0x1010));
    p = set.equalRange(Address(0x1000), Address(0x1010));
    QVERIFY(p.first == set.begin());
    QVERIFY(p.first != set.end());
    QVERIFY(p.second == set.end());

    p = set.equalRange(Address(0x800), Address(0x1000));
    QVERIFY(p.first == set.end());
    QVERIFY(p.second == set.end());

    p = set.equalRange(Address(0x800), Address(0x1008));
    QVERIFY(p.first == set.begin());
    QVERIFY(p.second == set.end());

    p = set.equalRange(Address(0x1000), Address(0x1008));
    QVERIFY(p.first == set.begin());
    QVERIFY(p.second == set.end());

    p = set.equalRange(Address(0x1008), Address(0x100C));
    QVERIFY(p.first == set.begin());
    QVERIFY(p.second == set.end());

    p = set.equalRange(Address(0x1008), Address(0x1010));
    QVERIFY(p.first == set.begin());
    QVERIFY(p.second == set.end());

    p = set.equalRange(Address(0x1008), Address(0x2000));
    QVERIFY(p.first == set.begin());
    QVERIFY(p.second == set.end());

    p = set.equalRange(Address(0x1010), Address(0x2000));
    QVERIFY(p.first == set.end());
    QVERIFY(p.second == set.end());

    IntervalSet<Address>::iterator itSecond = set.insert(Address(0x2000), Address(0x2020));
    IntervalSet<Address>::iterator itFirst = set.begin();

    p = set.equalRange(Address(0x1008), Address(0x1010));
    QVERIFY(p.first == itFirst);
    QVERIFY(p.second == itSecond);

    p = set.equalRange(Address(0x1008), Address(0x2010));
    QVERIFY(p.first == itFirst);
    QVERIFY(p.second == set.end());

    p = set.equalRange(Address(0x1080), Address(0x2010));
    QVERIFY(p.first == itSecond);
    QVERIFY(p.second == set.end());

    p = set.equalRange(Address(0x1080), Address(0x1080));
    QVERIFY(p.first == set.end());
    QVERIFY(p.second == set.end());

    p = set.equalRange(Address(0x2000), Address(0x2000));
    QVERIFY(p.first == set.end());
    QVERIFY(p.second == set.end());

    p = set.equalRange(Address(0x2000), Address(0x2020));
    QVERIFY(p.first == itSecond);
    QVERIFY(p.second == set.end());
}


void IntervalSetTest::testIsContained()
{
    IntervalSet<Address> set;
    QVERIFY(!set.isContained(Address(0x1000)));

    set.insert(Address(0x1000), Address(0x1010));
    QVERIFY(set.isContained(Address(0x1000)));
    QVERIFY(set.isContained(Address(0x1008)));
    QVERIFY(!set.isContained(Address(0x1010)));

    set.insert(Address(0x2000), Address(0x2020));
    QVERIFY(!set.isContained(Address(0x1080)));
    QVERIFY(!set.isContained(Address(0x2040)));
}


QTEST_GUILESS_MAIN(IntervalSetTest)
