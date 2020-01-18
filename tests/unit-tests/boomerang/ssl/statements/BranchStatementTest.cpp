#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BranchStatementTest.h"


#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/util/LocationSet.h"


void BranchStatementTest::testClone()
{
    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement);
        SharedStmt clone = bs->clone();

        QVERIFY(&(*clone) != &(*bs));
        QVERIFY(clone->isBranch());
        QVERIFY(clone->getID() != (uint32)-1);
        QVERIFY(clone->getID() != bs->getID());

        std::shared_ptr<BranchStatement> bsClone = clone->as<BranchStatement>();
        QCOMPARE(bsClone->getDest(), nullptr);
        QCOMPARE(bsClone->getCondExpr(), nullptr);
        QCOMPARE(bsClone->isComputed(), bs->isComputed());

        QCOMPARE(bsClone->getCondType(), bs->getCondType());
        QCOMPARE(bsClone->isFloatBranch(), bs->isFloatBranch());
    }

    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement);
        bs->setCondType(BranchType::JE, true);
        bs->setDest(Address(0x1000));
        SharedStmt clone = bs->clone();

        QVERIFY(&(*clone) != &(*bs));
        QVERIFY(clone->isBranch());
        QVERIFY(clone->getID() != (uint32)-1);
        QVERIFY(clone->getID() != bs->getID());

        std::shared_ptr<BranchStatement> bsClone = clone->as<BranchStatement>();
        QVERIFY(bsClone->getDest() != nullptr);
        QVERIFY(bsClone->getCondExpr() != nullptr);

        QVERIFY(&(*bsClone->getDest())     != &(*bs->getDest()));
        QVERIFY(&(*bsClone->getCondExpr()) != &(*bs->getCondExpr()));

        QCOMPARE(*bsClone->getDest(),     *bs->getDest());
        QCOMPARE(*bsClone->getCondExpr(), *bs->getCondExpr());

        QCOMPARE(bsClone->isComputed(), bs->isComputed());
        QCOMPARE(bsClone->getCondType(), bs->getCondType());
        QCOMPARE(bsClone->isFloatBranch(), bs->isFloatBranch());
    }
}


void BranchStatementTest::testGetDefinitions()
{
    {
        LocationSet defs;
        std::shared_ptr<BranchStatement> bs(new BranchStatement);

        bs->getDefinitions(defs, false);

        QVERIFY(defs.empty());
    }
}


void BranchStatementTest::testDefinesLoc()
{
    {
        const SharedExp condExp = Binary::get(opEquals, Location::regOf(REG_X86_EAX), Const::get(0));

        std::shared_ptr<BranchStatement> bs(new BranchStatement);
        bs->setDest(Address(0x1000));
        bs->setCondExpr(condExp);

        QVERIFY(!bs->definesLoc(Const::get(Address(0x1000))));
        QVERIFY(!bs->definesLoc(nullptr));
        QVERIFY(!bs->definesLoc(Location::regOf(REG_X86_ESP)));
        QVERIFY(!bs->definesLoc(condExp));
        QVERIFY(!bs->definesLoc(Location::regOf(REG_X86_EAX)));
    }
}


void BranchStatementTest::testSearch()
{
    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement);

        SharedExp result;
        QVERIFY(!bs->search(*Const::get(0x1000), result));
    }

    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement);
        bs->setDest(Address(0x1000));

        SharedExp result;
        QVERIFY(!bs->search(*Const::get(0), result));
        QVERIFY(bs->search(*Const::get(0x1000), result));
        QVERIFY(result != nullptr);
        QCOMPARE(*result, *Const::get(Address(0x1000)));

        bs->setCondExpr(Binary::get(opEquals, Location::regOf(REG_X86_ECX), Const::get(0)));
        QVERIFY(bs->search(*Terminal::get(opWildRegOf), result));
        QVERIFY(result != nullptr);
        QCOMPARE(*result, *Location::regOf(REG_X86_ECX));
    }
}


void BranchStatementTest::testSearchAll()
{
    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement);

        std::list<SharedExp> result;
        QVERIFY(!bs->searchAll(*Const::get(0x1000), result));
        QVERIFY(result.empty());
    }

    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement);
        bs->setDest(Address(0x1000));
        bs->setCondExpr(nullptr);

        std::list<SharedExp> result;
        QVERIFY(!bs->searchAll(*Const::get(0), result));
        QVERIFY(result.empty());

        QVERIFY(bs->searchAll(*Const::get(0x1000), result));
        QCOMPARE(result, { bs->getDest() });
    }

    {
        SharedExp ecx = Location::regOf(REG_X86_ECX);

        std::shared_ptr<BranchStatement> bs(new BranchStatement);
        bs->setDest(Address(0x1000));
        bs->setCondExpr(Binary::get(opEquals, ecx, Const::get(0)));

        std::list<SharedExp> result;
        QVERIFY(bs->searchAll(*Terminal::get(opWildRegOf), result));
        QCOMPARE(result, { ecx });
    }
}


void BranchStatementTest::testSearchAndReplace()
{
    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement);
        bs->setDest(Location::regOf(REG_X86_ECX));

        auto bsClone = bs->clone()->as<GotoStatement>();
        QVERIFY(!bs->searchAndReplace(*Const::get(Address(0x1000)), Const::get(0x800)));
        QCOMPARE(bs->toString(), bsClone->toString());

        bsClone->setDest(Address(0x1000));

        QVERIFY(bs->searchAndReplace(*Location::regOf(REG_X86_ECX), Const::get(Address(0x1000))));
        QCOMPARE(bs->toString(), bsClone->toString());
    }

    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement);
        bs->setDest(Location::regOf(REG_X86_EAX));
        bs->setCondExpr(Binary::get(opEquals, Location::regOf(REG_X86_EAX), Const::get(0)));

        auto bsClone = bs->clone()->as<BranchStatement>();
        bsClone->setDest(Location::regOf(REG_X86_ECX));
        bsClone->setCondExpr(Binary::get(opEquals, Location::regOf(REG_X86_ECX), Const::get(0)));

        QVERIFY(bs->searchAndReplace(*Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));
        QCOMPARE(bs->toString(), bsClone->toString());
    }
}


QTEST_GUILESS_MAIN(BranchStatementTest)
