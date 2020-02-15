#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ImplicitAssignTest.h"


#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/util/LocationSet.h"


void ImplicitAssignTest::testClone()
{
    {
        std::shared_ptr<ImplicitAssign> ias(new ImplicitAssign(Location::regOf(REG_X86_EAX)));
        SharedStmt clone = ias->clone();

        QVERIFY(&(*clone) != &(*ias));
        QVERIFY(clone->isImplicit());
        QVERIFY(clone->getID() != (uint32)-1);
        QVERIFY(clone->getID() != ias->getID());

        std::shared_ptr<ImplicitAssign> iasClone = clone->as<ImplicitAssign>();
        QVERIFY(iasClone->getLeft() != nullptr);
        QVERIFY(iasClone->getLeft() != ias->getLeft());
        QVERIFY(*iasClone->getLeft() == *ias->getLeft());
    }
}


void ImplicitAssignTest::testGetDefinitions()
{
    {
        LocationSet defs;

        // %eax := -
        std::shared_ptr<ImplicitAssign> imp(new ImplicitAssign(IntegerType::get(32, Sign::Signed), Location::regOf(REG_X86_EAX)));
        imp->getDefinitions(defs, false);
        QCOMPARE(defs.toString(), "r24");
    }
}


void ImplicitAssignTest::testDefinesLoc()
{
    {
        // %eax := -
        std::shared_ptr<ImplicitAssign> imp(new ImplicitAssign(Location::regOf(REG_X86_EAX)));

        QVERIFY(imp->definesLoc(Location::regOf(REG_X86_EAX)));
        QVERIFY(!imp->definesLoc(Location::regOf(REG_X86_ECX)));
    }

    {
        const SharedExp def = Ternary::get(opAt,
                                           Location::regOf(REG_X86_EAX),
                                           Const::get(0),
                                           Const::get(7));

        // %eax@[0:7] := -
        std::shared_ptr<ImplicitAssign> imp(new ImplicitAssign(def));

        QVERIFY(imp->definesLoc(def));
        QVERIFY(imp->definesLoc(Location::regOf(REG_X86_EAX)));
        QVERIFY(!imp->definesLoc(Location::regOf(REG_X86_ECX)));
    }
}


void ImplicitAssignTest::testSearch()
{
    {
        std::shared_ptr<ImplicitAssign> ias(new ImplicitAssign(Location::regOf(REG_X86_EAX)));

        SharedExp result;
        QVERIFY(!ias->search(*Const::get(0), result));

        QVERIFY(ias->search(*Terminal::get(opWildRegOf), result));
        QVERIFY(result != nullptr);
        QCOMPARE(*result, *Location::regOf(REG_X86_EAX));
    }
}


void ImplicitAssignTest::testSearchAll()
{
    {
        SharedExp eax = Location::regOf(REG_X86_EAX);
        std::shared_ptr<ImplicitAssign> ias(new ImplicitAssign(eax));

        std::list<SharedExp> result;
        QVERIFY(!ias->searchAll(*Const::get(0), result));

        QVERIFY(ias->searchAll(*Terminal::get(opWildRegOf), result));
        QCOMPARE(result, { eax });
    }
}


void ImplicitAssignTest::testSearchAndReplace()
{
    {
        std::shared_ptr<ImplicitAssign> ias1(new ImplicitAssign(Location::regOf(REG_X86_EAX)));
        std::shared_ptr<ImplicitAssign> ias2(new ImplicitAssign(Location::regOf(REG_X86_ECX)));

        QVERIFY(ias1->searchAndReplace(*Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));
        QCOMPARE(ias1->toString(), ias2->toString());
    }
}


QTEST_GUILESS_MAIN(ImplicitAssignTest)
