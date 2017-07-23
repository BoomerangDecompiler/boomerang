#include "Win32BinaryLoaderTest.h"

#include "boomerang/db/IBinaryImage.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/IBinarySection.h"
#include "boomerang/core/BinaryFileFactory.h"

//
#define SWITCH_BORLAND         (BOOMERANG_TEST_BASE "/tests/inputs/windows/switch_borland.exe")

#define TEST_PROPRIETARY 0

#if TEST_PROPRIETARY
#  define CALC_WINDOWS    (BOOMERANG_TEST_BASE "/tests/inputs/windows/calc.exe")
#  define CALC_WINXP      (BOOMERANG_TEST_BASE "/tests/inputs/windows/calcXP.exe")
#  define CALC_WIN2000    (BOOMERANG_TEST_BASE "/tests/inputs/windows/calc2000.exe")
#  define LPQ_WINDOWS     (BOOMERANG_TEST_BASE "/tests/inputs/windows/lpq.exe")
#endif


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

#if TEST_PROPRIETARY
	// Load Windows program calc.exe

	loader = bff.loadFile(CALC_WINDOWS);
	QVERIFY(loader != nullptr);
	QVERIFY(loader->getMainEntryPoint() != Address::INVALID);

	IBinaryImage *image = Boomerang::get()->getImage();
	QCOMPARE(image->getNumSections(), (size_t)5);
	QCOMPARE(image->getSectionInfo(0), QString(".text"));
	QCOMPARE(image->getSectionInfo(1), QString(".rdata"));
	QCOMPARE(image->getSectionInfo(2), QString(".data"));
	QCOMPARE(image->getSectionInfo(3), QString(".rsrc"));
	QCOMPARE(image->getSectionInfo(4), QString(".reloc"));

	// Test loading the "new style" exes, as found in WinXP etc
	loader = bff.loadFile(CALC_WINXP);
	QVERIFY(loader != nullptr);
	QCOMPARE(loader->getMainEntryPoint(), Address(0x01001F51));

	// Test loading the calc.exe found in Windows 2000 (more NT based)
	loader = bff.loadFile(CALC_WIN2000);
	QVERIFY(loader != nullptr);
	QCOMPARE(loader->getMainEntryPoint(), Address(0x01001680));

	// Test loading the lpq.exe program - console mode PE file
	loader = bff.loadFile(LPQ_WINDOWS);
	QVERIFY(loader != nullptr);
	QCOMPARE(loader->getMainEntryPoint(), Address(0x018C1000));
#endif

	// Borland
	loader = bff.loadFile(SWITCH_BORLAND);
	QVERIFY(loader != nullptr);
	QCOMPARE(loader->getMainEntryPoint(), Address(0x401150));
}

QTEST_MAIN(Win32BinaryLoaderTest)
