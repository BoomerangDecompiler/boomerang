#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "LibProcTest.h"


#include "boomerang/db/exp/Location.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/db/signature/PentiumSignature.h"


void LibProcTest::testIsLib()
{
    LibProc proc(Address::INVALID, "print", nullptr);
    QVERIFY(proc.isLib());
}


void LibProcTest::testIsNoReturn()
{
    LibProc proc(Address::INVALID, "abort", nullptr);
    QVERIFY(proc.isNoReturn());
    proc.setName("test");
    QVERIFY(!proc.isNoReturn());
    std::shared_ptr<Signature> sig(new Signature("test"));
    proc.setSignature(sig);
    QVERIFY(!proc.isNoReturn());
}


void LibProcTest::testGetProven()
{
    LibProc proc(Address::INVALID, "test", nullptr);
    QVERIFY(proc.getProven(nullptr) == nullptr);
    QVERIFY(proc.getProven(Location::regOf(REG_PENT_EBX)) == nullptr);

    proc.setSignature(std::make_shared<CallingConvention::StdC::PentiumSignature>("test"));
    QVERIFY(proc.getProven(Location::regOf(REG_PENT_EBX)) != nullptr);
    QCOMPARE(proc.getProven(Location::regOf(REG_PENT_EBX))->toString(),
             Location::regOf(REG_PENT_EBX)->toString());
}


void LibProcTest::testGetPremised()
{
    LibProc proc(Address::INVALID, "test", nullptr);
    QVERIFY(proc.getPremised(nullptr) == nullptr);
    QVERIFY(proc.getPremised(Location::regOf(REG_PENT_EBX)) == nullptr);
}


void LibProcTest::testIsPreserved()
{
    LibProc proc(Address::INVALID, "test", nullptr);
    QVERIFY(!proc.isPreserved(nullptr));
    QVERIFY(!proc.isPreserved(Location::regOf(REG_PENT_EBX)));

    proc.setSignature(std::make_shared<CallingConvention::StdC::PentiumSignature>("test"));
    QVERIFY(!proc.isPreserved(nullptr));
    QVERIFY(!proc.isPreserved(Location::regOf(REG_PENT_EAX)));
    QVERIFY(proc.isPreserved(Location::regOf(REG_PENT_EBX)));
}

QTEST_GUILESS_MAIN(LibProcTest)
