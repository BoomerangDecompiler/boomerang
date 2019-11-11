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


#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/util/StatementSet.h"


void StatementSetTest::testInsert()
{
    StatementSet set;

    std::shared_ptr<Assign> assign(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));
    set.insert(assign);

    auto it = set.begin();
    QVERIFY(it != set.end());
    QVERIFY(*it == assign);
}


void StatementSetTest::testRemove()
{
    StatementSet set;
    QVERIFY(!set.remove(nullptr));

    std::shared_ptr<Assign> assign(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));

    set.insert(assign);
    QVERIFY(set.remove(assign));
    QVERIFY(!set.remove(assign)); // not contained in set
}


void StatementSetTest::testContains()
{
    StatementSet set;
    QVERIFY(!set.contains(nullptr));

    std::shared_ptr<Assign> assign(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));
    set.insert(assign);
    QVERIFY(set.contains(assign));
    set.remove(assign);
    QVERIFY(!set.contains(assign));
}


void StatementSetTest::testDefinesLoc()
{
    StatementSet set;
    QVERIFY(!set.definesLoc(nullptr));

    std::shared_ptr<Assign> assign(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));
    set.insert(assign);
    QVERIFY(set.definesLoc(Location::regOf(REG_X86_ECX)));
    set.remove(assign);
    QVERIFY(!set.definesLoc(Location::regOf(REG_X86_ECX)));
}


void StatementSetTest::testIsSubSetOf()
{
    StatementSet set1, set2;
    QVERIFY(set1.isSubSetOf(set2));

    std::shared_ptr<Assign> assign(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));
    set1.insert(assign);
    QVERIFY(!set1.isSubSetOf(set2));
    QVERIFY(set2.isSubSetOf(set1));

    set2.insert(assign);
    QVERIFY(set1.isSubSetOf(set2));

    std::shared_ptr<Assign> assign2(new Assign(Location::regOf(REG_X86_EDX), Location::regOf(REG_X86_ECX)));
    set1.insert(assign2);
    QVERIFY(!set1.isSubSetOf(set2));
    QVERIFY(set2.isSubSetOf(set1));
}


void StatementSetTest::testMakeUnion()
{
    StatementSet set1, set2;

    set1.makeUnion(set2);
    QVERIFY(set1.begin() == set1.end());

    std::shared_ptr<Assign> assign1(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));
    std::shared_ptr<Assign> assign2(new Assign(Location::regOf(REG_X86_EDX), Location::regOf(REG_X86_ECX)));
    std::shared_ptr<Assign> assign3(new Assign(Location::regOf(REG_X86_EBX), Location::regOf(REG_X86_ESI)));

    set1.insert(assign1);
    set1.insert(assign2);
    set2.insert(assign2);
    set2.insert(assign3);

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

    std::shared_ptr<Assign> assign1(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));
    std::shared_ptr<Assign> assign2(new Assign(Location::regOf(REG_X86_EDX), Location::regOf(REG_X86_ECX)));
    std::shared_ptr<Assign> assign3(new Assign(Location::regOf(REG_X86_EBX), Location::regOf(REG_X86_ESI)));

    set1.insert(assign1);
    set1.insert(assign2);
    set2.insert(assign2);
    set2.insert(assign3);

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

    std::shared_ptr<Assign> assign1(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));
    std::shared_ptr<Assign> assign2(new Assign(Location::regOf(REG_X86_EDX), Location::regOf(REG_X86_ECX)));
    std::shared_ptr<Assign> assign3(new Assign(Location::regOf(REG_X86_EBX), Location::regOf(REG_X86_ESI)));

    set1.insert(assign1);
    set1.insert(assign2);
    set2.insert(assign2);
    set2.insert(assign3);

    set1.makeDiff(set2);
    QVERIFY(std::distance(set1.begin(), set1.end()) == 1);

    set1.makeDiff(set1); // self diff -> set1 == empty
    QVERIFY(std::distance(set1.begin(), set1.end()) == 0);
}


QTEST_GUILESS_MAIN(StatementSetTest)
