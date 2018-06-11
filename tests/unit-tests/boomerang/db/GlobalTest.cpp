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


#define SAMPLE(path)    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/" path))

#define HELLO_PENTIUM   SAMPLE("pentium/hello")
#define FBRANCH_PENTIUM SAMPLE("pentium/fbranch")


void GlobalTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");

    m_project.loadPlugins();
}


void GlobalTest::testGetInitialValue()
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


    QVERIFY(m_project.loadBinaryFile(FBRANCH_PENTIUM));
    prog = m_project.getProg();

    // float constant
    Global *five = prog->createGlobal(Address(0x080485CC), FloatType::get(32));
    result = five->getInitialValue();
    QVERIFY(result && result->isFltConst());
    QCOMPARE(result->access<Const>()->getFlt(), 5.0f);
}


QTEST_GUILESS_MAIN(GlobalTest)
