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

    cg.add(a, b);
    cg.add(c, d);

    cg.connect(a, b);
    QVERIFY(cg.isConnected(a, *b));
    QVERIFY(!cg.isConnected(a, *c));
    QVERIFY(!cg.isConnected(a, *d));
    QVERIFY(cg.isConnected(c, *d));

    cg.connect(a, c);
    QVERIFY(cg.isConnected(a, *c));
    QVERIFY(cg.isConnected(a, *b));
    QVERIFY(cg.isConnected(a, *d));
    QVERIFY(cg.isConnected(c, *b));
    QVERIFY(cg.isConnected(c, *d));
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


void ConnectionGraphTest::testUpdateConnection()
{
    ConnectionGraph cg;

    SharedExp a = Terminal::get(opZF);
    SharedExp b = Terminal::get(opCF);
    SharedExp c = Terminal::get(opFZF);
    SharedExp d = Terminal::get(opOF);

    cg.add(a, b);
    cg.add(c, d);

    // not connected before -> no change
    cg.updateConnection(a, c, d);
    QVERIFY(cg.isConnected(a, *b));
    QVERIFY(!cg.isConnected(a, *c));
    QVERIFY(!cg.isConnected(a, *d));

    cg.updateConnection(a, c, c);
    QVERIFY(cg.isConnected(a, *b));
    QVERIFY(!cg.isConnected(a, *c));
    QVERIFY(!cg.isConnected(a, *d));

    cg.updateConnection(c, d, a);
    QVERIFY(!cg.isConnected(c, *d));
    QVERIFY(cg.isConnected(c, *a));
    QVERIFY(cg.isConnected(a, *c));

    cg.updateConnection(b, a, d);
    QVERIFY(!cg.isConnected(b, *a));
    QVERIFY(cg.isConnected(b, *d));
    QVERIFY(cg.isConnected(d, *b));
}


QTEST_GUILESS_MAIN(ConnectionGraphTest)
