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
                "              0 *32* r2 := r3 + r1\n"
    );

    TEST_DECODE("add %g3, 1, %g2", "\x84\x00\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + 1\n"
    );

    TEST_DECODE("addcc %g3, %g1, %g2", "\x84\x80\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + r1\n"
                "              0 *v* %flags := ADDFLAGS( tmp, r1, r2 )\n"
    );

    TEST_DECODE("addcc %g3, 1, %g2", "\x84\x80\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + 1\n"
                "              0 *v* %flags := ADDFLAGS( tmp, 1, r2 )\n"
    );

    TEST_DECODE("addx %g3, %g1, %g2", "\x84\x40\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + (r1 + zfill(1, 32, %CF))\n"
    );

    TEST_DECODE("addx %g3, 1, %g2", "\x84\x40\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := (r3 + zfill(1, 32, %CF)) + 1\n"
    );

    TEST_DECODE("addxcc %g3, %g1, %g2", "\x84\xc0\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + (r1 + zfill(1, 32, %CF))\n"
                "              0 *v* %flags := ADDFLAGS( tmp, r1, r2 )\n"
    );

    TEST_DECODE("addxcc %g3, 1, %g2", "\x84\xc0\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := (r3 + zfill(1, 32, %CF)) + 1\n"
                "              0 *v* %flags := ADDFLAGS( tmp, 1, r2 )\n"
    );

    TEST_DECODE("and %g3, %g1, %g2", "\x84\x08\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 & r1\n"
    );

    TEST_DECODE("and %g3, 1, %g2", "\x84\x08\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 & 1\n"
    );

    TEST_DECODE("andcc %g3, %g1, %g2", "\x84\x88\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 & r1\n"
                "              0 *v* %flags := LOGICALFLAGS( r2 )\n"
    );

    TEST_DECODE("andcc %g3, 1, %g2", "\x84\x88\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 & 1\n"
                "              0 *v* %flags := LOGICALFLAGS( r2 )\n"
    );

    TEST_DECODE("andn %g3, %g1, %g2", "\x84\x28\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 & ~r1\n"
    );

    TEST_DECODE("andn %g3, 1, %g2", "\x84\x28\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 & -2\n"
    );

    TEST_DECODE("andncc %g3, %g1, %g2", "\x84\xa8\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 & ~r1\n"
                "              0 *v* %flags := LOGICALFLAGS( r2 )\n"
    );

    TEST_DECODE("andncc %g3, 1, %g2", "\x84\xa8\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 & -2\n"
                "              0 *v* %flags := LOGICALFLAGS( r2 )\n"
    );

    TEST_DECODE("ba 0x2000", "\x10\x80\x04\x00", ICLASS::SD,
                "0x00001000    0 GOTO 0x00002000\n"
    );

    TEST_DECODE("ba,a 0x2000", "\x30\x80\x04\x00", ICLASS::SU,
                "0x00001000    0 GOTO 0x00002000\n"
    );

    TEST_DECODE("bn 0x2000", "\x00\x80\x04\x00", ICLASS::NCT,
                "0x00001000    0 GOTO 0x00002000\n"
    );

    TEST_DECODE("bn,a 0x2000", "\x20\x80\x04\x00", ICLASS::SKIP,
                "0x00001000    0 GOTO 0x00002000\n"
    );

    TEST_DECODE("bne 0x2000", "\x12\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition not equals\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bne,a 0x2000", "\x32\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition not equals\n"
                "High level: %flags\n"
    );

    TEST_DECODE("be 0x2000", "\x02\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition equals\n"
                "High level: %flags\n"
    );

    TEST_DECODE("be,a 0x2000", "\x22\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition equals\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bg 0x2000", "\x14\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition signed greater\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bg,a 0x2000", "\x34\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition signed greater\n"
                "High level: %flags\n"
    );

    TEST_DECODE("ble 0x2000", "\x04\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition signed less or equals\n"
                "High level: %flags\n"
    );

    TEST_DECODE("ble,a 0x2000", "\x24\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition signed less or equals\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bge 0x2000", "\x16\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition signed greater or equals\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bge,a 0x2000", "\x36\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition signed greater or equals\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bl 0x2000", "\x06\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition signed less\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bl,a 0x2000", "\x26\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition signed less\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bgu 0x2000", "\x18\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition unsigned greater\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bgu,a 0x2000", "\x38\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition unsigned greater\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bleu 0x2000", "\x08\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition unsigned less or equals\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bleu,a 0x2000", "\x28\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition unsigned less or equals\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bcc 0x2000", "\x1a\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition unsigned greater or equals\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bcc,a 0x2000", "\x3a\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition unsigned greater or equals\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bcs 0x2000", "\x0a\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition unsigned less\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bcs,a 0x2000", "\x2a\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition unsigned less\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bpos 0x2000", "\x1c\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition plus\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bpos,a 0x2000", "\x3c\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition plus\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bneg 0x2000", "\x0c\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition minus\n"
                "High level: %flags\n"
    );

    TEST_DECODE("bneg,a 0x2000", "\x2c\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition minus\n"
                "High level: %flags\n"
    );

    // TODO
//     TEST_DECODE("bvc 0x2000", "\x1e\x80\x04\x00", ICLASS::SCD,
//                 "0x00001000    0 BRANCH 0x00002000, condition equals\n"
//                 "High level: %flags\n"
//     );
//
//     TEST_DECODE("bvc,a 0x2000", "\x3e\x80\x04\x00", ICLASS::SCDAN,
//                 "0x00001000    0 BRANCH 0x00002000, condition equals\n"
//                 "High level: %flags\n"
//     );
//
//     TEST_DECODE("bvs 0x2000", "\x0e\x80\x04\x00", ICLASS::SCD,
//                 "0x00001000    0 BRANCH 0x00002000, condition equals\n"
//                 "High level: %flags\n"
//     );
//
//     TEST_DECODE("bvs,a 0x2000", "\x2e\x80\x04\x00", ICLASS::SCDAN,
//                 "0x00001000    0 BRANCH 0x00002000, condition equals\n"
//                 "High level: %flags\n"
//     );

    TEST_DECODE("call 0x2000", "\x40\x00\x10\x00", ICLASS::SD,
                "0x00001000    0 <all> := CALL 0x5000(<all>)\n"
                "              Reaching definitions: <None>\n"
                "              Live variables: <None>\n"
    );

    // TODO CBcc

    // TODO CPop

    TEST_DECODE("fabss %f1, %f2", "\x85\xa0\x01\x21", ICLASS::NCT,
                "0x00001000    0 *32* r34 := (r33 < 0) ? -r33 : r33\n"
    );

    // TODO faddd

    TEST_DECODE("fadds %f3, %f1, %f2", "\x85\xa0\xc8\x21", ICLASS::NCT,
                "0x00001000    0 *32* r34 := r35 +f r33\n"
    );

    // TODO faddx

    // TODO fbfcc

    // TODO fcmpd

    // TODO fcmped

    // TODO fcmpes

    // TODO fcmpexx

    // TODO fcmps

    // TODO fcmpx

    // TODO fdivd

    // TODO fdivs

    // TODO fdivx

    // TODO fdtoi

    // TODO fdtos

    // TODO fdtox

    // TODO fitod

    // TODO fitos

    // TODO fitox

    // TODO fmovs

    // TODO fmuld

    // TODO fmuls

    // TODO fmulx

    // TODO fnegs

    // TODO fsqrtd

    // TODO fsqrts

    // TODO fsqrtx

    // TODO fstod

    // TODO fstoi

    // TODO fstox

    // TODO fsubd

    // TODO fsubs

    // TODO fsubx

    // TODO fxtod

    // TODO fxtoi

    // TODO fxtos

    // TODO iflush

    // TODO jmpl

    // TODO ld

    // TODO lda

    // TODO ldc

    // TODO ldcsr

    // TODO ldd

    // TODO ldda

    // TODO lddc

    // TODO lddf

    // TODO ldf

    // TODO ldfsr

    // TODO ldsb

    // TODO ldsba

    // TODO ldsh

    // TODO ldsha

    // TODO ldstub

    // TODO ldstuba

    // TODO ldub

    // TODO lduba

    // TODO lduh

    // TODO lduha

    // TODO mulscc

    TEST_DECODE("or %g3, %g1, %g2", "\x84\x10\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 | r1\n"
    );

    TEST_DECODE("or %g3, 1, %g2", "\x84\x10\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 | 1\n"
    );

    TEST_DECODE("orcc %g3, %g1, %g2", "\x84\x90\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 | r1\n"
                "              0 *v* %flags := LOGICALFLAGS( r2 )\n"
    );

    TEST_DECODE("orcc %g3, 1, %g2", "\x84\x90\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 | 1\n"
                "              0 *v* %flags := LOGICALFLAGS( r2 )\n"
    );

    TEST_DECODE("orn %g3, %g1, %g2", "\x84\x30\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 | ~r1\n"
    );

    TEST_DECODE("orn %g3, 1, %g2", "\x84\x30\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 | -2\n"
    );

    TEST_DECODE("orncc %g3, %g1, %g2", "\x84\xb0\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 | ~r1\n"
                "              0 *v* %flags := LOGICALFLAGS( r2 )\n"
    );

    TEST_DECODE("orncc %g3, 1, %g2", "\x84\xb0\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 | -2\n"
                "              0 *v* %flags := LOGICALFLAGS( r2 )\n"
    );

    // TODO rdpsr

    // TODO rdtbr

    // TODO rdwim

    // TODO rdy

    // TODO restore

    // TODO rett

    // TODO save

    // TODO sethi

    // TODO sll

    // TODO sra

    // TODO srl

    // TODO st

    // TODO sta

    // TODO stb

    // TODO stba

    // TODO stc

    // TODO stcsr

    // TODO std

    // TODO stda

    // TODO stdc

    // TODO stdcq

    // TODO stdf

    // TODO stdfq

    // TODO stf

    // TODO stfsr

    // TODO sth

    // TODO stha

    // TODO sub

    // TODO subcc

    // TODO subx

    // TODO subxcc

    // TODO swap

    // TODO swapa

    // TODO taddcc

    // TODO taddcctv

    // TODO ticc

    // TODO tsubcc

    // TODO tsubcctv

    // TODO unimp

    // TODO wrpsr

    // TODO wrtbr

    // TODO wrwim

    // TODO wry

    TEST_DECODE("xnor %g3, %g1, %g2", "\x84\x38\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 ^ ~r1\n"
    );

    TEST_DECODE("xnor %g3, 1, %g2", "\x84\x38\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 ^ -2\n"
    );

    TEST_DECODE("xnorcc %g3, %g1, %g2", "\x84\xb8\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 ^ ~r1\n"
                "              0 *v* %flags := LOGICALFLAGS( r2 )\n"
    );

    TEST_DECODE("xnorcc %g3, 1, %g2", "\x84\xb8\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 ^ -2\n"
                "              0 *v* %flags := LOGICALFLAGS( r2 )\n"
    );

    TEST_DECODE("xor %g3, %g1, %g2", "\x84\x18\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 ^ r1\n"
    );

    TEST_DECODE("xor %g3, 1, %g2", "\x84\x18\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 ^ 1\n"
    );

    TEST_DECODE("xorcc %g3, %g1, %g2", "\x84\x98\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 ^ r1\n"
                "              0 *v* %flags := LOGICALFLAGS( r2 )\n"
    );

    TEST_DECODE("xorcc %g3, 1, %g2", "\x84\x98\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 ^ 1\n"
                "              0 *v* %flags := LOGICALFLAGS( r2 )\n"
    );
}


QTEST_GUILESS_MAIN(SPARCDecoderTest)
