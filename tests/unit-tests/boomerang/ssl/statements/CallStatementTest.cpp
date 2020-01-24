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
#include "boomerang/passes/PassManager.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/util/LocationSet.h"


void CallStatementTest::testClone()
{
    SharedExp ecx = Location::regOf(REG_X86_ECX);

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        SharedStmt clone = call->clone();

        QVERIFY(&(*clone) != &(*call));
        QVERIFY(clone->isCall());
        QVERIFY(clone->getID() != (uint32)-1);
        QVERIFY(clone->getID() != call->getID());

        std::shared_ptr<CallStatement> callClone = clone->as<CallStatement>();

        QVERIFY(callClone->getDest() != nullptr);
        QCOMPARE(*callClone->getDest(), *call->getDest());
        QVERIFY(!callClone->isComputed());

        QCOMPARE(callClone->getDestProc(), nullptr);
        QCOMPARE(callClone->isReturnAfterCall(), false);
        QCOMPARE(callClone->getSignature(), nullptr);
    }

    {
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
    const SharedExp ecx = Location::regOf(REG_X86_ECX);

    {
        std::shared_ptr<CallStatement> call(new CallStatement(
            Binary::get(opPlus, Const::get(0x800), Const::get(0x800))));
        QVERIFY(call->isComputed());

        call->simplify();

        QVERIFY(call->getDest() != nullptr);
        QCOMPARE(*call->getDest(), *Const::get(0x1000));
        QVERIFY(!call->isComputed());
    }

    {
        StatementList args, defs;
        args.append(std::make_shared<Assign>(ecx, Binary::get(opPlus, Const::get(40), Const::get(2))));
        defs.append(args.front()->clone());

        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->setArguments(args);
        call->setDefines(defs);
        call->simplify();

        QCOMPARE(call->getArguments().toString(), "   0 *v* r25 := 42");
        QCOMPARE(call->getDefines().toString(),   "   0 *v* r25 := 42");
    }
}


void CallStatementTest::testTypeForExp()
{
    const SharedExp ecx = Location::regOf(REG_X86_ECX);

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));

        SharedConstType ty = call->getTypeForExp(ecx);
        QVERIFY(ty != nullptr);
        QCOMPARE(*ty, *VoidType::get());
    }

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));

        SharedConstType ty = call->getTypeForExp(Terminal::get(opPC));
        QVERIFY(ty != nullptr);
        QCOMPARE(*ty, *PointerType::get(VoidType::get()));
    }

    {
        StatementList defs;
        defs.append(std::make_shared<Assign>(IntegerType::get(32, Sign::Signed), ecx, Const::get(0)));

        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->setDefines(defs);

        SharedConstType ty = call->getTypeForExp(Const::get(0));
        QVERIFY(ty != nullptr);
        QCOMPARE(*ty, *VoidType::get());

        ty = call->getTypeForExp(ecx->clone()); // verify it is not comparing by address
        QVERIFY(ty != nullptr);
        QCOMPARE(*ty, *IntegerType::get(32, Sign::Signed));
    }

    QSKIP("TODO: setTypeForExp");
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
    Prog prog("test", &m_project);
    UserProc *srcProc = static_cast<UserProc *>(prog.getOrCreateFunction(Address(0x1000)));

    const SharedExp ecx = Location::regOf(REG_X86_ECX);

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x2000)));
        QVERIFY(call->getArguments().empty());
    }

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x2000)));
        call->setArguments(StatementList());
        QVERIFY(call->getArguments().empty());
    }

    {
        StatementList args;
        args.append(std::make_shared<Assign>(ecx, ecx));
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x2000)));

        call->setArguments(args);

        QVERIFY(!call->getArguments().empty());
        QCOMPARE(call->getArguments().toString(), "   0 *v* r25 := r25");

        QCOMPARE(call->getArguments().front()->getProc(), call->getProc());
        QCOMPARE(call->getArguments().front()->getNumber(), call->getNumber());
        QCOMPARE(call->getArguments().front()->getFragment(), call->getFragment());
    }

    {
        BasicBlock *bb = prog.getCFG()->createBB(BBType::Fall, createInsns(Address(0x1000), 1));
        IRFragment *frag = srcProc->getCFG()->createFragment(FragType::Fall, createRTLs(Address(0x1000), 1, 1), bb);

        StatementList args;
        args.append(std::make_shared<Assign>(ecx, ecx));
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x2000)));
        call->setFragment(frag);
        call->setProc(srcProc);
        call->setNumber(42);

        call->setArguments(args);

        QVERIFY(!call->getArguments().empty());
        QCOMPARE(call->getArguments().toString(), "  42 *v* r25 := r25");

        QCOMPARE(call->getArguments().front()->getProc(), call->getProc());
        QCOMPARE(call->getArguments().front()->getNumber(), call->getNumber());
        QCOMPARE(call->getArguments().front()->getFragment(), call->getFragment());
    }
}


void CallStatementTest::testSetSigArguments()
{
    const SharedExp ecx = Location::regOf(REG_X86_ECX);

    Prog prog("test", &m_project);
    UserProc *srcProc = static_cast<UserProc *>(prog.getOrCreateFunction(Address(0x1000)));

    LibProc *destLibProc = prog.getOrCreateLibraryProc("desLibProc");
    destLibProc->setSignature(Signature::instantiate(Machine::X86, CallConv::C, "destLibProc"));
    destLibProc->getSignature()->addParameter("param0", ecx, IntegerType::get(32, Sign::Signed));

    UserProc *destUserProc = static_cast<UserProc *>(prog.getOrCreateFunction(Address(0x2000)));
    destUserProc->setSignature(Signature::instantiate(Machine::X86, CallConv::C, "destUserProc"));

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));

        call->setSigArguments();

        QVERIFY(call->getArguments().empty());
    }

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->setSignature(Signature::instantiate(Machine::X86, CallConv::C, "test"));
        call->setSigArguments();
        QVERIFY(call->getArguments().empty());
    }

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->setProc(srcProc);
        call->setDestProc(destUserProc);

        call->setSigArguments();

        QCOMPARE(destUserProc->getCallers().size(), 1);
        QCOMPARE(*destUserProc->getCallers().begin(), call);

        QVERIFY(call->getSignature() != nullptr);
        QCOMPARE(*call->getSignature(), *destUserProc->getSignature());
        QVERIFY(call->getArguments().empty());
    }

    {
        BasicBlock *bb = prog.getCFG()->createBB(BBType::Fall, createInsns(Address(0x1000), 1));
        IRFragment *frag = srcProc->getCFG()->createFragment(FragType::Fall, createRTLs(Address(0x1000), 1, 1), bb);

        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->setProc(srcProc);
        call->setDestProc(destLibProc);
        call->setNumber(42);
        call->setFragment(frag);

        call->setSigArguments();

        QCOMPARE(destLibProc->getCallers().size(), 1);
        QCOMPARE(*destLibProc->getCallers().begin(), call);

        QVERIFY(call->getSignature() != nullptr);
        QCOMPARE(*call->getSignature(), *destLibProc->getSignature());

        QVERIFY(call->getArguments().size() == 1);
        QCOMPARE(call->getArguments().front()->getProc(), srcProc);
        QCOMPARE(call->getArguments().front()->getFragment(), frag);
        QCOMPARE(call->getArguments().front()->toString(), "  42 *i32* r25 := r25");
    }
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
    const SharedExp ecx = Location::regOf(REG_X86_ECX);

    std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
    QCOMPARE(call->getNumArguments(), 0);

    call->getArguments().append(std::make_shared<Assign>(ecx, ecx));
    QCOMPARE(call->getNumArguments(), 1);

    QSKIP("TODO: setNumArguments");
}


void CallStatementTest::testRemoveArgument()
{
    const SharedExp ecx = Location::regOf(REG_X86_ECX);

    std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
    call->getArguments().append(std::make_shared<Assign>(ecx, ecx));
    QCOMPARE(call->getNumArguments(), 1);

    call->removeArgument(0);

    QCOMPARE(call->getNumArguments(), 0);
}


void CallStatementTest::testArgumentType()
{
    const SharedExp ecx = Location::regOf(REG_X86_ECX);

    std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
    call->getArguments().append(std::make_shared<Assign>(ecx, ecx));

    SharedType ty = call->getArgumentType(0);
    QVERIFY(ty != nullptr);
    QCOMPARE(*ty, *VoidType::get());

    call->setArgumentType(0, IntegerType::get(32, Sign::Signed));

    ty = call->getArgumentType(0);
    QVERIFY(ty != nullptr);
    QCOMPARE(*ty, *IntegerType::get(32, Sign::Signed));
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
    Prog prog("test", &m_project);
    LibProc *destProc = prog.getOrCreateLibraryProc("destProc");

    std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
    QCOMPARE(call->getDestProc(), nullptr);

    call->setDestProc(destProc);

    QCOMPARE(call->getDestProc(), destProc);
}


void CallStatementTest::testReturnAfterCall()
{
    std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
    QVERIFY(!call->isReturnAfterCall());
    call->setReturnAfterCall(false);
    QVERIFY(!call->isReturnAfterCall());
    call->setReturnAfterCall(true);
    QVERIFY(call->isReturnAfterCall());
}


void CallStatementTest::testIsChildless()
{
    Prog prog("test", &m_project);
    LibProc *destProc = prog.getOrCreateLibraryProc("destProc");

    UserProc *destUserProc = static_cast<UserProc *>(prog.getOrCreateFunction(Address(0x1000)));

    BasicBlock *bb = prog.getCFG()->createBB(BBType::Ret, createInsns(Address(0x1000), 1));
    IRFragment *frag = destUserProc->getCFG()->createFragment(FragType::Ret, createRTLs(Address(0x1000), 1, 0), bb);
    std::shared_ptr<ReturnStatement> calleeRet(new ReturnStatement);
    frag->getRTLs()->back()->append(calleeRet);

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        QVERIFY(call->isChildless());
    }

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->setDestProc(destProc);
        QVERIFY(!call->isChildless());
    }

    {
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->setDestProc(destUserProc);
        QVERIFY(call->isChildless());
    }

    {
        destUserProc->setStatus(ProcStatus::FinalDone);
        std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
        call->setDestProc(destUserProc);
        call->setCalleeReturn(calleeRet);

        QVERIFY(!call->isChildless());
    }
}


void CallStatementTest::testIsCallToMemOffset()
{
    std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
    QVERIFY(!call->isCallToMemOffset());

    call->setDest(Location::regOf(REG_X86_ECX));
    QVERIFY(!call->isCallToMemOffset());

    call->setDest(Address(0x2000));
    QVERIFY(!call->isCallToMemOffset());

    call->setDest(Location::memOf(Location::regOf(REG_X86_ECX)));
    QVERIFY(!call->isCallToMemOffset());

    call->setDest(Location::memOf(Const::get(0x2000)));
    QVERIFY(call->isCallToMemOffset());
}


void CallStatementTest::testAddDefine()
{
    const SharedExp ecx = Location::regOf(REG_X86_ECX);

    std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
    QVERIFY(call->getDefines().empty());

    call->addDefine(std::make_shared<ImplicitAssign>(ecx));
    QVERIFY(call->getDefines().size() == 1);
    QCOMPARE(call->getDefines().toString(), "   0 *v* r25 := -");
}


void CallStatementTest::testRemoveDefine()
{
    const SharedExp eax = Location::regOf(REG_X86_EAX);
    const SharedExp ecx = Location::regOf(REG_X86_ECX);

    std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
    QVERIFY(!call->removeDefine(ecx));

    call->addDefine(std::make_shared<ImplicitAssign>(ecx));
    QVERIFY(!call->removeDefine(eax));
    QCOMPARE(call->getDefines().toString(), "   0 *v* r25 := -");

    call->addDefine(std::make_shared<ImplicitAssign>(ecx));
    QVERIFY(call->removeDefine(ecx));
    QCOMPARE(call->getDefines().toString(), "   0 *v* r25 := -");

    QVERIFY(call->removeDefine(ecx));
    QCOMPARE(call->getDefines().toString(), "");
}


void CallStatementTest::testSetDefines()
{
    const SharedExp eax = Location::regOf(REG_X86_EAX);
    const SharedExp ecx = Location::regOf(REG_X86_ECX);

    std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));

    {
        StatementList defs;
        call->setDefines(defs);
        QVERIFY(call->getDefines().empty());
    }

    {
        StatementList defs;
        defs.append(std::make_shared<ImplicitAssign>(ecx));
        call->setDefines(defs);
        QVERIFY(!call->getDefines().empty());
        QCOMPARE(call->getDefines().toString(), "   0 *v* r25 := -");
    }

    {
        StatementList defs;
        call->setDefines(defs);
        QVERIFY(call->getDefines().empty());
    }
}


void CallStatementTest::testFindDefFor()
{
    const SharedExp eax = Location::regOf(REG_X86_EAX);
    const SharedExp ecx = Location::regOf(REG_X86_ECX);

    std::shared_ptr<CallStatement> call(new CallStatement(Address(0x1000)));
    DefCollector *defCol = call->getDefCollector();

    QVERIFY(call->findDefFor(ecx) == nullptr);

    defCol->collectDef(std::make_shared<Assign>(ecx, eax));
    QVERIFY(call->findDefFor(ecx) != nullptr);
    QCOMPARE(*call->findDefFor(ecx), *eax);
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
