#include "Win32BinaryLoaderTest.h"

#include "boomerang/db/IBinaryImage.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/IBinarySection.h"
#include "boomerang/core/BinaryFileFactory.h"

//
#define SWITCH_BORLAND         (BOOMERANG_TEST_BASE "/tests/inputs/windows/switch_borland.exe")

static bool    logset = false;

void Win32BinaryLoaderTest::initTestCase()
{
    if (!logset) {
        logset = true;
		Boomerang::get()->setDataDirectory(BOOMERANG_TEST_BASE "/lib/boomerang/");
        Boomerang::get()->setLogger(new NullLogger());
    }
}


void Win32BinaryLoaderTest::testWinLoad()
{
	QSKIP("Disabled.");

	BinaryFileFactory bff;
	IFileLoader       *loader = nullptr;

	// Borland
	loader = bff.loadFile(SWITCH_BORLAND);
	QVERIFY(loader != nullptr);
	QCOMPARE(loader->getMainEntryPoint(), Address(0x401150));
}

QTEST_MAIN(Win32BinaryLoaderTest)
