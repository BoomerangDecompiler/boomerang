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

#include "boomerang/ssl/RTL.h"
#include "boomerang/util/Types.h"


struct InstructionData
{
public:
    Byte data[5];
};

Q_DECLARE_METATYPE(InstructionData)

#define TEST_DECODE(name, data, result)                                                            \
    QTest::newRow(name) << InstructionData{ data } << QString(result);


void CapstonePPCDecoderTest::initTestCase()
{
    m_project.loadPlugins();

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
    ptrdiff_t diff     = (HostAddress(&insnData) - sourceAddr).value();
    QVERIFY(m_decoder->decodeInstruction(sourceAddr, diff, result));
    QCOMPARE(result.rtl->toString(), expectedResult);
}


void CapstonePPCDecoderTest::testInstructions_data()
{
    QTest::addColumn<InstructionData>("insnData");
    QTest::addColumn<QString>("expectedResult");

    // instructions listed alphabetically

    TEST_DECODE("add r0, r1, r2", "\x7c\x01\x12\x14", "0x00001000    0 *32* r0 := r1 + r2\n");

    TEST_DECODE("add. r0, r1, r2", "\x7c\x01\x12\x15",
                "0x00001000    0 *32* r0 := r1 + r2\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n");

    // Capstone does not support addo and addo. yet


    TEST_DECODE("addc r0, r1, r2", "\x7c\x01\x10\x14",
                "0x00001000    0 *32* r0 := r1 + r2\n"
                "              0 *v* %flags := ADDFLAGSX( r0, r1, r2 )\n");

    TEST_DECODE("addc. r0, r1, r2", "\x7c\x01\x10\x15",
                "0x00001000    0 *32* r0 := r1 + r2\n"
                "              0 *v* %flags := ADDFLAGSX0( r0, r1, r2 )\n");

    // Capstone does not support addco and addco. yet

    TEST_DECODE("adde r0, r1, r2", "\x7c\x01\x11\x14",
                "0x00001000    0 *32* r0 := r1 + (r2 + r203)\n");

    TEST_DECODE("adde. r0, r1, r2", "\x7c\x01\x11\x15",
                "0x00001000    0 *32* r0 := r1 + (r2 + r203)\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n");

    TEST_DECODE("addi r0, r1, -10", "\x38\x01\xff\xf6", "0x00001000    0 *32* r0 := r1 - 10\n");


    TEST_DECODE("addic r0, r1, -10", "\x30\x01\xff\xf6",
                "0x00001000    0 *32* r0 := r1 - 10\n"
                "              0 *v* %flags := ADDFLAGSX( r0, r1, -10 )\n");

    TEST_DECODE("addic. r0, r1, -10", "\x34\x01\xff\xf6",
                "0x00001000    0 *32* r0 := r1 - 10\n"
                "              0 *v* %flags := ADDFLAGSX0( r0, r1, -10 )\n");

    TEST_DECODE("addis r0, r1, -10", "\x3c\x01\xff\xf6",
                "0x00001000    0 *32* r0 := r1 - 0xa0000\n");

    TEST_DECODE("addme r0, r1", "\x7c\x01\x01\xd4", "0x00001000    0 *32* r0 := (r1 + r203) - 1\n");

    TEST_DECODE("addme. r0, r1", "\x7c\x01\x01\xd5",
                "0x00001000    0 *32* r0 := (r1 + r203) - 1\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n");

    // Capstone does not support addmeo[.] and addme64[o] yet

    TEST_DECODE("addze r0, r1", "\x7c\x01\x01\x94", "0x00001000    0 *32* r0 := r1 + r203\n");

    TEST_DECODE("addze. r0, r1", "\x7c\x01\x01\x95",
                "0x00001000    0 *32* r0 := r1 + r203\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n");

    // Capstone does not support addzeo[.] and addze64[o] yet

    TEST_DECODE("and r0, r1, r2", "\x7c\x20\x10\x38", "0x00001000    0 *32* r0 := r1 & r2\n");

    TEST_DECODE("and. r0, r1, r2", "\x7c\x20\x10\x39",
                "0x00001000    0 *32* r0 := r1 & r2\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n");

    TEST_DECODE("andi. r0, r1, 10", "\x70\x20\x00\x0a",
                "0x00001000    0 *32* r0 := r1 & 10\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n");

    TEST_DECODE("andis. r0, r1, 10", "\x74\x20\x00\x0a",
                "0x00001000    0 *32* r0 := r1 & 0xa0000\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n");

    TEST_DECODE("andc r0, r1, r2", "\x7c\x20\x10\x78", "0x00001000    0 *32* r0 := r1 & ~r2\n");

    TEST_DECODE("andc. r0, r1, r2", "\x7c\x20\x10\x79",
                "0x00001000    0 *32* r0 := r1 & ~r2\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n");

    TEST_DECODE("b 0x0800", "\x4b\xff\xf8\x00", "0x00001000    0 GOTO 0x00000800\n");

    TEST_DECODE("b 0x2000", "\x48\x00\x10\x00", "0x00001000    0 GOTO 0x00002000\n");

    TEST_DECODE("ba 0x2000", "\x48\x00\x20\x02", "0x00001000    0 GOTO 0x00002000\n");

    TEST_DECODE("bl 0x2000", "\x48\x00\x10\x01",
                "0x00001000    0 <all> := CALL 0x2000(<all>)\n"
                "              Reaching definitions: <None>\n"
                "              Live variables: <None>\n");

    TEST_DECODE("bla 0x2000", "\x48\x00\x20\x03",
                "0x00001000    0 <all> := CALL 0x2000(<all>)\n"
                "              Reaching definitions: <None>\n"
                "              Live variables: <None>\n");

    // TODO: Conditional branches BC*

    TEST_DECODE("cmp 3, 0, 0, 1", "\x7d\x80\x08\x00",
                "0x00001000    0 *v* %flags := SUBFLAGSNS( r0, r1, r67 )\n");

    TEST_DECODE("cmpi 0, 0, 0, -10", "\x2c\x00\xff\xf6",
                "0x00001000    0 *v* %flags := SUBFLAGSNS( r0, -10, 0 )\n");

    TEST_DECODE("cmpi 3, 0, 0, -10", "\x2d\x80\xff\xf6",
                "0x00001000    0 *v* %flags := SUBFLAGSNS( r0, -10, r67 )\n");

    TEST_DECODE("cmpl 3, 0, 0, 1", "\x7d\x80\x08\x40",
                "0x00001000    0 *v* %flags := SUBFLAGSNL( r0, r1, r67 )\n");

    TEST_DECODE("cmpli 0, 0, 0, 1", "\x28\x00\x00\x01",
                "0x00001000    0 *v* %flags := SUBFLAGSNL( r0, 1, 0 )\n");

    TEST_DECODE("cmpli 3, 0, 0, 1", "\x29\x80\x00\x01",
                "0x00001000    0 *v* %flags := SUBFLAGSNL( r0, 1, r67 )\n");

    // TODO: Add semantics for cntlzw/cntlzw./cntlzd

    TEST_DECODE("crand 0, 2, 7", "\x4c\x02\x3a\x02",
                "0x00001000    0 *1* r100@[0:0] := (r100@[2:2]) & (r100@[7:7])\n");

    TEST_DECODE("crandc 0, 2, 7", "\x4c\x02\x39\x02",
                "0x00001000    0 *1* r100@[0:0] := (r100@[2:2]) & ~(r100@[7:7])\n");

    TEST_DECODE("creqv 0, 2, 7", "\x4c\x02\x3a\x42",
                "0x00001000    0 *1* r100@[0:0] := ~((r100@[2:2]) ^ (r100@[7:7]))\n");

    TEST_DECODE("crnand 0, 2, 7", "\x4c\x02\x39\xc2",
                "0x00001000    0 *1* r100@[0:0] := ~(r100@[2:2]) | ~(r100@[7:7])\n");

    TEST_DECODE("crnor 0, 2, 7", "\x4c\x02\x38\x42",
                "0x00001000    0 *1* r100@[0:0] := ~(r100@[2:2]) & ~(r100@[7:7])\n");

    TEST_DECODE("cror 0, 2, 7", "\x4c\x02\x3b\x82",
                "0x00001000    0 *1* r100@[0:0] := (r100@[2:2]) | (r100@[7:7])\n");

    TEST_DECODE("crorc 0, 2, 7", "\x4c\x02\x3b\x42",
                "0x00001000    0 *1* r100@[0:0] := (r100@[2:2]) | ~(r100@[7:7])\n");

    TEST_DECODE("crxor 0, 2, 7", "\x4c\x02\x39\x82",
                "0x00001000    0 *1* r100@[0:0] := (r100@[2:2]) ^ (r100@[7:7])\n");

    // TODO dcb*

    TEST_DECODE("divd 0, 1, 2", "\x7c\x01\x13\xd2",
                "0x00001000    0 *64* r0 := r1 / r2\n"
    );

    // Capstone does not support divdo yet

    TEST_DECODE("divdu 0, 1, 2", "\x7c\x01\x13\x92",
                "0x00001000    0 *64* r0 := r1 / r2\n"
    );

    // Capstone does not support divdou yet

    TEST_DECODE("divw 0, 1, 2", "\x7c\x01\x13\xd6", "0x00001000    0 *32* r0 := r1 / r2\n");

    TEST_DECODE("divw. 0, 1, 2", "\x7c\x01\x13\xd7",
                "0x00001000    0 *32* r0 := r1 / r2\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n");

    // Capstone does not support divwo[.] yet

    TEST_DECODE("divwu 0, 1, 2", "\x7c\x01\x13\x96", "0x00001000    0 *32* r0 := r1 / r2\n");

    TEST_DECODE("divwu. 0, 1, 2", "\x7c\x01\x13\x97",
                "0x00001000    0 *32* r0 := r1 / r2\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n");

    // Capstone does not support divwuo[.] yet

    TEST_DECODE("eqv 0, 1, 2", "\x7c\x20\x12\x38", "0x00001000    0 *32* r0 := ~(r1 ^ r2)\n");

    TEST_DECODE("eqv. 0, 1, 2", "\x7c\x20\x12\x39",
                "0x00001000    0 *32* r0 := ~(r1 ^ r2)\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n");

    TEST_DECODE("extsb 0, 1", "\x7c\x20\x07\x74", "0x00001000    0 *32* r0 := sgnex(8, 32, r1)\n");

    TEST_DECODE("extsb. 0, 1", "\x7c\x20\x07\x75",
                "0x00001000    0 *32* r0 := sgnex(8, 32, r1)\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n");

    TEST_DECODE("extsh 0, 1", "\x7c\x20\x07\x34", "0x00001000    0 *32* r0 := sgnex(16, 32, r1)\n");

    TEST_DECODE("extsb 0, 1", "\x7c\x20\x07\x35",
                "0x00001000    0 *32* r0 := sgnex(16, 32, r1)\n"
                "              0 *v* %flags := SETFLAGS0( r0 )\n");

    // TODO extsw

    TEST_DECODE("fabs 1, 2", "\xfc\x20\x12\x10", "0x00001000    0 *64* r33 := fabs(r34)\n");

    TEST_DECODE("fabs. 1, 2", "\xfc\x20\x12\x11", "0x00001000    0 *64* r33 := fabs(r34)\n");

    TEST_DECODE("fadd 1, 2, 3", "\xfc\x22\x18\x2a", "0x00001000    0 *64* r33 := r34 +f r35\n");

    TEST_DECODE("fadd. 1, 2, 3", "\xfc\x22\x18\x2b", "0x00001000    0 *64* r33 := r34 +f r35\n");

    TEST_DECODE("fadds 1, 2, 3", "\xec\x22\x18\x2a", "0x00001000    0 *64* r33 := r34 +f r35\n");

    TEST_DECODE("fadds. 1, 2, 3", "\xec\x22\x18\x2b", "0x00001000    0 *64* r33 := r34 +f r35\n");

    // TODO fcfid

    TEST_DECODE("fcmpu 1, 2, 3", "\xfc\x82\x18\x00",
                "0x00001000    0 *v* %flags := SETFFLAGSN( r34, r35, r65 )\n");

    // Capstone does not support fcmpo

    // TODO fctid[z]
    // TODO fctiw[z][.]


    TEST_DECODE("fdiv 1, 2, 3", "\xfc\x22\x18\x24", "0x00001000    0 *64* r33 := r34 /f r35\n");

    TEST_DECODE("fdiv. 1, 2, 3", "\xfc\x22\x18\x25", "0x00001000    0 *64* r33 := r34 /f r35\n");

    TEST_DECODE("fdivs 1, 2, 3", "\xec\x22\x18\x24", "0x00001000    0 *64* r33 := r34 /f r35\n");

    TEST_DECODE("fdivs. 1, 2, 3", "\xec\x22\x18\x25", "0x00001000    0 *64* r33 := r34 /f r35\n");

    // TODO: fmadd

    TEST_DECODE("fmr 3, 1", "\xfc\x60\x08\x90", "0x00001000    0 *64* r35 := r33\n");

    TEST_DECODE("fmr. 3, 1", "\xfc\x60\x08\x91", "0x00001000    0 *64* r35 := r33\n");

    // TODO: fmsub

    // TODO: fmul

    // TODO: fnabs

//     TEST_DECODE("fneg 3, 1", "\xfc\x60\x08\x50", "0x00001000    0 *64* r35 := 0.0 - r33\n");
//
//     TEST_DECODE("fneg. 3, 1", "\xfc\x60\x08\x51", "0x00001000    0 *64* r35 := 0.0 - r33\n");

    // TODO fnmadd[s][.]

    // TODO fnmsub[s][.]

    // TODO fres[.]

    // TODO frsp[.]

    // TODO fsqrte[.]

    // TODO fsel[.]

    // TODO fsqrt[s][.]

    // TODO fsub[.]

    // Insn cache block instructions icb* TODO

    TEST_DECODE("lbz 3, 5(2)", "\x88\x62\x00\x05",
                "0x00001000    0 *32* r3 := zfill(8, 32, m[r2 + 5])\n");

//     TEST_DECODE("lbzu 3, 4(2)", "\x8c\x62\x00\x05",
//                 "0x00001000    0 *8* r3 := zfill(8, 32, m[r2 + 4])\n"
//                 "              0 *8* r2 := r2 + 4\n"
//     );

    // lbz[u]e TODO

    TEST_DECODE("lbzx 3, 1(2)", "\x7c\x61\x10\xae",
                "0x00001000    0 *32* r3 := zfill(8, 32, m[r1 + r2])\n");

    TEST_DECODE("lbzux 3, 1(2)", "\x7c\x61\x10\xee",
                "0x00001000    0 *32* r3 := zfill(8, 32, m[r1 + r2])\n"
                "              0 *32* r1 := r1 + r2\n"
    );

    // lbz[u]xe TODO


    // TODO ldarxe

    // TODO ld[u][x]e

    TEST_DECODE("lfd 3 1(2)", "\xc8\x62\x00\x01",
                "0x00001000    0 *64* r35 := m[r2 + 1]\n"
    );

//     TEST_DECODE("lfdu 3, 1(2)", "\xcc\x62\x00\x01",
//                 "0x00001000    0 *64* r35 := m[r2 + 1]\n"
//                 "              0 *32* r2 := r2 + 1\n"
//     );

    // TODO lfd[u]e

    TEST_DECODE("lfdx 3, 1(2)", "\x7c\x61\x14\xae",
                "0x00001000    0 *64* r35 := m[r1 + r2]\n"
    );

    TEST_DECODE("lfdux 3, 1(2)", "\x7c\x61\x14\xee",
                "0x00001000    0 *64* r35 := m[r1 + r2]\n"
                "              0 *32* r1 := r1 + r2\n"
    );

    // TODO lfd[u]xe

    TEST_DECODE("lfs 3, 1(2)", "\xc0\x62\x00\x01",
                "0x00001000    0 *32* r35 := fsize(32, 64, m[r2 + 1])\n"
    );

//     TEST_DECODE("lfsu 3, 1(2)", "\xc4\x62\x00\x01",
//                 "0x00001000    0 *32* r35 := fsize(32, 64, m[r2 + 1])\n"
//                 "              0 *32* r2 := r2 + 1"
//     );

    // TODO lfs[u]e

    TEST_DECODE("lfsx 3, 1(2)", "\x7c\x61\x14\x2e",
                "0x00001000    0 *32* r35 := fsize(32, 64, m[r1 + r2])\n"
    );

    TEST_DECODE("lfsux 3, 1(2)", "\x7c\x61\x14\x6e",
                "0x00001000    0 *32* r35 := fsize(32, 64, m[r1 + r2])\n"
                "              0 *32* r1 := r1 + r2\n"
    );

    // TODO lfs[u]xe

    TEST_DECODE("lha 3, 1(2)", "\xa8\x62\x00\x01",
                "0x00001000    0 *32* r3 := sgnex(16, 32, m[r2 + 1])\n"
    );

//     TEST_DECODE("lhau 3, 1(2)", "\xac\x62\x00\x01",
//                 "0x00001000    0 *32* r3 := sgnex(16, 32, m[r2 + 1])\n"
//                 "              0 *32* r2 := r2 + 1"
//     );

    // TODO lha[u]e

    TEST_DECODE("lhax 3, 1(2)", "\x7c\x61\x12\xae",
                "0x00001000    0 *32* r3 := m[r1 + r2]\n"
    );

    TEST_DECODE("lhaux 3, 1(2)", "\x7c\x61\x12\xee",
                "0x00001000    0 *32* r3 := m[r1 + r2]\n"
                "              0 *32* r1 := r1 + r2\n"
    );

    // TODO lha[u]xe

    // TODO lhbrx[e]

    TEST_DECODE("lhz 3, 1(2)", "\xa0\x62\x00\x01",
                "0x00001000    0 *32* r3 := zfill(16, 32, m[r2 + 1])\n"
    );

//     TEST_DECODE("lhzu 3, 1(2)", "\xa4\x62\x00\x01",
//                 "0x00001000    0 *32* r3 := zfill(16, 32, m[r2 + 1])\n"
//                 "              0 *32* r2 := r2 + 1"
//     );

    // TODO lhz[u]e

    TEST_DECODE("lhzx 3, 1(2)", "\x7c\x61\x12\x2e",
                "0x00001000    0 *32* r3 := zfill(16, 32, m[r1 + r2])\n"
    );

    TEST_DECODE("lhzux 3, 1(2)", "\x7c\x61\x12\x6e",
                "0x00001000    0 *32* r3 := zfill(16, 32, m[r1 + r2])\n"
                "              0 *32* r1 := r1 + r2\n"
    );

    // TODO lhz[u]xe

    TEST_DECODE("lmw 30, 4(2)", "\xbb\xc2\x00\x04",
                "0x00001000    0 *32* r30 := m[r2 + 4]\n"
                "              0 *32* r31 := m[r2 + 8]\n"
    );

    // TODO lswi/lswx

    // TODO lwarx[e]

    // TODO lwbrx[e]

    TEST_DECODE("lwz 3, 1(2)", "\x80\x62\x00\x01",
                "0x00001000    0 *32* r3 := m[r2 + 1]\n"
    );


//     TEST_DECODE("lwzu 3, 1(2)", "\x84\x62\x00\x01",
//                 "0x00001000    0 *32* r3 := m[r2 + 1]\n"
//                 "              0 *32* r2 := r2 + 1"
//     );

    // TODO lwz[u]e

    TEST_DECODE("lwzx 3, 1(2)", "\x7c\x61\x10\x2e",
                "0x00001000    0 *32* r3 := m[r1 + r2]\n"
    );

    TEST_DECODE("lwzux 3, 1(2)", "\x7c\x61\x10\x6e",
                "0x00001000    0 *32* r3 := m[r1 + r2]\n"
                "              0 *32* r1 := r1 + r2\n"
    );

    // TODO lwz[u]xe

    // TODO mbar

    TEST_DECODE("mcrf 2, 3", "\x4d\x0c\x00\x00",
                "0x00001000    0 *4* r66 := r67\n"
    );

    // TODO mcrfs

    // TODO mcrxr

    // TODO mcrxr64

    // TODO mfapidi

    TEST_DECODE("mfcr 3", "\x7c\x60\x00\x26",
                "0x00001000    0 *32* r3 := (r64 << 28) + ((r65 << 24) + ((r66 << 20) + ((r67 << 16) + ((r68 << 12) + ((r69 << 8) + ((r70 << 4) + r71))))))\n"
    );

    // TODO mfdcr

    // TODO mffs/mffs.

    // TODO mfmsr

    // TODO mfspr

    // TODO msync

    // TODO mtcrf

    // TODO mtdcr

    // TODO mtfsb[0|1][.]

    // TODO mtfsf

    // TODO mtfsfi[.]

    // TODO mtmsr

    // TODO mtspr

    // TODO mulhd

    // TODO mulhdu

    // TODO mulhw[.]

    // TODO mulld[o]

    // TODO mulli

    // TODO mullw[o][.]

    TEST_DECODE("nand 9, 5, 1", "\x7c\xa9\x0b\xb8",
                "0x00001000    0 *32* r9 := ~r5 | ~r1\n"
    );

    TEST_DECODE("nand. 9, 5, 1", "\x7c\xa9\x0b\xb9",
                "0x00001000    0 *32* r9 := ~r5 | ~r1\n"
                "              0 *v* %flags := SETFLAGS0( r9 )\n"
    );

    TEST_DECODE("neg 5, 3", "\x7c\xa3\x00\xd0",
                "0x00001000    0 *32* r5 := 0 - r3\n"
    );

    TEST_DECODE("neg 5, 3", "\x7c\xa3\x00\xd1",
                "0x00001000    0 *32* r5 := 0 - r3\n"
                "              0 *v* %flags := SETFLAGS0( r5 )\n"
    );

    // TODO nego[.]

    TEST_DECODE("nor 8, 9, 4", "\x7d\x28\x20\xf8",
                "0x00001000    0 *32* r8 := ~r9 & ~r4\n"
    );

    TEST_DECODE("nor. 8, 9, 4", "\x7d\x28\x20\xf9",
                "0x00001000    0 *32* r8 := ~r9 & ~r4\n"
                "              0 *v* %flags := SETFLAGS0( r8 )\n"
    );

    TEST_DECODE("or 8, 9, 4", "\x7d\x28\x23\x78",
                "0x00001000    0 *32* r8 := r9 | r4\n"
    );

    TEST_DECODE("or. 8, 9, 4", "\x7d\x28\x23\x79",
                "0x00001000    0 *32* r8 := r9 | r4\n"
                "              0 *v* %flags := SETFLAGS0( r8 )\n"
    );

    TEST_DECODE("ori 8, 9, 0x10", "\x61\x28\x00\x10",
                "0x00001000    0 *32* r8 := r9 | 16\n"
    );

    TEST_DECODE("oris 8, 9, 0x10", "\x65\x28\x00\x10",
                "0x00001000    0 *32* r8 := r9 | 0x100000\n"
    );

    // TODO rfci

    // TODO rfi

    // TODO rld[i]cl

    // TODO rldicr

    // TODO rldic

    // TODO rldimi

    // TODO rlwimi[.]

    // TODO rlw[i]nm[.]

    // TODO sc

    // TODO sld

    // TODO slw[.]

    // TODO srad[i]

    // TODO sraw[i][.]

    // TODO srd

    // TODO srw[.]

    TEST_DECODE("stb 3, 1(2)", "\x98\x62\x00\x01",
                "0x00001000    0 *8* m[r2 + 1] := truncs(32, 8, r3)\n"
    );

    TEST_DECODE("stbu 3, 1(2)", "\x9c\x62\x00\x01",
                "0x00001000    0 *8* m[r2 + 1] := truncs(32, 8, r3)\n"
                "              0 *32* r3 := r2 + 1\n"
    );

    // TODO stb[u]e

    TEST_DECODE("stbx 3, 1(2)", "\x7c\x61\x11\xae",
                "0x00001000    0 *8* m[r1 + r2] := truncs(32, 8, r3)\n"
    );

    TEST_DECODE("stbux 3, 1(2)", "\x7c\x61\x11\xee",
                "0x00001000    0 *8* m[r1 + r2] := truncs(32, 8, r3)\n"
                "              0 *32* r1 := r1 + r2\n"
    );

    // TODO stb[u]xe


    // TODO stdcxe

    // TODO std[u]e

    // TODO std[u]xe

    TEST_DECODE("stfd 3, 1(2)", "\xd8\x62\x00\x01",
                "0x00001000    0 *64* m[r2 + 1] := r35\n"
    );

//     TEST_DECODE("stfdu 3, 1(2)", "\xdc\x62\x00\x01",
//                 "0x00001000    0 *64* m[r2 + 1] := r35\n"
//                 "              0 *32* r2 := r2 + 1"
//     );

    // TODO stfd[u]e

    TEST_DECODE("stfdx 3, 1(2)", "\x7c\x61\x15\xae",
                "0x00001000    0 *64* m[r1 + r2] := r35\n"
    );

    TEST_DECODE("stfdux 3, 1(2)", "\x7c\x61\x15\xee",
                "0x00001000    0 *64* m[r1 + r2] := r35\n"
                "              0 *32* r1 := r1 + r2\n"
    );

    // TODO stfd[u]xe

    // TODO stfiwx[e]

    TEST_DECODE("stfs 3, 1(2)", "\xd0\x62\x00\x01",
                "0x00001000    0 *32* m[r2 + 1] := fsize(64, 32, r35)\n"
    );

//     TEST_DECODE("stfsu 3, 1(2)", "\xd4\x62\x00\x01",
//                 "0x00001000    0 *32* m[r2 + 1] := fsize(64, 32, r35)\n"
//                 "              0 *32* r2 := r2 + 1\n"
//     );

    // TODO stfs[u]e

    TEST_DECODE("stfsx 3, 1(2)", "\x7c\x61\x15\x2e",
                "0x00001000    0 *32* m[r1 + r2] := fsize(64, 32, r35)\n"
    );

    TEST_DECODE("stfsux 3, 1(2)", "\x7c\x61\x15\x6e",
                "0x00001000    0 *32* m[r1 + r2] := fsize(64, 32, r35)\n"
                "              0 *32* r1 := r1 + r2\n"
    );

    // TODO stfs[u]xe

    TEST_DECODE("sth 3, 1(2)", "\xb0\x62\x00\x01",
                "0x00001000    0 *16* m[r2 + 1] := truncs(32, 16, r3)\n");

    TEST_DECODE("sthu 3, 1(2)", "\xb4\x62\x00\x01",
                "0x00001000    0 *16* m[r2 + 1] := truncs(32, 16, r3)\n"
                "              0 *32* r3 := r2 + 1\n");

    // TODO sth[u]e

    TEST_DECODE("sthx 3, 1(2)", "\x7c\x61\x13\x2e",
                "0x00001000    0 *16* m[r1 + r2] := truncs(32, 16, r3)\n");

    TEST_DECODE("sthux 3, 1(2)", "\x7c\x61\x13\x6e",
                "0x00001000    0 *16* m[r1 + r2] := truncs(32, 16, r3)\n"
                "              0 *32* r1 := r1 + r2\n");

    // TODO sth[u]xe

    // TODO sthbrx[e]

    TEST_DECODE("stmw 30, 4(2)", "\xbf\xc2\x00\x04",
                "0x00001000    0 *32* m[r2 + 4] := r30\n"
                "              0 *32* m[r2 + 8] := r31\n");

    // TODO stswi

    // TODO stswx

    TEST_DECODE("stw 3, 1(2)", "\x90\x62\x00\x01", "0x00001000    0 *32* m[r2 + 1] := r3\n");

    TEST_DECODE("stwu 3, 1(2)", "\x94\x62\x00\x01",
                "0x00001000    0 *32* m[r2 + 1] := r3\n"
                "              0 *32* r3 := r2 + 1\n");

    // TODO stw[u]e

    TEST_DECODE("stwx 3, 1(2)", "\x7c\x61\x11\x2e", "0x00001000    0 *32* m[r1 + r2] := r3\n");

    TEST_DECODE("stwux 3, 1(2)", "\x7c\x61\x11\x6e",
                "0x00001000    0 *32* m[r1 + r2] := r3\n"
                "              0 *32* r1 := r1 + r2\n");

    // TODO stw[u]xe

    // TODO stwbrx[e]

    // TODO stwcx[e].

    TEST_DECODE("subf 3, 1, 2", "\x7c\x61\x10\x50", "0x00001000    0 *32* r3 := r2 - r1\n");

    TEST_DECODE(
        "subf. 3, 1, 2", "\x7c\x61\x10\x51",
        "0x00001000    0 *32* r3 := r2 - r1\n              0 *v* %flags := SETFLAGS0( r3 )\n");

    // TODO subfo[.]

    TEST_DECODE("subfc 3, 1, 2", "\x7c\x61\x10\x10",
                "0x00001000    0 *32* r3 := r2 - r1\n"
                "              0 *v* %flags := SUBFLAGSX( r3, r2, r1 )\n");

    TEST_DECODE("subfc. 3, 1, 2", "\x7c\x61\x10\x11",
                "0x00001000    0 *32* r3 := r2 - r1\n"
                "              0 *v* %flags := SUBFLAGSX0( r3, r2, r1 )\n");

    // TODO subfco[.]

    // TODO subfe*

    TEST_DECODE("subfic 3, 1, 2", "\x20\x61\x00\x02",
                "0x00001000    0 *32* r3 := 2 - r1\n"
                "              0 *v* %flags := SUBFLAGSX( r3, 2, r1 )\n");

    // TODO subfme*

    TEST_DECODE("subfze 3, 1", "\x7c\x61\x01\x90", "0x00001000    0 *32* r3 := ~r1 + r203\n");

    TEST_DECODE("subfze. 3, 1", "\x7c\x61\x01\x91",
                "0x00001000    0 *32* r3 := ~r1 + r203\n"
                "              0 *v* %flags := SUBFLAGS0( r3 )\n");

    // TODO subfzeo[.]

    // TODO subfze64[o]

    // TODO td[i]

    // TODO tlb*

    // TODO tw[i]

    // TODO wrtee[i]

    TEST_DECODE("xor 3, 1, 2", "\x7c\x23\x12\x78", "0x00001000    0 *32* r3 := r1 ^ r2\n");

    TEST_DECODE("xor. 3, 1, 2", "\x7c\x23\x12\x79",
                "0x00001000    0 *32* r3 := r1 ^ r2\n"
                "              0 *v* %flags := SETFLAGS0( r3 )\n");

    TEST_DECODE("xori 2, 3, 0x40", "\x68\x62\x00\x40", "0x00001000    0 *32* r2 := r3 ^ 64\n");

    TEST_DECODE("xoris 2, 3, 1", "\x6c\x62\x00\x01", "0x00001000    0 *32* r2 := r3 ^ 0x10000\n");
}


QTEST_GUILESS_MAIN(CapstonePPCDecoderTest)
