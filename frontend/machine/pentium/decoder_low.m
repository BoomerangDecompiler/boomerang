/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2000, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       decoder_low.m
 * OVERVIEW:   An implementation of the low level decoding method.
 *             This has been factored out to make compilation of .m
 *             more modular and therefore quicker when changes are
 *             only made in one module.
 *============================================================================*/

/*
 * 11 Aug 99 - Mike: DIS_EADDR->DIS_EADDR32 etc (for movsx etc)
 *
 * 15 Sep 99 - Mike: Fixed MOVSX.Gv.Ew and MOVZX.Gv.Ew (were wrong register
 *              sizes)
 * 05 Jun 00 - Mike: IDX -> D_IDX (name clash with sslparser.h)
 * 26 Jun 00 - Mike: D_IDX -> DIS_IDX; all other such macros start with DIS_ now
 *  1 Aug 00 - Cristina: upgraded to support mltk version Apr 5 14:56:28 
 *              EDT 2000.  [deltaPC] renamed to nextPC as semantics of 
 *              [deltaPC] has changed.   Assignment of (nextPC - hostPC). 
 * 02 Oct 00 - Mike: Fixed semantics of FLD.STi with the DIS_IDXP1 macro
 * 20 Apr 01 - Mike: REP and REPNE are true prefixes now
 * 11 May 01 - Mike: Removed redundant [name]'s to reduce many warnings
 * 17 Oct 01 - Mike: Added calls to unused() for even fewer warnings;
 *              added PUSH.Ixow
 * 19 Oct 01 - Mike: Commented out most of the instructions that aren't in the
 *              SSL file, so when speculatively decoding, you don't get the
 *              "no entry for `MOV.Sw.Ew' in RTL dictionary" assert failure
 * 14 Mar 02 - Mike: Added PUSHF/POPF to the above (e.g. opcode 9D)
*/

#include "global.h"
#include "decoder.h"
#include "rtl.h"

void unused(int x);         // decoder.m

/*==============================================================================
 * FUNCTION:       NJMCDecoder::decodeLowLevelInstruction
 * OVERVIEW:       Decodes a machine instruction and returns an instantiated
 *                 list of RTs.
 * PARAMETERS:     hostPC - the address of the pc in the loaded Elf object
 *                 pc - the virtual address of the pc
 *                 rtlDict - the dictionary of RTL templates used to instantiate
 *                   the RTL for the instruction being decoded
 *                 result - a reference parameter that has a field that will be
 *                   set to false if an invalid or UNIMP instruction was decoded
 * RETURNS:        the instantiated list of RTs
 *============================================================================*/
list<RT*>* NJMCDecoder::decodeLowLevelInstruction (ADDRESS hostPC, ADDRESS pc,
    DecodeResult& result)
{

    // The list of instantiated RTs.
    list<RT*>* RTs = NULL;

    ADDRESS nextPC; 

    match [nextPC] hostPC to

    | XLATB() =>
        RTs = instantiate(pc,  "XLATB");

    | XCHG.Ev.Gvod(Eaddr, reg) =>
        RTs = instantiate(pc,  "XCHG.Ev.Gvod", DIS_EADDR32, DIS_REG32);

    | XCHG.Ev.Gvow(Eaddr, reg) =>
        RTs = instantiate(pc,  "XCHG.Ev.Gvow", DIS_EADDR16, DIS_REG16);

    | XCHG.Eb.Gb(Eaddr, reg) =>
        RTs = instantiate(pc,  "XCHG.Eb.Gb", DIS_EADDR8, DIS_REG8);

    | NOP() =>
        RTs = instantiate(pc,  "NOP");

    | SEG.FS() =>        // For now, treat as a 1 byte NOP
        RTs = instantiate(pc,  "NOP");

    | SEG.GS() =>
        RTs = instantiate(pc,  "NOP");

    | XCHGeAXod(r32) =>
        RTs = instantiate(pc,  "XCHGeAXod", DIS_R32);

    | XCHGeAXow(r32) =>
        RTs = instantiate(pc,  "XCHGeAXow", DIS_R32);

    | XADD.Ev.Gvod(Eaddr, reg) =>
        RTs = instantiate(pc,  "XADD.Ev.Gvod", DIS_EADDR32, DIS_REG32);

    | XADD.Ev.Gvow(Eaddr, reg) =>
        RTs = instantiate(pc,  "XADD.Ev.Gvow", DIS_EADDR16, DIS_REG16);

    | XADD.Eb.Gb(Eaddr, reg) =>
        RTs = instantiate(pc,  "XADD.Eb.Gb", DIS_EADDR8, DIS_REG8);

    | WRMSR() =>
        RTs = instantiate(pc,  "WRMSR");

    | WBINVD() =>
        RTs = instantiate(pc,  "WBINVD");

    | WAIT() =>
        RTs = instantiate(pc,  "WAIT");

    | VERW(Eaddr) =>
        RTs = instantiate(pc,  "VERW", DIS_EADDR32);

    | VERR(Eaddr) =>
        RTs = instantiate(pc,  "VERR", DIS_EADDR32);

    | TEST.Ev.Gvod(Eaddr, reg) =>
        RTs = instantiate(pc,  "TEST.Ev.Gvod", DIS_EADDR32, DIS_REG32);

    | TEST.Ev.Gvow(Eaddr, reg) =>
        RTs = instantiate(pc,  "TEST.Ev.Gvow", DIS_EADDR16, DIS_REG16);

    | TEST.Eb.Gb(Eaddr, reg) =>
        RTs = instantiate(pc,  "TEST.Eb.Gb", DIS_EADDR8, DIS_REG8);

    | TEST.Ed.Id(Eaddr, i32) =>
        RTs = instantiate(pc,  "TEST.Ed.Id", DIS_EADDR32, DIS_I32);

    | TEST.Ew.Iw(Eaddr, i16) =>
        RTs = instantiate(pc,  "TEST.Ew.Iw", DIS_EADDR16, DIS_I16);

    | TEST.Eb.Ib(Eaddr, i8) =>
        RTs = instantiate(pc,  "TEST.Eb.Ib", DIS_EADDR8, DIS_I8);

    | TEST.eAX.Ivod(i32) =>
        RTs = instantiate(pc,  "TEST.eAX.Ivod", DIS_I32);

    | TEST.eAX.Ivow(i16) =>
        RTs = instantiate(pc,  "TEST.eAX.Ivow", DIS_I16);

    | TEST.AL.Ib(i8) =>
        RTs = instantiate(pc,  "TEST.AL.Ib", DIS_I8);

    | STR(Mem) =>
        RTs = instantiate(pc,  "STR", DIS_MEM);

    | STOSvod() =>
        RTs = instantiate(pc,  "STOSvod");

    | STOSvow() =>
        RTs = instantiate(pc,  "STOSvow");

    | STOSB() =>
        RTs = instantiate(pc,  "STOSB");

    | STI() =>
        RTs = instantiate(pc,  "STI");

    | STD() =>
        RTs = instantiate(pc,  "STD");

    | STC() =>
        RTs = instantiate(pc,  "STC");

    | SMSW(Eaddr) =>
        RTs = instantiate(pc,  "SMSW", DIS_EADDR32);

    | SLDT(Eaddr) =>
        RTs = instantiate(pc,  "SLDT", DIS_EADDR32);

    | SHLD.CLod(Eaddr, reg) =>
        RTs = instantiate(pc,  "SHLD.CLod", DIS_EADDR32, DIS_REG32);

    | SHLD.CLow(Eaddr, reg) =>
        RTs = instantiate(pc,  "SHLD.CLow", DIS_EADDR16, DIS_REG16);

    | SHRD.CLod(Eaddr, reg) =>
        RTs = instantiate(pc,  "SHRD.CLod", DIS_EADDR32, DIS_REG32);

    | SHRD.CLow(Eaddr, reg) =>
        RTs = instantiate(pc,  "SHRD.CLow", DIS_EADDR16, DIS_REG16);

    | SHLD.Ibod(Eaddr, reg, count) =>
        RTs = instantiate(pc,  "SHLD.Ibod", DIS_EADDR32, DIS_REG32, DIS_COUNT);

    | SHLD.Ibow(Eaddr, reg, count) =>
        RTs = instantiate(pc,  "SHLD.Ibow", DIS_EADDR16, DIS_REG16, DIS_COUNT);

    | SHRD.Ibod(Eaddr, reg, count) =>
        RTs = instantiate(pc,  "SHRD.Ibod", DIS_EADDR32, DIS_REG32, DIS_COUNT);

    | SHRD.Ibow(Eaddr, reg, count) =>
        RTs = instantiate(pc,  "SHRD.Ibow", DIS_EADDR16, DIS_REG16, DIS_COUNT);

    | SIDT(Mem) =>
        RTs = instantiate(pc,  "SIDT", DIS_MEM);

    | SGDT(Mem) =>
        RTs = instantiate(pc,  "SGDT", DIS_MEM);

    // Sets are now in the high level instructions
    | SCASvod() =>
        RTs = instantiate(pc,  "SCASvod");

    | SCASvow() =>
        RTs = instantiate(pc,  "SCASvow");

    | SCASB() =>
        RTs = instantiate(pc,  "SCASB");

    | SAHF() =>
        RTs = instantiate(pc,  "SAHF");

    | RSM() =>
        RTs = instantiate(pc,  "RSM");

    | RET.far.Iw(i16) =>
        RTs = instantiate(pc,  "RET.far.Iw", DIS_I16);

    | RET.Iw(i16) =>
        RTs = instantiate(pc,  "RET.Iw", DIS_I16);

    | RET.far() =>
        RTs = instantiate(pc,  "RET.far");

    | RET() =>
        RTs = instantiate(pc,  "RET");

//   | REPNE() =>
//      RTs = instantiate(pc,  "REPNE");

//  | REP() =>
//      RTs = instantiate(pc,  "REP");

    | REP.CMPSB() [name] =>
        RTs = instantiate(pc,  name);

    | REP.CMPSvow() [name] =>
        RTs = instantiate(pc,  name);

    | REP.CMPSvod() [name] =>
        RTs = instantiate(pc,  name);

    | REP.LODSB() [name] =>
        RTs = instantiate(pc,  name);

    | REP.LODSvow() [name] =>
        RTs = instantiate(pc,  name);

    | REP.LODSvod() [name] =>
        RTs = instantiate(pc,  name);

    | REP.MOVSB() [name] =>
        RTs = instantiate(pc,  name);

    | REP.MOVSvow() [name] =>
        RTs = instantiate(pc,  name);

    | REP.MOVSvod() [name] =>
        RTs = instantiate(pc,  name);

    | REP.SCASB() [name] =>
        RTs = instantiate(pc,  name);

    | REP.SCASvow() [name] =>
        RTs = instantiate(pc,  name);

    | REP.SCASvod() [name] =>
        RTs = instantiate(pc,  name);

    | REP.STOSB() [name] =>
        RTs = instantiate(pc,  name);

    | REP.STOSvow() [name] =>
        RTs = instantiate(pc,  name);

    | REP.STOSvod() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.CMPSB() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.CMPSvow() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.CMPSvod() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.LODSB() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.LODSvow() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.LODSvod() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.MOVSB() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.MOVSvow() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.MOVSvod() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.SCASB() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.SCASvow() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.SCASvod() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.STOSB() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.STOSvow() [name] =>
        RTs = instantiate(pc,  name);

    | REPNE.STOSvod() [name] =>
        RTs = instantiate(pc,  name);

    | RDMSR() =>
        RTs = instantiate(pc,  "RDMSR");

    | SARB.Ev.Ibod(Eaddr, i8) =>
        RTs = instantiate(pc,  "SARB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | SARB.Ev.Ibow(Eaddr, i8) =>
        RTs = instantiate(pc,  "SARB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | SHRB.Ev.Ibod(Eaddr, i8) =>
        RTs = instantiate(pc,  "SHRB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | SHRB.Ev.Ibow(Eaddr, i8) =>
        RTs = instantiate(pc,  "SHRB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | SHLSALB.Ev.Ibod(Eaddr, i8) =>
        RTs = instantiate(pc,  "SHLSALB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | SHLSALB.Ev.Ibow(Eaddr, i8) =>
        RTs = instantiate(pc,  "SHLSALB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | RCRB.Ev.Ibod(Eaddr, i8) =>
        RTs = instantiate(pc,  "RCRB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | RCRB.Ev.Ibow(Eaddr, i8) =>
        RTs = instantiate(pc,  "RCRB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | RCLB.Ev.Ibod(Eaddr, i8) =>
        RTs = instantiate(pc,  "RCLB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | RCLB.Ev.Ibow(Eaddr, i8) =>
        RTs = instantiate(pc,  "RCLB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | RORB.Ev.Ibod(Eaddr, i8) =>
        RTs = instantiate(pc,  "RORB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | RORB.Ev.Ibow(Eaddr, i8) =>
        RTs = instantiate(pc,  "RORB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | ROLB.Ev.Ibod(Eaddr, i8) =>
        RTs = instantiate(pc,  "ROLB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | ROLB.Ev.Ibow(Eaddr, i8) =>
        RTs = instantiate(pc,  "ROLB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | SARB.Eb.Ib(Eaddr, i8) =>
        RTs = instantiate(pc,  "SARB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | SHRB.Eb.Ib(Eaddr, i8) =>
        RTs = instantiate(pc,  "SHRB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | SHLSALB.Eb.Ib(Eaddr, i8) =>
        RTs = instantiate(pc,  "SHLSALB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | RCRB.Eb.Ib(Eaddr, i8) =>
        RTs = instantiate(pc,  "RCRB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | RCLB.Eb.Ib(Eaddr, i8) =>
        RTs = instantiate(pc,  "RCLB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | RORB.Eb.Ib(Eaddr, i8) =>
        RTs = instantiate(pc,  "RORB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | ROLB.Eb.Ib(Eaddr, i8) =>
        RTs = instantiate(pc,  "ROLB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | SARB.Ev.CLod(Eaddr) =>
        RTs = instantiate(pc,  "SARB.Ev.CLod", DIS_EADDR32);

    | SARB.Ev.CLow(Eaddr) =>
        RTs = instantiate(pc,  "SARB.Ev.CLow", DIS_EADDR16);

    | SARB.Ev.1od(Eaddr) =>
        RTs = instantiate(pc,  "SARB.Ev.1od", DIS_EADDR32);

    | SARB.Ev.1ow(Eaddr) =>
        RTs = instantiate(pc,  "SARB.Ev.1ow", DIS_EADDR16);

    | SHRB.Ev.CLod(Eaddr) =>
        RTs = instantiate(pc,  "SHRB.Ev.CLod", DIS_EADDR32);

    | SHRB.Ev.CLow(Eaddr) =>
        RTs = instantiate(pc,  "SHRB.Ev.CLow", DIS_EADDR16);

    | SHRB.Ev.1od(Eaddr) =>
        RTs = instantiate(pc,  "SHRB.Ev.1od", DIS_EADDR32);

    | SHRB.Ev.1ow(Eaddr) =>
        RTs = instantiate(pc,  "SHRB.Ev.1ow", DIS_EADDR16);

    | SHLSALB.Ev.CLod(Eaddr) =>
        RTs = instantiate(pc,  "SHLSALB.Ev.CLod", DIS_EADDR32);

    | SHLSALB.Ev.CLow(Eaddr) =>
        RTs = instantiate(pc,  "SHLSALB.Ev.CLow", DIS_EADDR16);

    | SHLSALB.Ev.1od(Eaddr) =>
        RTs = instantiate(pc,  "SHLSALB.Ev.1od", DIS_EADDR32);

    | SHLSALB.Ev.1ow(Eaddr) =>
        RTs = instantiate(pc,  "SHLSALB.Ev.1ow", DIS_EADDR16);

    | RCRB.Ev.CLod(Eaddr) =>
        RTs = instantiate(pc,  "RCRB.Ev.CLod", DIS_EADDR32);

    | RCRB.Ev.CLow(Eaddr) =>
        RTs = instantiate(pc,  "RCRB.Ev.CLow", DIS_EADDR16);

    | RCRB.Ev.1od(Eaddr) =>
        RTs = instantiate(pc,  "RCRB.Ev.1od", DIS_EADDR32);

    | RCRB.Ev.1ow(Eaddr) =>
        RTs = instantiate(pc,  "RCRB.Ev.1ow", DIS_EADDR16);

    | RCLB.Ev.CLod(Eaddr) =>
        RTs = instantiate(pc,  "RCLB.Ev.CLod", DIS_EADDR32);

    | RCLB.Ev.CLow(Eaddr) =>
        RTs = instantiate(pc,  "RCLB.Ev.CLow", DIS_EADDR16);

    | RCLB.Ev.1od(Eaddr) =>
        RTs = instantiate(pc,  "RCLB.Ev.1od", DIS_EADDR32);

    | RCLB.Ev.1ow(Eaddr) =>
        RTs = instantiate(pc,  "RCLB.Ev.1ow", DIS_EADDR16);

    | RORB.Ev.CLod(Eaddr) =>
        RTs = instantiate(pc,  "RORB.Ev.CLod", DIS_EADDR32);

    | RORB.Ev.CLow(Eaddr) =>
        RTs = instantiate(pc,  "RORB.Ev.CLow", DIS_EADDR16);

    | RORB.Ev.1od(Eaddr) =>
        RTs = instantiate(pc,  "RORB.Ev.1od", DIS_EADDR32);

    | RORB.Ev.1ow(Eaddr) =>
        RTs = instantiate(pc,  "ORB.Ev.1owR", DIS_EADDR16);

    | ROLB.Ev.CLod(Eaddr) =>
        RTs = instantiate(pc,  "ROLB.Ev.CLod", DIS_EADDR32);

    | ROLB.Ev.CLow(Eaddr) =>
        RTs = instantiate(pc,  "ROLB.Ev.CLow", DIS_EADDR16);

    | ROLB.Ev.1od(Eaddr) =>
        RTs = instantiate(pc,  "ROLB.Ev.1od", DIS_EADDR32);

    | ROLB.Ev.1ow(Eaddr) =>
        RTs = instantiate(pc,  "ROLB.Ev.1ow", DIS_EADDR16);

    | SARB.Eb.CL(Eaddr) =>
        RTs = instantiate(pc,  "SARB.Eb.CL", DIS_EADDR32);

    | SARB.Eb.1(Eaddr) =>
        RTs = instantiate(pc,  "SARB.Eb.1", DIS_EADDR16);

    | SHRB.Eb.CL(Eaddr) =>
        RTs = instantiate(pc,  "SHRB.Eb.CL", DIS_EADDR8);

    | SHRB.Eb.1(Eaddr) =>
        RTs = instantiate(pc,  "SHRB.Eb.1", DIS_EADDR8);

    | SHLSALB.Eb.CL(Eaddr) =>
        RTs = instantiate(pc,  "SHLSALB.Eb.CL", DIS_EADDR8);

    | SHLSALB.Eb.1(Eaddr) =>
        RTs = instantiate(pc,  "SHLSALB.Eb.1", DIS_EADDR8);

    | RCRB.Eb.CL(Eaddr) =>
        RTs = instantiate(pc,  "RCRB.Eb.CL", DIS_EADDR8);

    | RCRB.Eb.1(Eaddr) =>
        RTs = instantiate(pc,  "RCRB.Eb.1", DIS_EADDR8);

    | RCLB.Eb.CL(Eaddr) =>
        RTs = instantiate(pc,  "RCLB.Eb.CL", DIS_EADDR8);

    | RCLB.Eb.1(Eaddr) =>
        RTs = instantiate(pc,  "RCLB.Eb.1", DIS_EADDR8);

    | RORB.Eb.CL(Eaddr) =>
        RTs = instantiate(pc,  "RORB.Eb.CL", DIS_EADDR8);

    | RORB.Eb.1(Eaddr) =>
        RTs = instantiate(pc,  "RORB.Eb.1", DIS_EADDR8);

    | ROLB.Eb.CL(Eaddr) =>
        RTs = instantiate(pc,  "ROLB.Eb.CL", DIS_EADDR8);

    | ROLB.Eb.1(Eaddr) =>
        RTs = instantiate(pc,  "ROLB.Eb.1", DIS_EADDR8);

    // There is no SSL for these, so don't call instantiate, it will only
    // cause an assert failure. Also, may as well treat these as invalid instr
//    | PUSHFod() =>
//        RTs = instantiate(pc,  "PUSHFod");

//    | PUSHFow() =>
//        RTs = instantiate(pc,  "PUSHFow");

//    | PUSHAod() =>
//        RTs = instantiate(pc,  "PUSHAod");

//    | PUSHAow() =>
//        RTs = instantiate(pc,  "PUSHAow");

    | PUSH.GS() =>
        RTs = instantiate(pc,  "PUSH.GS");

    | PUSH.FS() =>
        RTs = instantiate(pc,  "PUSH.FS");

    | PUSH.ES() =>
        RTs = instantiate(pc,  "PUSH.ES");

    | PUSH.DS() =>
        RTs = instantiate(pc,  "PUSH.DS");

    | PUSH.SS() =>
        RTs = instantiate(pc,  "PUSH.SS");

    | PUSH.CS() =>
        RTs = instantiate(pc,  "PUSH.CS");

    | PUSH.Ivod(i32) =>
        RTs = instantiate(pc,  "PUSH.Ivod", DIS_I32);

    | PUSH.Ivow(i16) =>
        RTs = instantiate(pc,  "PUSH.Ivow", DIS_I16);

    | PUSH.Ixob(i8) =>
        RTs = instantiate(pc,  "PUSH.Ixob", DIS_I8);

    | PUSH.Ixow(i8) =>
        RTs = instantiate(pc,  "PUSH.Ixow", DIS_I8);

    | PUSHod(r32) =>
        RTs = instantiate(pc,  "PUSHod", DIS_R32);

    | PUSHow(r32) =>
        RTs = instantiate(pc,  "PUSHow", DIS_R32);  // Check!

    | PUSH.Evod(Eaddr) =>
        RTs = instantiate(pc,  "PUSH.Evod", DIS_EADDR32);

    | PUSH.Evow(Eaddr) =>
        RTs = instantiate(pc,  "PUSH.Evow", DIS_EADDR16);

//    | POPFod() =>
//        RTs = instantiate(pc,  "POPFod");

//    | POPFow() =>
//        RTs = instantiate(pc,  "POPFow");

//    | POPAod() =>
//        RTs = instantiate(pc,  "POPAod");

//    | POPAow() =>
//        RTs = instantiate(pc,  "POPAow");

    | POP.GS() =>
        RTs = instantiate(pc,  "POP.GS");

    | POP.FS() =>
        RTs = instantiate(pc,  "POP.FS");

    | POP.DS() =>
        RTs = instantiate(pc,  "POP.DS");

    | POP.SS() =>
        RTs = instantiate(pc,  "POP.SS");

    | POP.ES() =>
        RTs = instantiate(pc,  "POP.ES");

    | POPod(r32) =>
        RTs = instantiate(pc,  "POPod", DIS_R32);

    | POPow(r32) =>
        RTs = instantiate(pc,  "POPow", DIS_R32);   // Check!

    | POP.Evod(Mem) =>
        RTs = instantiate(pc,  "POP.Evod", DIS_MEM);

    | POP.Evow(Mem) =>
        RTs = instantiate(pc,  "POP.Evow", DIS_MEM);

//    | OUTSvod() =>
//        RTs = instantiate(pc,  "OUTSvod");

//    | OUTSvow() =>
//        RTs = instantiate(pc,  "OUTSvow");

//    | OUTSB() =>
//        RTs = instantiate(pc,  "OUTSB");

//    | OUT.DX.eAXod() =>
//        RTs = instantiate(pc,  "OUT.DX.eAXod");

//    | OUT.DX.eAXow() =>
//        RTs = instantiate(pc,  "OUT.DX.eAXow");

//    | OUT.DX.AL() =>
//        RTs = instantiate(pc,  "OUT.DX.AL");

//    | OUT.Ib.eAXod(i8) =>
//        RTs = instantiate(pc,  "OUT.Ib.eAXod", DIS_I8);

//    | OUT.Ib.eAXow(i8) =>
//        RTs = instantiate(pc,  "OUT.Ib.eAXow", DIS_I8);

//    | OUT.Ib.AL(i8) =>
//        RTs = instantiate(pc,  "OUT.Ib.AL", DIS_I8);

    | NOTod(Eaddr) =>
        RTs = instantiate(pc,  "NOTod", DIS_EADDR32);

    | NOTow(Eaddr) =>
        RTs = instantiate(pc,  "NOTow", DIS_EADDR16);

    | NOTb(Eaddr) =>
        RTs = instantiate(pc,  "NOTb", DIS_EADDR8);

    | NEGod(Eaddr) =>
        RTs = instantiate(pc,  "NEGod", DIS_EADDR32);

    | NEGow(Eaddr) =>
        RTs = instantiate(pc,  "NEGow", DIS_EADDR16);

    | NEGb(Eaddr) =>
        RTs = instantiate(pc,  "NEGb", DIS_EADDR8);

    | MUL.AXod(Eaddr) =>
        RTs = instantiate(pc,  "MUL.AXod", DIS_EADDR32);

    | MUL.AXow(Eaddr) =>
        RTs = instantiate(pc,  "MUL.AXow", DIS_EADDR16);

    | MUL.AL(Eaddr) =>
        RTs = instantiate(pc,  "MUL.AL", DIS_EADDR8);

    | MOVZX.Gv.Ew(r32, Eaddr) =>
        RTs = instantiate(pc,  "MOVZX.Gv.Ew", DIS_R32, DIS_EADDR16);

    | MOVZX.Gv.Ebod(r32, Eaddr) =>
        RTs = instantiate(pc,  "MOVZX.Gv.Ebod", DIS_R32, DIS_EADDR8);

    | MOVZX.Gv.Ebow(r16, Eaddr) =>
        RTs = instantiate(pc,  "MOVZX.Gv.Ebow", DIS_R16, DIS_EADDR8);

    | MOVSX.Gv.Ew(r32, Eaddr) =>
        RTs = instantiate(pc,  "MOVSX.Gv.Ew", DIS_R32, DIS_EADDR16);

    | MOVSX.Gv.Ebod(r32, Eaddr) =>
        RTs = instantiate(pc,  "MOVSX.Gv.Ebod", DIS_R32, DIS_EADDR8);

    | MOVSX.Gv.Ebow(r16, Eaddr) =>
        RTs = instantiate(pc,  "MOVZX.Gv.Ebow", DIS_R16, DIS_EADDR8);

    | MOVSvod() =>
        RTs = instantiate(pc,  "MOVSvod");

    | MOVSvow() =>
        RTs = instantiate(pc,  "MOVSvow");

    | MOVSB() =>
        RTs = instantiate(pc,  "MOVSB");

//    | MOV.Rd.Dd(reg, dr) => 
//        unused(reg); unused(dr);
//        RTs = instantiate(pc,  "UNIMP");

//    | MOV.Dd.Rd(dr, reg) =>
//        unused(reg); unused(dr);
//        RTs = instantiate(pc,  "UNIMP");

//    | MOV.Rd.Cd(reg, cr) =>
//        unused(reg); unused(cr);
//        RTs = instantiate(pc,  "UNIMP");

//    | MOV.Cd.Rd(cr, reg) =>
//        unused(reg); unused(cr);
//        RTs = instantiate(pc,  "UNIMP");

    | MOV.Eb.Ivod(Eaddr, i32) =>
        RTs = instantiate(pc,  "MOV.Eb.Ivod", DIS_EADDR32, DIS_I32);

    | MOV.Eb.Ivow(Eaddr, i16) =>
        RTs = instantiate(pc,  "MOV.Eb.Ivow", DIS_EADDR16, DIS_I16);

    | MOV.Eb.Ib(Eaddr, i8) =>
        RTs = instantiate(pc,  "MOV.Eb.Ib", DIS_EADDR8, DIS_I8);

    | MOVid(r32, i32) =>
        RTs = instantiate(pc,  "MOVid", DIS_R32, DIS_I32);

    | MOViw(r16, i16) =>
        RTs = instantiate(pc,  "MOViw", DIS_R16, DIS_I16);  // Check!

    | MOVib(r8, i8) =>
        RTs = instantiate(pc,  "MOVib", DIS_R8, DIS_I8);

    | MOV.Ov.eAXod(off) =>
        RTs = instantiate(pc,  "MOV.Ov.eAXod", DIS_OFF);

    | MOV.Ov.eAXow(off) =>
        RTs = instantiate(pc,  "MOV.Ov.eAXow", DIS_OFF);

    | MOV.Ob.AL(off) =>
        RTs = instantiate(pc,  "MOV.Ob.AL", DIS_OFF);

    | MOV.eAX.Ovod(off) =>
        RTs = instantiate(pc,  "MOV.eAX.Ovod", DIS_OFF);

    | MOV.eAX.Ovow(off) =>
        RTs = instantiate(pc,  "MOV.eAX.Ovow", DIS_OFF);

    | MOV.AL.Ob(off) =>
        RTs = instantiate(pc,  "MOV.AL.Ob", DIS_OFF);

//    | MOV.Sw.Ew(Mem, sr16) =>
//        RTs = instantiate(pc,  "MOV.Sw.Ew", DIS_MEM, DIS_SR16);

//    | MOV.Ew.Sw(Mem, sr16) =>
//        RTs = instantiate(pc,  "MOV.Ew.Sw", DIS_MEM, DIS_SR16);

    | MOVrmod(reg, Eaddr) =>
        RTs = instantiate(pc,  "MOVrmod", DIS_REG32, DIS_EADDR32);

    | MOVrmow(reg, Eaddr) =>
        RTs = instantiate(pc,  "MOVrmow", DIS_REG16, DIS_EADDR16);

    | MOVrmb(reg, Eaddr) =>
        RTs = instantiate(pc,  "MOVrmb", DIS_REG8, DIS_EADDR8);

    | MOVmrod(Eaddr, reg) =>
        RTs = instantiate(pc,  "MOVmrod", DIS_EADDR32, DIS_REG32);

    | MOVmrow(Eaddr, reg) =>
        RTs = instantiate(pc,  "MOVmrow", DIS_EADDR16, DIS_REG16);

    | MOVmrb(Eaddr, reg) =>
        RTs = instantiate(pc,  "MOVmrb", DIS_EADDR8, DIS_REG8);

    | LTR(Eaddr) =>
        RTs = instantiate(pc,  "LTR", DIS_EADDR32);

    | LSS(reg, Mem) =>
        RTs = instantiate(pc,  "LSS", DIS_REG32, DIS_MEM);

    | LSLod(reg, Eaddr) =>
        RTs = instantiate(pc,  "LSLod", DIS_REG32, DIS_EADDR32);

    | LSLow(reg, Eaddr) =>
        RTs = instantiate(pc,  "LSLow", DIS_REG16, DIS_EADDR16);

    | LOOPNE(relocd) =>
        RTs = instantiate(pc,  "LOOPNE", dis_Num(relocd - hostPC - 2));

    | LOOPE(relocd) =>
        RTs = instantiate(pc,  "LOOPE", dis_Num(relocd-hostPC-2));

    | LOOP(relocd) =>
        RTs = instantiate(pc,  "LOOP", dis_Num(relocd-hostPC-2));

    | LGS(reg, Mem) =>
        RTs = instantiate(pc,  "LGS", DIS_REG32, DIS_MEM);

    | LFS(reg, Mem) =>
        RTs = instantiate(pc,  "LFS", DIS_REG32, DIS_MEM);

    | LES(reg, Mem) =>
        RTs = instantiate(pc,  "LES", DIS_REG32, DIS_MEM);

    | LEAVE() =>
        RTs = instantiate(pc,  "LEAVE");

    | LEAod(reg, Mem) =>
        RTs = instantiate(pc,  "LEA.od", DIS_REG32, DIS_MEM);

    | LEAow(reg, Mem) =>
        RTs = instantiate(pc,  "LEA.ow", DIS_REG16, DIS_MEM);

    | LDS(reg, Mem) =>
        RTs = instantiate(pc,  "LDS", DIS_REG32, DIS_MEM);

    | LARod(reg, Eaddr) =>
        RTs = instantiate(pc,  "LAR.od", DIS_REG32, DIS_EADDR32);

    | LARow(reg, Eaddr) =>
        RTs = instantiate(pc,  "LAR.ow", DIS_REG16, DIS_EADDR16);

    | LAHF() =>
        RTs = instantiate(pc,  "LAHF");

    /* Branches have been handled in decodeInstruction() now */
    | IRET() =>
        RTs = instantiate(pc,  "IRET");

    | INVLPG(Mem) =>
        RTs = instantiate(pc,  "INVLPG", DIS_MEM);

    | INVD() =>
        RTs = instantiate(pc,  "INVD");

    | INTO() =>
        RTs = instantiate(pc,  "INTO");

    | INT.Ib(i8) =>
        RTs = instantiate(pc,  "INT.Ib", DIS_I8);

    | INT3() =>
        RTs = instantiate(pc,  "INT3");

//    | INSvod() =>
//        RTs = instantiate(pc,  "INSvod");

//    | INSvow() =>
//        RTs = instantiate(pc,  "INSvow");

//    | INSB() =>
//        RTs = instantiate(pc,  "INSB");

    | INCod(r32) =>
        RTs = instantiate(pc,  "INCod", DIS_R32);

    | INCow(r32) =>
        RTs = instantiate(pc,  "INCow", DIS_R32);

    | INC.Evod(Eaddr) =>
        RTs = instantiate(pc,  "INC.Evod", DIS_EADDR32);

    | INC.Evow(Eaddr) =>
        RTs = instantiate(pc,  "INC.Evow", DIS_EADDR16);

    | INC.Eb(Eaddr) =>
        RTs = instantiate(pc,  "INC.Eb", DIS_EADDR8);

//    | IN.eAX.DXod() =>
//        RTs = instantiate(pc,  "IN.eAX.DXod");

//    | IN.eAX.DXow() =>
//        RTs = instantiate(pc,  "IN.eAX.DXow");

//    | IN.AL.DX() =>
//        RTs = instantiate(pc,  "IN.AL.DX");

//    | IN.eAX.Ibod(i8) =>
//        RTs = instantiate(pc,  "IN.eAX.Ibod", DIS_I8);

//    | IN.eAX.Ibow(i8) =>
//        RTs = instantiate(pc,  "IN.eAX.Ibow", DIS_I8);

//    | IN.AL.Ib(i8) =>
//        RTs = instantiate(pc,  "IN.AL.Ib", DIS_I8);

    | IMUL.Ivd(reg, Eaddr, i32) =>
        RTs = instantiate(pc,  "IMUL.Ivd", DIS_REG32, DIS_EADDR32, DIS_I32);

    | IMUL.Ivw(reg, Eaddr, i16) =>
        RTs = instantiate(pc,  "IMUL.Ivw", DIS_REG16, DIS_EADDR16, DIS_I16);

    | IMUL.Ibod(reg, Eaddr, i8) =>
        RTs = instantiate(pc,  "IMUL.Ibod", DIS_REG32, DIS_EADDR32, DIS_I8);

    | IMUL.Ibow(reg, Eaddr, i8) =>
        RTs = instantiate(pc,  "IMUL.Ibow", DIS_REG16, DIS_EADDR16, DIS_I8);

    | IMULrmod(reg, Eaddr) =>
        RTs = instantiate(pc,  "IMULrmod", DIS_REG32, DIS_EADDR32);

    | IMULrmow(reg, Eaddr) =>
        RTs = instantiate(pc,  "IMULrmow", DIS_REG16, DIS_EADDR16);

    | IMULod(Eaddr) =>
        RTs = instantiate(pc,  "IMULod", DIS_EADDR32);

    | IMULow(Eaddr) =>
        RTs = instantiate(pc,  "IMULow", DIS_EADDR16);

    | IMULb(Eaddr) =>
        RTs = instantiate(pc,  "IMULb", DIS_EADDR8);

    | IDIVeAX(Eaddr) =>
        RTs = instantiate(pc,  "IDIVeAX", DIS_EADDR32);

    | IDIVAX(Eaddr) =>
        RTs = instantiate(pc,  "IDIVAX", DIS_EADDR16);

    | IDIV(Eaddr) =>
        RTs = instantiate(pc,  "IDIV", DIS_EADDR8); /* ?? */

//  | HLT() =>
//      RTs = instantiate(pc,  "HLT");

    | ENTER(i16, i8) =>
        RTs = instantiate(pc,  "ENTER", DIS_I16, DIS_I8);

    | DIVeAX(Eaddr) =>
        RTs = instantiate(pc,  "DIVeAX", DIS_EADDR32);

    | DIVAX(Eaddr) =>
        RTs = instantiate(pc,  "DIVAX", DIS_EADDR16);

    | DIVAL(Eaddr) =>
        RTs = instantiate(pc,  "DIVAL", DIS_EADDR8);

    | DECod(r32) =>
        RTs = instantiate(pc,  "DECod", DIS_R32);

    | DECow(r32) =>
        RTs = instantiate(pc,  "DECow", DIS_R32);

    | DEC.Evod(Eaddr) =>
        RTs = instantiate(pc,  "DEC.Evod", DIS_EADDR32);

    | DEC.Evow(Eaddr) =>
        RTs = instantiate(pc,  "DEC.Evow", DIS_EADDR16);

    | DEC.Eb(Eaddr) =>
        RTs = instantiate(pc,  "DEC.Eb", DIS_EADDR8);

    | DAS() =>
        RTs = instantiate(pc,  "DAS");

    | DAA() =>
        RTs = instantiate(pc,  "DAA");

    | CDQ() =>
        RTs = instantiate(pc,  "CDQ");

    | CWD() =>
        RTs = instantiate(pc,  "CWD");

    | CPUID() =>
        RTs = instantiate(pc,  "CPUID");

    | CMPXCHG8B(Mem) =>
        RTs = instantiate(pc,  "CMPXCHG8B", DIS_MEM);

    | CMPXCHG.Ev.Gvod(Eaddr, reg) =>
        RTs = instantiate(pc,  "CMPXCHG.Ev.Gvod", DIS_EADDR32, DIS_REG32);

    | CMPXCHG.Ev.Gvow(Eaddr, reg) =>
        RTs = instantiate(pc,  "CMPXCHG.Ev.Gvow", DIS_EADDR16, DIS_REG16);

    | CMPXCHG.Eb.Gb(Eaddr, reg) =>
        RTs = instantiate(pc,  "CMPXCHG.Eb.Gb", DIS_EADDR8, DIS_REG8);

    | CMPSvod() =>
        RTs = instantiate(pc,  "CMPSvod");

    | CMPSvow() =>
        RTs = instantiate(pc,  "CMPSvow");

    | CMPSB() =>
        RTs = instantiate(pc,  "CMPSB");

    | CMC() =>
        RTs = instantiate(pc,  "CMC");

    | CLTS() =>
        RTs = instantiate(pc,  "CLTS");

    | CLI() =>
        RTs = instantiate(pc,  "CLI");

    | CLD() =>
        RTs = instantiate(pc,  "CLD");

    | CLC() =>
        RTs = instantiate(pc,  "CLC");

    | CWDE() =>
        RTs = instantiate(pc,  "CWDE");

    | CBW() =>
        RTs = instantiate(pc,  "CBW");

    /* Decode the following as a NOP. We see these in startup code, and anywhere
        that calls the OS (as lcall 7, 0) */
    | CALL.aPod(seg, off) =>
        unused(seg); unused(off);
        RTs = instantiate(pc, "NOP");

    | CALL.Jvod(relocd) [name] =>
        RTs = instantiate(pc,  name, dis_Num(relocd-hostPC-5));

    | CALL.Evod(Eaddr) =>
        RTs = instantiate(pc,  "CALL.Evod", DIS_EADDR32);

    | BTSiod(Eaddr, i8) =>
        RTs = instantiate(pc,  "BTSiod", DIS_I8, DIS_EADDR32);

    | BTSiow(Eaddr, i8) =>
        RTs = instantiate(pc,  "BTSiow", DIS_I8, DIS_EADDR16);

    | BTSod(Eaddr, reg) =>
        RTs = instantiate(pc,  "BTSod", DIS_EADDR32, DIS_REG32);

    | BTSow(Eaddr, reg) =>
        RTs = instantiate(pc,  "BTSow", DIS_EADDR16, DIS_REG16);

    | BTRiod(Eaddr, i8) =>
        RTs = instantiate(pc,  "BTRiod", DIS_EADDR32, DIS_I8);

    | BTRiow(Eaddr, i8) =>
        RTs = instantiate(pc,  "BTRiow", DIS_EADDR16, DIS_I8);

    | BTRod(Eaddr, reg) =>
        RTs = instantiate(pc,  "BTRod", DIS_EADDR32, DIS_REG32);

    | BTRow(Eaddr, reg) =>
        RTs = instantiate(pc,  "BTRow", DIS_EADDR16, DIS_REG16);

    | BTCiod(Eaddr, i8) =>
        RTs = instantiate(pc,  "BTCiod", DIS_EADDR32, DIS_I8);

    | BTCiow(Eaddr, i8) =>
        RTs = instantiate(pc,  "BTCiow", DIS_EADDR16, DIS_I8);

    | BTCod(Eaddr, reg) =>
        RTs = instantiate(pc,  "BTCod", DIS_EADDR32, DIS_REG32);

    | BTCow(Eaddr, reg) =>
        RTs = instantiate(pc,  "BTCow", DIS_EADDR16, DIS_REG16);

    | BTiod(Eaddr, i8) =>
        RTs = instantiate(pc,  "BTiod", DIS_EADDR32, DIS_I8);

    | BTiow(Eaddr, i8) =>
        RTs = instantiate(pc,  "BTiow", DIS_EADDR16, DIS_I8);

    | BTod(Eaddr, reg) =>
        RTs = instantiate(pc,  "BTod", DIS_EADDR32, DIS_REG32);

    | BTow(Eaddr, reg) =>
        RTs = instantiate(pc,  "BTow", DIS_EADDR16, DIS_REG16);

    | BSWAP(r32) =>
        RTs = instantiate(pc,  "BSWAP", DIS_R32);

    | BSRod(reg, Eaddr) =>
        RTs = instantiate(pc,  "BSRod", DIS_REG32, DIS_EADDR32);

    | BSRow(reg, Eaddr) =>
        RTs = instantiate(pc,  "BSRow", DIS_REG16, DIS_EADDR16);

    | BSFod(reg, Eaddr) =>
        RTs = instantiate(pc,  "BSFod", DIS_REG32, DIS_EADDR32);

    | BSFow(reg, Eaddr) =>
        RTs = instantiate(pc,  "BSFow", DIS_REG16, DIS_EADDR16);

    // Not "user" instructions:
//  | BOUNDod(reg, Mem) =>
//      RTs = instantiate(pc,  "BOUNDod", DIS_REG32, DIS_MEM);

//  | BOUNDow(reg, Mem) =>
//      RTs = instantiate(pc,  "BOUNDow", DIS_REG16, DIS_MEM);

//    | ARPL(Eaddr, reg ) =>
//        unused(Eaddr); unused(reg);
//        RTs = instantiate(pc,  "UNIMP");

//    | AAS() =>
//        RTs = instantiate(pc,  "AAS");

//    | AAM() =>
//        RTs = instantiate(pc,  "AAM");

//    | AAD() =>
//        RTs = instantiate(pc,  "AAD");

//    | AAA() =>
//        RTs = instantiate(pc,  "AAA");

    | CMPrmod(reg, Eaddr) =>
        RTs = instantiate(pc,  "CMPrmod", DIS_REG32, DIS_EADDR32);

    | CMPrmow(reg, Eaddr) =>
        RTs = instantiate(pc,  "CMPrmow", DIS_REG16, DIS_EADDR16);

    | XORrmod(reg, Eaddr) =>
        RTs = instantiate(pc,  "XORrmod", DIS_REG32, DIS_EADDR32);

    | XORrmow(reg, Eaddr) =>
        RTs = instantiate(pc,  "XORrmow", DIS_REG16, DIS_EADDR16);

    | SUBrmod(reg, Eaddr) =>
        RTs = instantiate(pc,  "SUBrmod", DIS_REG32, DIS_EADDR32);

    | SUBrmow(reg, Eaddr) =>
        RTs = instantiate(pc,  "SUBrmow", DIS_REG16, DIS_EADDR16);

    | ANDrmod(reg, Eaddr) =>
        RTs = instantiate(pc,  "ANDrmod", DIS_REG32, DIS_EADDR32);

    | ANDrmow(reg, Eaddr) =>
        RTs = instantiate(pc,  "ANDrmow", DIS_REG16, DIS_EADDR16);

    | SBBrmod(reg, Eaddr) =>
        RTs = instantiate(pc,  "SBBrmod", DIS_REG32, DIS_EADDR32);

    | SBBrmow(reg, Eaddr) =>
        RTs = instantiate(pc,  "SBBrmow", DIS_REG16, DIS_EADDR16);

    | ADCrmod(reg, Eaddr) =>
        RTs = instantiate(pc,  "ADCrmod", DIS_REG32, DIS_EADDR32);

    | ADCrmow(reg, Eaddr) =>
        RTs = instantiate(pc,  "ADCrmow", DIS_REG16, DIS_EADDR16);

    | ORrmod(reg, Eaddr) =>
        RTs = instantiate(pc,  "ORrmod", DIS_REG32, DIS_EADDR32);

    | ORrmow(reg, Eaddr) =>
        RTs = instantiate(pc,  "ORrmow", DIS_REG16, DIS_EADDR16);

    | ADDrmod(reg, Eaddr) =>
        RTs = instantiate(pc,  "ADDrmod", DIS_REG32, DIS_EADDR32);

    | ADDrmow(reg, Eaddr) =>
        RTs = instantiate(pc,  "ADDrmow", DIS_REG16, DIS_EADDR16);

    | CMPrmb(r8, Eaddr) =>
        RTs = instantiate(pc,  "CMPrmb", DIS_R8, DIS_EADDR8);

    | XORrmb(r8, Eaddr) =>
        RTs = instantiate(pc,  "XORrmb", DIS_R8, DIS_EADDR8);

    | SUBrmb(r8, Eaddr) =>
        RTs = instantiate(pc,  "SUBrmb", DIS_R8, DIS_EADDR8);

    | ANDrmb(r8, Eaddr) =>
        RTs = instantiate(pc,  "ANDrmb", DIS_R8, DIS_EADDR8);

    | SBBrmb(r8, Eaddr) =>
        RTs = instantiate(pc,  "SBBrmb", DIS_R8, DIS_EADDR8);

    | ADCrmb(r8, Eaddr) =>
        RTs = instantiate(pc,  "ADCrmb", DIS_R8, DIS_EADDR8);

    | ORrmb(r8, Eaddr) =>
        RTs = instantiate(pc,  "ORrmb", DIS_R8, DIS_EADDR8);

    | ADDrmb(r8, Eaddr) =>
        RTs = instantiate(pc,  "ADDrmb", DIS_R8, DIS_EADDR8);

    | CMPmrod(Eaddr, reg) =>
        RTs = instantiate(pc,  "CMPmrod", DIS_EADDR32, DIS_REG32);

    | CMPmrow(Eaddr, reg) =>
        RTs = instantiate(pc,  "CMPmrow", DIS_EADDR16, DIS_REG16);

    | XORmrod(Eaddr, reg) =>
        RTs = instantiate(pc,  "XORmrod", DIS_EADDR32, DIS_REG32);

    | XORmrow(Eaddr, reg) =>
        RTs = instantiate(pc,  "XORmrow", DIS_EADDR16, DIS_REG16);

    | SUBmrod(Eaddr, reg) =>
        RTs = instantiate(pc,  "SUBmrod", DIS_EADDR32, DIS_REG32);

    | SUBmrow(Eaddr, reg) =>
        RTs = instantiate(pc,  "SUBmrow", DIS_EADDR16, DIS_REG16);

    | ANDmrod(Eaddr, reg) =>
        RTs = instantiate(pc,  "ANDmrod", DIS_EADDR32, DIS_REG32);

    | ANDmrow(Eaddr, reg) =>
        RTs = instantiate(pc,  "ANDmrow", DIS_EADDR16, DIS_REG16);

    | SBBmrod(Eaddr, reg) =>
        RTs = instantiate(pc,  "SBBmrod", DIS_EADDR32, DIS_REG32);

    | SBBmrow(Eaddr, reg) =>
        RTs = instantiate(pc,  "SBBmrow", DIS_EADDR16, DIS_REG16);

    | ADCmrod(Eaddr, reg) =>
        RTs = instantiate(pc,  "ADCmrod", DIS_EADDR32, DIS_REG32);

    | ADCmrow(Eaddr, reg) =>
        RTs = instantiate(pc,  "ADCmrow", DIS_EADDR16, DIS_REG16);

    | ORmrod(Eaddr, reg) =>
        RTs = instantiate(pc,  "ORmrod", DIS_EADDR32, DIS_REG32);

    | ORmrow(Eaddr, reg) =>
        RTs = instantiate(pc,  "ORmrow", DIS_EADDR16, DIS_REG16);

    | ADDmrod(Eaddr, reg) =>
        RTs = instantiate(pc,  "ADDmrod", DIS_EADDR32, DIS_REG32);

    | ADDmrow(Eaddr, reg) =>
        RTs = instantiate(pc,  "ADDmrow", DIS_EADDR16, DIS_REG16);

    | CMPmrb(Eaddr, r8) =>
        RTs = instantiate(pc,  "CMPmrb", DIS_EADDR8, DIS_R8);

    | XORmrb(Eaddr, r8) =>
        RTs = instantiate(pc,  "XORmrb", DIS_EADDR8, DIS_R8);

    | SUBmrb(Eaddr, r8) =>
        RTs = instantiate(pc,  "SUBmrb", DIS_EADDR8, DIS_R8);

    | ANDmrb(Eaddr, r8) =>
        RTs = instantiate(pc,  "ANDmrb", DIS_EADDR8, DIS_R8);

    | SBBmrb(Eaddr, r8) =>
        RTs = instantiate(pc,  "SBBmrb", DIS_EADDR8, DIS_R8);

    | ADCmrb(Eaddr, r8) =>
        RTs = instantiate(pc,  "ADCmrb", DIS_EADDR8, DIS_R8);

    | ORmrb(Eaddr, r8) =>
        RTs = instantiate(pc,  "ORmrb", DIS_EADDR8, DIS_R8);

    | ADDmrb(Eaddr, r8) =>
        RTs = instantiate(pc,  "ADDmrb", DIS_EADDR8, DIS_R8);

    | CMPiodb(Eaddr, i8) =>
        RTs = instantiate(pc,  "CMPiodb", DIS_EADDR32, DIS_I8);

    | CMPiowb(Eaddr, i8) =>
        RTs = instantiate(pc,  "CMPiowb", DIS_EADDR16, DIS_I8);

    | XORiodb(Eaddr, i8) =>
        RTs = instantiate(pc,  "XORiodb", DIS_EADDR8, DIS_I8);

    | XORiowb(Eaddr, i8) =>
        RTs = instantiate(pc,  "XORiowb", DIS_EADDR16, DIS_I8);

    | SUBiodb(Eaddr, i8) =>
        RTs = instantiate(pc,  "SUBiodb", DIS_EADDR32, DIS_I8);

    | SUBiowb(Eaddr, i8) =>
        RTs = instantiate(pc,  "SUBiowb", DIS_EADDR16, DIS_I8);

    | ANDiodb(Eaddr, i8) =>
        RTs = instantiate(pc,  "ANDiodb", DIS_EADDR8, DIS_I8);

    | ANDiowb(Eaddr, i8) =>
        RTs = instantiate(pc,  "ANDiowb", DIS_EADDR16, DIS_I8);

    | SBBiodb(Eaddr, i8) =>
        RTs = instantiate(pc,  "SBBiodb", DIS_EADDR32, DIS_I8);

    | SBBiowb(Eaddr, i8) =>
        RTs = instantiate(pc,  "SBBiowb", DIS_EADDR16, DIS_I8);

    | ADCiodb(Eaddr, i8) =>
        RTs = instantiate(pc,  "ADCiodb", DIS_EADDR8, DIS_I8);

    | ADCiowb(Eaddr, i8) =>
        RTs = instantiate(pc,  "ADCiowb", DIS_EADDR16, DIS_I8);

    | ORiodb(Eaddr, i8) =>
        RTs = instantiate(pc,  "ORiodb", DIS_EADDR8, DIS_I8);

    | ORiowb(Eaddr, i8) =>
        RTs = instantiate(pc,  "ORiowb", DIS_EADDR16, DIS_I8);

    | ADDiodb(Eaddr, i8) =>
        RTs = instantiate(pc,  "ADDiodb", DIS_EADDR32, DIS_I8);

    | ADDiowb(Eaddr, i8) =>
        RTs = instantiate(pc,  "ADDiowb", DIS_EADDR16, DIS_I8);

    | CMPid(Eaddr, i32) =>
        RTs = instantiate(pc,  "CMPid", DIS_EADDR32, DIS_I32);

    | XORid(Eaddr, i32) =>
        RTs = instantiate(pc,  "XORid", DIS_EADDR32, DIS_I32);

    | SUBid(Eaddr, i32) =>
        RTs = instantiate(pc,  "SUBid", DIS_EADDR32, DIS_I32);

    | ANDid(Eaddr, i32) =>
        RTs = instantiate(pc,  "ANDid", DIS_EADDR32, DIS_I32);

    | SBBid(Eaddr, i32) =>
        RTs = instantiate(pc,  "SBBid", DIS_EADDR32, DIS_I32);

    | ADCid(Eaddr, i32) =>
        RTs = instantiate(pc,  "ADCid", DIS_EADDR32, DIS_I32);

    | ORid(Eaddr, i32) =>
        RTs = instantiate(pc,  "ORid", DIS_EADDR32, DIS_I32);

    | ADDid(Eaddr, i32) =>
        RTs = instantiate(pc,  "ADDid", DIS_EADDR32, DIS_I32);

    | CMPiw(Eaddr, i16) =>
        RTs = instantiate(pc,  "CMPiw", DIS_EADDR16, DIS_I16);

    | XORiw(Eaddr, i16) =>
        RTs = instantiate(pc,  "XORiw", DIS_EADDR16, DIS_I16);

    | SUBiw(Eaddr, i16) =>
        RTs = instantiate(pc,  "SUBiw", DIS_EADDR16, DIS_I16);

    | ANDiw(Eaddr, i16) =>
        RTs = instantiate(pc,  "ANDiw", DIS_EADDR16, DIS_I16);

    | SBBiw(Eaddr, i16) =>
        RTs = instantiate(pc,  "SBBiw", DIS_EADDR16, DIS_I16);

    | ADCiw(Eaddr, i16) =>
        RTs = instantiate(pc,  "ADCiw", DIS_EADDR16, DIS_I16);

    | ORiw(Eaddr, i16) =>
        RTs = instantiate(pc,  "ORiw", DIS_EADDR16, DIS_I16);

    | ADDiw(Eaddr, i16) =>
        RTs = instantiate(pc,  "ADDiw", DIS_EADDR16, DIS_I16);

    | CMPib(Eaddr, i8) =>
        RTs = instantiate(pc,  "CMPib", DIS_EADDR8, DIS_I8);

    | XORib(Eaddr, i8) =>
        RTs = instantiate(pc,  "XORib", DIS_EADDR8, DIS_I8);

    | SUBib(Eaddr, i8) =>
        RTs = instantiate(pc,  "SUBib", DIS_EADDR8, DIS_I8);

    | ANDib(Eaddr, i8) =>
        RTs = instantiate(pc,  "ANDib", DIS_EADDR8, DIS_I8);

    | SBBib(Eaddr, i8) =>
        RTs = instantiate(pc,  "SBBib", DIS_EADDR8, DIS_I8);

    | ADCib(Eaddr, i8) =>
        RTs = instantiate(pc,  "ADCib", DIS_EADDR8, DIS_I8);

    | ORib(Eaddr, i8) =>
        RTs = instantiate(pc,  "ORib", DIS_EADDR8, DIS_I8);

    | ADDib(Eaddr, i8) =>
        RTs = instantiate(pc,  "ADDib", DIS_EADDR8, DIS_I8);

    | CMPiEAX(i32) =>
        RTs = instantiate(pc,  "CMPiEAX", DIS_I32);

    | XORiEAX(i32) =>
        RTs = instantiate(pc,  "XORiEAX", DIS_I32);

    | SUBiEAX(i32) =>
        RTs = instantiate(pc,  "SUBiEAX", DIS_I32);

    | ANDiEAX(i32) =>
        RTs = instantiate(pc,  "ANDiEAX", DIS_I32);

    | SBBiEAX(i32) =>
        RTs = instantiate(pc,  "SBBiEAX", DIS_I32);

    | ADCiEAX(i32) =>
        RTs = instantiate(pc,  "ADCiEAX", DIS_I32);

    | ORiEAX(i32) =>
        RTs = instantiate(pc,  "ORiEAX", DIS_I32);

    | ADDiEAX(i32) =>
        RTs = instantiate(pc,  "ADDiEAX", DIS_I32);

    | CMPiAX(i16) =>
        RTs = instantiate(pc,  "CMPiAX", DIS_I16);

    | XORiAX(i16) =>
        RTs = instantiate(pc,  "XORiAX", DIS_I16);

    | SUBiAX(i16) =>
        RTs = instantiate(pc,  "SUBiAX", DIS_I16);

    | ANDiAX(i16) =>
        RTs = instantiate(pc,  "ANDiAX", DIS_I16);

    | SBBiAX(i16) =>
        RTs = instantiate(pc,  "SBBiAX", DIS_I16);

    | ADCiAX(i16) =>
        RTs = instantiate(pc,  "ADCiAX", DIS_I16);

    | ORiAX(i16) =>
        RTs = instantiate(pc,  "ORiAX", DIS_I16);

    | ADDiAX(i16) =>
        RTs = instantiate(pc,  "ADDiAX", DIS_I16);

    | CMPiAL(i8) =>
        RTs = instantiate(pc,  "CMPiAL", DIS_I8);

    | XORiAL(i8) =>
        RTs = instantiate(pc,  "XORiAL", DIS_I8);

    | SUBiAL(i8) =>
        RTs = instantiate(pc,  "SUBiAL", DIS_I8);

    | ANDiAL(i8) =>
        RTs = instantiate(pc,  "ANDiAL", DIS_I8);

    | SBBiAL(i8) =>
        RTs = instantiate(pc,  "SBBiAL", DIS_I8);

    | ADCiAL(i8) =>
        RTs = instantiate(pc,  "ADCiAL", DIS_I8);

    | ORiAL(i8) =>
        RTs = instantiate(pc,  "ORiAL", DIS_I8);

    | ADDiAL(i8) =>
        RTs = instantiate(pc,  "ADDiAL", DIS_I8);

    | LODSvod() =>
        RTs = instantiate(pc,  "LODSvod");

    | LODSvow() =>
        RTs = instantiate(pc,  "LODSvow");

    | LODSB() =>
        RTs = instantiate(pc,  "LODSB");

    /* Floating point instructions */
    | F2XM1() =>
        RTs = instantiate(pc,  "F2XM1");

    | FABS() =>
        RTs = instantiate(pc,  "FABS");

    | FADD.R32(Mem32) =>
        RTs = instantiate(pc,  "FADD.R32", DIS_MEM32);

    | FADD.R64(Mem64) =>
        RTs = instantiate(pc,  "FADD.R64", DIS_MEM64);

    | FADD.ST.STi(idx) =>
        RTs = instantiate(pc,  "FADD.St.STi", DIS_IDX);

    | FADD.STi.ST(idx) =>
        RTs = instantiate(pc,  "FADD.STi.ST", DIS_IDX);

    | FADDP.STi.ST(idx) =>
        RTs = instantiate(pc,  "FADDP.STi.ST", DIS_IDX);

    | FIADD.I32(Mem32) =>
        RTs = instantiate(pc,  "FIADD.I32", DIS_MEM32);

    | FIADD.I16(Mem16) =>
        RTs = instantiate(pc,  "FIADD.I16", DIS_MEM16);

    | FBLD(Mem80) =>
        RTs = instantiate(pc,  "FBLD", DIS_MEM80);

    | FBSTP(Mem80) =>
        RTs = instantiate(pc,  "FBSTP", DIS_MEM80);

    | FCHS() =>
        RTs = instantiate(pc,  "FCHS");

    | FNCLEX() =>
        RTs = instantiate(pc,  "FNCLEX");

    | FCOM.R32(Mem32) =>
        RTs = instantiate(pc,  "FCOM.R32", DIS_MEM32);

    | FCOM.R64(Mem64) =>
        RTs = instantiate(pc,  "FCOM.R64", DIS_MEM64);

    | FICOM.I32(Mem32) =>
        RTs = instantiate(pc,  "FICOM.I32", DIS_MEM32);

    | FICOM.I16(Mem16) =>
        RTs = instantiate(pc,  "FICOM.I16", DIS_MEM16);

    | FCOMP.R32(Mem32) =>
        RTs = instantiate(pc,  "FCOMP.R32", DIS_MEM32);

    | FCOMP.R64(Mem64) =>
        RTs = instantiate(pc,  "FCOMP.R64", DIS_MEM64);

    | FCOM.ST.STi(idx) =>
        RTs = instantiate(pc,  "FCOM.ST.STi", DIS_IDX);

    | FCOMP.ST.STi(idx) =>
        RTs = instantiate(pc,  "FCOMP.ST.STi", DIS_IDX);

    | FICOMP.I32(Mem32) =>
        RTs = instantiate(pc,  "FICOMP.I32", DIS_MEM32);

    | FICOMP.I16(Mem16) =>
        RTs = instantiate(pc,  "FICOMP.I16", DIS_MEM16);

    | FCOMPP() =>
        RTs = instantiate(pc,  "FCOMPP");

    | FCOMI.ST.STi(idx) [name] =>
        RTs = instantiate(pc, name, DIS_IDX);

    | FCOMIP.ST.STi(idx) [name] =>
        RTs = instantiate(pc, name, DIS_IDX);

    | FCOS() =>
        RTs = instantiate(pc,  "FCOS");

    | FDECSTP() =>
        RTs = instantiate(pc,  "FDECSTP");

    | FDIV.R32(Mem32) =>
        RTs = instantiate(pc,  "FDIV.R32", DIS_MEM32);

    | FDIV.R64(Mem64) =>
        RTs = instantiate(pc,  "FDIV.R64", DIS_MEM64);

    | FDIV.ST.STi(idx) =>
        RTs = instantiate(pc,  "FDIV.ST.STi", DIS_IDX);

    | FDIV.STi.ST(idx) =>
        RTs = instantiate(pc,  "FDIV.STi.ST", DIS_IDX);

    | FDIVP.STi.ST(idx) =>
        RTs = instantiate(pc,  "FDIVP.STi.ST", DIS_IDX);

    | FIDIV.I32(Mem32) =>
        RTs = instantiate(pc,  "FIDIV.I32", DIS_MEM32);

    | FIDIV.I16(Mem16) =>
        RTs = instantiate(pc,  "FIDIV.I16", DIS_MEM16);

    | FDIVR.R32(Mem32) =>
        RTs = instantiate(pc,  "FDIVR.R32", DIS_MEM32);

    | FDIVR.R64(Mem64) =>
        RTs = instantiate(pc,  "FDIVR.R64", DIS_MEM64);

    | FDIVR.ST.STi(idx) =>
        RTs = instantiate(pc,  "FDIVR.ST.STi", DIS_IDX);

    | FDIVR.STi.ST(idx) =>
        RTs = instantiate(pc,  "FDIVR.STi.ST", DIS_IDX);

    | FIDIVR.I32(Mem32) =>
        RTs = instantiate(pc,  "FIDIVR.I32", DIS_MEM32);

    | FIDIVR.I16(Mem16) =>
        RTs = instantiate(pc,  "FIDIVR.I16", DIS_MEM16);

    | FDIVRP.STi.ST(idx) =>
        RTs = instantiate(pc,  "FDIVRP.STi.ST", DIS_IDX);

    | FFREE(idx) =>
        RTs = instantiate(pc,  "FFREE", DIS_IDX);

    | FILD.lsI16(Mem16) =>
        RTs = instantiate(pc,  "FILD.lsI16", DIS_MEM16);

    | FILD.lsI32(Mem32) =>
        RTs = instantiate(pc,  "FILD.lsI32", DIS_MEM32);

    | FILD64(Mem64) =>
        RTs = instantiate(pc,  "FILD64", DIS_MEM64);

    | FINIT() =>
        RTs = instantiate(pc,  "FINIT");

    | FIST.lsI16(Mem16) =>
        RTs = instantiate(pc,  "FIST.lsI16", DIS_MEM16);

    | FIST.lsI32(Mem32) =>
        RTs = instantiate(pc,  "FIST.lsI32", DIS_MEM32);

    | FISTP.lsI16(Mem16) =>
        RTs = instantiate(pc,  "FISTP.lsI16", DIS_MEM16);

    | FISTP.lsI32(Mem32) =>
        RTs = instantiate(pc,  "FISTP.lsI32", DIS_MEM32);

    | FISTP64(Mem64) =>
        RTs = instantiate(pc,  "FISTP64", DIS_MEM64);

    | FLD.lsR32(Mem32) =>
        RTs = instantiate(pc,  "FLD.lsR32", DIS_MEM32);

    | FLD.lsR64(Mem64) =>
        RTs = instantiate(pc,  "FLD.lsR64", DIS_MEM64);

    | FLD80(Mem80) =>
        RTs = instantiate(pc,  "FLD80", DIS_MEM80);

/* This is a bit tricky. The FPUSH logically comes between the read of STi and
# the write to ST0. In particular, FLD ST0 is supposed to duplicate the TOS.
# This problem only happens with this load instruction, so there is a work
# around here that gives us the SSL a value of i that is one more than in
# the instruction */
    | FLD.STi(idx) =>
        RTs = instantiate(pc,  "FLD.STi", DIS_IDXP1);

    | FLD1() =>
        RTs = instantiate(pc,  "FLD1");

    | FLDL2T() =>
        RTs = instantiate(pc,  "FLDL2T");

    | FLDL2E() =>
        RTs = instantiate(pc,  "FLDL2E");

    | FLDPI() =>
        RTs = instantiate(pc,  "FLDPI");

    | FLDLG2() =>
        RTs = instantiate(pc,  "FLDLG2");

    | FLDLN2() =>
        RTs = instantiate(pc,  "FLDLN2");

    | FLDZ() =>
        RTs = instantiate(pc,  "FLDZ");

    | FLDCW(Mem16) =>
        RTs = instantiate(pc,  "FLDCW", DIS_MEM16);

    | FLDENV(Mem) =>
        RTs = instantiate(pc,  "FLDENV", DIS_MEM);

    | FMUL.R32(Mem32) =>
        RTs = instantiate(pc,  "FMUL.R32", DIS_MEM32);

    | FMUL.R64(Mem64) =>
        RTs = instantiate(pc,  "FMUL.R64", DIS_MEM64);

    | FMUL.ST.STi(idx) =>
        RTs = instantiate(pc,  "FMUL.ST.STi", DIS_IDX);

    | FMUL.STi.ST(idx) =>
        RTs = instantiate(pc,  "FMUL.STi.ST", DIS_IDX);

    | FMULP.STi.ST(idx) =>
        RTs = instantiate(pc,  "FMULP.STi.ST", DIS_IDX);

    | FIMUL.I32(Mem32) =>
        RTs = instantiate(pc,  "FIMUL.I32", DIS_MEM32);

    | FIMUL.I16(Mem16) =>
        RTs = instantiate(pc,  "FIMUL.I16", DIS_MEM16);

    | FNOP() =>
        RTs = instantiate(pc,  "FNOP");

    | FPATAN() =>
        RTs = instantiate(pc,  "FPATAN");

    | FPREM() =>
        RTs = instantiate(pc,  "FPREM");

    | FPREM1() =>
        RTs = instantiate(pc,  "FPREM1");

    | FPTAN() =>
        RTs = instantiate(pc,  "FPTAN");

    | FRNDINT() =>
        RTs = instantiate(pc,  "FRNDINT");

    | FRSTOR(Mem) =>
        RTs = instantiate(pc,  "FRSTOR", DIS_MEM);

    | FNSAVE(Mem) =>
        RTs = instantiate(pc,  "FNSAVE", DIS_MEM);

    | FSCALE() =>
        RTs = instantiate(pc,  "FSCALE");

    | FSIN() =>
        RTs = instantiate(pc,  "FSIN");

    | FSINCOS() =>
        RTs = instantiate(pc,  "FSINCOS");

    | FSQRT() =>
        RTs = instantiate(pc,  "FSQRT");

    | FST.lsR32(Mem32) =>
        RTs = instantiate(pc,  "FST.lsR32", DIS_MEM32);

    | FST.lsR64(Mem64) =>
        RTs = instantiate(pc,  "FST.lsR64", DIS_MEM64);

    | FSTP.lsR32(Mem32) =>
        RTs = instantiate(pc,  "FSTP.lsR32", DIS_MEM32);

    | FSTP.lsR64(Mem64) =>
        RTs = instantiate(pc,  "FSTP.lsR64", DIS_MEM64);

    | FSTP80(Mem80) =>
        RTs = instantiate(pc,  "FSTP80", DIS_MEM80);

    | FST.st.STi(idx) =>
        RTs = instantiate(pc,  "FST.st.STi", DIS_IDX);

    | FSTP.st.STi(idx) =>
        RTs = instantiate(pc,  "FSTP.st.STi", DIS_IDX);

    | FSTCW(Mem16) =>
        RTs = instantiate(pc,  "FSTCW", DIS_MEM16);

    | FSTENV(Mem) =>
        RTs = instantiate(pc,  "FSTENV", DIS_MEM);

    | FSTSW(Mem16) =>
        RTs = instantiate(pc,  "FSTSW", DIS_MEM16);

    | FSTSW.AX() =>
        RTs = instantiate(pc,  "FSTSW.AX");

    | FSUB.R32(Mem32) =>
        RTs = instantiate(pc,  "FSUB.R32", DIS_MEM32);

    | FSUB.R64(Mem64) =>
        RTs = instantiate(pc,  "FSUB.R64", DIS_MEM64);

    | FSUB.ST.STi(idx) =>
        RTs = instantiate(pc,  "FSUB.ST.STi", DIS_IDX);

    | FSUB.STi.ST(idx) =>
        RTs = instantiate(pc,  "FSUB.STi.ST", DIS_IDX);

    | FISUB.I32(Mem32) =>
        RTs = instantiate(pc,  "FISUB.I32", DIS_MEM32);

    | FISUB.I16(Mem16) =>
        RTs = instantiate(pc,  "FISUB.I16", DIS_MEM16);

    | FSUBP.STi.ST(idx) =>
        RTs = instantiate(pc,  "FSUBP.STi.ST", DIS_IDX);

    | FSUBR.R32(Mem32) =>
        RTs = instantiate(pc,  "FSUBR.R32", DIS_MEM32);

    | FSUBR.R64(Mem64) =>
        RTs = instantiate(pc,  "FSUBR.R64", DIS_MEM64);

    | FSUBR.ST.STi(idx) =>
        RTs = instantiate(pc,  "FSUBR.ST.STi", DIS_IDX);

    | FSUBR.STi.ST(idx) =>
        RTs = instantiate(pc,  "FSUBR.STi.ST", DIS_IDX);

    | FISUBR.I32(Mem32) =>
        RTs = instantiate(pc,  "FISUBR.I32", DIS_MEM32);

    | FISUBR.I16(Mem16) =>
        RTs = instantiate(pc,  "FISUBR.I16", DIS_MEM16);

    | FSUBRP.STi.ST(idx) =>
        RTs = instantiate(pc,  "FSUBRP.STi.ST", DIS_IDX);

    | FTST() =>
        RTs = instantiate(pc,  "FTST");

    | FUCOM(idx) =>
        RTs = instantiate(pc,  "FUCOM", DIS_IDX);

    | FUCOMP(idx) =>
        RTs = instantiate(pc,  "FUCOMP", DIS_IDX);

    | FUCOMPP() =>
        RTs = instantiate(pc,  "FUCOMPP");

    | FUCOMI.ST.STi(idx) [name] =>
        RTs = instantiate(pc, name, DIS_IDX);

    | FUCOMIP.ST.STi(idx) [name] =>
        RTs = instantiate(pc, name, DIS_IDX);

    | FXAM() =>
        RTs = instantiate(pc,  "FXAM");

    | FXCH(idx) =>
        RTs = instantiate(pc,  "FXCH", DIS_IDX);

    | FXTRACT() =>
        RTs = instantiate(pc,  "FXTRACT");

    | FYL2X() =>
        RTs = instantiate(pc,  "FYL2X");

    | FYL2XP1() =>
        RTs = instantiate(pc,  "FYL2XP1");

    else
        result.valid = false;
        result.rtl = NULL;
        result.numBytes = 0;
cout << "Invalid instruction at " << hex << pc << endl;

    endmatch

    // return # of bytes parsed
    result.numBytes = (nextPC - hostPC);
    return RTs;
}

