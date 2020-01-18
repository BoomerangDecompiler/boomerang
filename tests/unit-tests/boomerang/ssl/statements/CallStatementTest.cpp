#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CallStatementTest.h"


#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/util/LocationSet.h"


void CallStatementTest::testClone()
{
    {
        SharedExp ecx = Location::regOf(REG_X86_ECX);

        std::shared_ptr<CallStatement> call(new CallStatement(ecx));
        SharedStmt clone = call->clone();

        QVERIFY(&(*clone) != &(*call));
        QVERIFY(clone->isCall());
        QVERIFY(clone->getID() != (uint32)-1);
        QVERIFY(clone->getID() != call->getID());

        std::shared_ptr<CallStatement> callClone = clone->as<CallStatement>();

        QVERIFY(callClone->getDest() != nullptr);
        QCOMPARE(*callClone->getDest(), *call->getDest());
        QVERIFY(callClone->isComputed());

        QCOMPARE(callClone->getDestProc(), nullptr);
        QCOMPARE(callClone->isReturnAfterCall(), false);
        QCOMPARE(callClone->getSignature(), nullptr);
    }
}


void CallStatementTest::testGetDefinitions()
{
    {
        LocationSet defs;
        std::shared_ptr<CallStatement> call(new CallStatement(Location::regOf(REG_X86_ECX)));
        QVERIFY(call->isChildless());
        QVERIFY(call->getDefines().empty());

        call->getDefinitions(defs, true);
        QCOMPARE(defs.toString(), "");

        defs.clear();
        call->getDefinitions(defs, false);
        QCOMPARE(defs.toString(), "<all>");
    }

    {
        LocationSet defs;
        StatementList callDefines;

        std::shared_ptr<CallStatement> call(new CallStatement(Location::regOf(REG_X86_ECX)));
        callDefines.append(std::make_shared<ImplicitAssign>(Location::regOf(REG_X86_EAX)));
        call->setDefines(callDefines);

        call->getDefinitions(defs, true);
        QCOMPARE(defs.toString(), "r24");
    }
}


void CallStatementTest::testDefinesLoc()
{
    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->getDefines().append(std::make_shared<Assign>(Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));

        QVERIFY(!call->definesLoc(Location::regOf(REG_X86_ECX)));
        QVERIFY( call->definesLoc(Location::regOf(REG_X86_EAX)));
        QVERIFY(!call->definesLoc(Const::get(0x1000)));
    }
}


void CallStatementTest::testSearch()
{
    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));

        SharedExp result;
        QVERIFY(!call->search(*Const::get(0), result));
    }

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        SharedExp result;
        QVERIFY(call->search(*Const::get(0x1000), result));
        QVERIFY(result != nullptr);
        QCOMPARE(*result, *Const::get(0x1000));
    }

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->getDefines().append(std::make_shared<Assign>(Location::regOf(REG_X86_ECX), Const::get(0)));

        SharedExp result;
        QVERIFY(call->search(*Location::regOf(REG_X86_ECX), result));
        QVERIFY(result != nullptr);
        QCOMPARE(*result, *Location::regOf(REG_X86_ECX));
    }

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->getArguments().append(std::make_shared<Assign>(Location::regOf(REG_X86_ECX), Const::get(0)));

        SharedExp result;
        QVERIFY(call->search(*Location::regOf(REG_X86_ECX), result));
        QVERIFY(result != nullptr);
        QCOMPARE(*result, *Location::regOf(REG_X86_ECX));
    }
}


void CallStatementTest::testSearchAll()
{
    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));

        std::list<SharedExp> result;
        QVERIFY(!call->searchAll(*Const::get(0), result));
        QVERIFY(result.empty());
    }

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));

        std::list<SharedExp> result;
        QVERIFY(call->searchAll(*Const::get(0x1000), result));
        QCOMPARE(result, { call->getDest() });
    }

    {
        SharedExp ecx = Location::regOf(REG_X86_ECX);
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->getDefines().append(std::make_shared<Assign>(ecx, Const::get(0)));

        std::list<SharedExp> result;
        QVERIFY(call->searchAll(*Location::regOf(REG_X86_ECX), result));
        QCOMPARE(result, { ecx });
    }

    {
        SharedExp ecx = Location::regOf(REG_X86_ECX);
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->getArguments().append(std::make_shared<Assign>(ecx, Const::get(0)));

        std::list<SharedExp> result;
        QVERIFY(call->searchAll(*Location::regOf(REG_X86_ECX), result));
        QCOMPARE(result, { ecx });
    }
}


void CallStatementTest::testSearchAndReplace()
{
    QSKIP("TODO");
}


QTEST_GUILESS_MAIN(CallStatementTest)
