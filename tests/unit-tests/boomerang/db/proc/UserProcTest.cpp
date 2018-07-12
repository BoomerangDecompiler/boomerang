#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UserProcTest.h"


#include "boomerang/db/CFG.h"
#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/VoidType.h"


void UserProcTest::testRemoveStatement()
{
    UserProc proc(Address::INVALID, "test", nullptr);

    Assign *asgn = new Assign(VoidType::get(), Location::regOf(REG_PENT_EAX), Location::regOf(REG_PENT_ECX));

    QVERIFY(!proc.removeStatement(nullptr));
    QVERIFY(!proc.removeStatement(asgn));

    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x00000123), { asgn })));
    proc.getCFG()->createBB(BBType::Fall, std::move(bbRTLs));
    QVERIFY(proc.removeStatement(asgn));

    // todo: test that proven true cache is updated
}


void UserProcTest::testInsertAssignAfter()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { })));
    BasicBlock *entryBB = proc.getCFG()->createBB(BBType::Fall, std::move(bbRTLs));
    proc.setEntryBB();

    Assign *as = proc.insertAssignAfter(nullptr, Location::regOf(REG_PENT_EAX), Location::regOf(REG_PENT_ECX));
    QVERIFY(as != nullptr);
    QVERIFY(as->getProc() == &proc);
    QVERIFY(as->getBB() == entryBB);

    QVERIFY(proc.getEntryBB()->getRTLs()->front()->size() == 1);
    QVERIFY(*proc.getEntryBB()->getRTLs()->front()->begin() == as);

    Assign *as2 = proc.insertAssignAfter(as, Location::regOf(REG_PENT_EBX), Location::regOf(REG_PENT_EDX));
    QVERIFY(as2 != nullptr);
    QVERIFY(as->getProc() == &proc);
    QVERIFY(as->getBB() == entryBB);
    QVERIFY(proc.getEntryBB()->getRTLs()->front()->size() == 2);
    QVERIFY(*proc.getEntryBB()->getRTLs()->front()->begin() == as);
    QVERIFY(*std::next(proc.getEntryBB()->getRTLs()->front()->begin()) == as2);
}


void UserProcTest::testInsertStatementAfter()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { })));
    BasicBlock *entryBB = proc.getCFG()->createBB(BBType::Fall, std::move(bbRTLs));
    proc.setEntryBB();

    Assign *as = proc.insertAssignAfter(nullptr, Location::regOf(REG_PENT_EAX), Location::regOf(REG_PENT_ECX));
    Assign *as2 = new Assign(VoidType::get(), Location::regOf(REG_PENT_EDX), Location::regOf(REG_PENT_EBX));

    proc.insertStatementAfter(as, as2);
    QVERIFY(as2->getBB() == entryBB);
    QVERIFY(proc.getEntryBB()->getRTLs()->front()->size() == 2);
    QVERIFY(*proc.getEntryBB()->getRTLs()->front()->begin() == as);
    QVERIFY(*std::next(proc.getEntryBB()->getRTLs()->front()->begin()) == as2);
}


void UserProcTest::testAddParameterToSignature()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    proc.addParameterToSignature(Location::memOf(Binary::get(opPlus,
        Location::regOf(REG_PENT_ESP), Const::get(4)), &proc),
        VoidType::get());

    QCOMPARE(proc.getSignature()->getNumParams(), 1);

    // try to add the same parameter again
    proc.addParameterToSignature(Location::memOf(Binary::get(opPlus,
        Location::regOf(REG_PENT_ESP), Const::get(4)), &proc),
        VoidType::get());

    QCOMPARE(proc.getSignature()->getNumParams(), 1);
}


void UserProcTest::testInsertParameter()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    proc.insertParameter(Location::memOf(Binary::get(opPlus,
        Location::regOf(REG_PENT_ESP), Const::get(4)), &proc),
        VoidType::get());

    QCOMPARE(proc.getParameters().size(), (size_t)1);
    QCOMPARE(proc.getSignature()->getNumParams(), 1);

    // try to add the same parameter again
    proc.insertParameter(Location::memOf(Binary::get(opPlus,
        Location::regOf(REG_PENT_ESP), Const::get(4)), &proc),
        VoidType::get());

    QCOMPARE(proc.getParameters().size(), (size_t)1);
    QCOMPARE(proc.getSignature()->getNumParams(), 1);
}


void UserProcTest::testParamType()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    QVERIFY(proc.getParamType("invalidParam") == nullptr);

    proc.insertParameter(Location::memOf(Binary::get(opPlus,
        Location::regOf(REG_PENT_ESP), Const::get(4)), &proc),
        VoidType::get());

    SharedConstType ty = proc.getParamType("param1");
    QVERIFY(ty != nullptr);
    QCOMPARE(ty->toString(), VoidType::get()->toString());

    proc.setParamType("param1", IntegerType::get(32, Sign::Signed));
    QCOMPARE(proc.getParamType("param1")->toString(), IntegerType::get(32, Sign::Signed)->toString());

    proc.setParamType(5, VoidType::get()); // proc only has 1 parameter
    QCOMPARE(proc.getParamType("param1")->toString(), IntegerType::get(32, Sign::Signed)->toString());

    proc.setParamType(0, VoidType::get());
    QCOMPARE(proc.getParamType("param1")->toString(), VoidType::get()->toString());
}



QTEST_GUILESS_MAIN(UserProcTest)
