#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PhiAssignTest.h"


#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/util/LocationSet.h"


void PhiAssignTest::testClone()
{
    {
        std::shared_ptr<PhiAssign> phi(new PhiAssign(IntegerType::get(32, Sign::Signed), Location::regOf(REG_X86_EAX)));
        SharedStmt clone = phi->clone();

        QVERIFY(&(*clone) != &(*phi));
        QVERIFY(clone->isPhi());
        QVERIFY(clone->getID() != (uint32)-1);
        QVERIFY(clone->getID() != phi->getID());

        std::shared_ptr<PhiAssign> phiClone = clone->as<PhiAssign>();
        QCOMPARE(phiClone->getNumDefs(), 0);

        QVERIFY(phiClone->getLeft() != nullptr);
        QVERIFY(phiClone->getLeft() != phi->getLeft());
        QVERIFY(*phiClone->getLeft() == *phi->getLeft());

        QVERIFY(phiClone->getType() != nullptr);
        QVERIFY(phiClone->getType() != phi->getType());
        QVERIFY(*phiClone->getType() == *phi->getType());
    }
}



void PhiAssignTest::testGetDefinitions()
{
    {
        LocationSet defs;

        // %eax := phi()
        std::shared_ptr<PhiAssign> phi(new PhiAssign(IntegerType::get(32, Sign::Signed), Location::regOf(REG_X86_EAX)));
        phi->getDefinitions(defs, false);
        QCOMPARE(defs.toString(), "r24");
    }
}


void PhiAssignTest::testDefinesLoc()
{
    {
        // %eax := phi()
        std::shared_ptr<PhiAssign> phi(new PhiAssign(Location::regOf(REG_X86_EAX)));

        QVERIFY(phi->definesLoc(Location::regOf(REG_X86_EAX)));
        QVERIFY(!phi->definesLoc(Location::regOf(REG_X86_ECX)));
    }

    {
        const SharedExp def = Ternary::get(opAt,
                                           Location::regOf(REG_X86_EAX),
                                           Const::get(0),
                                           Const::get(7));

        // %eax@[0:7] := phi()
        std::shared_ptr<PhiAssign> phi(new PhiAssign(def));

        QVERIFY(phi->definesLoc(def));
        QVERIFY(phi->definesLoc(Location::regOf(REG_X86_EAX)));
        QVERIFY(!phi->definesLoc(Location::regOf(REG_X86_ECX)));
    }
}


void PhiAssignTest::testSearch()
{
    {
        std::shared_ptr<PhiAssign> phi(new PhiAssign(Location::regOf(REG_X86_EAX)));

        SharedExp result;
        QVERIFY(!phi->search(*Location::regOf(REG_X86_ECX), result));
        QVERIFY(phi->search(*Location::regOf(REG_X86_EAX), result));
        QVERIFY(result != nullptr);
        QCOMPARE(*result, *Location::regOf(REG_X86_EAX));
    }

    // TODO: phi with arguments
}


void PhiAssignTest::testSearchAll()
{
    // PhiAssign
    {
        SharedExp eax = Location::regOf(REG_X86_EAX);
        std::shared_ptr<PhiAssign> phi(new PhiAssign(eax));

        std::list<SharedExp> result;
        QVERIFY(!phi->searchAll(*Location::regOf(REG_X86_ECX), result));
        QVERIFY(result.empty());

        QVERIFY(phi->searchAll(*eax, result));
        QCOMPARE(result, { eax });
    }

    // TODO: phi with arguments
}


void PhiAssignTest::testSearchAndReplace()
{
    // PhiAssign
    {
        std::shared_ptr<PhiAssign> phi(new PhiAssign(Location::regOf(REG_X86_EAX)));
        std::shared_ptr<PhiAssign> phiClone = phi->clone()->as<PhiAssign>();
        phiClone->setLeft(Location::regOf(REG_X86_ECX));

        QVERIFY(phi->searchAndReplace(*Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));
        QCOMPARE(phi->toString(), phiClone->toString());
    }

    // TODO Phi with arguments
}


QTEST_GUILESS_MAIN(PhiAssignTest)
