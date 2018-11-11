#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UtilTest.h"


#include "boomerang/util/ByteUtil.h"
#include "boomerang/util/Util.h"



void UtilTest::testEscapeStr()
{
    QCOMPARE(Util::escapeStr(""),               QString(""));
    QCOMPARE(Util::escapeStr("Hello"),          QString("Hello"));
    QCOMPARE(Util::escapeStr("H\ne\tl\vl\bo\rworld\f\a"),  QString("H\\ne\\tl\\vl\\bo\\rworld\\f\\a"));
    QCOMPARE(Util::escapeStr("\"Test\""),       QString("\\\"Test\\\""));
}


void UtilTest::testInRange()
{
    QCOMPARE(Util::inRange(1, 1, 1), false);
    QCOMPARE(Util::inRange(1, 3, 1), false);
    QCOMPARE(Util::inRange(1, 0, 1), false);
    QCOMPARE(Util::inRange(0, 0, 1), true);
    QCOMPARE(Util::inRange(1, 0, 2), true);
    QCOMPARE(Util::inRange(3, 1, 2), false);
}


void UtilTest::testIsContained()
{
    std::list<int> list = { 1, 2, 3 };

    QCOMPARE(Util::isContained(list, 1), true);
    QCOMPARE(Util::isContained(list, 0), false);
    QCOMPARE(Util::isContained(std::list<int>(), 0), false);
}


void UtilTest::testGetLowerBitMask()
{
    QCOMPARE(Util::getLowerBitMask(0),  static_cast<QWord>(              0x00));
    QCOMPARE(Util::getLowerBitMask(2),  static_cast<QWord>(              0x03));
    QCOMPARE(Util::getLowerBitMask(16), static_cast<QWord>(            0xFFFF));
    QCOMPARE(Util::getLowerBitMask(32), static_cast<QWord>(        0xFFFFFFFF));
    QCOMPARE(Util::getLowerBitMask(63), static_cast<QWord>(0x7FFFFFFFFFFFFFFF));
    QCOMPARE(Util::getLowerBitMask(64), static_cast<QWord>(0xFFFFFFFFFFFFFFFF)); // (uint64_t)-1
}


void UtilTest::testSwapEndian()
{
    QCOMPARE(Util::swapEndian(static_cast<SWord>(            0x1122)),  static_cast<SWord>(            0x2211));
    QCOMPARE(Util::swapEndian(static_cast<DWord>(        0x11223344)),  static_cast<DWord>(        0x44332211));
    QCOMPARE(Util::swapEndian(static_cast<QWord>(0x1122334455667788)),  static_cast<QWord>(0x8877665544332211));
}


void UtilTest::testNormEndian()
{
    QCOMPARE(Util::normEndian(static_cast<SWord>(            0x1122), Endian::Little), static_cast<SWord>(            0x1122));
    QCOMPARE(Util::normEndian(static_cast<DWord>(        0x11223344), Endian::Little), static_cast<DWord>(        0x11223344));
    QCOMPARE(Util::normEndian(static_cast<QWord>(0x1122334455667788), Endian::Little), static_cast<QWord>(0x1122334455667788));

    QCOMPARE(Util::normEndian(static_cast<SWord>(            0x1122), Endian::Big), static_cast<SWord>(            0x2211));
    QCOMPARE(Util::normEndian(static_cast<DWord>(        0x11223344), Endian::Big), static_cast<DWord>(        0x44332211));
    QCOMPARE(Util::normEndian(static_cast<QWord>(0x1122334455667788), Endian::Big), static_cast<QWord>(0x8877665544332211));
}


void UtilTest::testRead()
{
    const Byte buffer[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };

    QCOMPARE(Util::readByte(buffer),        static_cast<Byte> (              0x11));

    QCOMPARE(Util::readWord(buffer,  Endian::Little), static_cast<SWord>(            0x2211));
    QCOMPARE(Util::readDWord(buffer, Endian::Little), static_cast<DWord>(        0x44332211));
    QCOMPARE(Util::readQWord(buffer, Endian::Little), static_cast<QWord>(0x8877665544332211));

    QCOMPARE(Util::readWord(buffer,  Endian::Big), static_cast<SWord>(            0x1122));
    QCOMPARE(Util::readDWord(buffer, Endian::Big), static_cast<DWord>(        0x11223344));
    QCOMPARE(Util::readQWord(buffer, Endian::Big), static_cast<QWord>(0x1122334455667788));
}


void UtilTest::testWrite()
{
    const Byte expectedBuffer[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
    Byte buffer[8] = { 0 };

    Util::writeByte(buffer, 0x11);
    QCOMPARE(memcmp(buffer, expectedBuffer, 1), 0);

    Util::writeWord(buffer,                 0x2211, Endian::Little);
    QCOMPARE(memcmp(buffer, expectedBuffer, 2), 0);
    Util::writeDWord(buffer,            0x44332211, Endian::Little);
    QCOMPARE(memcmp(buffer, expectedBuffer, 4), 0);
    Util::writeQWord(buffer, 0x8877665544332211ULL, Endian::Little);
    QCOMPARE(memcmp(buffer, expectedBuffer, 8), 0);

    Util::writeWord(buffer,                 0x1122, Endian::Big);
    QCOMPARE(memcmp(buffer, expectedBuffer, 2), 0);
    Util::writeDWord(buffer,            0x11223344, Endian::Big);
    QCOMPARE(memcmp(buffer, expectedBuffer, 4), 0);
    Util::writeQWord(buffer, 0x1122334455667788ULL, Endian::Big);
    QCOMPARE(memcmp(buffer, expectedBuffer, 8), 0);
}


void UtilTest::testSignExtend()
{
    QCOMPARE(Util::signExtend(static_cast<Byte>(0x10)), 0x10);
    QCOMPARE(Util::signExtend(static_cast<Byte>(0xFF)), -1);
}



QTEST_GUILESS_MAIN(UtilTest)
