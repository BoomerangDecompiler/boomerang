#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PalmBinaryLoaderTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySection.h"
#include "boomerang/core/Project.h"
#include "boomerang/util/Log.h"

#define STARTER_PALM    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/mc68328/Starter.prc"))


void PalmBinaryLoaderTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
}


void PalmBinaryLoaderTest::testPalmLoad()
{
    IProject& project = *Boomerang::get()->getOrCreateProject();

    project.loadBinaryFile(STARTER_PALM);
    IFileLoader *loader = project.getBestLoader(STARTER_PALM);

    QVERIFY(loader != nullptr);
    IBinaryImage *image = Boomerang::get()->getImage();

    QCOMPARE(image->getNumSections(), (size_t)8);
    QCOMPARE(image->getSection(0)->getName(), QString("code1"));
    QCOMPARE(image->getSection(1)->getName(), QString("MBAR1000"));
    QCOMPARE(image->getSection(2)->getName(), QString("tFRM1000"));
    QCOMPARE(image->getSection(3)->getName(), QString("Talt1001"));
    QCOMPARE(image->getSection(4)->getName(), QString("data0"));
    QCOMPARE(image->getSection(5)->getName(), QString("code0"));
    QCOMPARE(image->getSection(6)->getName(), QString("tAIN1000"));
    QCOMPARE(image->getSection(7)->getName(), QString("tver1000"));
}


QTEST_MAIN(PalmBinaryLoaderTest)
