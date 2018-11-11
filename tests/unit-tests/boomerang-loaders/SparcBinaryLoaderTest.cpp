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


#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/util/log/Log.h"


#define HELLO_SPARC    getFullSamplePath("sparc/hello")


void SparcBinaryLoaderTest::testSparcLoad()
{
    // Load SPARC hello world
    QVERIFY(m_project.loadBinaryFile(HELLO_SPARC));

    BinaryImage *image = m_project.getLoadedBinaryFile()->getImage();
    QVERIFY(image != nullptr);

    QCOMPARE(image->getNumSections(), 28);
    QCOMPARE(image->getSectionByIndex(1)->getName(), QString(".hash"));
    QCOMPARE(image->getSectionByIndex(27)->getName(), QString(".stab.indexstr"));
}


QTEST_GUILESS_MAIN(SparcBinaryLoaderTest)
