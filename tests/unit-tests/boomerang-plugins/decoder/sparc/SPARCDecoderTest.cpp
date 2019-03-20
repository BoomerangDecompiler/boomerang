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

    TEST_DECODE("fcmpd %f1, %f2", "\x81\xa8\x4a\x42", ICLASS::NCT,
                "0x00001000    0 *64* tmpd := r64 -f r65\n"
                "              0 *v* %fflags := SETFFLAGS( r64, r65 )\n"
    );

    TEST_DECODE("fcmped %f1, %f2", "\x81\xa8\x4a\xc2", ICLASS::NCT,
                "0x00001000    0 *64* tmpd := r64 -f r65\n"
                "              0 *v* %fflags := SETFFLAGS( r64, r65 )\n"
    );

    TEST_DECODE("fcmpes %f1, %f2", "\x81\xa8\x4a\xa2", ICLASS::NCT,
                "0x00001000    0 *32* tmpf := r33 -f r34\n"
                "              0 *v* %fflags := SETFFLAGS( r33, r34 )\n"
    );

//     TEST_DECODE("fcmpex %f1, %f2", "\x81\xa8\x4e\xe2", ICLASS::NCT,
//                 ""
//     );

    TEST_DECODE("fcmps %f1, %f2", "\x81\xa8\x4a\x22", ICLASS::NCT,
                "0x00001000    0 *32* tmpf := r33 -f r34\n"
                "              0 *v* %fflags := SETFFLAGS( r33, r34 )\n"
    );

    TEST_DECODE("fcmpx %f4, %f0", "\x81\xa9\x0a\x60", ICLASS::NCT,
                "0x00001000    0 *128* tmpD := r81 -f r80\n"
                "              0 *v* %fflags := SETFFLAGS( r81, r80 )\n"
    );

    TEST_DECODE("fdivd %f4, %f0, %f2", "\x85\xa1\x09\xc0", ICLASS::NCT,
                "0x00001000    0 *64* r65 := r66 /f r64\n"
    );

    TEST_DECODE("fdivs %f3, %f1, %f2", "\x85\xa0\xc9\xa1", ICLASS::NCT,
                "0x00001000    0 *32* r34 := r35 /f r33\n"
    );

    TEST_DECODE("fdivx %f8, %f0, %f4", "\x89\xa2\x09\xe0", ICLASS::NCT,
                "0x00001000    0 *128* r81 := r82 /f r80\n"
    );

    TEST_DECODE("fdtoi %f2, %f0", "\x81\xa0\x1a\x42", ICLASS::NCT,
                "0x00001000    0 *32* r32 := ftoi(64, 32, r65)\n"
    );

    TEST_DECODE("fdtos %f2, %f0", "\x81\xa0\x18\xc2", ICLASS::NCT,
                "0x00001000    0 *32* r32 := fsize(64, 32, r65)\n"
    );

    TEST_DECODE("fdtox %f2, %f0", "\x81\xa0\x19\xc2", ICLASS::NCT,
                "0x00001000    0 *128* r80 := fsize(64, 128, r65)\n"
    );

    TEST_DECODE("fitod %f1, %f2", "\x85\xa0\x19\x01", ICLASS::NCT,
                "0x00001000    0 *64* r65 := itof(32, 64, r33)\n"
    );

    TEST_DECODE("fitos %f2, %f1", "\x83\xa0\x18\x82", ICLASS::NCT,
                "0x00001000    0 *32* r33 := itof(32, 32, r34)\n"
    );

    TEST_DECODE("fitox %f1, %f4", "\x89\xa0\x19\x81", ICLASS::NCT,
                "0x00001000    0 *128* r81 := itof(32, 128, r33)\n"
    );

    TEST_DECODE("fmovs %f1, %f2", "\x85\xa0\x00\x21", ICLASS::NCT,
                "0x00001000    0 *32* r34 := r33\n"
    );

    TEST_DECODE("fmuld %f2, %f4, %f6", "\x8d\xa0\x89\x44", ICLASS::NCT,
                "0x00001000    0 *64* r67 := r65 *f r66\n"
    );

    TEST_DECODE("fmuls %f3, %f1, %f2", "\x85\xa0\xc9\x21", ICLASS::NCT,
                "0x00001000    0 *32* r34 := r35 *f r33\n"
    );

    TEST_DECODE("fmulx %f4, %f8, %f0", "\x81\xa1\x09\x68", ICLASS::NCT,
                "0x00001000    0 *128* r80 := r81 *f r82\n"
    );

    TEST_DECODE("fnegs %f1, %f2", "\x85\xa0\x00\xa1", ICLASS::NCT,
                "0x00001000    0 *32* r34 := -r33\n"
    );

    TEST_DECODE("fsqrtd %f4, %f2", "\x85\xa0\x05\x44", ICLASS::NCT,
                "0x00001000    0 *64* r65 := sqrt(r66)\n"
    );

    TEST_DECODE("fsqrts %f2, %f1", "\x83\xa0\x05\x22", ICLASS::NCT,
                "0x00001000    0 *32* r33 := sqrt(r34)\n"
    );

    TEST_DECODE("fsqrtx %f4, %f0", "\x81\xa0\x05\x64", ICLASS::NCT,
                "0x00001000    0 *128* r80 := sqrt(r81)\n"
    );

    TEST_DECODE("fstod %f1, %f2", "\x85\xa0\x19\x21", ICLASS::NCT,
                "0x00001000    0 *64* r65 := fsize(32, 64, r33)\n"
    );

    TEST_DECODE("fstoi %f1, %f2", "\x85\xa0\x1a\x21", ICLASS::NCT,
                "0x00001000    0 *32* r34 := ftoi(32, 32, r33)\n"
    );

    TEST_DECODE("fstox %f1, %f4", "\x89\xa0\x19\xa1", ICLASS::NCT,
                "0x00001000    0 *128* r81 := fsize(32, 128, r33)\n"
    );

    TEST_DECODE("fsubd %f4, %f2, %f0", "\x81\xa1\x08\xc2", ICLASS::NCT,
                "0x00001000    0 *64* r64 := r66 -f r65\n"
    );

    TEST_DECODE("fsubs %f3, %f1, %f2", "\x85\xa0\xc8\xa1", ICLASS::NCT,
                "0x00001000    0 *32* r34 := r35 -f r33\n"
    );

    TEST_DECODE("fsubx %f0, %f4, %f8", "\x91\xa0\x08\xe4", ICLASS::NCT,
                "0x00001000    0 *128* r82 := r80 -f r81\n"
    );

    TEST_DECODE("fxtod %f4, %f2", "\x85\xa0\x19\x64", ICLASS::NCT,
                "0x00001000    0 *64* r65 := fsize(128, 64, r81)\n"
    );

    TEST_DECODE("fxtoi %f4, %f1", "\x83\xa0\x1a\x64", ICLASS::NCT,
                "0x00001000    0 *32* r33 := ftoi(128, 32, r81)\n"
    );

    TEST_DECODE("fxtos %f4, %f1", "\x83\xa0\x18\xe4", ICLASS::NCT,
                "0x00001000    0 *32* r33 := fsize(128, 32, r81)\n"
    );

    // TODO iflush

    // FIXME: The semantics are wrong. The return address should be saved to %g4;
    // sometimes this instruction might also represent an indirect call or a return.
    TEST_DECODE("jmpl %g1+%g2, %g4", "\x89\xc0\x40\x02", ICLASS::DD,
                "0x00001000    0 CASE [r1 + r2]\n"
    );

    TEST_DECODE("jmpl %g1+0x0800, %g4", "\x89\xc0\x68\x00", ICLASS::DD,
                "0x00001000    0 CASE [r1 + 0x800]\n"
    );

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

    TEST_DECODE("mulscc %g3, %g1, %g2", "\x85\x20\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := (r3 >> 1) | (((%NF ^ %OF) = 1) ? 0xffffffff80000000 : 0)\n"
                "              0 *32* tmp2 := ((machine(\"%Y\")@[0:0]) = 1) ? r1 : 0\n"
                "              0 *32* machine(\"%Y\") := (machine(\"%Y\") >> 1) | (r3 << 31)\n"
                "              0 *32* r2 := tmp + tmp2\n"
                "              0 *v* %flags := ADDFLAGS( tmp, tmp2, r2 )\n"
    );

    TEST_DECODE("mulscc %g3, 3, %g2", "\x85\x20\xe0\x03", ICLASS::NCT,
                "0x00001000    0 *32* tmp := (r3 >> 1) | (((%NF ^ %OF) = 1) ? 0xffffffff80000000 : 0)\n"
                "              0 *32* tmp2 := ((machine(\"%Y\")@[0:0]) = 1) ? 3 : 0\n"
                "              0 *32* machine(\"%Y\") := (machine(\"%Y\") >> 1) | (r3 << 31)\n"
                "              0 *32* r2 := tmp + tmp2\n"
                "              0 *v* %flags := ADDFLAGS( tmp, tmp2, r2 )\n"
    );

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
