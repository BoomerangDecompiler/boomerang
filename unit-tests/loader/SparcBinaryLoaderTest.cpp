#include "SparcBinaryLoaderTest.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/util/Log.h"
#include "boomerang/core/BinaryFileFactory.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySection.h"


#define HELLO_SPARC  (BOOMERANG_TEST_BASE "/tests/inputs/sparc/hello")


void SparcBinaryLoaderTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "/lib/boomerang/");
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
