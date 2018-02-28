#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ConnectionGraphTest.h"


#include "boomerang/util/ConnectionGraph.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/exp/Location.h"


void ConnectionGraphTest::initTestCase()
{
}


void ConnectionGraphTest::testAdd()
{
    SharedExp e1 = Terminal::get(opCF);
    SharedExp e2 = Terminal::get(opZF);
    SharedExp e3 = Terminal::get(opCF);

    ConnectionGraph cg;
    QVERIFY(cg.add(e1, e2));
    QVERIFY(!cg.add(e1, e2)); // exact same exp already exists
    QVERIFY(!cg.add(e2, e1)); // reverse already exists
    QVERIFY(!cg.add(e2, e3)); // equal exp already exists
}


void ConnectionGraphTest::testConnect()
{
    SharedExp a = Terminal::get(opZF);
    SharedExp b = Terminal::get(opCF);
    SharedExp c = Terminal::get(opFZF);
    SharedExp d = Terminal::get(opOF);

    ConnectionGraph cg;

    cg.add(a, c);
    cg.connect(a, b);

    QVERIFY(cg.isConnected(a, *b));
    QVERIFY(cg.isConnected(a, *c));

    QVERIFY(!cg.isConnected(a, *d));
    QVERIFY(!cg.isConnected(b, *d));
}


void ConnectionGraphTest::testCount()
{
    SharedExp a = Terminal::get(opZF);
    SharedExp b = Terminal::get(opCF);
    SharedExp c = Terminal::get(opFZF);

    ConnectionGraph cg;

    QCOMPARE(cg.count(a), 0);

    cg.add(a, b);
    cg.add(a, c);

    QCOMPARE(cg.count(a), 2);
    QCOMPARE(cg.count(b), 1);
    QCOMPARE(cg.count(c), 1);
}


void ConnectionGraphTest::testIsConnected()
{
    SharedExp a = Terminal::get(opZF);
    SharedExp b = Terminal::get(opCF);
    SharedExp c = Terminal::get(opFZF);

    ConnectionGraph cg;

    QVERIFY(!cg.isConnected(a, *b));

    cg.add(a, b);
    cg.add(a, c);

    QVERIFY(cg.isConnected(a, *b));
    QVERIFY(!cg.isConnected(b, *c));
}


void ConnectionGraphTest::testAllRefsHaveDefs()
{
    ConnectionGraph cg;
    QVERIFY(cg.allRefsHaveDefs());

    Assign asgn(Location::regOf(25), Location::regOf(24));
    SharedExp ref1 = RefExp::get(Location::regOf(25), &asgn);
    cg.add(Location::regOf(30), ref1);

    QVERIFY(cg.allRefsHaveDefs());

    SharedExp ref2 = RefExp::get(Location::regOf(27), nullptr);
    cg.add(ref2, Location::regOf(32));

    QVERIFY(!cg.allRefsHaveDefs());
}


void ConnectionGraphTest::testUpdate()
{
    ConnectionGraph cg;

    SharedExp a = Terminal::get(opZF);
    SharedExp b = Terminal::get(opCF);
    SharedExp c = Terminal::get(opFZF);

    cg.add(a, b);
    cg.add(b, c);

    cg.update(a, b, c);

    QVERIFY(!cg.isConnected(a, *b));
    QVERIFY(cg.isConnected(a, *c));
    QVERIFY(cg.isConnected(b, *c));
}


QTEST_MAIN(ConnectionGraphTest)
