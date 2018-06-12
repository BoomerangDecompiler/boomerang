#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SparcBinaryLoaderTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/util/Log.h"


#define HELLO_SPARC    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/sparc/hello"))


void SparcBinaryLoaderTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");

    m_project.loadPlugins();
}


void SparcBinaryLoaderTest::cleanupTestCase()
{
    Boomerang::destroy();
}


void SparcBinaryLoaderTest::testSparcLoad()
{
    // Load SPARC hello world
    QVERIFY(m_project.loadBinaryFile(HELLO_SPARC));

    BinaryImage *image = m_project.getLoadedBinaryFile()->getImage();
    QVERIFY(image != nullptr);

    QCOMPARE(image->getNumSections(), 28);
    QCOMPARE(image->getSectionByIndex(1)->getName(), QString(".hash"));
    QCOMPARE(image->getSectionByIndex(27)->getName(), QString(".stab.indexstr"));
}


QTEST_GUILESS_MAIN(SparcBinaryLoaderTest)
