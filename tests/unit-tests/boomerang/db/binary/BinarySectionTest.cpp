#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BinarySectionTest.h"


#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/binary/BinarySection.h"

#include <QByteArray>


void BinarySectionTest::testIsAddressBss()
{
    BinarySection section(Address(0x1000), 0x1000, "testSection");
    QVERIFY(!section.isAddressBss(Address(0x0800))); // not in range

    section.setBss(true);
    section.setReadOnly(false);
    QVERIFY(section.isAddressBss(Address(0x1000)));
    section.setBss(false);
    section.setReadOnly(true);
    QVERIFY(!section.isAddressBss(Address(0x1000)));
    section.setReadOnly(false);
    QVERIFY(section.isAddressBss(Address(0x1000)));
    section.addDefinedArea(Address(0x1000), Address(0x2000));
    QVERIFY(!section.isAddressBss(Address(0x1000)));
}


void BinarySectionTest::testAnyDefinedValues()
{
    BinarySection section(Address(0x1000), 0x1000, "testSection");
    QVERIFY(!section.anyDefinedValues());

    section.addDefinedArea(Address(0x1000), Address(0x2000));
    QVERIFY(section.anyDefinedValues());
}


void BinarySectionTest::testResize()
{
    BinarySection section(Address(0x1000), 0x0800, "testSection");
    QCOMPARE(section.getSize(), 0x0800);
    section.resize(0x1000);
    QCOMPARE(section.getSize(), 0x1000);
}


void BinarySectionTest::testAddDefinedArea()
{
    BinarySection section(Address(0x1000), 0x1000, "testSection");
    section.addDefinedArea(Address(0x1000), Address(0x1800));
    QVERIFY(!section.isAddressBss(Address(0x1000)));
}


void BinarySectionTest::testAttributes()
{
    BinarySection section(Address(0x1000), 0x1000, "testSection");
    QVERIFY(!section.addressHasAttribute("ReadOnly", Address(0x1800)));
    section.setAttributeForRange("ReadOnly", Address(0x1000), Address(0x1800));
    QVERIFY(section.addressHasAttribute("ReadOnly", Address(0x1600)));
}

QTEST_GUILESS_MAIN(BinarySectionTest)
