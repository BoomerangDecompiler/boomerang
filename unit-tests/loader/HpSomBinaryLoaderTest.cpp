#include "HpSomBinaryLoaderTest.h"

#include "boomerang/core/Boomerang.h"

#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySection.h"
#include "boomerang/util/Log.h"

#define HELLO_HPPA             (BOOMERANG_TEST_BASE "/tests/inputs/hppa/hello")


void HpSomBinaryLoaderTest::initTestCase()
{
    Boomerang::get()->setDataDirectory(BOOMERANG_TEST_BASE "/lib/boomerang/");
}


void HpSomBinaryLoaderTest::testHppaLoad()
{
    QSKIP("Disabled.");

	// Load HPPA hello world
	BinaryFileFactory bff;
	IFileLoader       *loader = bff.loadFile(HELLO_HPPA);
	QVERIFY(loader != nullptr);
	IBinaryImage *image = Boomerang::get()->getImage();

	QCOMPARE(image->getNumSections(), (size_t)3);
	QCOMPARE(image->getSectionInfo(0)->getName(), QString("$TEXT$"));
	QCOMPARE(image->getSectionInfo(1)->getName(), QString("$DATA$"));
	QCOMPARE(image->getSectionInfo(2)->getName(), QString("$BSS$"));
}

QTEST_MAIN(HpSomBinaryLoaderTest)
