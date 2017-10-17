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
#include "boomerang/core/Project.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySection.h"

#include "boomerang/util/Log.h"

#define SWITCH_BORLAND    (BOOMERANG_TEST_BASE "/tests/inputs/windows/switch_borland.exe")

void Win32BinaryLoaderTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "/lib/boomerang/");
}


void Win32BinaryLoaderTest::testWinLoad()
{
    IProject& project = *Boomerang::get()->getOrCreateProject();

    project.loadBinaryFile(SWITCH_BORLAND);

    // Borland
    IFileLoader *loader = project.getBestLoader(SWITCH_BORLAND);
    QVERIFY(loader != nullptr);
    QCOMPARE(loader->getMainEntryPoint(), Address(0x00401150));
}


QTEST_MAIN(Win32BinaryLoaderTest)
