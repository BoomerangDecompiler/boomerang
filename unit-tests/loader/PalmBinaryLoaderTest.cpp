#include "PalmBinaryLoaderTest.h"

#include "boomerang/db/IBinaryImage.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/IBinarySection.h"
#include "boomerang/core/BinaryFileFactory.h"

#define STARTER_PALM           (BOOMERANG_TEST_BASE "/tests/inputs/mc68328/Starter.prc")

static bool logset = false;

void PalmBinaryLoaderTest::initTestCase()
{
    if (!logset) {
        logset = true;
		Boomerang::get()->setDataDirectory(BOOMERANG_TEST_BASE "/lib/boomerang/");
        Boomerang::get()->setLogger(new NullLogger());
    }
}


void PalmBinaryLoaderTest::testPalmLoad()
{
	BinaryFileFactory bff;
	IFileLoader       *loader = bff.loadFile(STARTER_PALM);

	QVERIFY(loader != nullptr);
	IBinaryImage *image = Boomerang::get()->getImage();

	QCOMPARE(image->getNumSections(), (size_t)8);
	QCOMPARE(image->getSectionInfo(0)->getName(), QString("code1"));
	QCOMPARE(image->getSectionInfo(1)->getName(), QString("MBAR1000"));
	QCOMPARE(image->getSectionInfo(2)->getName(), QString("tFRM1000"));
	QCOMPARE(image->getSectionInfo(3)->getName(), QString("Talt1001"));
	QCOMPARE(image->getSectionInfo(4)->getName(), QString("data0"));
	QCOMPARE(image->getSectionInfo(5)->getName(), QString("code0"));
	QCOMPARE(image->getSectionInfo(6)->getName(), QString("tAIN1000"));
	QCOMPARE(image->getSectionInfo(7)->getName(), QString("tver1000"));
}

QTEST_MAIN(PalmBinaryLoaderTest)
