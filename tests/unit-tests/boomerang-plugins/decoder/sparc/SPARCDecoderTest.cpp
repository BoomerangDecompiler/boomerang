#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SPARCDecoderTest.h"

#include "boomerang/ssl/RTL.h"
#include "boomerang/util/Types.h"


struct InstructionData
{
public:
    Byte data[5];
};

Q_DECLARE_METATYPE(InstructionData)
Q_DECLARE_METATYPE(ICLASS)

#define TEST_DECODE(name, data, expectedClass, result)                                             \
    QTest::newRow(name) << InstructionData{ data } << expectedClass << QString(result);


void SPARCDecoderTest::initTestCase()
{
    m_project.loadPlugins();

    Plugin *plugin = m_project.getPluginManager()->getPluginByName("SPARC decoder plugin");
    QVERIFY(plugin != nullptr);
    m_decoder = plugin->getIfc<IDecoder>();
    QVERIFY(m_decoder != nullptr);
}


void SPARCDecoderTest::testInstructions()
{
    QFETCH(InstructionData, insnData);
    QFETCH(ICLASS, expectedClass);
    QFETCH(QString, expectedResult);

    DecodeResult result;
    Address sourceAddr = Address(0x1000);
    ptrdiff_t diff     = (HostAddress(&insnData) - sourceAddr).value();
    QVERIFY(m_decoder->decodeInstruction(sourceAddr, diff, result));
    QCOMPARE(result.type, expectedClass);

    result.rtl->simplify();
    QCOMPARE(result.rtl->toString(), expectedResult);
}


void SPARCDecoderTest::testInstructions_data()
{
    QTest::addColumn<InstructionData>("insnData");
    QTest::addColumn<ICLASS>("expectedClass");
    QTest::addColumn<QString>("expectedResult");

    // Instructions (sorted alphabetically)
    // Note: Instruction data is in big endian order.

    TEST_DECODE("add %g3, %g1, %g2", "\x84\x00\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + r1\n");

    TEST_DECODE("add %g3, 1, %g2", "\x84\x00\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + 1\n");
}


QTEST_GUILESS_MAIN(SPARCDecoderTest)
