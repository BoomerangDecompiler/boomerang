#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "LocationSetTest.h"


#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/util/LocationSet.h"


void LocationSetTest::initTestCase()
{
}


void LocationSetTest::testAssign()
{
    LocationSet set1, set2;

    set1 = set2;
    QCOMPARE(set1.prints(), "");

    set2.insert(Location::regOf(24));
    set1 = set2;
    QCOMPARE(set1.prints(), "r24");
}


void LocationSetTest::testCompare()
{
    LocationSet set1, set2;
    QVERIFY(set1 == set2);
    QVERIFY(!(set1 != set2));

    set1.insert(Location::regOf(25));
    QVERIFY(!(set1 == set2));
    QVERIFY(set1 != set2);

    set2.insert(Location::regOf(25));
    QVERIFY(set1 == set2);
    QVERIFY(!(set1 != set2));

    set1.insert(Location::regOf(30));
    QVERIFY(!(set1 == set2));
    QVERIFY(set1 != set2);
}


void LocationSetTest::testEmpty()
{
    LocationSet set;
    QVERIFY(set.empty());

    set.insert(Location::regOf(30));
    QVERIFY(!set.empty());
}


void LocationSetTest::testSize()
{
    LocationSet set;
    QVERIFY(set.size() == 0);

    set.insert(Location::regOf(30));
    QVERIFY(set.size() == 1);

    set.insert(Location::regOf(32));
    QVERIFY(set.size() == 2);
}


void LocationSetTest::testClear()
{
    LocationSet set;

    set.clear();
    QVERIFY(set.empty());

    set.insert(Location::regOf(30));
    set.clear();
    QVERIFY(set.empty());
}


void LocationSetTest::testInsert()
{
    LocationSet set;

    set.insert(Location::regOf(30));
    QCOMPARE(set.prints(), "r30");

    set.insert(Location::regOf(30));
    QCOMPARE(set.prints(), "r30");

    set.insert(Location::regOf(32));
}


void LocationSetTest::testRemove()
{
    LocationSet set;
    set.remove(nullptr);
    QVERIFY(set.empty());

    set.insert(Location::regOf(30));
    set.remove(Location::regOf(30));
    QVERIFY(set.empty());

    set.insert(Location::regOf(30));
    set.insert(Location::regOf(32));
    set.remove(Location::regOf(32));
    QCOMPARE(set.prints(), "r30");

    // removing element that does not exist
    set.remove(Location::regOf(32));
    QCOMPARE(set.prints(), "r30");
}


void LocationSetTest::testContains()
{
    LocationSet set;

    QVERIFY(!set.contains(nullptr));

    set.insert(Location::regOf(30));
    QVERIFY(set.contains(Location::regOf(30)));
    QVERIFY(!set.contains(Location::regOf(32)));
}


void LocationSetTest::testExistsImplicit()
{
    LocationSet set;
    QVERIFY(!set.existsImplicit(nullptr));

    set.insert(Location::regOf(30));
    QVERIFY(!set.existsImplicit(Location::regOf(30)));
    QVERIFY(!set.existsImplicit(Location::regOf(32)));

    set.insert(RefExp::get(Location::regOf(32), nullptr));
    QVERIFY(set.existsImplicit(Location::regOf(32)));
    QVERIFY(!set.existsImplicit(Location::regOf(30)));
}



void LocationSetTest::testFindNS()
{
    LocationSet set;
    QVERIFY(set.findNS(nullptr) == nullptr);

    set.insert(Location::regOf(30));
    SharedExp e = set.findNS(Location::regOf(30));
    QVERIFY(e == nullptr);

    set.insert(RefExp::get(Location::regOf(32), nullptr));
    e = set.findNS(Location::regOf(32));
    QVERIFY(e != nullptr);

    QCOMPARE(e->prints(), "r32{-}");
}


void LocationSetTest::testFindDifferentRef()
{
    LocationSet set;

    SharedExp result;
    QVERIFY(!set.findDifferentRef(nullptr, result));

    set.insert(Location::regOf(24));
    QVERIFY(!set.findDifferentRef(RefExp::get(Location::regOf(24), nullptr), result));

    set.insert(RefExp::get(Location::regOf(24), nullptr));
    QVERIFY(!set.findDifferentRef(RefExp::get(Location::regOf(24), nullptr), result));

    Assign as1(Location::regOf(25), Location::regOf(26));
    Assign as2(Location::regOf(25), Location::regOf(26));

    as1.setNumber(10);
    as2.setNumber(20);

    set.insert(RefExp::get(Location::regOf(25), &as1));
    // no other ref
    QVERIFY(!set.findDifferentRef(RefExp::get(Location::regOf(25), &as1), result));

    set.insert(RefExp::get(Location::regOf(25), &as2));
    // return a different ref
    QVERIFY(set.findDifferentRef(RefExp::get(Location::regOf(25), &as1), result));
    QCOMPARE(result->prints(), "r25{20}");

    // should work even when the ref is not in the set
    set.remove(RefExp::get(Location::regOf(25), &as1));
    QVERIFY(set.findDifferentRef(RefExp::get(Location::regOf(25), &as1), result));
    QCOMPARE(result->prints(), "r25{20}");
}


void LocationSetTest::testAddSubscript()
{
    LocationSet set;
    set.addSubscript(nullptr);
    QCOMPARE(set.prints(), "");

    set.insert(Location::regOf(25));
    set.addSubscript(nullptr);
    QCOMPARE(set.prints(), "r25{-}");

    set.insert(Location::regOf(25));
    Assign as(Location::regOf(25), Location::regOf(26));
    as.setNumber(42);
    set.addSubscript(&as);
    QCOMPARE(set.prints(), "r25{-}, r25{42}");
}


void LocationSetTest::testMakeUnion()
{
    LocationSet set1, set2;

    set1.makeUnion(set2);
    QVERIFY(set1.empty());
    QVERIFY(set2.empty());

    set1.insert(Location::regOf(25));
    set1.insert(Location::regOf(26));
    set2.insert(Location::regOf(26));
    set2.insert(Location::regOf(27));

    QCOMPARE(set1.prints(), "r25, r26");
    QCOMPARE(set2.prints(), "r26, r27");

    set1.makeUnion(set2);

    QCOMPARE(set1.prints(), "r25, r26, r27");
    QCOMPARE(set2.prints(), "r26, r27");
}


void LocationSetTest::testMakeDiff()
{
    LocationSet set1, set2;
    set1.makeDiff(set2);
    QVERIFY(set1.empty());
    QVERIFY(set2.empty());

    set1.insert(Location::regOf(25));
    set1.insert(Location::regOf(26));
    set2.insert(Location::regOf(26));
    set2.insert(Location::regOf(27));

    QCOMPARE(set1.prints(), "r25, r26");
    QCOMPARE(set2.prints(), "r26, r27");

    set1.makeDiff(set2);
    QCOMPARE(set1.prints(), "r25");
    QCOMPARE(set2.prints(), "r26, r27");
}


void LocationSetTest::testSubstitute()
{
    LocationSet set;

    SharedExp r25 = Location::regOf(25);
    SharedExp r26 = Location::regOf(26);

    Assign a1(r25, r26);
    set.substitute(a1);
    QVERIFY(set.empty());

    set.insert(r25);
    set.substitute(a1);
    QCOMPARE(set.prints(), "r25, r26");

    Assign a2(r25, nullptr);
    set.substitute(a2);
    QCOMPARE(set.prints(), "r25, r26"); // no change

    Assign a3(r26, r25);
    set.substitute(a3);
    QCOMPARE(set.prints(), "r25, r26"); // no change

    Assign a4(r25, Terminal::get(opPC)); // will cause r25 to be removed because of terminal
    set.substitute(a4);
    QCOMPARE(set.prints(), "r26");

}


QTEST_MAIN(LocationSetTest)
