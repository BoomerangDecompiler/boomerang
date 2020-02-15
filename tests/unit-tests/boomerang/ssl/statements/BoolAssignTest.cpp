#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BoolAssignTest.h"


#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/statements/BoolAssign.h"
#include "boomerang/util/LocationSet.h"


Q_DECLARE_METATYPE(BranchType)


void BoolAssignTest::testAssign()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);

    {
        std::shared_ptr<BoolAssign> bas1(new BoolAssign(eax, BranchType::JE, ecx));
        std::shared_ptr<BoolAssign> bas2(new BoolAssign(ecx, BranchType::JE, eax));

        *bas2 = *bas1;

        QCOMPARE(*bas2->getLeft(), *bas1->getLeft());
        QCOMPARE(*bas2->getCondExpr(), *bas1->getCondExpr());
        QCOMPARE(bas2->getCond(), bas1->getCond());
    }
}


void BoolAssignTest::testClone()
{
    std::shared_ptr<BoolAssign> bas(new BoolAssign(Location::regOf(REG_X86_EAX), BranchType::JE, Location::regOf(REG_X86_ECX)));
    SharedStmt clone = bas->clone();

    QVERIFY(&(*clone) != &(*bas));
    QVERIFY(clone->isBool());
    QVERIFY(clone->getID() != (uint32)-1);
    QVERIFY(clone->getID() != bas->getID());

    std::shared_ptr<BoolAssign> basClone = clone->as<BoolAssign>();
    QVERIFY(basClone->getLeft() != nullptr);
    QVERIFY(basClone->getLeft() != bas->getLeft());
    QVERIFY(*basClone->getLeft() == *bas->getLeft());
}


void BoolAssignTest::testGetDefinitions()
{
    {
        LocationSet defs;

        SharedExp ecx = Location::regOf(REG_X86_ECX);
        SharedExp cond = Binary::get(opEquals, ecx, Const::get(0));

        std::shared_ptr<BoolAssign> asgn(new BoolAssign(ecx, BranchType::JE, cond));
        asgn->getDefinitions(defs, false);

        QCOMPARE(defs.toString(), "r25");
    }
}


void BoolAssignTest::testDefinesLoc()
{
    {
        // %eax := (%ecx != 0)
        SharedExp eax = Location::regOf(REG_X86_EAX);
        SharedExp ecx = Location::regOf(REG_X86_ECX);

        const SharedExp condExp = Binary::get(opEquals, ecx, Const::get(0));

        std::shared_ptr<BoolAssign> asgn(new BoolAssign(eax, BranchType::JE, condExp));

        QVERIFY(asgn->definesLoc(Location::regOf(REG_X86_EAX)));
        QVERIFY(!asgn->definesLoc(Location::regOf(REG_X86_ECX)));
        QVERIFY(!asgn->definesLoc(condExp));
    }

    {
        SharedExp eax = Location::regOf(REG_X86_EAX);
        SharedExp ecx = Location::regOf(REG_X86_ECX);

        const SharedExp condExp = Binary::get(opEquals, ecx, Const::get(0));
        const SharedExp def = Ternary::get(opAt,
                                           eax,
                                           Const::get(0),
                                           Const::get(7));

        // %eax@[0:7] := (%ecx != 0)
        std::shared_ptr<BoolAssign> asgn(new BoolAssign(def, BranchType::JE, condExp));

        QVERIFY(asgn->definesLoc(def));
        QVERIFY(asgn->definesLoc(eax));
        QVERIFY(!asgn->definesLoc(ecx));
        QVERIFY(!asgn->definesLoc(condExp));
    }
}

void BoolAssignTest::testSearch()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);

    SharedExp cond = Binary::get(opEquals, ecx, Const::get(0));
    std::shared_ptr<BoolAssign> bas(new BoolAssign(eax, BranchType::JE, cond));

    SharedExp result;
    QVERIFY(!bas->search(*Const::get(32), result));

    QVERIFY(bas->search(*eax, result));
    QVERIFY(result != nullptr);
    QCOMPARE(*result, *eax);

    QVERIFY(bas->search(*ecx, result));
    QVERIFY(result != nullptr);
    QCOMPARE(*result, *ecx);
}


void BoolAssignTest::testSearchAll()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);
    SharedExp cond = Binary::get(opEquals, ecx, Const::get(0));

    std::shared_ptr<BoolAssign> bas(new BoolAssign(eax, BranchType::JE, cond));

    std::list<SharedExp> result;
    QVERIFY(!bas->searchAll(*Const::get(32), result));
    QVERIFY(result.empty());

    QVERIFY(bas->searchAll(*eax, result));
    QCOMPARE(result, { eax });

    result.clear();
    QVERIFY(bas->searchAll(*ecx, result));
    QCOMPARE(result, { ecx });
}


void BoolAssignTest::testSearchAndReplace()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);

    SharedExp cond1 = Binary::get(opEquals, eax, Const::get(0));
    std::shared_ptr<BoolAssign> bas1(new BoolAssign(eax, BranchType::JE, cond1));

    SharedExp cond2 = Binary::get(opEquals, ecx, Const::get(0));
    std::shared_ptr<BoolAssign> bas2(new BoolAssign(ecx, BranchType::JE, cond2));

    QVERIFY(bas1->searchAndReplace(*eax, ecx));
    QCOMPARE(bas1->toString(), bas2->toString());
}


void BoolAssignTest::testCond()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);

    SharedExp cond1 = Binary::get(opEquals, eax, Const::get(0));
    std::shared_ptr<BoolAssign> bas1(new BoolAssign(eax, BranchType::JE, cond1));

    QVERIFY(bas1->getCondExpr() == cond1);

    SharedExp cond2 = Binary::get(opEquals, ecx, Const::get(0));
    bas1->setCondExpr(cond2);
    QVERIFY(bas1->getCondExpr() == cond2);

    bas1->setCondType(BranchType::JNE, false);
    QVERIFY(bas1->getCondExpr() != nullptr);
    QCOMPARE(bas1->getCondExpr()->toString(), "%flags");

    bas1->setCondType(BranchType::JNE, true);
    QVERIFY(bas1->getCondExpr() != nullptr);
    QCOMPARE(bas1->getCondExpr()->toString(), "%flags"); // or %fflags?
}


void BoolAssignTest::testSubExps()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);

    SharedExp cond1 = Binary::get(opEquals, eax, Const::get(0));
    std::shared_ptr<BoolAssign> bas1(new BoolAssign(eax, BranchType::JE, cond1));

    QVERIFY(bas1->getLeft() == eax);
    QVERIFY(bas1->getRight() == cond1);

    cond1 = Binary::get(opEquals, ecx, Const::get(0));
    bas1->setLeft(ecx);
    bas1->setCondExpr(cond1);

    QVERIFY(bas1->getLeft() == ecx);
    QVERIFY(bas1->getRight() == cond1);

}


void BoolAssignTest::testPrint()
{
    SharedExp eax = Location::regOf(REG_X86_EAX);
    SharedExp ecx = Location::regOf(REG_X86_ECX);

    QFETCH(BranchType, bt);
    QFETCH(bool, isFloat);
    QFETCH(QString, expectedResult);

    std::shared_ptr<BoolAssign> bas(new BoolAssign(eax, bt, ecx));
    bas->setFloat(isFloat);

    expectedResult = QString("   0 BOOL r24 := ") + expectedResult + QString("\nHigh level: r25\n");

    QCOMPARE(bas->toString(), expectedResult);
}


#define TEST_PRINT(name, bt, isFloat, ccStr) \
    do { \
        QTest::newRow(name) \
            << bt \
            << isFloat \
            << QString(ccStr); \
    } while (false);


void BoolAssignTest::testPrint_data()
{
    QTest::addColumn<BranchType>("bt");
    QTest::addColumn<bool>("isFloat");
    QTest::addColumn<QString>("expectedResult");

    TEST_PRINT("JE",    BranchType::JE,    false, "CC(equals)");
    TEST_PRINT("JNE",   BranchType::JNE,   false, "CC(not equals)");
    TEST_PRINT("JSL",   BranchType::JSL,   false, "CC(signed less)");
    TEST_PRINT("JSLE",  BranchType::JSLE,  false, "CC(signed less or equals)");
    TEST_PRINT("JSGE",  BranchType::JSGE,  false, "CC(signed greater or equals)");
    TEST_PRINT("JSG",   BranchType::JSG,   false, "CC(signed greater)");
    TEST_PRINT("JUL",   BranchType::JUL,   false, "CC(unsigned less)");
    TEST_PRINT("JULE",  BranchType::JULE,  false, "CC(unsigned less or equals)");
    TEST_PRINT("JUGE",  BranchType::JUGE,  false, "CC(unsigned greater or equals)");
    TEST_PRINT("JUG",   BranchType::JUG,   false, "CC(unsigned greater)");
    TEST_PRINT("JMI",   BranchType::JMI,   false, "CC(minus)");
    TEST_PRINT("JPOS",  BranchType::JPOS,  false, "CC(plus)");
    TEST_PRINT("JOF",   BranchType::JOF,   false, "CC(overflow)");
    TEST_PRINT("JNOF",  BranchType::JNOF,  false, "CC(no overflow)");
    TEST_PRINT("JPAR",  BranchType::JPAR,  false, "CC(ev parity)");
    TEST_PRINT("JNPAR", BranchType::JNPAR, false, "CC(odd parity)");

    TEST_PRINT("JE,f",    BranchType::JE,    true, "CC(equals), float");
    TEST_PRINT("JNE,f",   BranchType::JNE,   true, "CC(not equals), float");
    TEST_PRINT("JSL,f",   BranchType::JSL,   true, "CC(signed less), float");
    TEST_PRINT("JSLE,f",  BranchType::JSLE,  true, "CC(signed less or equals), float");
    TEST_PRINT("JSGE,f",  BranchType::JSGE,  true, "CC(signed greater or equals), float");
    TEST_PRINT("JSG,f",   BranchType::JSG,   true, "CC(signed greater), float");
    TEST_PRINT("JUL,f",   BranchType::JUL,   true, "CC(unsigned less), float");
    TEST_PRINT("JULE,f",  BranchType::JULE,  true, "CC(unsigned less or equals), float");
    TEST_PRINT("JUGE,f",  BranchType::JUGE,  true, "CC(unsigned greater or equals), float");
    TEST_PRINT("JUG,f",   BranchType::JUG,   true, "CC(unsigned greater), float");
    TEST_PRINT("JMI,f",   BranchType::JMI,   true, "CC(minus), float");
    TEST_PRINT("JPOS,f",  BranchType::JPOS,  true, "CC(plus), float");
    TEST_PRINT("JOF,f",   BranchType::JOF,   true, "CC(overflow), float");
    TEST_PRINT("JNOF,f",  BranchType::JNOF,  true, "CC(no overflow), float");
    TEST_PRINT("JPAR,f",  BranchType::JPAR,  true, "CC(ev parity), float");
    TEST_PRINT("JNPAR,f", BranchType::JNPAR, true, "CC(odd parity), float");
}


QTEST_GUILESS_MAIN(BoolAssignTest)
