#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CapstonePPCDecoderTest.h"

#include "boomerang/util/Types.h"
#include "boomerang/ssl/RTL.h"


struct InstructionData
{
public:
    Byte data[5];
};

Q_DECLARE_METATYPE(InstructionData)

#define TEST_DECODE(name, data, result) \
    QTest::newRow(name) << \
        InstructionData{ data } << \
        QString(result);


void CapstonePPCDecoderTest::setupTestCase()
{
    Plugin *plugin = m_project.getPluginManager()->getPluginByName("Capstone PPC decoder plugin");
    QVERIFY(plugin != nullptr);
    m_decoder = plugin->getIfc<IDecoder>();
    QVERIFY(m_decoder != nullptr);
}


void CapstonePPCDecoderTest::testInstructions()
{
    QFETCH(InstructionData, insnData);
    QFETCH(QString, expectedResult);

    DecodeResult result;
    Address sourceAddr = Address(0x1000);
    ptrdiff_t diff = (HostAddress(&insnData) - sourceAddr).value();
    QVERIFY(m_decoder->decodeInstruction(sourceAddr, diff, result));
    QCOMPARE(result.rtl->toString(), expectedResult);
}


void CapstonePPCDecoderTest::testInstructions_data()
{
    QTest::addColumn<InstructionData>("insnData");
    QTest::addColumn<QString>("expectedResult");

    // instructions listed alphabetically

    TEST_DECODE("add r0, r1, r2", "\x7c\x01\x12\x14",
                "0x00001000    0 *32* r0 := r1 + r2\n"
    );

    TEST_DECODE("add. r0, r1, r2", "\x7c\x01\x12\x15",
                "0x00001000    0 *32* r0 := r1 + r2\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n"
    );

    // Capstone does not support addo and addo. yet


    TEST_DECODE("addc r0, r1, r2", "\x7c\x01\x10\x14",
                "0x00001000    0 *32* r0 := r1 + r2\n"
                "              0 *v* %flags := ADDFLAGSX( r0, r1, r2 )\n"
    );

    TEST_DECODE("addc. r0, r1, r2", "\x7c\x01\x10\x15",
                "0x00001000    0 *32* r0 := r1 + r2\n"
                "              0 *v* %flags := ADDFLAGSX0( r0, r1, r2 )\n"
    );

    // Capstone does not support addco and addco. yet

    TEST_DECODE("adde r0, r1, r2", "\x7c\x01\x11\x14",
                "0x00001000    0 *32* r0 := r1 + (r2 + r203)\n"
    );

    TEST_DECODE("adde. r0, r1, r2", "\x7c\x01\x11\x15",
                "0x00001000    0 *32* r0 := r1 + (r2 + r203)\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n"
    );

    TEST_DECODE("addi r0, r1, -10", "\x38\x01\xff\xf6",
                "0x00001000    0 *32* r0 := r1 - 10\n"
    );


    TEST_DECODE("addic r0, r1, -10", "\x30\x01\xff\xf6",
                "0x00001000    0 *32* r0 := r1 - 10\n"
                "              0 *v* %flags := ADDFLAGSX( r0, r1, -10 )\n"
    );

    TEST_DECODE("addic. r0, r1, -10", "\x34\x01\xff\xf6",
                "0x00001000    0 *32* r0 := r1 - 10\n"
                "              0 *v* %flags := ADDFLAGSX0( r0, r1, -10 )\n"
    );

    TEST_DECODE("addis r0, r1, -10", "\x3c\x01\xff\xf6",
                "0x00001000    0 *32* r0 := r1 - 0xa0000\n"
    );

    TEST_DECODE("addme r0, r1", "\x7c\x01\x01\xd4",
                "0x00001000    0 *32* r0 := (r1 + r203) - 1\n"
    );

    TEST_DECODE("addme. r0, r1", "\x7c\x01\x01\xd5",
                "0x00001000    0 *32* r0 := (r1 + r203) - 1\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n"
    );

    // Capstone does not support addmeo[.] and addme64[o] yet

    TEST_DECODE("addze r0, r1", "\x7c\x01\x01\x94",
                "0x00001000    0 *32* r0 := r1 + r203\n"
    );

    TEST_DECODE("addze. r0, r1", "\x7c\x01\x01\x95",
                "0x00001000    0 *32* r0 := r1 + r203\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n"
    );

    // Capstone does not support addzeo[.] and addze64[o] yet

    TEST_DECODE("and r0, r1, r2", "\x7c\x20\x10\x38",
                "0x00001000    0 *32* r0 := r1 & r2\n"
    );

    TEST_DECODE("and. r0, r1, r2", "\x7c\x20\x10\x39",
                "0x00001000    0 *32* r0 := r1 & r2\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n"
    );

    TEST_DECODE("andi. r0, r1, 10", "\x70\x20\x00\x0a",
                "0x00001000    0 *32* r0 := r1 & 10\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n"
    );

    TEST_DECODE("andis. r0, r1, 10", "\x74\x20\x00\x0a",
                "0x00001000    0 *32* r0 := r1 & 0xa0000\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n"
    );

    TEST_DECODE("andc r0, r1, r2", "\x7c\x20\x10\x78",
                "0x00001000    0 *32* r0 := r1 & ~r2\n"
    );

    TEST_DECODE("andc. r0, r1, r2", "\x7c\x20\x10\x79",
                "0x00001000    0 *32* r0 := r1 & ~r2\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n"
    );

    TEST_DECODE("b 0x0800", "\x4b\xff\xf8\x00",
                "0x00001000    0 GOTO 0x00000800\n"
    );

    TEST_DECODE("b 0x2000", "\x48\x00\x10\x00",
                "0x00001000    0 GOTO 0x00002000\n"
    );

    TEST_DECODE("ba 0x2000", "\x48\x00\x20\x02",
                "0x00001000    0 GOTO 0x00002000\n"
    );

    TEST_DECODE("bl 0x2000", "\x48\x00\x10\x01",
                "0x00001000    0 *32* r0 := 0x1004\n"
                "              0 <all> := CALL 0x2000(<all>)\n"
                "              Reaching definitions: <None>\n"
                "              Live variables: <None>\n"
    );

    TEST_DECODE("bla 0x2000", "\x48\x00\x20\x03",
                "0x00001000    0 *32* r0 := 0x1004\n"
                "              0 <all> := CALL 0x2000(<all>)\n"
                "              Reaching definitions: <None>\n"
                "              Live variables: <None>\n"
    );

    // TODO: Conditional branches BC*

    TEST_DECODE("cmp 3, 0, 0, 1", "\x7d\x80\x08\x00",
                "0x00001000    0 *v* %flags := SUBFLAGSNS( r0, r1, r67 )\n"
    );

    TEST_DECODE("cmpi 0, 0, 0, -10", "\x2c\x00\xff\xf6",
                "0x00001000    0 *v* %flags := SUBFLAGSNS( r0, -10, 0 )\n"
    );

    TEST_DECODE("cmpi 3, 0, 0, -10", "\x2d\x80\xff\xf6",
                "0x00001000    0 *v* %flags := SUBFLAGSNS( r0, -10, r67 )\n"
    );

    TEST_DECODE("cmpl 3, 0, 0, 1", "\x7d\x80\x08\x40",
                "0x00001000    0 *v* %flags := SUBFLAGSNL( r0, r1, r67 )\n"
    );

    TEST_DECODE("cmpli 0, 0, 0, 1", "\x28\x00\x00\x01",
                "0x00001000    0 *v* %flags := SUBFLAGSNL( r0, 1, 0 )\n"
    );

    TEST_DECODE("cmpli 3, 0, 0, 1", "\x29\x80\x00\x01",
                "0x00001000    0 *v* %flags := SUBFLAGSNL( r0, 1, r67 )\n"
    );

    // TODO: Add semantics for cntlzw/cntlzw./cntlzd

    TEST_DECODE("crand 0, 2, 7", "\x4c\x02\x3a\x02",
                "0x00001000    0 *1* r100@[0:0] := (r100@[2:2]) & (r100@[7:7])\n"
    );

    TEST_DECODE("crandc 0, 2, 7", "\x4c\x02\x39\x02",
                "0x00001000    0 *1* r100@[0:0] := (r100@[2:2]) & ~(r100@[7:7])\n"
    );

    TEST_DECODE("creqv 0, 2, 7", "\x4c\x02\x3a\x42",
                "0x00001000    0 *1* r100@[0:0] := ~((r100@[2:2]) ^ (r100@[7:7]))\n"
    );

    TEST_DECODE("crnand 0, 2, 7", "\x4c\x02\x39\xc2",
                "0x00001000    0 *1* r100@[0:0] := ~(r100@[2:2]) | ~(r100@[7:7])\n"
    );

    TEST_DECODE("crnor 0, 2, 7", "\x4c\x02\x38\x42",
                "0x00001000    0 *1* r100@[0:0] := ~(r100@[2:2]) & ~(r100@[7:7])\n"
    );

    TEST_DECODE("cror 0, 2, 7", "\x4c\x02\x3b\x82",
                "0x00001000    0 *1* r100@[0:0] := (r100@[2:2]) | (r100@[7:7])\n"
    );

    TEST_DECODE("crorc 0, 2, 7", "\x4c\x02\x3b\x42",
                "0x00001000    0 *1* r100@[0:0] := (r100@[2:2]) | ~(r100@[7:7])\n"
    );

    TEST_DECODE("crxor 0, 2, 7", "\x4c\x02\x39\x82",
                "0x00001000    0 *1* r100@[0:0] := (r100@[2:2]) ^ (r100@[7:7])\n"
    );

    // TODO dcb*

    TEST_DECODE("divd 0, 1, 2", "\x7c\x01\x13\xd2",
                ""
    );

    // Capstone does not support divdo yet

    TEST_DECODE("divdu 0, 1, 2", "\x7c\x01\x13\x92",
                ""
    );

    // Capstone does not support divdou yet

    TEST_DECODE("divw 0, 1, 2", "\x7c\x01\x13\xd6",
                "0x00001000    0 *32* r0 := r1 / r2\n"
    );

    TEST_DECODE("divw. 0, 1, 2", "\x7c\x01\x13\xd7",
                "0x00001000    0 *32* r0 := r1 / r2\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n"
    );

    // Capstone does not support divwo[.] yet

    TEST_DECODE("divwu 0, 1, 2", "\x7c\x01\x13\x96",
                "0x00001000    0 *32* r0 := r1 / r2\n"
    );

    TEST_DECODE("divwu. 0, 1, 2", "\x7c\x01\x13\x97",
                "0x00001000    0 *32* r0 := r1 / r2\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n"
    );

    // Capstone does not support divwuo[.] yet

    TEST_DECODE("eqv 0, 1, 2", "\x7c\x20\x12\x38",
                "0x00001000    0 *32* r0 := ~(r1 ^ r2)\n"
    );

    TEST_DECODE("eqv. 0, 1, 2", "\x7c\x20\x12\x39",
                "0x00001000    0 *32* r0 := ~(r1 ^ r2)\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n"
    );

    TEST_DECODE("extsb 0, 1", "\x7c\x20\x07\x74",
                "0x00001000    0 *32* r0 := sgnex(8, 32, r1)\n"
    );

    TEST_DECODE("extsb. 0, 1", "\x7c\x20\x07\x75",
                "0x00001000    0 *32* r0 := sgnex(8, 32, r1)\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n"
    );

    TEST_DECODE("extsh 0, 1", "\x7c\x20\x07\x34",
                "0x00001000    0 *32* r0 := sgnex(16, 32, r1)\n"
    );

    TEST_DECODE("extsb 0, 1", "\x7c\x20\x07\x35",
                "0x00001000    0 *32* r0 := sgnex(16, 32, r1)\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n"
    );

    // TODO extsw
}


QTEST_GUILESS_MAIN(CapstonePPCDecoderTest)
