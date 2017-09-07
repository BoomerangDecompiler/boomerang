#include "IntervalMapTest.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/util/IntervalMap.h"


#include <QTextStream>
#include <QDebug>


void IntervalMapTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "/lib/boomerang/");
}


void IntervalMapTest::testIsEmpty()
{
    IntervalMap<Address, int> map;
    QVERIFY(map.isEmpty());
    map.insert(Interval<Address>(Address::ZERO, Address(0x1000)), 10);
    QVERIFY(!map.isEmpty());
}


void IntervalMapTest::testFind()
{
    IntervalMap<Address, int> map;
    IntervalMap<Address, int>::iterator it1 = map.insert(Address(0x1000), Address(0x2000), 10);
    IntervalMap<Address, int>::iterator it2 = map.insert(Address(0x2000), Address(0x3000), 20);

    QVERIFY(map.find(Address::ZERO)   == map.end());
    QVERIFY(map.find(Address(0x1000)) == it1);
    QVERIFY(map.find(Address(0x1800)) == it1);
    QVERIFY(map.find(Address(0x2000)) == it2);
    QVERIFY(map.find(Address(0x3000)) == map.end());
}

QTEST_MAIN(IntervalMapTest)
