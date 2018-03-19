#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BinaryImageTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/binary/BinaryImage.h"

#include <QByteArray>

char sectionData[8] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };


void BinaryImageTest::initTestCase()
{
}


void BinaryImageTest::testGetNumSections()
{
    BinaryImage img;
    QCOMPARE(img.getNumSections(), 0);

    img.createSection("sect1", Address(0x1000), Address(0x2000));
    QCOMPARE(img.getNumSections(), 1);
}


void BinaryImageTest::testHasSections()
{
    BinaryImage img;
    QVERIFY(!img.hasSections());

    img.createSection("sect1", Address(0x1000), Address(0x2000));
    QVERIFY(img.hasSections());
}


void BinaryImageTest::testCreateSection()
{
    BinaryImage img;

    BinarySection *sect1 = img.createSection("sect1", Address(0x1000), Address(0x2000));
    QCOMPARE(sect1->getSourceAddr(), Address(0x1000));
    QCOMPARE(sect1->getSize(), 0x1000);
    QCOMPARE(sect1->getName(), QString("sect1"));

    // blocking; create anyways
    BinarySection *sect2 = img.createSection("sect2", Address(0x1800), Address(0x2800));
    QCOMPARE(img.getNumSections(), 2);
    QCOMPARE(sect2->getSourceAddr(), Address(0x1800));
    QCOMPARE(sect2->getSize(), 0x1000);
    QCOMPARE(sect2->getName(), QString("sect2"));

    BinarySection *nonexistent = img.createSection("nonexistent", Address::INVALID, Address::INVALID);
    QVERIFY(nonexistent == nullptr);

    BinarySection *invalid = img.createSection("invalid", Address(0x2000), Address(0x1000));
    QVERIFY(invalid == nullptr);
}


void BinaryImageTest::testGetSectionByIndex()
{
    BinaryImage img;
    QVERIFY(img.getSectionByIndex(0) == nullptr);

    BinarySection *sect1 = img.createSection("sect1", Address(0x1000), Address(0x2000));
    QCOMPARE(img.getSectionByIndex(0), sect1);
}


void BinaryImageTest::testGetSectionByName()
{
    BinaryImage img;
    QVERIFY(img.getSectionByName("") == nullptr);

    BinarySection *sect1 = img.createSection("sect1", Address(0x1000), Address(0x2000));
    QCOMPARE(img.getSectionByName("sect1"), sect1);
    QVERIFY(img.getSectionByName("nonexistent") == nullptr);
}


void BinaryImageTest::testGetSectionByAddr()
{
    BinaryImage img;
    QVERIFY(img.getSectionByAddr(Address(0x1000)) == nullptr);

    BinarySection *sect1 = img.createSection("sect1", Address(0x1000), Address(0x2000));
    QVERIFY(img.getSectionByAddr(Address(0x1000)) == sect1);
    QVERIFY(img.getSectionByAddr(Address(0x1800)) == sect1);
    QVERIFY(img.getSectionByAddr(Address(0x2000)) == nullptr);
}


void BinaryImageTest::testUpdateTextLimits()
{
    BinaryImage img;
    img.updateTextLimits();
    QCOMPARE(img.getLimitTextLow(), Address::INVALID);
    QCOMPARE(img.getLimitTextHigh(), Address::INVALID);
    QVERIFY(img.getTextDelta() == 0);

    // single section that is not code
    BinarySection *sect1 = img.createSection("sect1", Address(0x1000), Address(0x2000));
    img.updateTextLimits();
    QCOMPARE(img.getLimitTextLow(), Address::INVALID);
    QCOMPARE(img.getLimitTextHigh(), Address::INVALID);
    QVERIFY(img.getTextDelta() == 0);

    sect1->setCode(true);
    img.updateTextLimits();
    QCOMPARE(img.getLimitTextLow(), Address(0x1000));
    QCOMPARE(img.getLimitTextHigh(), Address(0x2000));
    QVERIFY(img.getTextDelta() != 0);

    /// .plt does not count as code
    BinarySection *plt = img.createSection(".plt", Address(0x2000), Address(0x3000));
    plt->setCode(true);
    img.updateTextLimits();
    QCOMPARE(img.getLimitTextLow(), Address(0x1000));
    QCOMPARE(img.getLimitTextHigh(), Address(0x2000));
    QVERIFY(img.getTextDelta() != 0);
}


void BinaryImageTest::testRead()
{
    BinaryImage img;
    QCOMPARE(img.readNative1(Address(0x1000)), static_cast<Byte>(0xFF));
    QCOMPARE(img.readNative2(Address(0x1000)), static_cast<SWord>(0x0000));
    QCOMPARE(img.readNative4(Address(0x1000)), static_cast<DWord>(0x00000000));
    QCOMPARE(img.readNative8(Address(0x1000)), static_cast<QWord>(0x0000000000000000));

    // section not mapped to data. Verify no AV occurs.
    BinarySection *sect1 = img.createSection("sect1", Address(0x1000), Address(0x1008));
    QCOMPARE(img.readNative1(Address(0x1000)), static_cast<Byte>(0xFF));
    QCOMPARE(img.readNative2(Address(0x1000)), static_cast<SWord>(0x0000));
    QCOMPARE(img.readNative4(Address(0x1000)), static_cast<DWord>(0x00000000));
    QCOMPARE(img.readNative8(Address(0x1000)), static_cast<QWord>(0x0000000000000000));

    // section mapped to data. Verify correct read
    sect1->setHostAddr(HostAddress(sectionData));
    QCOMPARE(img.readNative1(Address(0x1000)), static_cast<Byte>(0x00));
    QCOMPARE(img.readNative2(Address(0x1000)), static_cast<SWord>(0x1100));
    QCOMPARE(img.readNative4(Address(0x1000)), static_cast<DWord>(0x33221100));
    QCOMPARE(img.readNative8(Address(0x1000)), static_cast<QWord>(0x7766554433221100));

    // read crosses section boundary (makes no sense for readNative1)
    QCOMPARE(img.readNative2(Address(0x1007)), static_cast<SWord>(0x0000));
    QCOMPARE(img.readNative4(Address(0x1005)), static_cast<DWord>(0x00000000));
    QCOMPARE(img.readNative8(Address(0x1001)), static_cast<QWord>(0x0000000000000000));
}


void BinaryImageTest::testWrite()
{
    BinaryImage img;
    QVERIFY(!img.writeNative4(Address(0x1000), static_cast<DWord>(0x00112233)));

    // section not mapped to data. Verify no AV occurs.
    BinarySection *sect1 = img.createSection("sect1", Address(0x1000), Address(0x1008));
    QVERIFY(!img.writeNative4(Address(0x1000), static_cast<DWord>(0x0011223344)));

    // section mapped to data. Verify correct read
    sect1->setHostAddr(HostAddress(sectionData));

    // note that this line will change \ref sectionData!
    QVERIFY(img.writeNative4(Address(0x1000), static_cast<DWord>(0xBADCAB1E)));
    QCOMPARE(img.readNative4(Address(0x1000)), static_cast<DWord>(0xBADCAB1E));

    // write crosses section boundary
    QVERIFY(!img.writeNative4(Address(0x1005), static_cast<DWord>(0x1BADCA11)));
}


void BinaryImageTest::testIsReadOnly()
{
    BinaryImage img;
    QVERIFY(!img.isReadOnly(Address(0x1000)));
    QVERIFY(!img.isReadOnly(Address::INVALID));

    BinarySection *sect1 = img.createSection("sect1", Address(0x1000), Address(0x2000));
    QVERIFY(!img.isReadOnly(Address(0x1800)));
    sect1->setReadOnly(true);
    QVERIFY(img.isReadOnly(Address(0x1800)));
    sect1->setReadOnly(false);

    sect1->setAttributeForRange("ReadOnly", true, Address(0x1400), Address(0x2000));
    QVERIFY(!img.isReadOnly(Address(0x1200)));
    QVERIFY(img.isReadOnly(Address(0x1800)));
}


QTEST_MAIN(BinaryImageTest)
