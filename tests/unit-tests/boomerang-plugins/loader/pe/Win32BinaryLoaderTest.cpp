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


#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"

#include "boomerang/util/log/Log.h"


#define SWITCH_BORLAND    getFullSamplePath("windows/switch_borland.exe")


void Win32BinaryLoaderTest::testWinLoad()
{
    QVERIFY(m_project.loadBinaryFile(SWITCH_BORLAND));

    // Borland
    BinaryFile *binary = m_project.getLoadedBinaryFile();
    QVERIFY(binary != nullptr);
    QCOMPARE(binary->getMainEntryPoint(), Address(0x00401150));
}


QTEST_GUILESS_MAIN(Win32BinaryLoaderTest)
