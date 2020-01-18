#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "GotoStatementTest.h"


#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/GotoStatement.h"
#include "boomerang/util/LocationSet.h"


void GotoStatementTest::testClone()
{
    {
        std::shared_ptr<GotoStatement> gs(new GotoStatement(Address(0x0800)));
        SharedStmt clone = gs->clone();

        QVERIFY(&(*clone) != &(*gs));
        QVERIFY(clone->isGoto());
        QVERIFY(clone->getID() != (uint32)-1);
        QVERIFY(clone->getID() != gs->getID());

        std::shared_ptr<GotoStatement> gsClone = clone->as<GotoStatement>();
        QVERIFY(gsClone->getDest() != nullptr);
        QCOMPARE(*gsClone->getDest(), *gs->getDest());
        QVERIFY(!gsClone->isComputed());
    }

    {
        std::shared_ptr<GotoStatement> gs(new GotoStatement(Address(0x1000)));
        SharedStmt clone = gs->clone();

        QVERIFY(&(*clone) != &(*gs));
        QVERIFY(clone->isGoto());
        QVERIFY(clone->getID() != (uint32)-1);
        QVERIFY(clone->getID() != gs->getID());

        std::shared_ptr<GotoStatement> gsClone = clone->as<GotoStatement>();
        QVERIFY(gsClone->getDest() != nullptr);
        QVERIFY(gsClone->getDest() != gs->getDest()); // ptr compare
        QVERIFY(*gsClone->getDest() == *Const::get(Address(0x1000)));
        QVERIFY(!gsClone->isComputed());
    }
}


void GotoStatementTest::testGetDefinitions()
{
    {
        LocationSet defs;
        std::shared_ptr<GotoStatement> gs(new GotoStatement(Address(0x1000)));

        gs->getDefinitions(defs, false);

        QVERIFY(defs.empty());
    }
}


void GotoStatementTest::testDefinesLoc()
{
    {
        std::shared_ptr<GotoStatement> gs(new GotoStatement(Address(0x1000)));

        QVERIFY(!gs->definesLoc(Const::get(Address(0x1000))));
        QVERIFY(!gs->definesLoc(nullptr));
        QVERIFY(!gs->definesLoc(Location::regOf(REG_X86_ESP)));
    }
}


void GotoStatementTest::testSearch()
{
    {
        std::shared_ptr<GotoStatement> gs(new GotoStatement(Address(0x0800)));

        SharedExp result;
        QVERIFY(!gs->search(*Const::get(0x1000), result));
    }

    {
        std::shared_ptr<GotoStatement> gs(new GotoStatement(Address(0x1000)));

        SharedExp result;
        QVERIFY(!gs->search(*Const::get(0), result));
        QVERIFY(gs->search(*Const::get(0x1000), result));
        QVERIFY(result != nullptr);
        QCOMPARE(*result, *Const::get(Address(0x1000)));
    }
}


void GotoStatementTest::testSearchAll()
{
    {
        std::shared_ptr<GotoStatement> gs(new GotoStatement(Address(0x0800)));

        std::list<SharedExp> result;
        QVERIFY(!gs->searchAll(*Const::get(0x1000), result));
        QVERIFY(result.empty());
    }

    {
        std::shared_ptr<GotoStatement> gs(new GotoStatement(Address(0x1000)));

        std::list<SharedExp> result;
        QVERIFY(!gs->searchAll(*Const::get(0), result));
        QVERIFY(result.empty());

        QVERIFY(gs->searchAll(*Const::get(0x1000), result));
        QCOMPARE(result, { gs->getDest() });
    }
}


void GotoStatementTest::testSearchAndReplace()
{
    {
        std::shared_ptr<GotoStatement> gs(new GotoStatement(Location::regOf(REG_X86_ECX)));

        auto gsClone = gs->clone()->as<GotoStatement>();
        QVERIFY(!gs->searchAndReplace(*Const::get(Address(0x1000)), Const::get(0x800)));
        QCOMPARE(gs->toString(), gsClone->toString());

        gsClone->setDest(Address(0x1000));

        QVERIFY(gs->searchAndReplace(*Location::regOf(REG_X86_ECX), Const::get(Address(0x1000))));
        QCOMPARE(gs->toString(), gsClone->toString());
    }
}


QTEST_GUILESS_MAIN(GotoStatementTest)
