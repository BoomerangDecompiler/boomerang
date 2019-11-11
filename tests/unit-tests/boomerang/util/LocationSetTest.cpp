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


#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/util/LocationSet.h"


void LocationSetTest::testAssign()
{
    LocationSet set1, set2;
    set1 = set2;
    QCOMPARE(set1, LocationSet());

    set2.insert(Location::regOf(REG_X86_EAX));
    set1 = set2;
    QCOMPARE(set1, LocationSet({ Location::regOf(REG_X86_EAX) }));
}


void LocationSetTest::testCompare()
{
    LocationSet set1, set2;
    QVERIFY(set1 == set2);
    QVERIFY(!(set1 != set2));

    set1.insert(Location::regOf(REG_X86_ECX));
    QVERIFY(!(set1 == set2));
    QVERIFY(set1 != set2);

    set2.insert(Location::regOf(REG_X86_ECX));
    QVERIFY(set1 == set2);
    QVERIFY(!(set1 != set2));

    set1.insert(Location::regOf(REG_X86_EDI));
    QVERIFY(!(set1 == set2));
    QVERIFY(set1 != set2);
}


void LocationSetTest::testEmpty()
{
    LocationSet set;
    QVERIFY(set.empty());

    set.insert(Location::regOf(REG_X86_EDI));
    QVERIFY(!set.empty());
}


void LocationSetTest::testSize()
{
    LocationSet set;
    QVERIFY(set.size() == 0);

    set.insert(Location::regOf(REG_X86_ESI));
    QVERIFY(set.size() == 1);

    set.insert(Location::regOf(REG_X86_EDI));
    QVERIFY(set.size() == 2);
}


void LocationSetTest::testClear()
{
    LocationSet set;

    set.clear();
    QVERIFY(set.empty());

    set.insert(Location::regOf(REG_X86_ESI));
    set.clear();
    QVERIFY(set.empty());
}


void LocationSetTest::testInsert()
{
    LocationSet set;

    set.insert(Location::regOf(REG_X86_ESI));
    QCOMPARE(set, LocationSet({ Location::regOf(REG_X86_ESI) }));

    set.insert(Location::regOf(REG_X86_ESI));
    QCOMPARE(set, LocationSet({ Location::regOf(REG_X86_ESI) }));

    set.insert(Location::regOf(REG_X86_EDI));
    QCOMPARE(set, LocationSet({ Location::regOf(REG_X86_ESI), Location::regOf(REG_X86_EDI) }));
}


void LocationSetTest::testRemove()
{
    LocationSet set;
    set.remove(nullptr);
    QVERIFY(set.empty());

    set.insert(Location::regOf(REG_X86_ESI));
    set.remove(Location::regOf(REG_X86_ESI));
    QVERIFY(set.empty());

    set.insert(Location::regOf(REG_X86_ESI));
    set.insert(Location::regOf(REG_X86_EDI));
    set.remove(Location::regOf(REG_X86_EDI));
    QCOMPARE(set, LocationSet({ Location::regOf(REG_X86_ESI) }));

    // removing element that does not exist
    set.remove(Location::regOf(REG_X86_EDI));
    QCOMPARE(set, LocationSet({ Location::regOf(REG_X86_ESI) }));
}


void LocationSetTest::testContains()
{
    LocationSet set;

    QVERIFY(!set.contains(nullptr));

    set.insert(Location::regOf(REG_X86_ESI));
    QVERIFY(set.contains(Location::regOf(REG_X86_ESI)));
    QVERIFY(!set.contains(Location::regOf(REG_X86_EDI)));
}


void LocationSetTest::testContainsImplicit()
{
    LocationSet set;
    QVERIFY(!set.containsImplicit(nullptr));

    set.insert(Location::regOf(REG_X86_ESI));
    QVERIFY(!set.containsImplicit(Location::regOf(REG_X86_ESI)));
    QVERIFY(!set.containsImplicit(Location::regOf(REG_X86_EDI)));

    set.insert(RefExp::get(Location::regOf(REG_X86_EDI), nullptr));
    QVERIFY(set.containsImplicit(Location::regOf(REG_X86_EDI)));
    QVERIFY(!set.containsImplicit(Location::regOf(REG_X86_ESI)));
}


void LocationSetTest::testFindNS()
{
    LocationSet set;
    QVERIFY(set.findNS(nullptr) == nullptr);

    set.insert(Location::regOf(REG_X86_ESI));
    SharedExp e = set.findNS(Location::regOf(REG_X86_ESI));
    QVERIFY(e == nullptr);

    set.insert(RefExp::get(Location::regOf(REG_X86_EDI), nullptr));
    e = set.findNS(Location::regOf(REG_X86_EDI));
    QVERIFY(e != nullptr);

    QCOMPARE(e->toString(), QString("r31{-}"));
}


void LocationSetTest::testFindDifferentRef()
{
    LocationSet set;

    SharedExp result;
    QVERIFY(!set.findDifferentRef(nullptr, result));

    set.insert(Location::regOf(REG_X86_EAX));
    QVERIFY(!set.findDifferentRef(RefExp::get(Location::regOf(REG_X86_EAX), nullptr), result));

    set.insert(RefExp::get(Location::regOf(REG_X86_EAX), nullptr));
    QVERIFY(!set.findDifferentRef(RefExp::get(Location::regOf(REG_X86_EAX), nullptr), result));

    std::shared_ptr<Assign> as1(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));
    std::shared_ptr<Assign> as2(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));

    as1->setNumber(10);
    as2->setNumber(20);

    set.insert(RefExp::get(Location::regOf(REG_X86_ECX), as1));
    // no other ref
    QVERIFY(!set.findDifferentRef(RefExp::get(Location::regOf(REG_X86_ECX), as1), result));

    set.insert(RefExp::get(Location::regOf(REG_X86_ECX), as2));
    // return a different ref
    QVERIFY(set.findDifferentRef(RefExp::get(Location::regOf(REG_X86_ECX), as1), result));
    QCOMPARE(result->toString(), QString("r25{20}"));

    // should work even when the ref is not in the set
    set.remove(RefExp::get(Location::regOf(REG_X86_ECX), as1));
    QVERIFY(set.findDifferentRef(RefExp::get(Location::regOf(REG_X86_ECX), as1), result));
    QCOMPARE(result->toString(), QString("r25{20}"));
}


void LocationSetTest::testAddSubscript()
{
    LocationSet set;
    set.addSubscript(nullptr);
    QCOMPARE(set, LocationSet());

    set.insert(Location::regOf(REG_X86_ECX));
    set.addSubscript(nullptr);
    QCOMPARE(set, LocationSet({ RefExp::get(Location::regOf(REG_X86_ECX), nullptr) }));

    set.insert(Location::regOf(REG_X86_ECX));
    std::shared_ptr<Assign> as(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));
    as->setNumber(42);
    set.addSubscript(as);
    QCOMPARE(set, LocationSet({
        RefExp::get(Location::regOf(REG_X86_ECX), nullptr),
        RefExp::get(Location::regOf(REG_X86_ECX), as) }));
}


void LocationSetTest::testMakeUnion()
{
    LocationSet set1, set2;

    set1.makeUnion(set2);
    QVERIFY(set1.empty());
    QVERIFY(set2.empty());

    set1.insert(Location::regOf(REG_X86_ECX));
    set1.insert(Location::regOf(REG_X86_EDX));
    set2.insert(Location::regOf(REG_X86_EDX));
    set2.insert(Location::regOf(REG_X86_EBX));

    QCOMPARE(set1, LocationSet({ Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX) }));
    QCOMPARE(set2, LocationSet({ Location::regOf(REG_X86_EDX), Location::regOf(REG_X86_EBX) }));

    set1.makeUnion(set2);

    QCOMPARE(set1, LocationSet({ Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX), Location::regOf(REG_X86_EBX), }));
    QCOMPARE(set2, LocationSet({ Location::regOf(REG_X86_EDX), Location::regOf(REG_X86_EBX) }));
}


void LocationSetTest::testMakeDiff()
{
    LocationSet set1, set2;
    set1.makeDiff(set2);
    QVERIFY(set1.empty());
    QVERIFY(set2.empty());

    set1.insert(Location::regOf(REG_X86_ECX));
    set1.insert(Location::regOf(REG_X86_EDX));
    set2.insert(Location::regOf(REG_X86_EDX));
    set2.insert(Location::regOf(REG_X86_EBX));

    QCOMPARE(set1, LocationSet({ Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX) }));
    QCOMPARE(set2, LocationSet({ Location::regOf(REG_X86_EDX), Location::regOf(REG_X86_EBX) }));

    set1.makeDiff(set2);
    QCOMPARE(set1, LocationSet({ Location::regOf(REG_X86_ECX) }));
    QCOMPARE(set2, LocationSet({ Location::regOf(REG_X86_EDX), Location::regOf(REG_X86_EBX) }));
}


QTEST_GUILESS_MAIN(LocationSetTest)
