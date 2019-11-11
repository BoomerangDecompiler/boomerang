#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StatementListTest.h"


#include "boomerang/util/StatementList.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/util/LocationSet.h"
#include "boomerang/util/StatementSet.h"


void StatementListTest::testEmpty()
{
    StatementList list;
    QVERIFY(list.empty());

    std::shared_ptr<Assign> a1(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));
    list.append(a1);

    QVERIFY(!list.empty());
}


void StatementListTest::testSize()
{
    StatementList list;
    QVERIFY(list.size() == 0);

    std::shared_ptr<Assign> a1(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));

    list.append(a1);
    QVERIFY(list.size() == 1);

    list.append(a1);
    QVERIFY(list.size() == 2);
}


void StatementListTest::testMakeIsect()
{
    StatementList list1, list2;
    LocationSet locs;

    list1.makeIsect(list1, locs);
    QVERIFY(list1.empty());
    QVERIFY(locs.empty());

    std::shared_ptr<Assign> a1(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));

    list1.append(a1);
    list1.makeIsect(list1, locs);
    QVERIFY(list1.empty());
    QVERIFY(locs.empty());

    list1.append(a1);
    locs.insert(Location::regOf(REG_X86_ECX));
    list1.makeIsect(list1, locs);
    QCOMPARE(list1.toString(), QString("   0 *v* r25 := r26"));

    list2.makeIsect(list1, locs);
    QCOMPARE(list1.toString(), QString("   0 *v* r25 := r26"));

    locs.clear();
    list2.makeIsect(list1, locs);
    QVERIFY(list2.empty());
}


void StatementListTest::testAppend()
{
    StatementList list;
    std::shared_ptr<Assign> assign(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));

    list.append(assign);
    QCOMPARE(list.toString(), QString("   0 *v* r25 := r26"));

    list.append(StatementList());
    QCOMPARE(list.toString(), QString("   0 *v* r25 := r26"));

    list.append(list);
    QCOMPARE(list.toString(), QString("   0 *v* r25 := r26,\t   0 *v* r25 := r26"));

    StatementSet set;
    std::shared_ptr<Assign> asgn(new Assign(Location::regOf(REG_X86_ESI), Location::regOf(REG_X86_EDI)));
    set.insert(asgn);

    list.append(set);
    QCOMPARE(list.toString(), QString("   0 *v* r25 := r26,\t   0 *v* r25 := r26,\t   0 *v* r30 := r31"));
}


void StatementListTest::testRemove()
{
    StatementList list;
    QVERIFY(!list.remove(nullptr));

    std::shared_ptr<Assign> assign(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));
    list.append(assign);
    list.append(assign);

    QVERIFY(list.remove(assign));
    QVERIFY(list.remove(assign));
    QVERIFY(!list.remove(assign));
}


void StatementListTest::testRemoveFirstDefOf()
{
    StatementList list;
    QVERIFY(!list.removeFirstDefOf(nullptr));

    std::shared_ptr<Assign> assign(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));
    list.append(assign);

    QVERIFY(!list.removeFirstDefOf(Location::regOf(REG_X86_EDX)));
    QVERIFY(list.size() == 1);
    QVERIFY(list.removeFirstDefOf(Location::regOf(REG_X86_ECX)));
    QVERIFY(list.empty());
}


void StatementListTest::testExistsOnLeft()
{
    StatementList list;
    std::shared_ptr<Assign> assign(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));

    QVERIFY(!list.existsOnLeft(nullptr));

    list.append(assign);
    QVERIFY(list.existsOnLeft(Location::regOf(REG_X86_ECX)));
    QVERIFY(!list.existsOnLeft(Location::regOf(REG_X86_EDX)));
}


void StatementListTest::testFindOnLeft()
{
    StatementList list;
    QVERIFY(list.findOnLeft(nullptr) == nullptr);

    std::shared_ptr<Assign> assign(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_EDX)));
    list.append(assign);

    QVERIFY(list.findOnLeft(Location::regOf(REG_X86_ECX)) == assign);
    QVERIFY(list.findOnLeft(Location::regOf(REG_X86_EDX)) == nullptr);
}


QTEST_GUILESS_MAIN(StatementListTest)
