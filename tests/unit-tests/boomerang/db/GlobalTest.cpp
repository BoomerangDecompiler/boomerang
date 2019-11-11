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


#include "boomerang/core/Settings.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/db/Global.h"
#include "boomerang/db/Prog.h"
#include "boomerang/ssl/type/CompoundType.h"
#include "boomerang/ssl/type/CharType.h"
#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/VoidType.h"


#define SAMPLE(path)    (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/" path))

#define HELLO_X86    SAMPLE("x86/hello")
#define FBRANCH_X86  SAMPLE("x86/fbranch")
#define SUMARRAY_X86 SAMPLE("x86/sumarray")



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
    QVERIFY(m_project.loadBinaryFile(FBRANCH_X86));
    Global *bssGlob = m_project.getProg()->createGlobal(Address(0x080496DC));
    QVERIFY(bssGlob != nullptr);
    QVERIFY(bssGlob->getInitialValue() == nullptr);
}


void GlobalTest::testReadInitialValue()
{
    {
        QVERIFY(m_project.loadBinaryFile(FBRANCH_X86));
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
        Global *shortConst = prog->createGlobal(Address(0x008048570), IntegerType::get(16, Sign::Signed));
        QVERIFY(shortConst && shortConst->getInitialValue());
        QCOMPARE(shortConst->getInitialValue()->toString(), QString("0xffff"));

        Global *intConst = prog->createGlobal(Address(0x080483B2), IntegerType::get(32, Sign::Unsigned));
        QVERIFY(intConst->getInitialValue() != nullptr);
        QCOMPARE(intConst->getInitialValue()->toString(), QString("0x40140000"));

        Global *qwordConst = prog->createGlobal(Address(0x08048480), IntegerType::get(64));
        QVERIFY(qwordConst && qwordConst->getInitialValue());
        QCOMPARE(qwordConst->getInitialValue()->toString(), QString("0x85b9680cec83d8ddLL"));

        // float constant
        Global *fiveFloat = prog->createGlobal(Address(0x080485CC), FloatType::get(32));
        SharedConstExp result = fiveFloat->getInitialValue();
        QVERIFY(result && result->isFltConst());
        QCOMPARE(result->access<Const>()->getFlt(), 5.0f);

        // double constant
        Global *fiveDouble = prog->createGlobal(Address(0x0804857C), FloatType::get(64));
        QVERIFY(fiveDouble && fiveDouble->getInitialValue());
        QCOMPARE(fiveDouble->getInitialValue()->toString(), QString("1.80121e+159"));

        Global glob1(VoidType::get(), Address::ZERO, "", prog);
        QVERIFY(glob1.getInitialValue() == nullptr);
    }

    {
        QVERIFY(m_project.loadBinaryFile(HELLO_X86));
        Prog *prog = m_project.getProg();

        // string constant
        Global *hello = prog->createGlobal(Address(0x080483FC), ArrayType::get(CharType::get(), 15));
        SharedExp result = hello->getInitialValue();
        QVERIFY(result != nullptr && result->isStrConst());
        QCOMPARE(result->access<Const>()->getStr(), QString("Hello, world!\n"));

        // integer constant
        Global *zero = prog->createGlobal(Address(0x080483DE), IntegerType::get(32, Sign::Signed));
        result = zero->getInitialValue();
        QVERIFY(result && result->isIntConst());
        QCOMPARE(result->access<Const>()->getInt(), 0);
    }

    {
        // arrays
        QVERIFY(m_project.loadBinaryFile(SUMARRAY_X86));
        Prog *prog = m_project.getProg();

        Global *intArrGlob = prog->createGlobal(Address(0x08049460), ArrayType::get(IntegerType::get(32)));
        QVERIFY(intArrGlob != nullptr);

        SharedExp init = intArrGlob->getInitialValue();
        QVERIFY(init != nullptr);
        QCOMPARE(init->toString(), QString("1, 2, 3, 4, 5, 6, 7, 8, 9, 10"));

        // compound type
        auto structTy = CompoundType::get();
        structTy->addMember(IntegerType::get(32), "first");
        structTy->addMember(IntegerType::get(32), "second");

        Global *structGlob = prog->createGlobal(Address(0x08049478), structTy);
        QVERIFY(structGlob && structGlob->getInitialValue());
        QCOMPARE(structGlob->getInitialValue()->toString(), QString("7, 8"));
    }
}


QTEST_GUILESS_MAIN(GlobalTest)
