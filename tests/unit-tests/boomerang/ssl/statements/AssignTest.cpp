#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "AssignTest.h"


#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/util/LocationSet.h"

void AssignTest::testClone()
{
    {
        // %eax := %ebx
        std::shared_ptr<Assign> asgn(new Assign(IntegerType::get(32, Sign::Signed),
                                                Location::regOf(REG_X86_EAX),
                                                Location::regOf(REG_X86_EBX)));

        SharedStmt clone = asgn->clone();

        QVERIFY(&(*clone) != &(*asgn));
        QVERIFY(clone->isAssign());
        QVERIFY(clone->getID() != (uint32)-1);
        QVERIFY(clone->getID() != asgn->getID());

        std::shared_ptr<Assign> asgnClone = clone->as<Assign>();

        QVERIFY(asgnClone->getLeft() != nullptr);
        QVERIFY(asgnClone->getLeft() != asgn->getLeft());
        QVERIFY(*asgnClone->getLeft() == *asgn->getLeft());

        QVERIFY(asgnClone->getRight() != nullptr);
        QVERIFY(asgnClone->getRight() != asgn->getRight());
        QVERIFY(*asgnClone->getRight() == *asgn->getRight());

        QVERIFY(asgnClone->getGuard() == nullptr);
    }

    {
        // %ecx = 0 => %eax := %ebx
        std::shared_ptr<Assign> asgn(new Assign(IntegerType::get(32, Sign::Signed),
                                                Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_EBX),
                                                Binary::get(opEquals, Location::regOf(REG_X86_EAX), Const::get(0))));

        SharedStmt clone = asgn->clone();

        QVERIFY(&(*clone) != &(*asgn));
        QVERIFY(clone->isAssign());
        QVERIFY(clone->getID() != (uint32)-1);
        QVERIFY(clone->getID() != asgn->getID());

        std::shared_ptr<Assign> asgnClone = clone->as<Assign>();

        QVERIFY(asgnClone->getLeft() != nullptr);
        QVERIFY(asgnClone->getLeft() != asgn->getLeft());
        QVERIFY(*asgnClone->getLeft() == *asgn->getLeft());

        QVERIFY(asgnClone->getRight() != nullptr);
        QVERIFY(asgnClone->getRight() != asgn->getRight());
        QVERIFY(*asgnClone->getRight() == *asgn->getRight());

        QVERIFY(asgnClone->getGuard() != nullptr);
        QVERIFY(asgnClone->getGuard() != asgn->getGuard());
        QVERIFY(*asgnClone->getGuard() == *asgn->getGuard());
    }
}


void AssignTest::testGetDefinitions()
{
    {
        // foo@[m:n] really only defines foo
        LocationSet defs;

        // %eax@[0:7] := %ch
        std::shared_ptr<Assign> asgn(new Assign(
            Ternary::get(opAt,
                         Location::regOf(REG_X86_EAX),
                         Const::get(0),
                         Const::get(7)),
            Location::regOf(REG_X86_CH)));

        asgn->getDefinitions(defs, false);
        QCOMPARE(defs.toString(), "r24");
    }

    {
        LocationSet defs;

        // %flags := %ah
        std::shared_ptr<Assign> asgn(new Assign(Terminal::get(opFlags), Location::regOf(REG_X86_AH)));
        asgn->getDefinitions(defs, false);

        QCOMPARE(defs.toString(), "%flags, %ZF, %CF, %NF, %OF");
    }
}


void AssignTest::testDefinesLoc()
{
    {
        // %eax := %ecx
        std::shared_ptr<Assign> asgn(new Assign(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));

        QVERIFY(asgn->definesLoc(Location::regOf(REG_X86_EAX)));
        QVERIFY(!asgn->definesLoc(Location::regOf(REG_X86_ECX)));
    }

    {
        const SharedExp def = Ternary::get(opAt,
                                           Location::regOf(REG_X86_EAX),
                                           Const::get(0),
                                           Const::get(7));

        // %eax@[0:7] := %ch
        std::shared_ptr<Assign> asgn(new Assign(def, Location::regOf(REG_X86_CX)));

        QVERIFY(asgn->definesLoc(def));
        QVERIFY(asgn->definesLoc(Location::regOf(REG_X86_EAX)));
        QVERIFY(!asgn->definesLoc(Location::regOf(REG_X86_CH)));
    }
}


void AssignTest::testSearch()
{
    {
        std::shared_ptr<Assign> asgn(new Assign(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));

        SharedExp result;
        QVERIFY(!asgn->search(*Location::regOf(REG_X86_EDX), result));

        QVERIFY(asgn->search(*Location::regOf(REG_X86_EAX), result));
        QVERIFY(result != nullptr);
        QCOMPARE(*result, *Location::regOf(REG_X86_EAX));

        QVERIFY( asgn->search(*Location::regOf(REG_X86_ECX), result));
        QVERIFY(result != nullptr);
        QCOMPARE(*result, *Location::regOf(REG_X86_ECX));
    }
}


void AssignTest::testSearchAll()
{
    {
        SharedExp eax = Location::regOf(REG_X86_EAX);
        SharedExp ecx = Location::regOf(REG_X86_ECX);
        std::shared_ptr<Assign> asgn(new Assign(eax, ecx));

        std::list<SharedExp> result;
        QVERIFY(!asgn->searchAll(*Location::regOf(REG_X86_EDX), result));
        QVERIFY(result.empty());

        QVERIFY(asgn->searchAll(*eax, result));
        QCOMPARE(result, { eax });

        result.clear();
        QVERIFY(asgn->searchAll(*ecx, result));
        QCOMPARE(result, { ecx });

        // %eax := %eax
        asgn->setRight(eax);
        result.clear();
        QVERIFY(asgn->searchAll(*eax, result));
        QCOMPARE(result, std::list<SharedExp>({ eax, eax }));
    }
}


void AssignTest::testSearchAndReplace()
{
    {
        std::shared_ptr<Assign> asgn1(new Assign(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_EAX)));
        std::shared_ptr<Assign> asgn2(new Assign(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_ECX)));

        QVERIFY(asgn1->searchAndReplace(*Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));
        QCOMPARE(asgn1->toString(), asgn2->toString());
    }

    {
        std::shared_ptr<Assign> asgn1(new Assign(Binary::get(opNotEqual, Location::regOf(REG_X86_EAX), Const::get(0)), Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_EAX)));
        std::shared_ptr<Assign> asgn2(new Assign(Binary::get(opNotEqual, Location::regOf(REG_X86_ECX), Const::get(0)), Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_ECX)));

        QVERIFY(asgn1->searchAndReplace(*Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));
        QCOMPARE(asgn1->toString(), asgn2->toString());
    }
}


void AssignTest::testGuard()
{
    std::shared_ptr<Assign> asgn(new Assign(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));
    QVERIFY(!asgn->isGuarded());
    QVERIFY(asgn->getGuard() == nullptr);

    asgn->setGuard(Terminal::get(opTrue));

    QVERIFY(asgn->isGuarded());
    QVERIFY(asgn->getGuard() != nullptr);
    QCOMPARE(asgn->getGuard()->toString(), QString("true"));
}


void AssignTest::testSubExps()
{
    SharedExp ecx = Location::regOf(REG_X86_ECX);
    SharedExp eax = Location::regOf(REG_X86_EAX);

    std::shared_ptr<Assign> asgn(new Assign(eax, ecx));

    QVERIFY(asgn->getLeft() == eax);
    QVERIFY(asgn->getRight() == ecx);

    asgn->setLeft(ecx);
    asgn->setRight(eax);

    QVERIFY(asgn->getLeft() == ecx);
    QVERIFY(asgn->getRight() == eax);
}


QTEST_GUILESS_MAIN(AssignTest)
