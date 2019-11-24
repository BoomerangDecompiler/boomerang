#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BasicBlockTest.h"

#include "boomerang/db/BasicBlock.h"


static std::vector<MachineInstruction> createInsns(Address startAddr, std::size_t count)
{
    std::vector<MachineInstruction> result(count);

    int i = 0;
    for (MachineInstruction &insn : result) {
        insn.m_addr = startAddr + (i++);
        insn.m_size = 1;
    }

    return result;
}


void BasicBlockTest::testType()
{
    {
        BasicBlock bb(Address(0x1000));
        QCOMPARE(bb.getType(), BBType::Invalid);
        QVERIFY(bb.isType(BBType::Invalid));

        bb.setType(BBType::Oneway);
        QVERIFY(bb.isType(BBType::Oneway));
    }

    {
        BasicBlock bb(BBType::Twoway, createInsns(Address(0x1000), 1));

        QVERIFY(bb.isType(BBType::Twoway));
        QCOMPARE(bb.getType(), BBType::Twoway);
    }
}


void BasicBlockTest::testAddress()
{
    {
        BasicBlock bb(Address(0x1000));
        QCOMPARE(bb.getLowAddr(), Address(0x1000));
        QCOMPARE(bb.getHiAddr(), Address::INVALID);
    }

    {
        BasicBlock bb(BBType::Twoway, createInsns(Address(0x1000), 1));

        QCOMPARE(bb.getLowAddr(), Address(0x1000));
        QCOMPARE(bb.getHiAddr(), Address(0x1001));
    }
}


void BasicBlockTest::testIsComplete()
{
    {
        BasicBlock bb(Address(0x1000));
        QVERIFY(!bb.isComplete());
    }

    {
        BasicBlock bb(BBType::Twoway, createInsns(Address(0x1000), 1));
        QVERIFY(bb.isComplete());
    }
}


void BasicBlockTest::testCompleteBB()
{
    BasicBlock bb(Address(0x1000));
    QCOMPARE(bb.getLowAddr(), Address(0x1000));
    QVERIFY(!bb.isComplete());

    bb.completeBB(createInsns(Address(0x2000), 1));
    QVERIFY(bb.isComplete());
    QCOMPARE(bb.getLowAddr(), Address(0x2000));
    QCOMPARE(bb.getHiAddr(), Address(0x2001));
}


QTEST_GUILESS_MAIN(BasicBlockTest)
