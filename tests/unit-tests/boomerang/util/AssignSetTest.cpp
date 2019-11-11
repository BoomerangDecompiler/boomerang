#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "AssignSetTest.h"


#include "boomerang/util/StatementSet.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/exp/Location.h"


void AssignSetTest::testClear()
{
    AssignSet set;

    set.clear();
    QVERIFY(set.empty());

    std::shared_ptr<Assign> assign(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EAX)));
    set.insert(assign);

    set.clear();
    QVERIFY(set.empty());
}


void AssignSetTest::testEmpty()
{
    AssignSet set;
    QVERIFY(set.empty());
    QVERIFY(set.size() == 0);

    std::shared_ptr<Assign> assign(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EAX)));
    set.insert(assign);
    QVERIFY(!set.empty());
    QVERIFY(set.size() == 1);
}


void AssignSetTest::testSize()
{
    AssignSet set;
    QVERIFY(set.size() == 0);

    std::shared_ptr<Assign> assign(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EAX)));
    set.insert(assign);
    QVERIFY(set.size() == 1);
}


void AssignSetTest::testInsert()
{
    AssignSet set;

    std::shared_ptr<Assign> assign(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EAX)));
    set.insert(assign);

    AssignSet::iterator it = set.begin();
    QVERIFY(it != set.end());
    QCOMPARE((*it)->toString(), QString("   0 *v* r25 := r24"));

    set.insert(assign);
    QVERIFY(set.size() == 1); // don't insert twice
}


void AssignSetTest::testRemove()
{
    AssignSet set;
    std::shared_ptr<Assign> assign1(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EAX)));
    std::shared_ptr<Assign> assign2(new Assign(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));

    QCOMPARE(set.remove(assign1), false);
    QVERIFY(set.empty());

    set.insert(assign1);
    set.insert(assign2);
    QCOMPARE(set.remove(assign2), true);
    QVERIFY(set.size() == 1);
    QCOMPARE(set.remove(assign2), false);
    QVERIFY(set.size() == 1);
    QCOMPARE(set.remove(assign1), true);
    QVERIFY(set.empty());
}


void AssignSetTest::testMakeUnion()
{
    AssignSet set1, set2;
    std::shared_ptr<Assign> assign1(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EAX)));
    std::shared_ptr<Assign> assign2(new Assign(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));

    set1.insert(assign1);
    set1.makeUnion(set2);

    QVERIFY(set1.size() == 1);
    QVERIFY(set2.empty());
    QCOMPARE((*set1.begin())->toString(), QString("   0 *v* r25 := r24"));

    set2.insert(assign2);
    set1.makeUnion(set2);
    QVERIFY(set1.size() == 2);
    QVERIFY(set2.size() == 1);

    set1.clear();
    set1.makeUnion(set2);
    QVERIFY(set1.size() == 1);
    QVERIFY(set2.size() == 1);
}


void AssignSetTest::testMakeDiff()
{
    AssignSet set1, set2;
    std::shared_ptr<Assign> assign1(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EAX)));
    std::shared_ptr<Assign> assign2(new Assign(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));

    set1.insert(assign1);
    QVERIFY(set1.size() == 1);
    QVERIFY(set2.empty());

    set1.insert(assign2);
    set2.insert(assign2);

    set1.makeDiff(set2);
    QVERIFY(set1.size() == 1);
    QVERIFY(set2.size() == 1);
}


void AssignSetTest::testMakeIsect()
{
    AssignSet set1, set2;
    std::shared_ptr<Assign> assign1(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EAX)));
    std::shared_ptr<Assign> assign2(new Assign(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));

    set1.insert(assign1);
    set1.makeIsect(set2);
    QVERIFY(set1.empty());

    set1.insert(assign1);
    set2.insert(assign2);
    set1.makeIsect(set2);
    QVERIFY(set1.empty());
    QVERIFY(set2.size() == 1);

    set1.insert(assign1);
    set1.insert(assign2);
    set1.makeIsect(set2);
    QVERIFY(set1.size() == 1);
}


void AssignSetTest::testIsSubSetOf()
{
    AssignSet set1, set2;
    std::shared_ptr<Assign> assign1(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EAX)));
    std::shared_ptr<Assign> assign2(new Assign(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));

    QVERIFY(set1.isSubSetOf(set2));
    set2.insert(assign2);
    QVERIFY(set1.isSubSetOf(set2));

    set1.insert(assign1);
    QVERIFY(!set1.isSubSetOf(set2));
    set1.insert(assign2);
    QVERIFY(!set1.isSubSetOf(set2));
    QVERIFY(set2.isSubSetOf(set1));

    QVERIFY(set1.isSubSetOf(set1));
}


void AssignSetTest::testDefinesLoc()
{
    AssignSet set1;
    std::shared_ptr<Assign> assign1(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EAX)));
    set1.insert(assign1);

    QVERIFY(!set1.definesLoc(nullptr));
    QVERIFY(!set1.definesLoc(Location::regOf(REG_X86_EAX)));
    QVERIFY(set1.definesLoc(Location::regOf(REG_X86_ECX)));
}


void AssignSetTest::testLookupLoc()
{
    AssignSet set1;
    std::shared_ptr<Assign> assign1(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EAX)));
    set1.insert(assign1);

    QVERIFY(set1.lookupLoc(nullptr) == nullptr);
    QCOMPARE(set1.lookupLoc(Location::regOf(REG_X86_ECX)), assign1);
    QVERIFY(set1.lookupLoc(Location::regOf(REG_X86_EAX)) == nullptr);
}


QTEST_GUILESS_MAIN(AssignSetTest)
