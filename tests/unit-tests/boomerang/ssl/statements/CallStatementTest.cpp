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


#include "boomerang/core/Settings.h"
#include "boomerang/db/LowLevelCFG.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/signature/X86Signature.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/util/LocationSet.h"
#include "boomerang/passes/PassManager.h"


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


void CallStatementTest::testNumber()
{
    const SharedExp ebx = Location::regOf(REG_X86_EBX);

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        QCOMPARE(call->getNumber(), 0);

        call->setNumber(1);
        QCOMPARE(call->getNumber(), 1);
    }

    {
        StatementList args;
        args.append(std::make_shared<Assign>(ebx, ebx));

        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->setArguments(args);

        for (SharedStmt arg : call->getArguments()) {
            QCOMPARE(arg->getNumber(), call->getNumber());
        }

        call->setNumber(42);
        QCOMPARE(call->getNumber(), 42);

        for (SharedStmt arg : call->getArguments()) {
            QCOMPARE(arg->getNumber(), call->getNumber());
        }
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


void CallStatementTest::testSimplify()
{
    QSKIP("TODO");
}


void CallStatementTest::testTypeForExp()
{
    QSKIP("TODO");
}


void CallStatementTest::testToString()
{
    Prog prog("test", &m_project);
    UserProc *srcProc = static_cast<UserProc *>(prog.getOrCreateFunction(Address(0x1000)));
    LibProc *destProc = prog.getOrCreateLibraryProc("destProc");
    destProc->setSignature(Signature::instantiate(Machine::X86, CallConv::C, "destProc"));

    std::shared_ptr<CallStatement> call(new CallStatement(Address(0x2000)));
    call->setProc(srcProc);

    QCOMPARE(call->toString(),
             "   0 <all> := CALL 0x00002000(<all>)\n"
             "              Reaching definitions: <None>\n"
             "              Live variables: <None>");

    call->setDest(Location::regOf(REG_X86_ECX));

    QCOMPARE(call->toString(),
             "   0 <all> := CALL r25(<all>)\n"
             "              Reaching definitions: <None>\n"
             "              Live variables: <None>");

    call->setDestProc(destProc);

    QCOMPARE(call->toString(),
             "   0 CALL destProc(\n"
             "              )\n"
             "              Reaching definitions: <None>\n"
             "              Live variables: <None>");

    call->addDefine(std::make_shared<ImplicitAssign>(Location::regOf(REG_X86_EAX)));

    QCOMPARE(call->toString(),
             "   0 { *v* r24 } := CALL destProc(\n"
             "              )\n"
             "              Reaching definitions: <None>\n"
             "              Live variables: <None>");

    call->addDefine(std::make_shared<ImplicitAssign>(Location::regOf(REG_X86_ECX)));

    QCOMPARE(call->toString(),
             "   0 { *v* r24, *v* r25 } := CALL destProc(\n"
             "              )\n"
             "              Reaching definitions: <None>\n"
             "              Live variables: <None>");

    StatementList args;
    args.append(std::make_shared<Assign>(Location::regOf(REG_X86_ECX), Location::regOf(REG_X86_ECX)));

    call->setArguments(StatementList({ args }));

    QCOMPARE(call->toString(),
             "   0 { *v* r24, *v* r25 } := CALL destProc(\n"
             "                *v* r25 := r25\n"
             "              )\n"
             "              Reaching definitions: <None>\n"
             "              Live variables: <None>");
}


void CallStatementTest::testArguments()
{
    QSKIP("TODO");
}


void CallStatementTest::testSetSigArguments()
{
    QSKIP("TODO");
}


void CallStatementTest::testUpdateArguments()
{
    QSKIP("TODO");
}


void CallStatementTest::testArgumentExp()
{
    QSKIP("TODO");
}


void CallStatementTest::testNumArguments()
{
    QSKIP("TODO");
}


void CallStatementTest::testRemoveArguments()
{
    QSKIP("TODO");
}


void CallStatementTest::testArgumentType()
{
    QSKIP("TODO");
}


void CallStatementTest::testEliminateDuplicateArgs()
{
    const SharedExp ebx = Location::regOf(REG_X86_EBX);
    const SharedExp ecx = Location::regOf(REG_X86_ECX);

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));

        call->eliminateDuplicateArgs();
        QCOMPARE(call->getNumArguments(), 0);
    }

    {
        StatementList args;
        args.append(std::make_shared<Assign>(ebx, ebx));

        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->setArguments(args);

        call->eliminateDuplicateArgs();

        QCOMPARE(call->getNumArguments(), 1);
        QCOMPARE(call->getArguments().toString(), "   0 *v* r27 := r27");
    }

    {
        StatementList args;
        args.append(std::make_shared<Assign>(ebx, ebx));
        args.append(std::make_shared<Assign>(ebx, ebx));

        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->setArguments(args);

        call->eliminateDuplicateArgs();

        QCOMPARE(call->getNumArguments(), 1);
        QCOMPARE(call->getArguments().toString(), "   0 *v* r27 := r27");
    }

    {
        StatementList args;
        args.append(std::make_shared<Assign>(ebx, ebx));
        args.append(std::make_shared<Assign>(ebx, ecx));

        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->setArguments(args);

        call->eliminateDuplicateArgs();

        QCOMPARE(call->getNumArguments(), 1);
        QCOMPARE(call->getArguments().toString(), "   0 *v* r27 := r27");
    }

    {
        StatementList args;
        args.append(std::make_shared<Assign>(ebx, ebx));
        args.append(std::make_shared<Assign>(ecx, ebx));

        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->setArguments(args);

        call->eliminateDuplicateArgs();

        QCOMPARE(call->getNumArguments(), 2);
        QCOMPARE(call->getArguments().toString(), "   0 *v* r27 := r27,\t   0 *v* r25 := r27");
    }
}


void CallStatementTest::testDestProc()
{
    QSKIP("TODO");
}


void CallStatementTest::testReturnAfterCall()
{
    QSKIP("TODO");
}

void CallStatementTest::testIsChildless()
{
    QSKIP("TODO");
}


void CallStatementTest::testIsCallToMemOffset()
{
    QSKIP("TODO");
}


void CallStatementTest::testAddDefine()
{
    QSKIP("TODO");
}


void CallStatementTest::testRemoveDefine()
{
    QSKIP("TODO");
}


void CallStatementTest::testSetDefines()
{
    QSKIP("TODO");
}


void CallStatementTest::testFindDefFor()
{
    QSKIP("TODO");
}


void CallStatementTest::testCalcResults()
{
    QSKIP("TODO");
}


void CallStatementTest::testGetProven()
{
    Prog prog("test", &m_project);
    UserProc *srcProc = static_cast<UserProc *>(prog.getOrCreateFunction(Address(0x1000)));
    LibProc *destProc = prog.getOrCreateLibraryProc("destProc");
    destProc->setSignature(Signature::instantiate(Machine::X86, CallConv::C, "destProc"));

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x2000)));
        QVERIFY(call->getProven(Location::regOf(REG_X86_ECX)) == nullptr);
    }

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x2000)));
        call->setProc(srcProc);
        call->setDestProc(destProc);

        // it's an x86 signature, so ebx is preserved
        const SharedExp proven = call->getProven(Location::regOf(REG_X86_EBX));
        QVERIFY(proven != nullptr);
        QCOMPARE(*proven, *Location::regOf(REG_X86_EBX));
    }
}


void CallStatementTest::testLocaliseExp()
{
    QSKIP("TODO");
}


void CallStatementTest::testLocaliseComp()
{
    QSKIP("TODO");
}


void CallStatementTest::testBypassRef()
{
    QSKIP("TODO");
}


void CallStatementTest::testDoEllipsisProcessing()
{
    QSKIP("TODO");
}


void CallStatementTest::testTryConvertToDirect()
{
    QSKIP("TODO");
}


QTEST_GUILESS_MAIN(CallStatementTest)
