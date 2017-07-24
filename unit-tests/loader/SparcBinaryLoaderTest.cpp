#include "SparcBinaryLoaderTest.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/util/Log.h"
#include "boomerang/core/BinaryFileFactory.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySection.h"

static bool logset = false;

#define HELLO_SPARC  (BOOMERANG_TEST_BASE "/tests/inputs/sparc/hello")


void SparcBinaryLoaderTest::initTestCase()
{
    if (!logset) {
        logset = true;
        Boomerang::get()->setDataDirectory(BOOMERANG_TEST_BASE "/lib/boomerang/");
        Boomerang::get()->setLogger(new NullLogger());
    }
}


void SparcBinaryLoaderTest::testSparcLoad()
{
    // Load SPARC hello world
    BinaryFileFactory bff;
    IFileLoader       *loader = bff.loadFile(HELLO_SPARC);

    QVERIFY(loader != nullptr);

    IBinaryImage *image = Boomerang::get()->getImage();
    QVERIFY(image != nullptr);

    QCOMPARE(image->getNumSections(), (size_t)28);
    QCOMPARE(image->getSectionInfo(1)->getName(), QString(".hash"));
    QCOMPARE(image->getSectionInfo(27)->getName(), QString(".stab.indexstr"));
}

QTEST_MAIN(SparcBinaryLoaderTest)
