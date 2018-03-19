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


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/binary/BinarySection.h"

#include <QByteArray>


void BinarySectionTest::initTestCase()
{
}


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


void BinarySectionTest::testClearDefinedArea()
{
    BinarySection section(Address(0x1000), 0x1000, "testSection");
    section.clearDefinedArea();
    QVERIFY(!section.anyDefinedValues());

    section.addDefinedArea(Address(0x1000), Address(0x0800));
    section.clearDefinedArea();
    QVERIFY(!section.anyDefinedValues());
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
    QVariantMap varMap = section.getAttributesForRange(Address(0x1000), Address(0x2000));
    QVERIFY(varMap.empty());

    section.setAttributeForRange("ReadOnly", true, Address(0x1000), Address(0x1800));
    varMap = section.getAttributesForRange(Address(0x1000), Address(0x2000));
    QVERIFY(!varMap.empty());
    QVERIFY(section.isAttributeInRange("ReadOnly", Address(0x1000), Address(0x2000)));
}

QTEST_MAIN(BinarySectionTest)
