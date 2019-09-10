#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "RegDBTest.h"

#include "boomerang/ssl/RegDB.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Ternary.h"

void RegDBTest::testClear()
{
    RegDB db;
    db.clear(); // Verify it does not crash

    QVERIFY(db.createReg(RegType::Int, REG_PENT_AX, "%ax", 16));
    db.clear();
    QVERIFY(!db.isRegDefined("%ax"));
    QVERIFY(!db.isRegNumDefined(REG_PENT_AX));
}


void RegDBTest::testIsRegDefined()
{
    RegDB db;
    QVERIFY(!db.isRegDefined("%ax"));

    QVERIFY(db.createReg(RegType::Int, REG_PENT_AX, "%ax", 16));
    QVERIFY(db.isRegDefined("%ax"));
    QVERIFY(!db.isRegDefined("%foo"));
}


void RegDBTest::testIsRegNumDefined()
{
    RegDB db;
    QVERIFY(!db.isRegNumDefined(REG_PENT_AX));

    QVERIFY(db.createReg(RegType::Int, REG_PENT_AX, "%ax", 16));
    QVERIFY(db.isRegNumDefined(REG_PENT_AX));
    QVERIFY(!db.isRegNumDefined(REG_PENT_SS));
}


void RegDBTest::testGetRegByNum()
{
    RegDB db;
    QVERIFY(db.getRegByNum(REG_PENT_AX) == nullptr);

    QVERIFY(db.createReg(RegType::Int, REG_PENT_AX, "%ax", 16));
    const Register *reg = db.getRegByNum(REG_PENT_AX);
    QVERIFY(reg != nullptr);
    QVERIFY(reg->getName() == "%ax");
    QVERIFY(reg->getRegType() == RegType::Int);
    QVERIFY(reg->getSize() == 16);

    QVERIFY(db.getRegByNum(REG_PENT_SS) == nullptr);
}


void RegDBTest::testGetRegByName()
{
    RegDB db;
    QVERIFY(db.getRegByName("%ax") == nullptr);

    QVERIFY(db.createReg(RegType::Int, REG_PENT_AX, "%ax", 16));
    const Register *reg = db.getRegByName("%ax");
    QVERIFY(reg != nullptr);
    QVERIFY(reg->getName() == "%ax");
    QVERIFY(reg->getRegType() == RegType::Int);
    QVERIFY(reg->getSize() == 16);

    QVERIFY(db.getRegByName("%foo") == nullptr);
}


void RegDBTest::testGetRegNumByName()
{
    RegDB db;

    QVERIFY(db.getRegNumByName("%ax") == RegNumSpecial); // not found

    QVERIFY(db.createReg(RegType::Int, REG_PENT_AX, "%ax", 16));
    QVERIFY(db.getRegNumByName("%ax") == REG_PENT_AX);

    QVERIFY(db.createReg(RegType::Int, RegNumSpecial, "%foo", 16));
    QVERIFY(db.getRegNumByName("%foo") == RegNumSpecial);
}


void RegDBTest::testGetRegNameByNum()
{
    RegDB db;
    QVERIFY(db.getRegNameByNum(REG_PENT_AX) == "");

    QVERIFY(db.createReg(RegType::Int, REG_PENT_AX, "%ax", 16));
    QVERIFY(db.getRegNameByNum(REG_PENT_AX) == "%ax");
    QVERIFY(db.getRegNameByNum(REG_PENT_SS) == "");

    QVERIFY(db.getRegNameByNum(RegNumSpecial) == "");
    QVERIFY(db.createReg(RegType::Int, RegNumSpecial, "%bar", 32));
    QVERIFY(db.getRegNameByNum(RegNumSpecial) == "");
}


void RegDBTest::testGetRegSizeByNum()
{
    RegDB db;
    QVERIFY(db.getRegSizeByNum(REG_PENT_AX) == 32);

    QVERIFY(db.createReg(RegType::Int, REG_PENT_AX, "%ax", 16));
    QVERIFY(db.getRegSizeByNum(REG_PENT_AX) == 16);
    QVERIFY(db.getRegSizeByNum(REG_PENT_SS) == 32);

    QVERIFY(db.getRegSizeByNum(RegNumSpecial) == 32);
    QVERIFY(db.createReg(RegType::Int, RegNumSpecial, "%bar", 8));
    QVERIFY(db.getRegSizeByNum(RegNumSpecial) == 32);
}


void RegDBTest::testCreateReg()
{
    RegDB db;
    QVERIFY(!db.createReg(RegType::Invalid, RegNum(10), "%foo", 10));
    QVERIFY(!db.createReg(RegType::Int, RegNum(10), "", 10));
    QVERIFY(!db.createReg(RegType::Int, RegNum(10), "%foo", 0));
    QVERIFY(!db.createReg(RegType::Int, RegNum(10), "%foo", -1));

    QVERIFY(db.createReg(RegType::Int, RegNum(0), "%foo", 16));
    QVERIFY(!db.createReg(RegType::Int, RegNum(0), "%foo", 16));    // same name -> should fail
    QVERIFY(!db.createReg(RegType::Float, RegNum(0), "%foo2", 16)); // Must be same type
    QVERIFY(!db.createReg(RegType::Int, RegNum(0), "%foo2", 20));   // Must be same size
    QVERIFY(db.createReg(RegType::Int, RegNum(0), "%foo2", 16));    // foo2 is an alias for foo

    QVERIFY(db.createReg(RegType::Float, RegNumSpecial, "%foo3", 16));
    QVERIFY(db.createReg(RegType::Float, RegNumSpecial, "%foo4", 16));
    QVERIFY(db.createReg(RegType::Int, RegNumSpecial, "%foo5", 16));
    QVERIFY(db.createReg(RegType::Float, RegNumSpecial, "%foo6", 20));
    QVERIFY(!db.createReg(RegType::Float, RegNumSpecial, "%foo6", 20)); // same name
}


void RegDBTest::testCreateRegRelation()
{
    RegDB db;

    QVERIFY(!db.createRegRelation("%foo", "%bar", 0));

    QVERIFY(db.createReg(RegType::Int, RegNum(0), "%foo", 32));
    QVERIFY(!db.createRegRelation("%foo", "%bar", 0));
    QVERIFY(!db.createRegRelation("%foo", "%foo", 0));

    QVERIFY(db.createReg(RegType::Int, RegNum(0), "%foo2", 32)); // alias
    QVERIFY(!db.createRegRelation("%foo", "%bar", 0));

    QVERIFY(db.createReg(RegType::Float, RegNum(1), "%bar", 32)); // same size
    QVERIFY( db.createRegRelation("%foo", "%bar", 0));

    QVERIFY(db.createReg(RegType::Float, RegNum(2), "%bar_lo", 16)); // different size
    QVERIFY(db.createReg(RegType::Float, RegNum(3), "%bar_hi", 16)); // different size
    QVERIFY( db.createRegRelation("%bar", "%bar_lo", 0));
    QVERIFY( db.createRegRelation("%bar", "%bar_hi", 16));      // non-zero offset
    QVERIFY(!db.createRegRelation("%bar", "%bar_hi", 16));      // cannot have same relation twice

    QVERIFY(db.createReg(RegType::Int, RegNum(10), "%eip", 32));
    QVERIFY(db.createReg(RegType::Int, RegNum(11), "%ip", 16));
    QVERIFY(db.createReg(RegType::Int, RegNum(-1), "%eip_1", 32));
    QVERIFY(db.createReg(RegType::Int, RegNum(-1), "%ip_1", 16));

    QVERIFY(!db.createRegRelation("%eip_1", "%ip_1", 0));
    QVERIFY(!db.createRegRelation("%eip_1", "%ip", 0));
    QVERIFY( db.createRegRelation("%eip",   "%ip_1", 0));
}


void RegDBTest::testProcessOverlappedRegs()
{
    RegDB db;

    QVERIFY(db.createReg(RegType::Int, REG_PENT_EAX, "%eax", 32));
    QVERIFY(db.createReg(RegType::Int, REG_PENT_AX, "%ax",   16));
    QVERIFY(db.createReg(RegType::Int, REG_PENT_AH, "%ah",    8));
    QVERIFY(db.createReg(RegType::Int, REG_PENT_AL, "%al",    8));
    QVERIFY(db.createRegRelation("%eax", "%ax", 0));
    QVERIFY(db.createRegRelation("%ax",  "%al", 0));
    QVERIFY(db.createRegRelation("%ax",  "%ah", 8));

    QVERIFY(db.createReg(RegType::Int, REG_PENT_EDX, "%edx", 32));
    QVERIFY(db.createReg(RegType::Int, REG_PENT_DX, "%dx",   16));
    QVERIFY(db.createReg(RegType::Int, REG_PENT_DH, "%dh",    8));
    QVERIFY(db.createReg(RegType::Int, REG_PENT_DL, "%dl",    8));
    QVERIFY(db.createRegRelation("%edx", "%dx", 0));
    QVERIFY(db.createRegRelation("%dx",  "%dl", 0));
    QVERIFY(db.createRegRelation("%dx",  "%dh", 8));

    // assign another register
    std::shared_ptr<Assign> as(new Assign(Location::regOf(REG_PENT_AX), Location::regOf(REG_PENT_DX)));
    QCOMPARE(db.processOverlappedRegs(as, { REG_PENT_EAX, REG_PENT_AX, REG_PENT_DX, REG_PENT_AH })->toString(),
             "0x00000000    0 *j32* r24 := (r24 & 0xffffffffffff0000) | zfill(16, 32, r0)\n              0 *j8* r12 := r0@[8:15]\n");

    // No other regs are used -> empty RTL
    QCOMPARE(db.processOverlappedRegs(as, { })->toString(),
             "0x00000000\n");

    // assign const
    as->setRight(Const::get(-5));
    QCOMPARE(db.processOverlappedRegs(as, { REG_PENT_EAX, REG_PENT_AX, REG_PENT_AH, REG_PENT_AL })->toString(),
             "0x00000000    0 *j32* r24 := (r24 & 0xffffffffffff0000) | zfill(16, 32, r0)\n              0 *j8* r12 := r0@[8:15]\n              0 *j8* r8 := r0@[0:7]\n");

    // assign with register offset
    as->setLeft(Location::regOf(REG_PENT_AH));
    as->setRight(Ternary::get(opAt, Location::regOf(REG_PENT_DX), Const::get(5), Const::get(12)));
    QCOMPARE(db.processOverlappedRegs(as, { REG_PENT_AX, REG_PENT_AH, REG_PENT_DX })->toString(),
             "0x00000000    0 *j16* r0 := (r0 & 0xffffffffffff00ff) | (zfill(8, 16, r12) << 8)\n");

    // verify guard expressions are propagated
    as->setGuard(Binary::get(opGtr, Location::regOf(REG_PENT_DX), Const::get(0)));
    QCOMPARE(db.processOverlappedRegs(as, { REG_PENT_AX, REG_PENT_AH, REG_PENT_DX })->toString(),
             "0x00000000    0 *j16* r2 > 0 => r0 := (r0 & 0xffffffffffff00ff) | (zfill(8, 16, r12) << 8)\n");

    // lhs not a constant register -> fail
    as->setLeft(Location::regOf(Location::regOf(REG_PENT_AX)));
    QCOMPARE(db.processOverlappedRegs(as, { REG_PENT_AX, REG_PENT_AH, REG_PENT_DX }), nullptr);

    // special register
    as->setLeft(Location::regOf(RegNumSpecial));
    QCOMPARE(db.processOverlappedRegs(as, { REG_PENT_AX, REG_PENT_AH, REG_PENT_DX }), nullptr);

    // nonexistent register
    as->setLeft(Location::regOf(REG_PENT_ESI));
    QCOMPARE(db.processOverlappedRegs(as, { REG_PENT_AX, REG_PENT_AH, REG_PENT_DX }), nullptr);
}


QTEST_GUILESS_MAIN(RegDBTest)
