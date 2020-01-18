#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CaseStatementTest.h"


#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/util/LocationSet.h"


void CaseStatementTest::testClone()
{
    {
        SharedExp ecx = Location::regOf(REG_X86_ECX);

        std::shared_ptr<CaseStatement> cs(new CaseStatement(ecx));
        SharedStmt clone = cs->clone();

        QVERIFY(&(*clone) != &(*cs));
        QVERIFY(clone->isCase());
        QVERIFY(clone->getID() != (uint32)-1);
        QVERIFY(clone->getID() != cs->getID());

        std::shared_ptr<CaseStatement> csClone = clone->as<CaseStatement>();
        QVERIFY(csClone->getDest() != nullptr);
        QCOMPARE(*csClone->getDest(), *cs->getDest());
        QVERIFY(csClone->isComputed());
        QVERIFY(csClone->getSwitchInfo() == nullptr);
    }
}


void CaseStatementTest::testGetDefinitions()
{
    {
        LocationSet defs;
        std::shared_ptr<CaseStatement> cs(new CaseStatement(Location::regOf(REG_X86_ECX)));

        cs->getDefinitions(defs, false);

        QVERIFY(defs.empty());
    }
}


void CaseStatementTest::testDefinesLoc()
{
    {
        const SharedExp destExp = Location::regOf(REG_X86_ECX);

        std::shared_ptr<CaseStatement> cs(new CaseStatement(destExp));

        QVERIFY(!cs->definesLoc(Const::get(Address(0x1000))));
        QVERIFY(!cs->definesLoc(nullptr));
        QVERIFY(!cs->definesLoc(Location::regOf(REG_X86_ESP)));
        QVERIFY(!cs->definesLoc(Location::regOf(REG_X86_ECX)));
    }
}


void CaseStatementTest::testSearch()
{
    {
        std::shared_ptr<CaseStatement> cs(new CaseStatement(Location::regOf(REG_X86_ECX)));

        SharedExp result;
        QVERIFY(!cs->search(*Const::get(0x1000), result));
    }

    {
        std::shared_ptr<CaseStatement> cs(new CaseStatement(Location::regOf(REG_X86_ECX)));

        SharedExp result;
        QVERIFY(!cs->search(*Const::get(0), result));
        QVERIFY(cs->search(*Terminal::get(opWildRegOf), result));
        QVERIFY(result != nullptr);
        QCOMPARE(*result, *Location::regOf(REG_X86_ECX));
    }
}


void CaseStatementTest::testSearchAll()
{
    {
        std::shared_ptr<CaseStatement> cs(new CaseStatement(Location::regOf(REG_X86_ECX)));

        std::list<SharedExp> result;
        QVERIFY(!cs->searchAll(*Const::get(0x1000), result));
        QVERIFY(result.empty());
    }

    {
        std::shared_ptr<CaseStatement> cs(new CaseStatement(Location::regOf(REG_X86_ECX)));

        std::list<SharedExp> result;
        QVERIFY(!cs->searchAll(*Const::get(0), result));
        QVERIFY(result.empty());

        QVERIFY(cs->searchAll(*Terminal::get(opWildRegOf), result));
        QCOMPARE(result, { cs->getDest() });
    }
}


void CaseStatementTest::testSearchAndReplace()
{
    {
        std::shared_ptr<CaseStatement> cs(new CaseStatement(Location::regOf(REG_X86_EAX)));

        std::shared_ptr<CaseStatement> csClone = cs->clone()->as<CaseStatement>();
        csClone->setDest(Location::regOf(REG_X86_ECX));

        QVERIFY(cs->searchAndReplace(*Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));
        QCOMPARE(cs->toString(), csClone->toString());
    }
}


QTEST_GUILESS_MAIN(CaseStatementTest)
