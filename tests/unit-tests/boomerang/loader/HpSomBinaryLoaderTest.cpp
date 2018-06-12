#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "HpSomBinaryLoaderTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/util/Log.h"

#define HELLO_HPPA    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/hppa/hello"))


void HpSomBinaryLoaderTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
}


void HpSomBinaryLoaderTest::cleanupTestCase()
{
    Boomerang::destroy();
}


void HpSomBinaryLoaderTest::testHppaLoad()
{
    QSKIP("Disabled.");

    // Load HPPA hello world
    QVERIFY(m_project.loadBinaryFile(HELLO_HPPA));
    BinaryImage *image = m_project.getLoadedBinaryFile()->getImage();

    QCOMPARE(image->getNumSections(), 3);
    QCOMPARE(image->getSectionByIndex(0)->getName(), QString("$TEXT$"));
    QCOMPARE(image->getSectionByIndex(1)->getName(), QString("$DATA$"));
    QCOMPARE(image->getSectionByIndex(2)->getName(), QString("$BSS$"));
}


QTEST_GUILESS_MAIN(HpSomBinaryLoaderTest)
