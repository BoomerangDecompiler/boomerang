#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StatementSetTest.h"


#include "boomerang/db/exp/Location.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/util/StatementSet.h"


void StatementSetTest::testInsert()
{
    StatementSet set;

    Assign assign(Location::regOf(PENT_REG_ECX), Location::regOf(PENT_REG_EDX));
    set.insert(&assign);

    auto it = set.begin();
    QVERIFY(it != set.end());
    QVERIFY(*it == &assign);
}


void StatementSetTest::testRemove()
{
    StatementSet set;
    QVERIFY(!set.remove(nullptr));

    Assign assign(Location::regOf(PENT_REG_ECX), Location::regOf(PENT_REG_EDX));

    set.insert(&assign);
    QVERIFY(set.remove(&assign));
    QVERIFY(!set.remove(&assign)); // not contained in set
}


void StatementSetTest::testContains()
{
    StatementSet set;
    QVERIFY(!set.contains(nullptr));

    Assign assign(Location::regOf(PENT_REG_ECX), Location::regOf(PENT_REG_EDX));
    set.insert(&assign);
    QVERIFY(set.contains(&assign));
    set.remove(&assign);
    QVERIFY(!set.contains(&assign));
}


void StatementSetTest::testDefinesLoc()
{
    StatementSet set;
    QVERIFY(!set.definesLoc(nullptr));

    Assign assign(Location::regOf(PENT_REG_ECX), Location::regOf(PENT_REG_EDX));
    set.insert(&assign);
    QVERIFY(set.definesLoc(Location::regOf(PENT_REG_ECX)));
    set.remove(&assign);
    QVERIFY(!set.definesLoc(Location::regOf(PENT_REG_ECX)));
}


void StatementSetTest::testIsSubSetOf()
{
    StatementSet set1, set2;
    QVERIFY(set1.isSubSetOf(set2));

    Assign assign(Location::regOf(PENT_REG_ECX), Location::regOf(PENT_REG_EDX));
    set1.insert(&assign);
    QVERIFY(!set1.isSubSetOf(set2));
    QVERIFY(set2.isSubSetOf(set1));

    set2.insert(&assign);
    QVERIFY(set1.isSubSetOf(set2));

    Assign assign2(Location::regOf(PENT_REG_EDX), Location::regOf(PENT_REG_ECX));
    set1.insert(&assign2);
    QVERIFY(!set1.isSubSetOf(set2));
    QVERIFY(set2.isSubSetOf(set1));
}


void StatementSetTest::testMakeUnion()
{
    StatementSet set1, set2;

    set1.makeUnion(set2);
    QVERIFY(set1.begin() == set1.end());

    Assign assign1(Location::regOf(PENT_REG_ECX), Location::regOf(PENT_REG_EDX));
    Assign assign2(Location::regOf(PENT_REG_EDX), Location::regOf(PENT_REG_ECX));
    Assign assign3(Location::regOf(PENT_REG_EBX), Location::regOf(PENT_REG_ESI));

    set1.insert(&assign1);
    set1.insert(&assign2);
    set2.insert(&assign2);
    set2.insert(&assign3);

    set1.makeUnion(set1); // self union
    QVERIFY(std::distance(set1.begin(), set1.end()) == 2);
    set1.makeUnion(set2);
    QVERIFY(std::distance(set1.begin(), set1.end()) == 3);
}


void StatementSetTest::testMakeIsect()
{
    StatementSet set1, set2;
    set1.makeIsect(set2);
    QVERIFY(set1.begin() == set1.end());

    Assign assign1(Location::regOf(PENT_REG_ECX), Location::regOf(PENT_REG_EDX));
    Assign assign2(Location::regOf(PENT_REG_EDX), Location::regOf(PENT_REG_ECX));
    Assign assign3(Location::regOf(PENT_REG_EBX), Location::regOf(PENT_REG_ESI));

    set1.insert(&assign1);
    set1.insert(&assign2);
    set2.insert(&assign2);
    set2.insert(&assign3);

    set1.makeIsect(set1); // self intersection
    QVERIFY(std::distance(set1.begin(), set1.end()) == 2);

    set1.makeIsect(set2);
    QVERIFY(std::distance(set1.begin(), set1.end()) == 1);

}


void StatementSetTest::testMakeDiff()
{
    StatementSet set1, set2;
    set1.makeDiff(set2);
    QVERIFY(set1.begin() == set1.end());

    Assign assign1(Location::regOf(PENT_REG_ECX), Location::regOf(PENT_REG_EDX));
    Assign assign2(Location::regOf(PENT_REG_EDX), Location::regOf(PENT_REG_ECX));
    Assign assign3(Location::regOf(PENT_REG_EBX), Location::regOf(PENT_REG_ESI));

    set1.insert(&assign1);
    set1.insert(&assign2);
    set2.insert(&assign2);
    set2.insert(&assign3);

    set1.makeDiff(set2);
    QVERIFY(std::distance(set1.begin(), set1.end()) == 1);

    set1.makeDiff(set1); // self diff -> set1 == empty
    QVERIFY(std::distance(set1.begin(), set1.end()) == 0);
}


QTEST_GUILESS_MAIN(StatementSetTest)
