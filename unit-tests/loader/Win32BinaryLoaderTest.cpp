#include "Win32BinaryLoaderTest.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/core/BinaryFileFactory.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySection.h"

#include "boomerang/util/Log.h"

#define SWITCH_BORLAND         (BOOMERANG_TEST_BASE "/tests/inputs/windows/switch_borland.exe")

void Win32BinaryLoaderTest::initTestCase()
{
    Boomerang::get()->setDataDirectory(BOOMERANG_TEST_BASE "/lib/boomerang/");
}


void Win32BinaryLoaderTest::testWinLoad()
{
	BinaryFileFactory bff;
	IFileLoader       *loader = nullptr;

	// Borland
	loader = bff.loadFile(SWITCH_BORLAND);
	QVERIFY(loader != nullptr);
	QCOMPARE(loader->getMainEntryPoint(), Address(0x00401150));
}

QTEST_MAIN(Win32BinaryLoaderTest)
