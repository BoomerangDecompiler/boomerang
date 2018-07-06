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


#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/util/Log.h"


#define STARTER_PALM    getFullSamplePath("mc68328/Starter.prc")


void PalmBinaryLoaderTest::testPalmLoad()
{
    QVERIFY(m_project.loadBinaryFile(STARTER_PALM));

    BinaryImage *image = m_project.getLoadedBinaryFile()->getImage();

    QCOMPARE(image->getNumSections(), 8);
    QCOMPARE(image->getSectionByIndex(0)->getName(), QString("code1"));
    QCOMPARE(image->getSectionByIndex(1)->getName(), QString("MBAR1000"));
    QCOMPARE(image->getSectionByIndex(2)->getName(), QString("tFRM1000"));
    QCOMPARE(image->getSectionByIndex(3)->getName(), QString("Talt1001"));
    QCOMPARE(image->getSectionByIndex(4)->getName(), QString("data0"));
    QCOMPARE(image->getSectionByIndex(5)->getName(), QString("code0"));
    QCOMPARE(image->getSectionByIndex(6)->getName(), QString("tAIN1000"));
    QCOMPARE(image->getSectionByIndex(7)->getName(), QString("tver1000"));
}


QTEST_GUILESS_MAIN(PalmBinaryLoaderTest)
