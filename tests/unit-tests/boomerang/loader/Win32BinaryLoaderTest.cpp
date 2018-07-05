#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Win32BinaryLoaderTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"

#include "boomerang/util/Log.h"


#define SWITCH_BORLAND    (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/windows/switch_borland.exe"))


void Win32BinaryLoaderTest::initTestCase()
{
    m_project.getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    m_project.getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
    m_project.loadPlugins();
}


void Win32BinaryLoaderTest::cleanupTestCase()
{
    Boomerang::destroy();
}


void Win32BinaryLoaderTest::testWinLoad()
{
    QVERIFY(m_project.loadBinaryFile(SWITCH_BORLAND));

    // Borland
    BinaryFile *binary = m_project.getLoadedBinaryFile();
    QVERIFY(binary != nullptr);
    QCOMPARE(binary->getMainEntryPoint(), Address(0x00401150));
}


QTEST_GUILESS_MAIN(Win32BinaryLoaderTest)
