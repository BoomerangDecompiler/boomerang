#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "GlobalTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/Global.h"
#include "boomerang/db/Prog.h"
#include "boomerang/type/type/CompoundType.h"
#include "boomerang/type/type/CharType.h"
#include "boomerang/type/type/ArrayType.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/FloatType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/SizeType.h"


#define SAMPLE(path)    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/" path))

#define HELLO_PENTIUM   SAMPLE("pentium/hello")
#define FBRANCH_PENTIUM SAMPLE("pentium/fbranch")


void GlobalTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");

    m_project.loadPlugins();
}


void GlobalTest::testContainsAddress()
{
    Global glob1(VoidType::get(), Address(0x1000), "", nullptr);
    QVERIFY(!glob1.containsAddress(Address(0xFFF)));
    QVERIFY(glob1.containsAddress(Address(0x1000)));
    QVERIFY(!glob1.containsAddress(Address(0x1001)));

    Global glob2(IntegerType::get(32), Address(0x1000), "", nullptr);
    QVERIFY(!glob2.containsAddress(Address(0xFFF)));
    QVERIFY(glob2.containsAddress(Address(0x1001)));
    QVERIFY(!glob2.containsAddress(Address(0x1004)));
}


void GlobalTest::testGetInitialValue()
{
    QVERIFY(m_project.loadBinaryFile(FBRANCH_PENTIUM));
    Global *bssGlob = m_project.getProg()->createGlobal(Address(0x080496DC));
    QVERIFY(bssGlob != nullptr);
    QVERIFY(bssGlob->getInitialValue() == nullptr);
}


void GlobalTest::testReadInitialValue()
{
    {
        QVERIFY(m_project.loadBinaryFile(FBRANCH_PENTIUM));
        Prog *prog = m_project.getProg();

        Global *nullptrGlob = prog->createGlobal(Address(0x0804830A), PointerType::get(VoidType::get()));
        QVERIFY(nullptrGlob != nullptr);
        QVERIFY(nullptrGlob->getInitialValue() != nullptr);
        QCOMPARE(nullptrGlob->getInitialValue()->toString(), Const::get(0)->toString());

        Global *stringGlob = prog->createGlobal(Address(0x8048440), PointerType::get(CharType::get()));
        QVERIFY(stringGlob->getInitialValue() != nullptr);
        QCOMPARE(stringGlob->getInitialValue()->toString(), QString("\"Less\""));

        Global *stringGlob2 = prog->createGlobal(Address(0x08048583), ArrayType::get(CharType::get()));
        QVERIFY(stringGlob2->getInitialValue() != nullptr);
        QCOMPARE(stringGlob2->getInitialValue()->toString(), QString("\"a is %f, b is %f\n\""));

        Global *ptrStringGlob2 = prog->createGlobal(Address(0x080483B9), PointerType::get(CharType::get()));
        QVERIFY(ptrStringGlob2->getInitialValue() != nullptr);
        QCOMPARE(ptrStringGlob2->getInitialValue()->toString(), QString("global_0x08048583"));

        // size const
        Global *sizeConst = prog->createGlobal(Address(0x08048390), SizeType::get(8));
        QVERIFY(sizeConst->getInitialValue() != nullptr);
        QCOMPARE(sizeConst->getInitialValue()->toString(), QString("85")); // 0x55

        // int const
        Global *intConst = prog->createGlobal(Address(0x080483B2), IntegerType::get(32, -1));
        QVERIFY(intConst->getInitialValue() != nullptr);
        QCOMPARE(intConst->getInitialValue()->toString(), QString("0x40140000"));

        // float constant
        Global *five = prog->createGlobal(Address(0x080485CC), FloatType::get(32));
        SharedConstExp result = five->getInitialValue();
        QVERIFY(result && result->isFltConst());
        QCOMPARE(result->access<Const>()->getFlt(), 5.0f);

        Global glob1(VoidType::get(), Address::ZERO, "", prog);
        QVERIFY(glob1.getInitialValue() == nullptr);
    }

    {
        QVERIFY(m_project.loadBinaryFile(HELLO_PENTIUM));
        Prog *prog = m_project.getProg();

        // string constant
        Global *hello = prog->createGlobal(Address(0x080483FC), ArrayType::get(CharType::get(), 15));
        SharedExp result = hello->getInitialValue();
        QVERIFY(result != nullptr && result->isStrConst());
        QCOMPARE(result->access<Const>()->getStr(), QString("Hello, world!\n"));

        // integer constant
        Global *zero = prog->createGlobal(Address(0x080483DE), IntegerType::get(32, 1));
        result = zero->getInitialValue();
        QVERIFY(result && result->isIntConst());
        QCOMPARE(result->access<Const>()->getInt(), 0);
    }
}


QTEST_GUILESS_MAIN(GlobalTest)
