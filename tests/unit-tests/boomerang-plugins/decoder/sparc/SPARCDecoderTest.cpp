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

    Plugin *plugin = m_project.getPluginManager()->getPluginByName("Capstone SPARC decoder plugin");
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

    TEST_DECODE("call 0x5000", "\x40\x00\x10\x00", ICLASS::SD,
                "0x00001000    0 <all> := CALL 0x5000(<all>)\n"
                "              Reaching definitions: <None>\n"
                "              Live variables: <None>\n"
    );

    TEST_DECODE("call %g3", "\x9f\xc0\xc0\x00", ICLASS::DD,
                "0x00001000    0 <all> := CALL r3(<all>)\n"
                "              Reaching definitions: <None>\n"
                "              Live variables: <None>\n"
    );

    // TODO CBcc

    // TODO CPop

//     TEST_DECODE("divscc %g3, %g1, %g2", "\x84\xe8\xc0\x01", ICLASS::NCT,
//                 ""
//     );
//
//     TEST_DECODE("divscc %g3, 3, %g2", "\x84\xe8\xe0\x03", ICLASS::NCT,
//                 ""
//     );

    TEST_DECODE("fabss %f1, %f2", "\x85\xa0\x01\x21", ICLASS::NCT,
                "0x00001000    0 *32* r34 := (r33 < 0) ? -r33 : r33\n"
    );

    // TODO faddd

    TEST_DECODE("fadds %f3, %f1, %f2", "\x85\xa0\xc8\x21", ICLASS::NCT,
                "0x00001000    0 *32* r34 := r35 +f r33\n"
    );

    // TODO faddx

    TEST_DECODE("fba 0x2000", "\x01\x80\x04\x00", ICLASS::NCT,
                "0x00001000    0 GOTO 0x00002000\n"
    );

    TEST_DECODE("fba,a 0x2000", "\x21\x80\x04\x00", ICLASS::SKIP,
                "0x00001000    0 GOTO 0x00002000\n"
    );

    TEST_DECODE("fbn 0x2000", "\x11\x80\x04\x00", ICLASS::SD,
                "0x00001000    0 GOTO 0x00002000\n"
    );

    TEST_DECODE("fbn,a 0x2000", "\x31\x80\x04\x00", ICLASS::SU,
                "0x00001000    0 GOTO 0x00002000\n"
    );

//     TEST_DECODE("fbu 0x2000", "\x0f\x80\x04\x00", ICLASS::SCD,
//                 ""
//     );
//
//     TEST_DECODE("fbu,a 0x2000", "\x2f\x80\x04\x00", ICLASS::SCDAN,
//                 ""
//     );

    TEST_DECODE("fbg 0x2000", "\x0d\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition signed greater float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbg,a 0x2000", "\x2d\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition signed greater float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbug 0x2000", "\x0b\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition signed greater float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbug,a 0x2000", "\x2b\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition signed greater float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbl 0x2000", "\x09\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition signed less float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbl,a 0x2000", "\x29\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition signed less float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbul 0x2000", "\x07\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition signed less float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbul,a 0x2000", "\x27\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition signed less float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fblg 0x2000", "\x05\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition not equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fblg,a 0x2000", "\x25\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition not equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbne 0x2000", "\x03\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition not equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbne,a 0x2000", "\x23\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition not equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbe 0x2000", "\x13\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbe,a 0x2000", "\x33\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbue 0x2000", "\x15\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbue,a 0x2000", "\x35\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbge 0x2000", "\x17\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition signed greater or equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbge,a 0x2000", "\x37\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition signed greater or equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbuge 0x2000", "\x19\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition signed greater or equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbuge,a 0x2000", "\x39\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition signed greater or equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fble 0x2000", "\x1b\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition signed less or equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fble,a 0x2000", "\x3b\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition signed less or equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbule 0x2000", "\x1d\x80\x04\x00", ICLASS::SCD,
                "0x00001000    0 BRANCH 0x00002000, condition signed less or equals float\n"
                "High level: %fflags\n"
    );

    TEST_DECODE("fbule,a 0x2000", "\x3d\x80\x04\x00", ICLASS::SCDAN,
                "0x00001000    0 BRANCH 0x00002000, condition signed less or equals float\n"
                "High level: %fflags\n"
    );

//     TEST_DECODE("fbo 0x2000", "\x1f\x80\x04\x00", ICLASS::SCD,
//                 ""
//     );
//
//     TEST_DECODE("fbo,a 0x2000", "\x3f\x80\x04\x00", ICLASS::SCDAN,
//                 ""
//     );

    TEST_DECODE("fcmpd %f2, %f4", "\x81\xa8\x8a\x44", ICLASS::NCT,
                "0x00001000    0 *64* tmpd := r65 -f r66\n"
                "              0 *v* %fflags := SETFFLAGS( r65, r66 )\n"
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

    TEST_DECODE("fcmpq %f4, %f0", "\x81\xa9\x0a\x60", ICLASS::NCT,
                "0x00001000    0 *128* tmpD := r81 -f r80\n"
                "              0 *v* %fflags := SETFFLAGS( r81, r80 )\n"
    );

    TEST_DECODE("fdivd %f4, %f0, %f2", "\x85\xa1\x09\xc0", ICLASS::NCT,
                "0x00001000    0 *64* r65 := r66 /f r64\n"
    );

    TEST_DECODE("fdivs %f3, %f1, %f2", "\x85\xa0\xc9\xa1", ICLASS::NCT,
                "0x00001000    0 *32* r34 := r35 /f r33\n"
    );

    TEST_DECODE("fdivq %f8, %f0, %f4", "\x89\xa2\x09\xe0", ICLASS::NCT,
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

    TEST_DECODE("jmp 0x800", "\x81\xc0\x28\x00", ICLASS::SD,
                "0x00001000    0 GOTO 0x00000800\n"
    );

    // FIXME: The semantics are wrong. The return address should be saved to %g4;
    // sometimes this instruction might also represent an indirect call or a return.
    TEST_DECODE("jmpl %g1+%g2, %g4", "\x89\xc0\x40\x02", ICLASS::DD,
                "0x00001000    0 CASE [r1 + r2]\n"
    );

    TEST_DECODE("jmpl %g1+0x0800, %g4", "\x89\xc0\x68\x00", ICLASS::DD,
                "0x00001000    0 CASE [r1 + 0x800]\n"
    );

    TEST_DECODE("jmpl 0x800, %o7", "\x9f\xc0\x28\x00", ICLASS::SD,
                "0x00001000    0 <all> := CALL 0x800(<all>)\n"
                "              Reaching definitions: <None>\n"
                "              Live variables: <None>\n"
    );

    TEST_DECODE("ld [%g0], %g1", "\xc2\x00\x20\x00", ICLASS::NCT,
                "0x00001000    0 *32* r1 := m[0]\n"  //< TODO shouldn't this read m[%g0] ?
    );

    TEST_DECODE("ld [0xFFFFFFFF], %g1", "\xc2\x00\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *32* r1 := m[-1]\n"
    );

    TEST_DECODE("ld [%g3], %g1", "\xc2\x00\xe0\x00", ICLASS::NCT,
                "0x00001000    0 *32* r1 := m[r3]\n"
    );

    TEST_DECODE("ld [%g3 + 0x10], %g1", "\xc2\x00\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *32* r1 := m[r3 + 16]\n"
    );

    TEST_DECODE("ld [%g3 + %g1], %g2", "\xc4\x00\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := m[r3 + r1]\n"
    );

    // TODO lda

    // TODO ldc

    // TODO ldcsr

    TEST_DECODE("ldd [0], %g2", "\xc4\x18\x20\x00", ICLASS::NCT,
                "0x00001000    0 *32* r2 := m[0]\n"
                "              0 *32* r3 := m[4]\n"
    );

    TEST_DECODE("ldd [0xFFFFFFFF], %g2", "\xc4\x18\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *32* r2 := m[-1]\n"
                "              0 *32* r3 := m[3]\n"
    );

    TEST_DECODE("ldd [%g3], %g2", "\xc4\x18\xe0\x00", ICLASS::NCT,
                "0x00001000    0 *32* r2 := m[r3]\n"
                "              0 *32* r3 := m[r3 + 4]\n"
    );

    TEST_DECODE("ldd [%g3 + 0x10], %g2", "\xc4\x18\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *32* r2 := m[r3 + 16]\n"
                "              0 *32* r3 := m[r3 + 20]\n"
    );

    TEST_DECODE("ldd [%g3 + %g1], %g2", "\xc4\x18\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := m[r3 + r1]\n"
                "              0 *32* r3 := m[(r3 + r1) + 4]\n"
    );

    // TODO ldda

    // TODO lddc

    TEST_DECODE("lddf [0], %f2", "\xc5\x18\x20\x00", ICLASS::NCT,
                "0x00001000    0 *64* r65 := m[0]\n"
    );

    TEST_DECODE("lddf [0xFFFFFFFF], %f2", "\xc5\x18\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *64* r65 := m[-1]\n"
    );

    TEST_DECODE("lddf [%g3], %f2", "\xc5\x18\xe0\x00", ICLASS::NCT,
                "0x00001000    0 *64* r65 := m[r3]\n"
    );

    TEST_DECODE("lddf [%g3 + 0x10], %f2", "\xc5\x18\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *64* r65 := m[r3 + 16]\n"
    );

    TEST_DECODE("lddf [%g3 + %g1], %f2", "\xc5\x18\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *64* r65 := m[r3 + r1]\n"
    );

    TEST_DECODE("ldf [0], %f2", "\xc5\x00\x20\x00", ICLASS::NCT,
                "0x00001000    0 *32* r34 := m[0]\n"
    );

    TEST_DECODE("ldf [0xFFFFFFFF], %f2", "\xc5\x00\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *32* r34 := m[-1]\n"
    );

    TEST_DECODE("ldf [%g3], %f2", "\xc5\x00\xe0\x00", ICLASS::NCT,
                "0x00001000    0 *32* r34 := m[r3]\n"
    );

    TEST_DECODE("ldf [%g3 + 0x10], %f2", "\xc5\x00\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *32* r34 := m[r3 + 16]\n"
    );

    TEST_DECODE("ldf [%g3 + %g1], %f2", "\xc5\x00\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r34 := m[r3 + r1]\n"
    );

//     TEST_DECODE("ld [0], %fsr", "\xc5\x08\x20\x00", ICLASS::NCT,
//                 "0x00001000    0 *32* machine(\"%FSR\") := m[0]\n"
//     );
//
//     TEST_DECODE("ld [0xFFFFFFFF], %fsr", "\xc5\x08\x3f\xff", ICLASS::NCT,
//                 "0x00001000    0 *32* machine(\"%FSR\") := m[-1]\n"
//     );
//
//     TEST_DECODE("ld [%g3], %fsr", "\xc5\x08\xe0\x00", ICLASS::NCT,
//                 "0x00001000    0 *32* machine(\"%FSR\") := m[r3]\n"
//     );
//
//     TEST_DECODE("ld [%g3 + 0x10], %fsr", "\xc5\x08\xe0\x10", ICLASS::NCT,
//                 "0x00001000    0 *32* machine(\"%FSR\") := m[r3 + 16]\n"
//     );
//
//     TEST_DECODE("ld [%g3 + %g1], %fsr", "\xc5\x08\xc0\x01", ICLASS::NCT,
//                 "0x00001000    0 *32* machine(\"%FSR\") := m[r3 + r1]\n"
//     );

    TEST_DECODE("ldsb [0], %g1", "\xc2\x48\x20\x00", ICLASS::NCT,
                "0x00001000    0 *32* r1 := sgnex(8, 32, m[0])\n"  //< TODO shouldn't this read m[%g0] ?
    );

    TEST_DECODE("ldsb [0xFFFFFFFF], %g1", "\xc2\x48\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *32* r1 := sgnex(8, 32, m[-1])\n"
    );

    TEST_DECODE("ldsb [%g3], %g1", "\xc2\x48\xc0\x00", ICLASS::NCT,
                "0x00001000    0 *32* r1 := sgnex(8, 32, m[r3])\n"
    );

    TEST_DECODE("ldsb [%g3 + 0x10], %g1", "\xc2\x48\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *32* r1 := sgnex(8, 32, m[r3 + 16])\n"
    );

    TEST_DECODE("ldsb [%g3 + %g1], %g1", "\xc2\x48\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r1 := sgnex(8, 32, m[r3 + r1])\n"
    );

    // TODO ldsba

    TEST_DECODE("ldsh [0], %g1", "\xc2\x50\x20\x00", ICLASS::NCT,
                "0x00001000    0 *32* r1 := sgnex(16, 32, m[0])\n"  //< TODO shouldn't this read m[%g0] ?
    );

    TEST_DECODE("ldsh [0xFFFFFFFF], %g1", "\xc2\x50\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *32* r1 := sgnex(16, 32, m[-1])\n"
    );

    TEST_DECODE("ldsh [%g3], %g1", "\xc2\x50\xc0\x00", ICLASS::NCT,
                "0x00001000    0 *32* r1 := sgnex(16, 32, m[r3])\n"
    );

    TEST_DECODE("ldsh [%g3 + 0x10], %g1", "\xc2\x50\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *32* r1 := sgnex(16, 32, m[r3 + 16])\n"
    );

    TEST_DECODE("ldsh [%g3 + %g1], %g1", "\xc2\x50\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r1 := sgnex(16, 32, m[r3 + r1])\n"
    );

    // TODO ldsha

//     TEST_DECODE("ldstub [0], %g1", "\xc2\x68\x20\x00", ICLASS::NCT,
//                 "0x00001000    0 *32* r1 := zfill(8, 32, m[0])\n"
//                 "              0 *8* m[0] := m[0] | 255\n"  //< TODO shouldn't this read m[%g0] ?
//     );
//
//     TEST_DECODE("ldstub [0xFFFFFFFF], %g1", "\xc2\x68\x3f\xff", ICLASS::NCT,
//                 "0x00001000    0 *32* r1 := zfill(8, 32, m[-1])\n"
//                 "              0 *8* m[-1] := m[-1] | 255\n"
//     );
//
//     TEST_DECODE("ldstub [%g3], %g1", "\xc2\x68\xc0\x00", ICLASS::NCT,
//                 "0x00001000    0 *32* r1 := zfill(8, 32, m[r3])\n"
//                 "              0 *8* m[r3] := m[r3] | 255\n"
//     );
//
//     TEST_DECODE("ldstub [%g3 + 0x10], %g1", "\xc2\x68\xe0\x10", ICLASS::NCT,
//                 "0x00001000    0 *32* r1 := zfill(8, 32, m[r3 + 16])\n"
//                 "              0 *8* m[r3 + 16] := m[r3 + 16] | 255\n"
//     );
//
//     TEST_DECODE("ldstub [%g3 + %g1], %g1", "\xc2\x68\xc0\x01", ICLASS::NCT,
//                 "0x00001000    0 *32* r1 := zfill(8, 32, m[r3 + r1])\n"
//                 "              0 *8* m[r3 + r1] := m[r3 + r1] | 255\n"
//     );

    // TODO ldstuba

    TEST_DECODE("ldub [0], %g1", "\xc2\x08\x20\x00", ICLASS::NCT,
                "0x00001000    0 *32* r1 := zfill(8, 32, m[0])\n" //< TODO shouldn't this read m[%g0] ?
    );

    TEST_DECODE("ldub [0xFFFFFFFF], %g1", "\xc2\x08\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *32* r1 := zfill(8, 32, m[-1])\n"
    );

    TEST_DECODE("ldub [%g3], %g1", "\xc2\x08\xc0\x00", ICLASS::NCT,
                "0x00001000    0 *32* r1 := zfill(8, 32, m[r3])\n"
    );

    TEST_DECODE("ldub [%g3 + 0x10], %g1", "\xc2\x08\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *32* r1 := zfill(8, 32, m[r3 + 16])\n"
    );

    TEST_DECODE("ldub [%g3 + %g1], %g1", "\xc2\x08\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r1 := zfill(8, 32, m[r3 + r1])\n"
    );

    // TODO lduba

    TEST_DECODE("lduh [0], %g1", "\xc2\x10\x20\x00", ICLASS::NCT,
                "0x00001000    0 *32* r1 := zfill(16, 32, m[0])\n" //< TODO shouldn't this read m[%g0] ?
    );

    TEST_DECODE("lduh [0xFFFFFFFF], %g1", "\xc2\x10\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *32* r1 := zfill(16, 32, m[-1])\n"
    );

    TEST_DECODE("lduh [%g3], %g1", "\xc2\x10\xc0\x00", ICLASS::NCT,
                "0x00001000    0 *32* r1 := zfill(16, 32, m[r3])\n"
    );

    TEST_DECODE("lduh [%g3 + 0x10], %g1", "\xc2\x10\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *32* r1 := zfill(16, 32, m[r3 + 16])\n"
    );

    TEST_DECODE("lduh [%g3 + %g1], %g1", "\xc2\x10\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r1 := zfill(16, 32, m[r3 + r1])\n"
    );

    // TODO lduha

//     TEST_DECODE("mulscc %g3, %g1, %g2", "\x85\x20\xc0\x01", ICLASS::NCT,
//                 "0x00001000    0 *32* tmp := (r3 >> 1) | (((%NF ^ %OF) = 1) ? 0xffffffff80000000 : 0)\n"
//                 "              0 *32* tmp2 := ((r100@[0:0]) = 1) ? r1 : 0\n"
//                 "              0 *32* r100 := (r100 >> 1) | (r3 << 31)\n"
//                 "              0 *32* r2 := tmp + tmp2\n"
//                 "              0 *v* %flags := ADDFLAGS( tmp, tmp2, r2 )\n"
//     );
//
//     TEST_DECODE("mulscc %g3, 3, %g2", "\x85\x20\xe0\x03", ICLASS::NCT,
//                 "0x00001000    0 *32* tmp := (r3 >> 1) | (((%NF ^ %OF) = 1) ? 0xffffffff80000000 : 0)\n"
//                 "              0 *32* tmp2 := ((r100@[0:0]) = 1) ? 3 : 0\n"
//                 "              0 *32* r100 := (r100 >> 1) | (r3 << 31)\n"
//                 "              0 *32* r2 := tmp + tmp2\n"
//                 "              0 *v* %flags := ADDFLAGS( tmp, tmp2, r2 )\n"
//     );

    TEST_DECODE("or %g0, %g3, %g2", "\x84\x10\x00\x03", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3\n"
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

//     TEST_DECODE("rd %psr, %g1", "\x83\x48\x00\x00", ICLASS::NCT,
//                 "0x00001000    0 *32* r1 := machine(\"%PSR\")\n"
//     );
//
//     TEST_DECODE("rd %tbr, %g1", "\x83\x58\x00\x00", ICLASS::NCT,
//                 "0x00001000    0 *32* r1 := machine(\"%TBR\")\n"
//     );
//
//     TEST_DECODE("rd %wim, %g1", "\x83\x50\x00\x00", ICLASS::NCT,
//                 "0x00001000    0 *32* r1 := machine(\"%WIM\")\n"
//     );
//
//     TEST_DECODE("rd %Y, %g2", "\x85\x40\x00\x00", ICLASS::NCT,
//                 "0x00001000    0 *32* r2 := r100\n"
//     );

    TEST_DECODE("restore %g0, 0, %g1", "\x83\xe8\x20\x00", ICLASS::NCT,
                "0x00001000    0 *32* tmp := 0\n"
                "              0 *32* r8 := r24\n"
                "              0 *32* r9 := r25\n"
                "              0 *32* r10 := r26\n"
                "              0 *32* r11 := r27\n"
                "              0 *32* r12 := r28\n"
                "              0 *32* r13 := r29\n"
                "              0 *32* r14 := r30\n"
                "              0 *32* r15 := r31\n"
                "              0 *32* r1 := tmp\n"
                "              0 *32* r16 := m[r14]\n"
                "              0 *32* r17 := m[r14 + 4]\n"
                "              0 *32* r18 := m[r14 + 8]\n"
                "              0 *32* r19 := m[r14 + 12]\n"
                "              0 *32* r20 := m[r14 + 16]\n"
                "              0 *32* r21 := m[r14 + 20]\n"
                "              0 *32* r22 := m[r14 + 24]\n"
                "              0 *32* r23 := m[r14 + 28]\n"
                "              0 *32* r24 := m[r14 + 32]\n"
                "              0 *32* r25 := m[r14 + 36]\n"
                "              0 *32* r26 := m[r14 + 40]\n"
                "              0 *32* r27 := m[r14 + 44]\n"
                "              0 *32* r28 := m[r14 + 48]\n"
                "              0 *32* r29 := m[r14 + 52]\n"
                "              0 *32* r30 := m[r14 + 56]\n"
                "              0 *32* r31 := m[r14 + 60]\n"
                "              0 *32* r1 := tmp\n"
    );

    TEST_DECODE("restore %g0, -1, %g1", "\x83\xe8\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *32* tmp := -1\n"
                "              0 *32* r8 := r24\n"
                "              0 *32* r9 := r25\n"
                "              0 *32* r10 := r26\n"
                "              0 *32* r11 := r27\n"
                "              0 *32* r12 := r28\n"
                "              0 *32* r13 := r29\n"
                "              0 *32* r14 := r30\n"
                "              0 *32* r15 := r31\n"
                "              0 *32* r1 := tmp\n"
                "              0 *32* r16 := m[r14]\n"
                "              0 *32* r17 := m[r14 + 4]\n"
                "              0 *32* r18 := m[r14 + 8]\n"
                "              0 *32* r19 := m[r14 + 12]\n"
                "              0 *32* r20 := m[r14 + 16]\n"
                "              0 *32* r21 := m[r14 + 20]\n"
                "              0 *32* r22 := m[r14 + 24]\n"
                "              0 *32* r23 := m[r14 + 28]\n"
                "              0 *32* r24 := m[r14 + 32]\n"
                "              0 *32* r25 := m[r14 + 36]\n"
                "              0 *32* r26 := m[r14 + 40]\n"
                "              0 *32* r27 := m[r14 + 44]\n"
                "              0 *32* r28 := m[r14 + 48]\n"
                "              0 *32* r29 := m[r14 + 52]\n"
                "              0 *32* r30 := m[r14 + 56]\n"
                "              0 *32* r31 := m[r14 + 60]\n"
                "              0 *32* r1 := tmp\n"
    );

    TEST_DECODE("restore %g3, 0, %g1", "\x83\xe8\xe0\x00", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r8 := r24\n"
                "              0 *32* r9 := r25\n"
                "              0 *32* r10 := r26\n"
                "              0 *32* r11 := r27\n"
                "              0 *32* r12 := r28\n"
                "              0 *32* r13 := r29\n"
                "              0 *32* r14 := r30\n"
                "              0 *32* r15 := r31\n"
                "              0 *32* r1 := tmp\n"
                "              0 *32* r16 := m[r14]\n"
                "              0 *32* r17 := m[r14 + 4]\n"
                "              0 *32* r18 := m[r14 + 8]\n"
                "              0 *32* r19 := m[r14 + 12]\n"
                "              0 *32* r20 := m[r14 + 16]\n"
                "              0 *32* r21 := m[r14 + 20]\n"
                "              0 *32* r22 := m[r14 + 24]\n"
                "              0 *32* r23 := m[r14 + 28]\n"
                "              0 *32* r24 := m[r14 + 32]\n"
                "              0 *32* r25 := m[r14 + 36]\n"
                "              0 *32* r26 := m[r14 + 40]\n"
                "              0 *32* r27 := m[r14 + 44]\n"
                "              0 *32* r28 := m[r14 + 48]\n"
                "              0 *32* r29 := m[r14 + 52]\n"
                "              0 *32* r30 := m[r14 + 56]\n"
                "              0 *32* r31 := m[r14 + 60]\n"
                "              0 *32* r1 := tmp\n"
    );

    TEST_DECODE("restore %g3, 0x10, %g1", "\x83\xe8\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3 + 16\n"
                "              0 *32* r8 := r24\n"
                "              0 *32* r9 := r25\n"
                "              0 *32* r10 := r26\n"
                "              0 *32* r11 := r27\n"
                "              0 *32* r12 := r28\n"
                "              0 *32* r13 := r29\n"
                "              0 *32* r14 := r30\n"
                "              0 *32* r15 := r31\n"
                "              0 *32* r1 := tmp\n"
                "              0 *32* r16 := m[r14]\n"
                "              0 *32* r17 := m[r14 + 4]\n"
                "              0 *32* r18 := m[r14 + 8]\n"
                "              0 *32* r19 := m[r14 + 12]\n"
                "              0 *32* r20 := m[r14 + 16]\n"
                "              0 *32* r21 := m[r14 + 20]\n"
                "              0 *32* r22 := m[r14 + 24]\n"
                "              0 *32* r23 := m[r14 + 28]\n"
                "              0 *32* r24 := m[r14 + 32]\n"
                "              0 *32* r25 := m[r14 + 36]\n"
                "              0 *32* r26 := m[r14 + 40]\n"
                "              0 *32* r27 := m[r14 + 44]\n"
                "              0 *32* r28 := m[r14 + 48]\n"
                "              0 *32* r29 := m[r14 + 52]\n"
                "              0 *32* r30 := m[r14 + 56]\n"
                "              0 *32* r31 := m[r14 + 60]\n"
                "              0 *32* r1 := tmp\n"
    );

    TEST_DECODE("restore %g3, %g1, %g1", "\x83\xe8\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3 + r1\n"
                "              0 *32* r8 := r24\n"
                "              0 *32* r9 := r25\n"
                "              0 *32* r10 := r26\n"
                "              0 *32* r11 := r27\n"
                "              0 *32* r12 := r28\n"
                "              0 *32* r13 := r29\n"
                "              0 *32* r14 := r30\n"
                "              0 *32* r15 := r31\n"
                "              0 *32* r1 := tmp\n"
                "              0 *32* r16 := m[r14]\n"
                "              0 *32* r17 := m[r14 + 4]\n"
                "              0 *32* r18 := m[r14 + 8]\n"
                "              0 *32* r19 := m[r14 + 12]\n"
                "              0 *32* r20 := m[r14 + 16]\n"
                "              0 *32* r21 := m[r14 + 20]\n"
                "              0 *32* r22 := m[r14 + 24]\n"
                "              0 *32* r23 := m[r14 + 28]\n"
                "              0 *32* r24 := m[r14 + 32]\n"
                "              0 *32* r25 := m[r14 + 36]\n"
                "              0 *32* r26 := m[r14 + 40]\n"
                "              0 *32* r27 := m[r14 + 44]\n"
                "              0 *32* r28 := m[r14 + 48]\n"
                "              0 *32* r29 := m[r14 + 52]\n"
                "              0 *32* r30 := m[r14 + 56]\n"
                "              0 *32* r31 := m[r14 + 60]\n"
                "              0 *32* r1 := tmp\n"
    );

    TEST_DECODE("ret", "\x81\xc7\xe0\x08", ICLASS::DD,
                "0x00001000    0 RET\n"
                "              Modifieds: <None>\n"
                "              Reaching definitions: <None>\n"
    );

    // TODO rett

    TEST_DECODE("save %g0, 0, %g1", "\x83\xe0\x20\x00", ICLASS::NCT,
                "0x00001000    0 *32* tmp := 0\n"
                "              0 *32* m[r14] := r16\n"
                "              0 *32* m[r14 + 4] := r17\n"
                "              0 *32* m[r14 + 8] := r18\n"
                "              0 *32* m[r14 + 12] := r19\n"
                "              0 *32* m[r14 + 16] := r20\n"
                "              0 *32* m[r14 + 20] := r21\n"
                "              0 *32* m[r14 + 24] := r22\n"
                "              0 *32* m[r14 + 28] := r23\n"
                "              0 *32* m[r14 + 32] := r24\n"
                "              0 *32* m[r14 + 36] := r25\n"
                "              0 *32* m[r14 + 40] := r26\n"
                "              0 *32* m[r14 + 44] := r27\n"
                "              0 *32* m[r14 + 48] := r28\n"
                "              0 *32* m[r14 + 52] := r29\n"
                "              0 *32* m[r14 + 56] := r30\n"
                "              0 *32* m[r14 + 60] := r31\n"
                "              0 *32* r24 := r8\n"
                "              0 *32* r25 := r9\n"
                "              0 *32* r26 := r10\n"
                "              0 *32* r27 := r11\n"
                "              0 *32* r28 := r12\n"
                "              0 *32* r29 := r13\n"
                "              0 *32* r30 := r14\n"
                "              0 *32* r31 := r15\n"
                "              0 *32* r1 := tmp\n"
    );

    TEST_DECODE("save %g0, -1, %g1", "\x83\xe0\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *32* tmp := -1\n"
                "              0 *32* m[r14] := r16\n"
                "              0 *32* m[r14 + 4] := r17\n"
                "              0 *32* m[r14 + 8] := r18\n"
                "              0 *32* m[r14 + 12] := r19\n"
                "              0 *32* m[r14 + 16] := r20\n"
                "              0 *32* m[r14 + 20] := r21\n"
                "              0 *32* m[r14 + 24] := r22\n"
                "              0 *32* m[r14 + 28] := r23\n"
                "              0 *32* m[r14 + 32] := r24\n"
                "              0 *32* m[r14 + 36] := r25\n"
                "              0 *32* m[r14 + 40] := r26\n"
                "              0 *32* m[r14 + 44] := r27\n"
                "              0 *32* m[r14 + 48] := r28\n"
                "              0 *32* m[r14 + 52] := r29\n"
                "              0 *32* m[r14 + 56] := r30\n"
                "              0 *32* m[r14 + 60] := r31\n"
                "              0 *32* r24 := r8\n"
                "              0 *32* r25 := r9\n"
                "              0 *32* r26 := r10\n"
                "              0 *32* r27 := r11\n"
                "              0 *32* r28 := r12\n"
                "              0 *32* r29 := r13\n"
                "              0 *32* r30 := r14\n"
                "              0 *32* r31 := r15\n"
                "              0 *32* r1 := tmp\n"
    );

    TEST_DECODE("save %g3, 0, %g1", "\x83\xe0\xe0\x00", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* m[r14] := r16\n"
                "              0 *32* m[r14 + 4] := r17\n"
                "              0 *32* m[r14 + 8] := r18\n"
                "              0 *32* m[r14 + 12] := r19\n"
                "              0 *32* m[r14 + 16] := r20\n"
                "              0 *32* m[r14 + 20] := r21\n"
                "              0 *32* m[r14 + 24] := r22\n"
                "              0 *32* m[r14 + 28] := r23\n"
                "              0 *32* m[r14 + 32] := r24\n"
                "              0 *32* m[r14 + 36] := r25\n"
                "              0 *32* m[r14 + 40] := r26\n"
                "              0 *32* m[r14 + 44] := r27\n"
                "              0 *32* m[r14 + 48] := r28\n"
                "              0 *32* m[r14 + 52] := r29\n"
                "              0 *32* m[r14 + 56] := r30\n"
                "              0 *32* m[r14 + 60] := r31\n"
                "              0 *32* r24 := r8\n"
                "              0 *32* r25 := r9\n"
                "              0 *32* r26 := r10\n"
                "              0 *32* r27 := r11\n"
                "              0 *32* r28 := r12\n"
                "              0 *32* r29 := r13\n"
                "              0 *32* r30 := r14\n"
                "              0 *32* r31 := r15\n"
                "              0 *32* r1 := tmp\n"
    );

    TEST_DECODE("save %g3, 0x10, %g1", "\x83\xe0\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3 + 16\n"
                "              0 *32* m[r14] := r16\n"
                "              0 *32* m[r14 + 4] := r17\n"
                "              0 *32* m[r14 + 8] := r18\n"
                "              0 *32* m[r14 + 12] := r19\n"
                "              0 *32* m[r14 + 16] := r20\n"
                "              0 *32* m[r14 + 20] := r21\n"
                "              0 *32* m[r14 + 24] := r22\n"
                "              0 *32* m[r14 + 28] := r23\n"
                "              0 *32* m[r14 + 32] := r24\n"
                "              0 *32* m[r14 + 36] := r25\n"
                "              0 *32* m[r14 + 40] := r26\n"
                "              0 *32* m[r14 + 44] := r27\n"
                "              0 *32* m[r14 + 48] := r28\n"
                "              0 *32* m[r14 + 52] := r29\n"
                "              0 *32* m[r14 + 56] := r30\n"
                "              0 *32* m[r14 + 60] := r31\n"
                "              0 *32* r24 := r8\n"
                "              0 *32* r25 := r9\n"
                "              0 *32* r26 := r10\n"
                "              0 *32* r27 := r11\n"
                "              0 *32* r28 := r12\n"
                "              0 *32* r29 := r13\n"
                "              0 *32* r30 := r14\n"
                "              0 *32* r31 := r15\n"
                "              0 *32* r1 := tmp\n"
    );

    TEST_DECODE("save %g3, %g1, %g1", "\x83\xe0\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3 + r1\n"
                "              0 *32* m[r14] := r16\n"
                "              0 *32* m[r14 + 4] := r17\n"
                "              0 *32* m[r14 + 8] := r18\n"
                "              0 *32* m[r14 + 12] := r19\n"
                "              0 *32* m[r14 + 16] := r20\n"
                "              0 *32* m[r14 + 20] := r21\n"
                "              0 *32* m[r14 + 24] := r22\n"
                "              0 *32* m[r14 + 28] := r23\n"
                "              0 *32* m[r14 + 32] := r24\n"
                "              0 *32* m[r14 + 36] := r25\n"
                "              0 *32* m[r14 + 40] := r26\n"
                "              0 *32* m[r14 + 44] := r27\n"
                "              0 *32* m[r14 + 48] := r28\n"
                "              0 *32* m[r14 + 52] := r29\n"
                "              0 *32* m[r14 + 56] := r30\n"
                "              0 *32* m[r14 + 60] := r31\n"
                "              0 *32* r24 := r8\n"
                "              0 *32* r25 := r9\n"
                "              0 *32* r26 := r10\n"
                "              0 *32* r27 := r11\n"
                "              0 *32* r28 := r12\n"
                "              0 *32* r29 := r13\n"
                "              0 *32* r30 := r14\n"
                "              0 *32* r31 := r15\n"
                "              0 *32* r1 := tmp\n"
    );

    TEST_DECODE("save %sp, -0x70, %sp", "\x9d\xe3\xbf\x90", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r14 - 112\n"
                "              0 *32* m[r14] := r16\n"
                "              0 *32* m[r14 + 4] := r17\n"
                "              0 *32* m[r14 + 8] := r18\n"
                "              0 *32* m[r14 + 12] := r19\n"
                "              0 *32* m[r14 + 16] := r20\n"
                "              0 *32* m[r14 + 20] := r21\n"
                "              0 *32* m[r14 + 24] := r22\n"
                "              0 *32* m[r14 + 28] := r23\n"
                "              0 *32* m[r14 + 32] := r24\n"
                "              0 *32* m[r14 + 36] := r25\n"
                "              0 *32* m[r14 + 40] := r26\n"
                "              0 *32* m[r14 + 44] := r27\n"
                "              0 *32* m[r14 + 48] := r28\n"
                "              0 *32* m[r14 + 52] := r29\n"
                "              0 *32* m[r14 + 56] := r30\n"
                "              0 *32* m[r14 + 60] := r31\n"
                "              0 *32* r24 := r8\n"
                "              0 *32* r25 := r9\n"
                "              0 *32* r26 := r10\n"
                "              0 *32* r27 := r11\n"
                "              0 *32* r28 := r12\n"
                "              0 *32* r29 := r13\n"
                "              0 *32* r30 := r14\n"
                "              0 *32* r31 := r15\n"
                "              0 *32* r14 := tmp\n"
    );

    TEST_DECODE("sdiv %g3, %g1, %g2", "\x84\x78\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *64* tmpl := (zfill(32, 64, r100) << 32) | zfill(32, 64, r3)\n"
                "              0 *32* r2 := truncs(64, 32, tmpl /! sgnex(32, 64, r1))\n"
    );

    TEST_DECODE("sdiv %g3, 2, %g1", "\x82\x78\xe0\x02", ICLASS::NCT,
                "0x00001000    0 *64* tmpl := (zfill(32, 64, r100) << 32) | zfill(32, 64, r3)\n"
                "              0 *32* r1 := truncs(64, 32, tmpl /! 2)\n"
    );

    TEST_DECODE("sdiv %g3, -1, %g2", "\x84\x78\xff\xff", ICLASS::NCT,
                "0x00001000    0 *64* tmpl := (zfill(32, 64, r100) << 32) | zfill(32, 64, r3)\n"
                "              0 *32* r2 := truncs(64, 32, tmpl /! 18446744073709551615LL)\n" // FIXME
    );

    TEST_DECODE("sdivcc %g3, %g1, %g2", "\x84\xf8\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *64* tmpl := (zfill(32, 64, r100) << 32) | zfill(32, 64, r3)\n"
                "              0 *32* r2 := truncs(64, 32, tmpl /! sgnex(32, 64, r1))\n"
                "              0 *v* %flags := DIVFLAGS( r3, r1, r2 )\n"
    );

    TEST_DECODE("sdivcc %g3, 2, %g1", "\x82\xf8\xe0\x02", ICLASS::NCT,
                "0x00001000    0 *64* tmpl := (zfill(32, 64, r100) << 32) | zfill(32, 64, r3)\n"
                "              0 *32* r1 := truncs(64, 32, tmpl /! 2)\n"
                "              0 *v* %flags := DIVFLAGS( r3, 2, r1 )\n"
    );

    TEST_DECODE("sdivcc %g3, -1, %g2", "\x84\xf8\xff\xff", ICLASS::NCT,
                "0x00001000    0 *64* tmpl := (zfill(32, 64, r100) << 32) | zfill(32, 64, r3)\n"
                "              0 *32* r2 := truncs(64, 32, tmpl /! 18446744073709551615LL)\n" // FIXME
                "              0 *v* %flags := DIVFLAGS( r3, -1, r2 )\n"
    );

    TEST_DECODE("sethi 0, %0", "\x01\x00\x00\x00", ICLASS::NOP,
                "0x00001000\n"
    );

    TEST_DECODE("sethi 1, %g1", "\x03\x00\x00\x01", ICLASS::NCT,
                "0x00001000    0 *32* r1 := 0x400\n"
    );

    TEST_DECODE("sll %g3, %g1, %g2", "\x85\x28\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 << r1\n"
    );

    TEST_DECODE("sll %g3, 1, %g2", "\x85\x28\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 * 2\n"
    );

    TEST_DECODE("smul %g3, %g1, %g2", "\x84\x58\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *64* tmpl := sgnex(32, 64, r3) *! sgnex(32, 64, r1)\n"
                "              0 *32* r2 := truncs(64, 32, tmpl)\n"
                "              0 *32* r100 := tmpl@[32:63]\n"
    );

    TEST_DECODE("smul %g3, 2, %g1", "\x82\x58\xe0\x02", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *64* tmpl := sgnex(32, 64, r3) *! 2\n"
                "              0 *32* r1 := truncs(64, 32, tmpl)\n"
                "              0 *32* r100 := tmpl@[32:63]\n"
    );

    TEST_DECODE("smul %g3, -1, %g2", "\x84\x58\xff\xff", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *64* tmpl := sgnex(32, 64, r3) *! 18446744073709551615LL\n"
                "              0 *32* r2 := truncs(64, 32, tmpl)\n"
                "              0 *32* r100 := tmpl@[32:63]\n"
    );

    TEST_DECODE("smulcc %g3, %g1, %g2", "\x84\xd8\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *64* tmpl := sgnex(32, 64, r3) *! sgnex(32, 64, r1)\n"
                "              0 *32* r2 := truncs(64, 32, tmpl)\n"
                "              0 *32* r100 := tmpl@[32:63]\n"
                "              0 *v* %flags := MULTFLAGS( tmp, r1, r2 )\n"
    );

    TEST_DECODE("smulcc %g3, 2, %g1", "\x82\xd8\xe0\x02", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *64* tmpl := sgnex(32, 64, r3) *! 2\n"
                "              0 *32* r1 := truncs(64, 32, tmpl)\n"
                "              0 *32* r100 := tmpl@[32:63]\n"
                "              0 *v* %flags := MULTFLAGS( tmp, 2, r1 )\n"
    );

    TEST_DECODE("smulcc %g3, -1, %g2", "\x84\xd8\xff\xff", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *64* tmpl := sgnex(32, 64, r3) *! 18446744073709551615LL\n"
                "              0 *32* r2 := truncs(64, 32, tmpl)\n"
                "              0 *32* r100 := tmpl@[32:63]\n"
                "              0 *v* %flags := MULTFLAGS( tmp, -1, r2 )\n"
    );

    TEST_DECODE("sra %g3, %g1, %g2", "\x85\x38\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 >>A r1\n"
    );

    TEST_DECODE("sra %g3, 1, %g2", "\x85\x38\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 >>A 1\n"
    );

    TEST_DECODE("srl %g3, %g1, %g2", "\x85\x30\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 >> r1\n"
    );

    TEST_DECODE("srl %g3, 1, %g2", "\x85\x30\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* r2 := r3 >> 1\n"
    );

    TEST_DECODE("st %g1, [0]", "\xc2\x20\x20\x00", ICLASS::NCT,
                "0x00001000    0 *32* m[0] := r1\n"  //< TODO shouldn't this modify m[%g0] ?
    );

    TEST_DECODE("st %g1, [0xFFFFFFFF]", "\xc2\x20\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *32* m[-1] := r1\n"
    );

    TEST_DECODE("st %g1, [%g3]", "\xc2\x20\xc0\x00", ICLASS::NCT,
                "0x00001000    0 *32* m[r3] := r1\n"
    );

    TEST_DECODE("st %g1, [%g3 + 0x10]", "\xc2\x20\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *32* m[r3 + 16] := r1\n"
    );

    TEST_DECODE("st %g2, [%g3 + %g1]", "\xc4\x20\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* m[r3 + r1] := r2\n"
    );

    // TODO sta

    TEST_DECODE("stb %g1, [0]", "\xc2\x28\x20\x00", ICLASS::NCT,
                "0x00001000    0 *8* m[0] := truncs(32, 8, r1)\n"  //< TODO shouldn't this modify m[%g0] ?
    );

    TEST_DECODE("stb %g1, [0xFFFFFFFF]", "\xc2\x28\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *8* m[-1] := truncs(32, 8, r1)\n"
    );

    TEST_DECODE("stb %g1, [%g3]", "\xc2\x28\xc0\x00", ICLASS::NCT,
                "0x00001000    0 *8* m[r3] := truncs(32, 8, r1)\n"
    );

    TEST_DECODE("stb %g1, [%g3 + 0x10]", "\xc2\x28\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *8* m[r3 + 16] := truncs(32, 8, r1)\n"
    );

    TEST_DECODE("stb %g2, [%g3 + %g1]", "\xc4\x28\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *8* m[r3 + r1] := truncs(32, 8, r2)\n"
    );

    // TODO stba

    // TODO stc

    // TODO stcsr

//     TEST_DECODE("std %g2, [0]", "\xc4\x38\x20\x00", ICLASS::NCT,
//                 "0x00001000    0 *32* m[0] := r2\n"
//                 "              0 *32* m[4] := r3\n" //< TODO shouldn't this modify m[%g0] ?
//     );
//
//     TEST_DECODE("std %g2, [0xFFFFFFFF]", "\xc4\x38\x3f\xff", ICLASS::NCT,
//                 "0x00001000    0 *32* m[-1] := r2\n"
//                 "              0 *32* m[3] := r3\n"
//     );
//
//     TEST_DECODE("std %g2, [%g4]", "\xc4\x39\x00\x00", ICLASS::NCT,
//                 "0x00001000    0 *32* m[r4] := r2\n"
//                 "              0 *32* m[r4 + 4] := r3\n"
//     );
//
//     TEST_DECODE("std %g2, [%g4 + 0x10]", "\xc4\x39\x20\x10", ICLASS::NCT,
//                 "0x00001000    0 *32* m[r4 + 16] := r2\n"
//                 "              0 *32* m[r4 + 20] := r3\n"
//     );
//
//     TEST_DECODE("std %g2, [%g4 + %g2]", "\xc4\x39\x00\x02", ICLASS::NCT,
//                 "0x00001000    0 *32* m[r4 + r2] := r2\n"
//                 "              0 *32* m[(r4 + r2) + 4] := r3\n"
//     );

    // TODO stda

    // TODO stdc

    // TODO stdcq

    TEST_DECODE("stdf %f2, [0]", "\xc5\x38\x20\x00", ICLASS::NCT,
                "0x00001000    0 *64* m[0] := r65\n"
    );

    TEST_DECODE("stdf %f2, [0xFFFFFFFF]", "\xc5\x38\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *64* m[-1] := r65\n"
    );

    TEST_DECODE("stdf %f2, [%g4]", "\xc5\x39\x00\x00", ICLASS::NCT,
                "0x00001000    0 *64* m[r4] := r65\n"
    );

    TEST_DECODE("stdf %f2, [%g4 + 0x10]", "\xc5\x39\x20\x10", ICLASS::NCT,
                "0x00001000    0 *64* m[r4 + 16] := r65\n"
    );

    TEST_DECODE("stdf %f2, [%g4 + %g2]", "\xc5\x39\x00\x02", ICLASS::NCT,
                "0x00001000    0 *64* m[r4 + r2] := r65\n"
    );

    // TODO stdfq

    TEST_DECODE("stf %g1, [0]", "\xc5\x20\x20\x00", ICLASS::NCT,
                "0x00001000    0 *32* m[0] := r34\n"  //< TODO shouldn't this modify m[%g0] ?
    );

    TEST_DECODE("stf %g1, [0xFFFFFFFF]", "\xc5\x20\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *32* m[-1] := r34\n"
    );

    TEST_DECODE("stf %g1, [%g3]", "\xc5\x20\xc0\x00", ICLASS::NCT,
                "0x00001000    0 *32* m[r3] := r34\n"
    );

    TEST_DECODE("stf %g1, [%g3 + 0x10]", "\xc5\x20\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *32* m[r3 + 16] := r34\n"
    );

    TEST_DECODE("stf %g2, [%g3 + %g1]", "\xc5\x20\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* m[r3 + r1] := r34\n"
    );

//     TEST_DECODE("st %fsr, [0]", "\xc1\x28\x20\x00", ICLASS::NCT,
//                 "0x00001000    0 *32* m[0] := machine(\"%FSR\")\n"
//     );
//
//     TEST_DECODE("st %fsr, [0xFFFFFFFF]", "\xc1\x28\x3f\xff", ICLASS::NCT,
//                 "0x00001000    0 *32* m[-1] := machine(\"%FSR\")\n"
//     );
//
//     TEST_DECODE("st %fsr, [%g3]", "\xc1\x28\xc0\x00", ICLASS::NCT,
//                 "0x00001000    0 *32* m[r3] := machine(\"%FSR\")\n"
//     );
//
//     TEST_DECODE("st %fsr, [%g3 + 0x10]", "\xc1\x28\xe0\x10", ICLASS::NCT,
//                 "0x00001000    0 *32* m[r3 + 16] := machine(\"%FSR\")\n"
//     );
//
//     TEST_DECODE("st %fsr, [%g3 + %g1]", "\xc1\x28\xc0\x01", ICLASS::NCT,
//                 "0x00001000    0 *32* m[r3 + r1] := machine(\"%FSR\")\n"
//     );

    TEST_DECODE("sth %g1, [0]", "\xc2\x30\x20\x00", ICLASS::NCT,
                "0x00001000    0 *16* m[0] := truncs(32, 16, r1)\n"
    );

    TEST_DECODE("sth %g1, [0xFFFFFFFF]", "\xc2\x30\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *16* m[-1] := truncs(32, 16, r1)\n"
    );

    TEST_DECODE("sth %g1, [%g3]", "\xc2\x30\xc0\x00", ICLASS::NCT,
                "0x00001000    0 *16* m[r3] := truncs(32, 16, r1)\n"
    );

    TEST_DECODE("sth %g1, [%g3 + 0x10]", "\xc2\x30\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *16* m[r3 + 16] := truncs(32, 16, r1)\n"
    );

    TEST_DECODE("sth %g2, [%g3 + %g1]", "\xc2\x30\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *16* m[r3 + r1] := truncs(32, 16, r1)\n"
    );

    // TODO stha

    TEST_DECODE("sub %g3, %g1, %g2", "\x84\x20\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 - r1\n"
    );

    TEST_DECODE("sub %g3, 1, %g2", "\x84\x20\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 - 1\n"
    );

    TEST_DECODE("sub %g3, -1, %g2", "\x84\x20\xff\xff", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + 1\n"
    );

    TEST_DECODE("subcc %g3, %g1, %g2", "\x84\xa0\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 - r1\n"
                "              0 *v* %flags := SUBFLAGS( tmp, r1, r2 )\n"
    );

    TEST_DECODE("subcc %g3, 1, %g2", "\x84\xa0\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 - 1\n"
                "              0 *v* %flags := SUBFLAGS( tmp, 1, r2 )\n"
    );

    TEST_DECODE("subcc %g3, -1, %g2", "\x84\xa0\xff\xff", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + 1\n"
                "              0 *v* %flags := SUBFLAGS( tmp, -1, r2 )\n"
    );

    TEST_DECODE("subx %g3, %g1, %g2", "\x84\x60\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 - (r1 + zfill(1, 32, %CF))\n"
    );

    TEST_DECODE("subx %g3, 1, %g2", "\x84\x60\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := (r3 - zfill(1, 32, %CF)) - 1\n"
    );

    TEST_DECODE("subx %g3, -1, %g2", "\x84\x60\xff\xff", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := (r3 - zfill(1, 32, %CF)) + 1\n"
    );

    TEST_DECODE("subxcc %g3, %g1, %g2", "\x84\xe0\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 - (r1 + zfill(1, 32, %CF))\n"
                "              0 *v* %flags := SUBFLAGS( tmp, r1, r2 )\n"
    );

    TEST_DECODE("subxcc %g3, 1, %g2", "\x84\xe0\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := (r3 - zfill(1, 32, %CF)) - 1\n"
                "              0 *v* %flags := SUBFLAGS( tmp, 1, r2 )\n"
    );

    TEST_DECODE("subxcc %g3, -1, %g2", "\x84\xe0\xff\xff", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := (r3 - zfill(1, 32, %CF)) + 1\n"
                "              0 *v* %flags := SUBFLAGS( tmp, -1, r2 )\n"
    );

    TEST_DECODE("swap [0], %g1", "\xc2\x78\x20\x00", ICLASS::NCT,
                "0x00001000    0 *32* tmp1 := r1\n"
                "              0 *32* r1 := m[0]\n"
                "              0 *32* m[0] := tmp1\n"
    );

    TEST_DECODE("swap [0xFFFFFFFF], %g1", "\xc2\x78\x3f\xff", ICLASS::NCT,
                "0x00001000    0 *32* tmp1 := r1\n"
                "              0 *32* r1 := m[-1]\n"
                "              0 *32* m[-1] := tmp1\n"
    );

    TEST_DECODE("swap [%g3], %g1", "\xc2\x78\xc0\x00", ICLASS::NCT,
                "0x00001000    0 *32* tmp1 := r1\n"
                "              0 *32* r1 := m[r3]\n"
                "              0 *32* m[r3] := tmp1\n"
    );

    TEST_DECODE("swap [%g3 + 0x10], %g1", "\xc2\x78\xe0\x10", ICLASS::NCT,
                "0x00001000    0 *32* tmp1 := r1\n"
                "              0 *32* r1 := m[r3 + 16]\n"
                "              0 *32* m[r3 + 16] := tmp1\n"
    );

    TEST_DECODE("swap [%g3 + %g1], %g2", "\xc2\x78\xc0\x02", ICLASS::NCT,
                "0x00001000    0 *32* tmp1 := r1\n"
                "              0 *32* r1 := m[r3 + r2]\n"
                "              0 *32* m[r3 + r2] := tmp1\n"
    );

    // TODO swapa

    TEST_DECODE("taddcc %g3, %g1, %g2", "\x85\x00\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + r1\n"
                "              0 *v* %flags := TADDFLAGS( tmp, r1, r2 )\n"
    );

    TEST_DECODE("taddcc %g3, 1, %g2", "\x85\x00\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + 1\n"
                "              0 *v* %flags := TADDFLAGS( tmp, 1, r2 )\n"
    );

    TEST_DECODE("taddcc %g3, -1, %g2", "\x85\x00\xff\xff", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 - 1\n"
                "              0 *v* %flags := TADDFLAGS( tmp, -1, r2 )\n"
    );

    TEST_DECODE("taddcctv %g3, %g1, %g2", "\x85\x00\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + r1\n"
                "              0 *v* %flags := TADDFLAGS( tmp, r1, r2 )\n"
    );

    TEST_DECODE("taddcctv %g3, 1, %g2", "\x85\x00\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + 1\n"
                "              0 *v* %flags := TADDFLAGS( tmp, 1, r2 )\n"
    );

    TEST_DECODE("taddcctv %g3, -1, %g2", "\x85\x00\xff\xff", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 - 1\n"
                "              0 *v* %flags := TADDFLAGS( tmp, -1, r2 )\n"
    );

    // TODO ticc

    TEST_DECODE("tsubcc %g3, %g1, %g2", "\x85\x08\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 - r1\n"
                "              0 *v* %flags := TSUBFLAGS( tmp, r1, r2 )\n"
    );

    TEST_DECODE("tsubcc %g3, 1, %g2", "\x85\x08\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 - 1\n"
                "              0 *v* %flags := TSUBFLAGS( tmp, 1, r2 )\n"
    );

    TEST_DECODE("tsubcc %g3, -1, %g2", "\x85\x08\xff\xff", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + 1\n"
                "              0 *v* %flags := TSUBFLAGS( tmp, -1, r2 )\n"
    );

    TEST_DECODE("tsubcctv %g3, %g1, %g2", "\x85\x08\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 - r1\n"
                "              0 *v* %flags := TSUBFLAGS( tmp, r1, r2 )\n"
    );

    TEST_DECODE("tsubcctv %g3, 1, %g2", "\x85\x08\xe0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 - 1\n"
                "              0 *v* %flags := TSUBFLAGS( tmp, 1, r2 )\n"
    );

    TEST_DECODE("tsubcctv %g3, -1, %g2", "\x85\x08\xff\xff", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *32* r2 := r3 + 1\n"
                "              0 *v* %flags := TSUBFLAGS( tmp, -1, r2 )\n"
    );

    TEST_DECODE("udiv %g3, %g1, %g2", "\x84\x70\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *64* tmpl := (zfill(32, 64, r100) << 32) | zfill(32, 64, r3)\n"
                "              0 *32* r2 := truncu(64, 32, tmpl / zfill(32, 64, r1))\n"
    );

    TEST_DECODE("udiv %g3, 2, %g1", "\x82\x70\xe0\x02", ICLASS::NCT,
                "0x00001000    0 *64* tmpl := (zfill(32, 64, r100) << 32) | zfill(32, 64, r3)\n"
                "              0 *32* r1 := truncu(64, 32, tmpl / 2)\n"
    );

    TEST_DECODE("udivcc %g3, %g1, %g2", "\x84\xf0\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *64* tmpl := (zfill(32, 64, r100) << 32) | zfill(32, 64, r3)\n"
                "              0 *32* r2 := truncu(64, 32, tmpl / zfill(32, 64, r1))\n"
                "              0 *v* %flags := DIVFLAGS( tmpl, r1, r2 )\n"
    );

    TEST_DECODE("udivcc %g3, 2, %g1", "\x82\xf0\xe0\x02", ICLASS::NCT,
                "0x00001000    0 *64* tmpl := (zfill(32, 64, r100) << 32) | zfill(32, 64, r3)\n"
                "              0 *32* r1 := truncu(64, 32, tmpl / 2)\n"
                "              0 *v* %flags := DIVFLAGS( tmpl, 2, r1 )\n"
    );

    TEST_DECODE("umul %g3, %g1, %g2", "\x84\x50\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *64* tmpl := zfill(32, 64, r3) * zfill(32, 64, r1)\n"
                "              0 *32* r2 := truncs(64, 32, tmpl)\n"
                "              0 *32* r100 := tmpl@[32:63]\n"
    );

    TEST_DECODE("umul %g3, 2, %g1", "\x82\x50\xe0\x02", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *64* tmpl := zfill(32, 64, r3) * 2\n"
                "              0 *32* r1 := truncs(64, 32, tmpl)\n"
                "              0 *32* r100 := tmpl@[32:63]\n"
    );

    TEST_DECODE("umulcc %g3, %g1, %g2", "\x84\xd0\xc0\x01", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *64* tmpl := zfill(32, 64, r3) * zfill(32, 64, r1)\n"
                "              0 *32* r2 := truncs(64, 32, tmpl)\n"
                "              0 *32* r100 := tmpl@[32:63]\n"
                "              0 *v* %flags := MULTFLAGS( tmp, r1, r2 )\n"
    );

    TEST_DECODE("umulcc %g3, 2, %g1", "\x82\xd0\xe0\x02", ICLASS::NCT,
                "0x00001000    0 *32* tmp := r3\n"
                "              0 *64* tmpl := zfill(32, 64, r3) * 2\n"
                "              0 *32* r1 := truncs(64, 32, tmpl)\n"
                "              0 *32* r100 := tmpl@[32:63]\n"
                "              0 *v* %flags := MULTFLAGS( tmp, 2, r1 )\n"
    );

    // TODO unimp

//     TEST_DECODE("wr %g1, %g2, %psr", "\x81\x88\x40\x02", ICLASS::NCT,
//                 "0x00001000    0 *32* machine(\"%PSR\") := r1 ^ r2\n"
//     );
//
//     TEST_DECODE("wr %g1, 2, %psr", "\x81\x88\x60\x02", ICLASS::NCT,
//                 "0x00001000    0 *32* machine(\"%PSR\") := r1 ^ 2\n"
//     );
//
//     TEST_DECODE("wr %g1, ~1, %psr", "\x81\x88\x7f\xfe", ICLASS::NCT,
//                 "0x00001000    0 *32* machine(\"%PSR\") := r1 ^ -2\n"
//     );
//
//     TEST_DECODE("wr %g1, %g2, %tbr", "\x81\x98\x40\x02", ICLASS::NCT,
//                 "0x00001000    0 *32* machine(\"%TBR\") := machine(\"%TBR\") | ((r1 ^ r2) << 12)\n"
//     );
//
//     TEST_DECODE("wr %g1, 2, %tbr", "\x81\x98\x60\x02", ICLASS::NCT,
//                 "0x00001000    0 *32* machine(\"%TBR\") := machine(\"%TBR\") | ((r1 ^ 2) << 12)\n"
//     );
//
//     TEST_DECODE("wr %g1, ~1, %tbr", "\x81\x98\x7f\xfe", ICLASS::NCT,
//                 "0x00001000    0 *32* machine(\"%TBR\") := machine(\"%TBR\") | ((r1 ^ -2) << 12)\n"
//     );
//
//     TEST_DECODE("wr %g1, %g2, %wim", "\x81\x90\x40\x02", ICLASS::NCT,
//                 "0x00001000    0 *32* machine(\"%WIM\") := r1 ^ r2\n"
//     );
//
//     TEST_DECODE("wr %g1, 2, %wim", "\x81\x90\x60\x02", ICLASS::NCT,
//                 "0x00001000    0 *32* machine(\"%WIM\") := r1 ^ 2\n"
//     );
//
//     TEST_DECODE("wr %g1, ~1, %wim", "\x81\x90\x7f\xfe", ICLASS::NCT,
//                 "0x00001000    0 *32* machine(\"%WIM\") := r1 ^ -2\n"
//     );

    TEST_DECODE("wr %g1, %g2, %y", "\x81\x80\x40\x02", ICLASS::NCT,
                "0x00001000    0 *32* r100 := r1 ^ r2\n"
    );

    TEST_DECODE("wr %g1, 2, %y", "\x81\x80\x60\x02", ICLASS::NCT,
                "0x00001000    0 *32* r100 := r1 ^ 2\n"
    );

    TEST_DECODE("wr %g1, ~1, %y", "\x81\x80\x7f\xfe", ICLASS::NCT,
                "0x00001000    0 *32* r100 := r1 ^ -2\n"
    );

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
