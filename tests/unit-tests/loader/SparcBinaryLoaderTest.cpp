#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SparcBinaryLoaderTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySection.h"
#include "boomerang/core/Project.h"
#include "boomerang/util/Log.h"


#define HELLO_SPARC    (BOOMERANG_TEST_BASE "tests/inputs/sparc/hello")


void SparcBinaryLoaderTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "lib/boomerang/");
}


void SparcBinaryLoaderTest::testSparcLoad()
{
    // Load SPARC hello world
    IProject& project = *Boomerang::get()->getOrCreateProject();

    project.loadBinaryFile(HELLO_SPARC);
    IFileLoader *loader = project.getBestLoader(HELLO_SPARC);

    QVERIFY(loader != nullptr);

    IBinaryImage *image = Boomerang::get()->getImage();
    QVERIFY(image != nullptr);

    QCOMPARE(image->getNumSections(), (size_t)28);
    QCOMPARE(image->getSection(1)->getName(), QString(".hash"));
    QCOMPARE(image->getSection(27)->getName(), QString(".stab.indexstr"));
}


QTEST_MAIN(SparcBinaryLoaderTest)
