/*
 * Copyright (C) 2002, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       disassembler.m
 * OVERVIEW:   disassembler skeleton file for x86 SLED specification
 *
 * Created: 7 Jan 02 - Cristina, based on machine/pentium/decoder_low.m and 
 *             machine/sparc/disassembler.m
 *============================================================================*/


#include "global.h"
#include "decoder.h"


/*==============================================================================
 * FUNCTION:       NJMCDecoder::decodeAssemblyInstruction
 * OVERVIEW:       Decodes a machine instruction and displays its assembly
 *                 representation onto the external array _assembly[].
 * PARAMETERS:     pc - the native address of the pc
 *                 delta - the difference between the native address and
 *                  the host address of the pc
 * RETURNS:        number of bytes taken up by the decoded instruction
 *                  (i.e. number of bytes processed)
 *============================================================================*/
int NJMCDecoder::decodeAssemblyInstruction (ADDRESS pc, int delta)
{
	ADDRESS hostPC = pc + delta; 
    ADDRESS nextPC; 

    sprintf(_assembly, "%X: %08X  ", pc, getDword(hostPC) );
    char* str = _assembly + strlen(_assembly);

    match [nextPC] hostPC to

    | XLATB() =>
        sprintf (str, "%s", "XLATB");

    | XCHG.Ev.Gvod(Eaddr, reg) =>
        sprintf (str, "%s %s,%s", "XCHG.Ev.Gvod", DIS_EADDR32, DIS_REG32);

    | XCHG.Ev.Gvow(Eaddr, reg) =>
        sprintf (str, "%s %s,%s", "XCHG.Ev.Gvow", DIS_EADDR16, DIS_REG16);

    | XCHG.Eb.Gb(Eaddr, reg) =>
        sprintf (str, "%s %s,%s", "XCHG.Eb.Gb", DIS_EADDR8, DIS_REG8);

    | NOP() =>
        sprintf (str, "%s", "NOP");

    | SEG.FS() [name] =>   
        sprintf (str, "%s", name);

    | SEG.GS() [name] =>
        sprintf (str, "%s", name);

    | XCHGeAXod(r32) =>
        sprintf (str,  "XCHGeAXod", DIS_R32);

    | XCHGeAXow(r32) =>
        sprintf (str,  "XCHGeAXow", DIS_R32);

    | XADD.Ev.Gvod(Eaddr, reg) =>
        sprintf (str,  "XADD.Ev.Gvod", DIS_EADDR32, DIS_REG32);

    | XADD.Ev.Gvow(Eaddr, reg) =>
        sprintf (str,  "XADD.Ev.Gvow", DIS_EADDR16, DIS_REG16);

    | XADD.Eb.Gb(Eaddr, reg) =>
        sprintf (str,  "XADD.Eb.Gb", DIS_EADDR8, DIS_REG8);

    | WRMSR() =>
        sprintf (str,  "WRMSR");

    | WBINVD() =>
        sprintf (str,  "WBINVD");

    | WAIT() =>
        sprintf (str,  "WAIT");

    | VERW(Eaddr) =>
        sprintf (str,  "VERW", DIS_EADDR32);

    | VERR(Eaddr) =>
        sprintf (str,  "VERR", DIS_EADDR32);

    | TEST.Ev.Gvod(Eaddr, reg) =>
        sprintf (str,  "TEST.Ev.Gvod", DIS_EADDR32, DIS_REG32);

    | TEST.Ev.Gvow(Eaddr, reg) =>
        sprintf (str,  "TEST.Ev.Gvow", DIS_EADDR16, DIS_REG16);

    | TEST.Eb.Gb(Eaddr, reg) =>
        sprintf (str,  "TEST.Eb.Gb", DIS_EADDR8, DIS_REG8);

    | TEST.Ed.Id(Eaddr, i32) =>
        sprintf (str,  "TEST.Ed.Id", DIS_EADDR32, DIS_I32);

    | TEST.Ew.Iw(Eaddr, i16) =>
        sprintf (str,  "TEST.Ew.Iw", DIS_EADDR16, DIS_I16);

    | TEST.Eb.Ib(Eaddr, i8) =>
        sprintf (str,  "TEST.Eb.Ib", DIS_EADDR8, DIS_I8);

    | TEST.eAX.Ivod(i32) =>
        sprintf (str,  "TEST.eAX.Ivod", DIS_I32);

    | TEST.eAX.Ivow(i16) =>
        sprintf (str,  "TEST.eAX.Ivow", DIS_I16);

    | TEST.AL.Ib(i8) =>
        sprintf (str,  "TEST.AL.Ib", DIS_I8);

    | STR(Mem) =>
        sprintf (str,  "STR", DIS_MEM);

    | STOSvod() =>
        sprintf (str,  "STOSvod");

    | STOSvow() =>
        sprintf (str,  "STOSvow");

    | STOSB() =>
        sprintf (str,  "STOSB");

    | STI() =>
        sprintf (str,  "STI");

    | STD() =>
        sprintf (str,  "STD");

    | STC() =>
        sprintf (str,  "STC");

    | SMSW(Eaddr) =>
        sprintf (str,  "SMSW", DIS_EADDR32);

    | SLDT(Eaddr) =>
        sprintf (str,  "SLDT", DIS_EADDR32);

    | SHLD.CLod(Eaddr, reg) =>
        sprintf (str,  "SHLD.CLod", DIS_EADDR32, DIS_REG32);

    | SHLD.CLow(Eaddr, reg) =>
        sprintf (str,  "SHLD.CLow", DIS_EADDR16, DIS_REG16);

    | SHRD.CLod(Eaddr, reg) =>
        sprintf (str,  "SHRD.CLod", DIS_EADDR32, DIS_REG32);

    | SHRD.CLow(Eaddr, reg) =>
        sprintf (str,  "SHRD.CLow", DIS_EADDR16, DIS_REG16);

    | SHLD.Ibod(Eaddr, reg, count) =>
        sprintf (str,  "SHLD.Ibod", DIS_EADDR32, DIS_REG32, DIS_COUNT);

    | SHLD.Ibow(Eaddr, reg, count) =>
        sprintf (str,  "SHLD.Ibow", DIS_EADDR16, DIS_REG16, DIS_COUNT);

    | SHRD.Ibod(Eaddr, reg, count) =>
        sprintf (str,  "SHRD.Ibod", DIS_EADDR32, DIS_REG32, DIS_COUNT);

    | SHRD.Ibow(Eaddr, reg, count) =>
        sprintf (str,  "SHRD.Ibow", DIS_EADDR16, DIS_REG16, DIS_COUNT);

    | SIDT(Mem) =>
        sprintf (str,  "SIDT", DIS_MEM);

    | SGDT(Mem) =>
        sprintf (str,  "SGDT", DIS_MEM);

    // Sets are now in the high level instructions
    | SCASvod() =>
        sprintf (str,  "SCASvod");

    | SCASvow() =>
        sprintf (str,  "SCASvow");

    | SCASB() =>
        sprintf (str,  "SCASB");

    | SAHF() =>
        sprintf (str,  "SAHF");

    | RSM() =>
        sprintf (str,  "RSM");

    | RET.far.Iw(i16) =>
        sprintf (str,  "RET.far.Iw", DIS_I16);

    | RET.Iw(i16) =>
        sprintf (str,  "RET.Iw", DIS_I16);

    | RET.far() =>
        sprintf (str,  "RET.far");

    | RET() =>
        sprintf (str,  "RET");

//   | REPNE() =>
//      sprintf (str,  "REPNE");

//  | REP() =>
//      sprintf (str,  "REP");

    | REP.CMPSB() [name] =>
        sprintf (str,  name);

    | REP.CMPSvow() [name] =>
        sprintf (str,  name);

    | REP.CMPSvod() [name] =>
        sprintf (str,  name);

    | REP.LODSB() [name] =>
        sprintf (str,  name);

    | REP.LODSvow() [name] =>
        sprintf (str,  name);

    | REP.LODSvod() [name] =>
        sprintf (str,  name);

    | REP.MOVSB() [name] =>
        sprintf (str,  name);

    | REP.MOVSvow() [name] =>
        sprintf (str,  name);

    | REP.MOVSvod() [name] =>
        sprintf (str,  name);

    | REP.SCASB() [name] =>
        sprintf (str,  name);

    | REP.SCASvow() [name] =>
        sprintf (str,  name);

    | REP.SCASvod() [name] =>
        sprintf (str,  name);

    | REP.STOSB() [name] =>
        sprintf (str,  name);

    | REP.STOSvow() [name] =>
        sprintf (str,  name);

    | REP.STOSvod() [name] =>
        sprintf (str,  name);

    | REPNE.CMPSB() [name] =>
        sprintf (str,  name);

    | REPNE.CMPSvow() [name] =>
        sprintf (str,  name);

    | REPNE.CMPSvod() [name] =>
        sprintf (str,  name);

    | REPNE.LODSB() [name] =>
        sprintf (str,  name);

    | REPNE.LODSvow() [name] =>
        sprintf (str,  name);

    | REPNE.LODSvod() [name] =>
        sprintf (str,  name);

    | REPNE.MOVSB() [name] =>
        sprintf (str,  name);

    | REPNE.MOVSvow() [name] =>
        sprintf (str,  name);

    | REPNE.MOVSvod() [name] =>
        sprintf (str,  name);

    | REPNE.SCASB() [name] =>
        sprintf (str,  name);

    | REPNE.SCASvow() [name] =>
        sprintf (str,  name);

    | REPNE.SCASvod() [name] =>
        sprintf (str,  name);

    | REPNE.STOSB() [name] =>
        sprintf (str,  name);

    | REPNE.STOSvow() [name] =>
        sprintf (str,  name);

    | REPNE.STOSvod() [name] =>
        sprintf (str,  name);

    | RDMSR() =>
        sprintf (str,  "RDMSR");

    | SARB.Ev.Ibod(Eaddr, i8) =>
        sprintf (str,  "SARB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | SARB.Ev.Ibow(Eaddr, i8) =>
        sprintf (str,  "SARB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | SHRB.Ev.Ibod(Eaddr, i8) =>
        sprintf (str,  "SHRB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | SHRB.Ev.Ibow(Eaddr, i8) =>
        sprintf (str,  "SHRB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | SHLSALB.Ev.Ibod(Eaddr, i8) =>
        sprintf (str,  "SHLSALB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | SHLSALB.Ev.Ibow(Eaddr, i8) =>
        sprintf (str,  "SHLSALB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | RCRB.Ev.Ibod(Eaddr, i8) =>
        sprintf (str,  "RCRB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | RCRB.Ev.Ibow(Eaddr, i8) =>
        sprintf (str,  "RCRB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | RCLB.Ev.Ibod(Eaddr, i8) =>
        sprintf (str,  "RCLB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | RCLB.Ev.Ibow(Eaddr, i8) =>
        sprintf (str,  "RCLB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | RORB.Ev.Ibod(Eaddr, i8) =>
        sprintf (str,  "RORB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | RORB.Ev.Ibow(Eaddr, i8) =>
        sprintf (str,  "RORB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | ROLB.Ev.Ibod(Eaddr, i8) =>
        sprintf (str,  "ROLB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | ROLB.Ev.Ibow(Eaddr, i8) =>
        sprintf (str,  "ROLB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | SARB.Eb.Ib(Eaddr, i8) =>
        sprintf (str,  "SARB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | SHRB.Eb.Ib(Eaddr, i8) =>
        sprintf (str,  "SHRB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | SHLSALB.Eb.Ib(Eaddr, i8) =>
        sprintf (str,  "SHLSALB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | RCRB.Eb.Ib(Eaddr, i8) =>
        sprintf (str,  "RCRB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | RCLB.Eb.Ib(Eaddr, i8) =>
        sprintf (str,  "RCLB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | RORB.Eb.Ib(Eaddr, i8) =>
        sprintf (str,  "RORB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | ROLB.Eb.Ib(Eaddr, i8) =>
        sprintf (str,  "ROLB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | SARB.Ev.CLod(Eaddr) =>
        sprintf (str,  "SARB.Ev.CLod", DIS_EADDR32);

    | SARB.Ev.CLow(Eaddr) =>
        sprintf (str,  "SARB.Ev.CLow", DIS_EADDR16);

    | SARB.Ev.1od(Eaddr) =>
        sprintf (str,  "SARB.Ev.1od", DIS_EADDR32);

    | SARB.Ev.1ow(Eaddr) =>
        sprintf (str,  "SARB.Ev.1ow", DIS_EADDR16);

    | SHRB.Ev.CLod(Eaddr) =>
        sprintf (str,  "SHRB.Ev.CLod", DIS_EADDR32);

    | SHRB.Ev.CLow(Eaddr) =>
        sprintf (str,  "SHRB.Ev.CLow", DIS_EADDR16);

    | SHRB.Ev.1od(Eaddr) =>
        sprintf (str,  "SHRB.Ev.1od", DIS_EADDR32);

    | SHRB.Ev.1ow(Eaddr) =>
        sprintf (str,  "SHRB.Ev.1ow", DIS_EADDR16);

    | SHLSALB.Ev.CLod(Eaddr) =>
        sprintf (str,  "SHLSALB.Ev.CLod", DIS_EADDR32);

    | SHLSALB.Ev.CLow(Eaddr) =>
        sprintf (str,  "SHLSALB.Ev.CLow", DIS_EADDR16);

    | SHLSALB.Ev.1od(Eaddr) =>
        sprintf (str,  "SHLSALB.Ev.1od", DIS_EADDR32);

    | SHLSALB.Ev.1ow(Eaddr) =>
        sprintf (str,  "SHLSALB.Ev.1ow", DIS_EADDR16);

    | RCRB.Ev.CLod(Eaddr) =>
        sprintf (str,  "RCRB.Ev.CLod", DIS_EADDR32);

    | RCRB.Ev.CLow(Eaddr) =>
        sprintf (str,  "RCRB.Ev.CLow", DIS_EADDR16);

    | RCRB.Ev.1od(Eaddr) =>
        sprintf (str,  "RCRB.Ev.1od", DIS_EADDR32);

    | RCRB.Ev.1ow(Eaddr) =>
        sprintf (str,  "RCRB.Ev.1ow", DIS_EADDR16);

    | RCLB.Ev.CLod(Eaddr) =>
        sprintf (str,  "RCLB.Ev.CLod", DIS_EADDR32);

    | RCLB.Ev.CLow(Eaddr) =>
        sprintf (str,  "RCLB.Ev.CLow", DIS_EADDR16);

    | RCLB.Ev.1od(Eaddr) =>
        sprintf (str,  "RCLB.Ev.1od", DIS_EADDR32);

    | RCLB.Ev.1ow(Eaddr) =>
        sprintf (str,  "RCLB.Ev.1ow", DIS_EADDR16);

    | RORB.Ev.CLod(Eaddr) =>
        sprintf (str,  "RORB.Ev.CLod", DIS_EADDR32);

    | RORB.Ev.CLow(Eaddr) =>
        sprintf (str,  "RORB.Ev.CLow", DIS_EADDR16);

    | RORB.Ev.1od(Eaddr) =>
        sprintf (str,  "RORB.Ev.1od", DIS_EADDR32);

    | RORB.Ev.1ow(Eaddr) =>
        sprintf (str,  "ORB.Ev.1owR", DIS_EADDR16);

    | ROLB.Ev.CLod(Eaddr) =>
        sprintf (str,  "ROLB.Ev.CLod", DIS_EADDR32);

    | ROLB.Ev.CLow(Eaddr) =>
        sprintf (str,  "ROLB.Ev.CLow", DIS_EADDR16);

    | ROLB.Ev.1od(Eaddr) =>
        sprintf (str,  "ROLB.Ev.1od", DIS_EADDR32);

    | ROLB.Ev.1ow(Eaddr) =>
        sprintf (str,  "ROLB.Ev.1ow", DIS_EADDR16);

    | SARB.Eb.CL(Eaddr) =>
        sprintf (str,  "SARB.Eb.CL", DIS_EADDR32);

    | SARB.Eb.1(Eaddr) =>
        sprintf (str,  "SARB.Eb.1", DIS_EADDR16);

    | SHRB.Eb.CL(Eaddr) =>
        sprintf (str,  "SHRB.Eb.CL", DIS_EADDR8);

    | SHRB.Eb.1(Eaddr) =>
        sprintf (str,  "SHRB.Eb.1", DIS_EADDR8);

    | SHLSALB.Eb.CL(Eaddr) =>
        sprintf (str,  "SHLSALB.Eb.CL", DIS_EADDR8);

    | SHLSALB.Eb.1(Eaddr) =>
        sprintf (str,  "SHLSALB.Eb.1", DIS_EADDR8);

    | RCRB.Eb.CL(Eaddr) =>
        sprintf (str,  "RCRB.Eb.CL", DIS_EADDR8);

    | RCRB.Eb.1(Eaddr) =>
        sprintf (str,  "RCRB.Eb.1", DIS_EADDR8);

    | RCLB.Eb.CL(Eaddr) =>
        sprintf (str,  "RCLB.Eb.CL", DIS_EADDR8);

    | RCLB.Eb.1(Eaddr) =>
        sprintf (str,  "RCLB.Eb.1", DIS_EADDR8);

    | RORB.Eb.CL(Eaddr) =>
        sprintf (str,  "RORB.Eb.CL", DIS_EADDR8);

    | RORB.Eb.1(Eaddr) =>
        sprintf (str,  "RORB.Eb.1", DIS_EADDR8);

    | ROLB.Eb.CL(Eaddr) =>
        sprintf (str,  "ROLB.Eb.CL", DIS_EADDR8);

    | ROLB.Eb.1(Eaddr) =>
        sprintf (str,  "ROLB.Eb.1", DIS_EADDR8);

    | PUSHFod() =>
        sprintf (str,  "PUSHFod");

    | PUSHFow() =>
        sprintf (str,  "PUSHFow");

//    | PUSHAod() =>
//        sprintf (str,  "PUSHAod");

//    | PUSHAow() =>
//        sprintf (str,  "PUSHAow");

    | PUSH.GS() =>
        sprintf (str,  "PUSH.GS");

    | PUSH.FS() =>
        sprintf (str,  "PUSH.FS");

    | PUSH.ES() =>
        sprintf (str,  "PUSH.ES");

    | PUSH.DS() =>
        sprintf (str,  "PUSH.DS");

    | PUSH.SS() =>
        sprintf (str,  "PUSH.SS");

    | PUSH.CS() =>
        sprintf (str,  "PUSH.CS");

    | PUSH.Ivod(i32) =>
        sprintf (str,  "PUSH.Ivod", DIS_I32);

    | PUSH.Ivow(i16) =>
        sprintf (str,  "PUSH.Ivow", DIS_I16);

    | PUSH.Ixob(i8) =>
        sprintf (str,  "PUSH.Ixob", DIS_I8);

    | PUSH.Ixow(i8) =>
        sprintf (str,  "PUSH.Ixow", DIS_I8);

    | PUSHod(r32) =>
        sprintf (str,  "PUSHod", DIS_R32);

    | PUSHow(r32) =>
        sprintf (str,  "PUSHow", DIS_R32);  // Check!

    | PUSH.Evod(Eaddr) =>
        sprintf (str,  "PUSH.Evod", DIS_EADDR32);

    | PUSH.Evow(Eaddr) =>
        sprintf (str,  "PUSH.Evow", DIS_EADDR16);

    | POPFod() =>
        sprintf (str,  "POPFod");

    | POPFow() =>
        sprintf (str,  "POPFow");

//    | POPAod() =>
//        sprintf (str,  "POPAod");

//    | POPAow() =>
//        sprintf (str,  "POPAow");

    | POP.GS() =>
        sprintf (str,  "POP.GS");

    | POP.FS() =>
        sprintf (str,  "POP.FS");

    | POP.DS() =>
        sprintf (str,  "POP.DS");

    | POP.SS() =>
        sprintf (str,  "POP.SS");

    | POP.ES() =>
        sprintf (str,  "POP.ES");

    | POPod(r32) =>
        sprintf (str,  "POPod", DIS_R32);

    | POPow(r32) =>
        sprintf (str,  "POPow", DIS_R32);   // Check!

    | POP.Evod(Mem) =>
        sprintf (str,  "POP.Evod", DIS_MEM);

    | POP.Evow(Mem) =>
        sprintf (str,  "POP.Evow", DIS_MEM);

//    | OUTSvod() =>
//        sprintf (str,  "OUTSvod");

//    | OUTSvow() =>
//        sprintf (str,  "OUTSvow");

//    | OUTSB() =>
//        sprintf (str,  "OUTSB");

//    | OUT.DX.eAXod() =>
//        sprintf (str,  "OUT.DX.eAXod");

//    | OUT.DX.eAXow() =>
//        sprintf (str,  "OUT.DX.eAXow");

//    | OUT.DX.AL() =>
//        sprintf (str,  "OUT.DX.AL");

//    | OUT.Ib.eAXod(i8) =>
//        sprintf (str,  "OUT.Ib.eAXod", DIS_I8);

//    | OUT.Ib.eAXow(i8) =>
//        sprintf (str,  "OUT.Ib.eAXow", DIS_I8);

//    | OUT.Ib.AL(i8) =>
//        sprintf (str,  "OUT.Ib.AL", DIS_I8);

    | NOTod(Eaddr) =>
        sprintf (str,  "NOTod", DIS_EADDR32);

    | NOTow(Eaddr) =>
        sprintf (str,  "NOTow", DIS_EADDR16);

    | NOTb(Eaddr) =>
        sprintf (str,  "NOTb", DIS_EADDR8);

    | NEGod(Eaddr) =>
        sprintf (str,  "NEGod", DIS_EADDR32);

    | NEGow(Eaddr) =>
        sprintf (str,  "NEGow", DIS_EADDR16);

    | NEGb(Eaddr) =>
        sprintf (str,  "NEGb", DIS_EADDR8);

    | MUL.AXod(Eaddr) =>
        sprintf (str,  "MUL.AXod", DIS_EADDR32);

    | MUL.AXow(Eaddr) =>
        sprintf (str,  "MUL.AXow", DIS_EADDR16);

    | MUL.AL(Eaddr) =>
        sprintf (str,  "MUL.AL", DIS_EADDR8);

    | MOVZX.Gv.Ew(r32, Eaddr) =>
        sprintf (str,  "MOVZX.Gv.Ew", DIS_R32, DIS_EADDR16);

    | MOVZX.Gv.Ebod(r32, Eaddr) =>
        sprintf (str,  "MOVZX.Gv.Ebod", DIS_R32, DIS_EADDR8);

    | MOVZX.Gv.Ebow(r16, Eaddr) =>
        sprintf (str,  "MOVZX.Gv.Ebow", DIS_R16, DIS_EADDR8);

    | MOVSX.Gv.Ew(r32, Eaddr) =>
        sprintf (str,  "MOVSX.Gv.Ew", DIS_R32, DIS_EADDR16);

    | MOVSX.Gv.Ebod(r32, Eaddr) =>
        sprintf (str,  "MOVSX.Gv.Ebod", DIS_R32, DIS_EADDR8);

    | MOVSX.Gv.Ebow(r16, Eaddr) =>
        sprintf (str,  "MOVZX.Gv.Ebow", DIS_R16, DIS_EADDR8);

    | MOVSvod() =>
        sprintf (str,  "MOVSvod");

    | MOVSvow() =>
        sprintf (str,  "MOVSvow");

    | MOVSB() =>
        sprintf (str,  "MOVSB");

//    | MOV.Rd.Dd(reg, dr) => 
//        unused(reg); unused(dr);
//        sprintf (str,  "UNIMP");

//    | MOV.Dd.Rd(dr, reg) =>
//        unused(reg); unused(dr);
//        sprintf (str,  "UNIMP");

//    | MOV.Rd.Cd(reg, cr) =>
//        unused(reg); unused(cr);
//        sprintf (str,  "UNIMP");

//    | MOV.Cd.Rd(cr, reg) =>
//        unused(reg); unused(cr);
//        sprintf (str,  "UNIMP");

    | MOV.Eb.Ivod(Eaddr, i32) =>
        sprintf (str,  "MOV.Eb.Ivod", DIS_EADDR32, DIS_I32);

    | MOV.Eb.Ivow(Eaddr, i16) =>
        sprintf (str,  "MOV.Eb.Ivow", DIS_EADDR16, DIS_I16);

    | MOV.Eb.Ib(Eaddr, i8) =>
        sprintf (str,  "MOV.Eb.Ib", DIS_EADDR8, DIS_I8);

    | MOVid(r32, i32) =>
        sprintf (str,  "MOVid", DIS_R32, DIS_I32);

    | MOViw(r16, i16) =>
        sprintf (str,  "MOViw", DIS_R16, DIS_I16);  // Check!

    | MOVib(r8, i8) =>
        sprintf (str,  "MOVib", DIS_R8, DIS_I8);

    | MOV.Ov.eAXod(off) =>
        sprintf (str,  "MOV.Ov.eAXod", DIS_OFF);

    | MOV.Ov.eAXow(off) =>
        sprintf (str,  "MOV.Ov.eAXow", DIS_OFF);

    | MOV.Ob.AL(off) =>
        sprintf (str,  "MOV.Ob.AL", DIS_OFF);

    | MOV.eAX.Ovod(off) =>
        sprintf (str,  "MOV.eAX.Ovod", DIS_OFF);

    | MOV.eAX.Ovow(off) =>
        sprintf (str,  "MOV.eAX.Ovow", DIS_OFF);

    | MOV.AL.Ob(off) =>
        sprintf (str,  "MOV.AL.Ob", DIS_OFF);

//    | MOV.Sw.Ew(Mem, sr16) =>
//        sprintf (str,  "MOV.Sw.Ew", DIS_MEM, DIS_SR16);

//    | MOV.Ew.Sw(Mem, sr16) =>
//        sprintf (str,  "MOV.Ew.Sw", DIS_MEM, DIS_SR16);

    | MOVrmod(reg, Eaddr) =>
        sprintf (str,  "MOVrmod", DIS_REG32, DIS_EADDR32);

    | MOVrmow(reg, Eaddr) =>
        sprintf (str,  "MOVrmow", DIS_REG16, DIS_EADDR16);

    | MOVrmb(reg, Eaddr) =>
        sprintf (str,  "MOVrmb", DIS_REG8, DIS_EADDR8);

    | MOVmrod(Eaddr, reg) =>
        sprintf (str,  "MOVmrod", DIS_EADDR32, DIS_REG32);

    | MOVmrow(Eaddr, reg) =>
        sprintf (str,  "MOVmrow", DIS_EADDR16, DIS_REG16);

    | MOVmrb(Eaddr, reg) =>
        sprintf (str,  "MOVmrb", DIS_EADDR8, DIS_REG8);

    | LTR(Eaddr) =>
        sprintf (str,  "LTR", DIS_EADDR32);

    | LSS(reg, Mem) =>
        sprintf (str,  "LSS", DIS_REG32, DIS_MEM);

    | LSLod(reg, Eaddr) =>
        sprintf (str,  "LSLod", DIS_REG32, DIS_EADDR32);

    | LSLow(reg, Eaddr) =>
        sprintf (str,  "LSLow", DIS_REG16, DIS_EADDR16);

    | LOOPNE(relocd) =>
        sprintf (str,  "LOOPNE", dis_Num(relocd - hostPC - 2));

    | LOOPE(relocd) =>
        sprintf (str,  "LOOPE", dis_Num(relocd-hostPC-2));

    | LOOP(relocd) =>
        sprintf (str,  "LOOP", dis_Num(relocd-hostPC-2));

    | LGS(reg, Mem) =>
        sprintf (str,  "LGS", DIS_REG32, DIS_MEM);

    | LFS(reg, Mem) =>
        sprintf (str,  "LFS", DIS_REG32, DIS_MEM);

    | LES(reg, Mem) =>
        sprintf (str,  "LES", DIS_REG32, DIS_MEM);

    | LEAVE() =>
        sprintf (str,  "LEAVE");

    | LEAod(reg, Mem) =>
        sprintf (str,  "LEA.od", DIS_REG32, DIS_MEM);

    | LEAow(reg, Mem) =>
        sprintf (str,  "LEA.ow", DIS_REG16, DIS_MEM);

    | LDS(reg, Mem) =>
        sprintf (str,  "LDS", DIS_REG32, DIS_MEM);

    | LARod(reg, Eaddr) =>
        sprintf (str,  "LAR.od", DIS_REG32, DIS_EADDR32);

    | LARow(reg, Eaddr) =>
        sprintf (str,  "LAR.ow", DIS_REG16, DIS_EADDR16);

    | LAHF() =>
        sprintf (str,  "LAHF");

    /* Branches have been handled in decodeInstruction() now */
    | IRET() =>
        sprintf (str,  "IRET");

    | INVLPG(Mem) =>
        sprintf (str,  "INVLPG", DIS_MEM);

    | INVD() =>
        sprintf (str,  "INVD");

    | INTO() =>
        sprintf (str,  "INTO");

    | INT.Ib(i8) =>
        sprintf (str,  "INT.Ib", DIS_I8);

    | INT3() =>
        sprintf (str,  "INT3");

//    | INSvod() =>
//        sprintf (str,  "INSvod");

//    | INSvow() =>
//        sprintf (str,  "INSvow");

//    | INSB() =>
//        sprintf (str,  "INSB");

    | INCod(r32) =>
        sprintf (str,  "INCod", DIS_R32);

    | INCow(r32) =>
        sprintf (str,  "INCow", DIS_R32);

    | INC.Evod(Eaddr) =>
        sprintf (str,  "INC.Evod", DIS_EADDR32);

    | INC.Evow(Eaddr) =>
        sprintf (str,  "INC.Evow", DIS_EADDR16);

    | INC.Eb(Eaddr) =>
        sprintf (str,  "INC.Eb", DIS_EADDR8);

//    | IN.eAX.DXod() =>
//        sprintf (str,  "IN.eAX.DXod");

//    | IN.eAX.DXow() =>
//        sprintf (str,  "IN.eAX.DXow");

//    | IN.AL.DX() =>
//        sprintf (str,  "IN.AL.DX");

//    | IN.eAX.Ibod(i8) =>
//        sprintf (str,  "IN.eAX.Ibod", DIS_I8);

//    | IN.eAX.Ibow(i8) =>
//        sprintf (str,  "IN.eAX.Ibow", DIS_I8);

//    | IN.AL.Ib(i8) =>
//        sprintf (str,  "IN.AL.Ib", DIS_I8);

    | IMUL.Ivd(reg, Eaddr, i32) =>
        sprintf (str,  "IMUL.Ivd", DIS_REG32, DIS_EADDR32, DIS_I32);

    | IMUL.Ivw(reg, Eaddr, i16) =>
        sprintf (str,  "IMUL.Ivw", DIS_REG16, DIS_EADDR16, DIS_I16);

    | IMUL.Ibod(reg, Eaddr, i8) =>
        sprintf (str,  "IMUL.Ibod", DIS_REG32, DIS_EADDR32, DIS_I8);

    | IMUL.Ibow(reg, Eaddr, i8) =>
        sprintf (str,  "IMUL.Ibow", DIS_REG16, DIS_EADDR16, DIS_I8);

    | IMULrmod(reg, Eaddr) =>
        sprintf (str,  "IMULrmod", DIS_REG32, DIS_EADDR32);

    | IMULrmow(reg, Eaddr) =>
        sprintf (str,  "IMULrmow", DIS_REG16, DIS_EADDR16);

    | IMULod(Eaddr) =>
        sprintf (str,  "IMULod", DIS_EADDR32);

    | IMULow(Eaddr) =>
        sprintf (str,  "IMULow", DIS_EADDR16);

    | IMULb(Eaddr) =>
        sprintf (str,  "IMULb", DIS_EADDR8);

    | IDIVeAX(Eaddr) =>
        sprintf (str,  "IDIVeAX", DIS_EADDR32);

    | IDIVAX(Eaddr) =>
        sprintf (str,  "IDIVAX", DIS_EADDR16);

    | IDIV(Eaddr) =>
        sprintf (str,  "IDIV", DIS_EADDR8); /* ?? */

//  | HLT() =>
//      sprintf (str,  "HLT");

    | ENTER(i16, i8) =>
        sprintf (str,  "ENTER", DIS_I16, DIS_I8);

    | DIVeAX(Eaddr) =>
        sprintf (str,  "DIVeAX", DIS_EADDR32);

    | DIVAX(Eaddr) =>
        sprintf (str,  "DIVAX", DIS_EADDR16);

    | DIVAL(Eaddr) =>
        sprintf (str,  "DIVAL", DIS_EADDR8);

    | DECod(r32) =>
        sprintf (str,  "DECod", DIS_R32);

    | DECow(r32) =>
        sprintf (str,  "DECow", DIS_R32);

    | DEC.Evod(Eaddr) =>
        sprintf (str,  "DEC.Evod", DIS_EADDR32);

    | DEC.Evow(Eaddr) =>
        sprintf (str,  "DEC.Evow", DIS_EADDR16);

    | DEC.Eb(Eaddr) =>
        sprintf (str,  "DEC.Eb", DIS_EADDR8);

    | DAS() =>
        sprintf (str,  "DAS");

    | DAA() =>
        sprintf (str,  "DAA");

    | CDQ() =>
        sprintf (str,  "CDQ");

    | CWD() =>
        sprintf (str,  "CWD");

    | CPUID() =>
        sprintf (str,  "CPUID");

    | CMPXCHG8B(Mem) =>
        sprintf (str,  "CMPXCHG8B", DIS_MEM);

    | CMPXCHG.Ev.Gvod(Eaddr, reg) =>
        sprintf (str,  "CMPXCHG.Ev.Gvod", DIS_EADDR32, DIS_REG32);

    | CMPXCHG.Ev.Gvow(Eaddr, reg) =>
        sprintf (str,  "CMPXCHG.Ev.Gvow", DIS_EADDR16, DIS_REG16);

    | CMPXCHG.Eb.Gb(Eaddr, reg) =>
        sprintf (str,  "CMPXCHG.Eb.Gb", DIS_EADDR8, DIS_REG8);

    | CMPSvod() =>
        sprintf (str,  "CMPSvod");

    | CMPSvow() =>
        sprintf (str,  "CMPSvow");

    | CMPSB() =>
        sprintf (str,  "CMPSB");

    | CMC() =>
        sprintf (str,  "CMC");

    | CLTS() =>
        sprintf (str,  "CLTS");

    | CLI() =>
        sprintf (str,  "CLI");

    | CLD() =>
        sprintf (str,  "CLD");

    | CLC() =>
        sprintf (str,  "CLC");

    | CWDE() =>
        sprintf (str,  "CWDE");

    | CBW() =>
        sprintf (str,  "CBW");

    /* Decode the following as a NOP. We see these in startup code, and anywhere
        that calls the OS (as lcall 7, 0) */
    | CALL.aPod(seg, off) =>
        unused(seg); unused(off);
        sprintf (str, "NOP");

    | CALL.Jvod(relocd) [name] =>
        sprintf (str,  name, dis_Num(relocd-hostPC-5));

    | CALL.Evod(Eaddr) =>
        sprintf (str,  "CALL.Evod", DIS_EADDR32);

    | BTSiod(Eaddr, i8) =>
        sprintf (str,  "BTSiod", DIS_I8, DIS_EADDR32);

    | BTSiow(Eaddr, i8) =>
        sprintf (str,  "BTSiow", DIS_I8, DIS_EADDR16);

    | BTSod(Eaddr, reg) =>
        sprintf (str,  "BTSod", DIS_EADDR32, DIS_REG32);

    | BTSow(Eaddr, reg) =>
        sprintf (str,  "BTSow", DIS_EADDR16, DIS_REG16);

    | BTRiod(Eaddr, i8) =>
        sprintf (str,  "BTRiod", DIS_EADDR32, DIS_I8);

    | BTRiow(Eaddr, i8) =>
        sprintf (str,  "BTRiow", DIS_EADDR16, DIS_I8);

    | BTRod(Eaddr, reg) =>
        sprintf (str,  "BTRod", DIS_EADDR32, DIS_REG32);

    | BTRow(Eaddr, reg) =>
        sprintf (str,  "BTRow", DIS_EADDR16, DIS_REG16);

    | BTCiod(Eaddr, i8) =>
        sprintf (str,  "BTCiod", DIS_EADDR32, DIS_I8);

    | BTCiow(Eaddr, i8) =>
        sprintf (str,  "BTCiow", DIS_EADDR16, DIS_I8);

    | BTCod(Eaddr, reg) =>
        sprintf (str,  "BTCod", DIS_EADDR32, DIS_REG32);

    | BTCow(Eaddr, reg) =>
        sprintf (str,  "BTCow", DIS_EADDR16, DIS_REG16);

    | BTiod(Eaddr, i8) =>
        sprintf (str,  "BTiod", DIS_EADDR32, DIS_I8);

    | BTiow(Eaddr, i8) =>
        sprintf (str,  "BTiow", DIS_EADDR16, DIS_I8);

    | BTod(Eaddr, reg) =>
        sprintf (str,  "BTod", DIS_EADDR32, DIS_REG32);

    | BTow(Eaddr, reg) =>
        sprintf (str,  "BTow", DIS_EADDR16, DIS_REG16);

    | BSWAP(r32) =>
        sprintf (str,  "BSWAP", DIS_R32);

    | BSRod(reg, Eaddr) =>
        sprintf (str,  "BSRod", DIS_REG32, DIS_EADDR32);

    | BSRow(reg, Eaddr) =>
        sprintf (str,  "BSRow", DIS_REG16, DIS_EADDR16);

    | BSFod(reg, Eaddr) =>
        sprintf (str,  "BSFod", DIS_REG32, DIS_EADDR32);

    | BSFow(reg, Eaddr) =>
        sprintf (str,  "BSFow", DIS_REG16, DIS_EADDR16);

    // Not "user" instructions:
//  | BOUNDod(reg, Mem) =>
//      sprintf (str,  "BOUNDod", DIS_REG32, DIS_MEM);

//  | BOUNDow(reg, Mem) =>
//      sprintf (str,  "BOUNDow", DIS_REG16, DIS_MEM);

//    | ARPL(Eaddr, reg ) =>
//        unused(Eaddr); unused(reg);
//        sprintf (str,  "UNIMP");

//    | AAS() =>
//        sprintf (str,  "AAS");

//    | AAM() =>
//        sprintf (str,  "AAM");

//    | AAD() =>
//        sprintf (str,  "AAD");

//    | AAA() =>
//        sprintf (str,  "AAA");

    | CMPrmod(reg, Eaddr) =>
        sprintf (str,  "CMPrmod", DIS_REG32, DIS_EADDR32);

    | CMPrmow(reg, Eaddr) =>
        sprintf (str,  "CMPrmow", DIS_REG16, DIS_EADDR16);

    | XORrmod(reg, Eaddr) =>
        sprintf (str,  "XORrmod", DIS_REG32, DIS_EADDR32);

    | XORrmow(reg, Eaddr) =>
        sprintf (str,  "XORrmow", DIS_REG16, DIS_EADDR16);

    | SUBrmod(reg, Eaddr) =>
        sprintf (str,  "SUBrmod", DIS_REG32, DIS_EADDR32);

    | SUBrmow(reg, Eaddr) =>
        sprintf (str,  "SUBrmow", DIS_REG16, DIS_EADDR16);

    | ANDrmod(reg, Eaddr) =>
        sprintf (str,  "ANDrmod", DIS_REG32, DIS_EADDR32);

    | ANDrmow(reg, Eaddr) =>
        sprintf (str,  "ANDrmow", DIS_REG16, DIS_EADDR16);

    | SBBrmod(reg, Eaddr) =>
        sprintf (str,  "SBBrmod", DIS_REG32, DIS_EADDR32);

    | SBBrmow(reg, Eaddr) =>
        sprintf (str,  "SBBrmow", DIS_REG16, DIS_EADDR16);

    | ADCrmod(reg, Eaddr) =>
        sprintf (str,  "ADCrmod", DIS_REG32, DIS_EADDR32);

    | ADCrmow(reg, Eaddr) =>
        sprintf (str,  "ADCrmow", DIS_REG16, DIS_EADDR16);

    | ORrmod(reg, Eaddr) =>
        sprintf (str,  "ORrmod", DIS_REG32, DIS_EADDR32);

    | ORrmow(reg, Eaddr) =>
        sprintf (str,  "ORrmow", DIS_REG16, DIS_EADDR16);

    | ADDrmod(reg, Eaddr) =>
        sprintf (str,  "ADDrmod", DIS_REG32, DIS_EADDR32);

    | ADDrmow(reg, Eaddr) =>
        sprintf (str,  "ADDrmow", DIS_REG16, DIS_EADDR16);

    | CMPrmb(r8, Eaddr) =>
        sprintf (str,  "CMPrmb", DIS_R8, DIS_EADDR8);

    | XORrmb(r8, Eaddr) =>
        sprintf (str,  "XORrmb", DIS_R8, DIS_EADDR8);

    | SUBrmb(r8, Eaddr) =>
        sprintf (str,  "SUBrmb", DIS_R8, DIS_EADDR8);

    | ANDrmb(r8, Eaddr) =>
        sprintf (str,  "ANDrmb", DIS_R8, DIS_EADDR8);

    | SBBrmb(r8, Eaddr) =>
        sprintf (str,  "SBBrmb", DIS_R8, DIS_EADDR8);

    | ADCrmb(r8, Eaddr) =>
        sprintf (str,  "ADCrmb", DIS_R8, DIS_EADDR8);

    | ORrmb(r8, Eaddr) =>
        sprintf (str,  "ORrmb", DIS_R8, DIS_EADDR8);

    | ADDrmb(r8, Eaddr) =>
        sprintf (str,  "ADDrmb", DIS_R8, DIS_EADDR8);

    | CMPmrod(Eaddr, reg) =>
        sprintf (str,  "CMPmrod", DIS_EADDR32, DIS_REG32);

    | CMPmrow(Eaddr, reg) =>
        sprintf (str,  "CMPmrow", DIS_EADDR16, DIS_REG16);

    | XORmrod(Eaddr, reg) =>
        sprintf (str,  "XORmrod", DIS_EADDR32, DIS_REG32);

    | XORmrow(Eaddr, reg) =>
        sprintf (str,  "XORmrow", DIS_EADDR16, DIS_REG16);

    | SUBmrod(Eaddr, reg) =>
        sprintf (str,  "SUBmrod", DIS_EADDR32, DIS_REG32);

    | SUBmrow(Eaddr, reg) =>
        sprintf (str,  "SUBmrow", DIS_EADDR16, DIS_REG16);

    | ANDmrod(Eaddr, reg) =>
        sprintf (str,  "ANDmrod", DIS_EADDR32, DIS_REG32);

    | ANDmrow(Eaddr, reg) =>
        sprintf (str,  "ANDmrow", DIS_EADDR16, DIS_REG16);

    | SBBmrod(Eaddr, reg) =>
        sprintf (str,  "SBBmrod", DIS_EADDR32, DIS_REG32);

    | SBBmrow(Eaddr, reg) =>
        sprintf (str,  "SBBmrow", DIS_EADDR16, DIS_REG16);

    | ADCmrod(Eaddr, reg) =>
        sprintf (str,  "ADCmrod", DIS_EADDR32, DIS_REG32);

    | ADCmrow(Eaddr, reg) =>
        sprintf (str,  "ADCmrow", DIS_EADDR16, DIS_REG16);

    | ORmrod(Eaddr, reg) =>
        sprintf (str,  "ORmrod", DIS_EADDR32, DIS_REG32);

    | ORmrow(Eaddr, reg) =>
        sprintf (str,  "ORmrow", DIS_EADDR16, DIS_REG16);

    | ADDmrod(Eaddr, reg) =>
        sprintf (str,  "ADDmrod", DIS_EADDR32, DIS_REG32);

    | ADDmrow(Eaddr, reg) =>
        sprintf (str,  "ADDmrow", DIS_EADDR16, DIS_REG16);

    | CMPmrb(Eaddr, r8) =>
        sprintf (str,  "CMPmrb", DIS_EADDR8, DIS_R8);

    | XORmrb(Eaddr, r8) =>
        sprintf (str,  "XORmrb", DIS_EADDR8, DIS_R8);

    | SUBmrb(Eaddr, r8) =>
        sprintf (str,  "SUBmrb", DIS_EADDR8, DIS_R8);

    | ANDmrb(Eaddr, r8) =>
        sprintf (str,  "ANDmrb", DIS_EADDR8, DIS_R8);

    | SBBmrb(Eaddr, r8) =>
        sprintf (str,  "SBBmrb", DIS_EADDR8, DIS_R8);

    | ADCmrb(Eaddr, r8) =>
        sprintf (str,  "ADCmrb", DIS_EADDR8, DIS_R8);

    | ORmrb(Eaddr, r8) =>
        sprintf (str,  "ORmrb", DIS_EADDR8, DIS_R8);

    | ADDmrb(Eaddr, r8) =>
        sprintf (str,  "ADDmrb", DIS_EADDR8, DIS_R8);

    | CMPiodb(Eaddr, i8) =>
        sprintf (str,  "CMPiodb", DIS_EADDR32, DIS_I8);

    | CMPiowb(Eaddr, i8) =>
        sprintf (str,  "CMPiowb", DIS_EADDR16, DIS_I8);

    | XORiodb(Eaddr, i8) =>
        sprintf (str,  "XORiodb", DIS_EADDR8, DIS_I8);

    | XORiowb(Eaddr, i8) =>
        sprintf (str,  "XORiowb", DIS_EADDR16, DIS_I8);

    | SUBiodb(Eaddr, i8) =>
        sprintf (str,  "SUBiodb", DIS_EADDR32, DIS_I8);

    | SUBiowb(Eaddr, i8) =>
        sprintf (str,  "SUBiowb", DIS_EADDR16, DIS_I8);

    | ANDiodb(Eaddr, i8) =>
        sprintf (str,  "ANDiodb", DIS_EADDR8, DIS_I8);

    | ANDiowb(Eaddr, i8) =>
        sprintf (str,  "ANDiowb", DIS_EADDR16, DIS_I8);

    | SBBiodb(Eaddr, i8) =>
        sprintf (str,  "SBBiodb", DIS_EADDR32, DIS_I8);

    | SBBiowb(Eaddr, i8) =>
        sprintf (str,  "SBBiowb", DIS_EADDR16, DIS_I8);

    | ADCiodb(Eaddr, i8) =>
        sprintf (str,  "ADCiodb", DIS_EADDR8, DIS_I8);

    | ADCiowb(Eaddr, i8) =>
        sprintf (str,  "ADCiowb", DIS_EADDR16, DIS_I8);

    | ORiodb(Eaddr, i8) =>
        sprintf (str,  "ORiodb", DIS_EADDR8, DIS_I8);

    | ORiowb(Eaddr, i8) =>
        sprintf (str,  "ORiowb", DIS_EADDR16, DIS_I8);

    | ADDiodb(Eaddr, i8) =>
        sprintf (str,  "ADDiodb", DIS_EADDR32, DIS_I8);

    | ADDiowb(Eaddr, i8) =>
        sprintf (str,  "ADDiowb", DIS_EADDR16, DIS_I8);

    | CMPid(Eaddr, i32) =>
        sprintf (str,  "CMPid", DIS_EADDR32, DIS_I32);

    | XORid(Eaddr, i32) =>
        sprintf (str,  "XORid", DIS_EADDR32, DIS_I32);

    | SUBid(Eaddr, i32) =>
        sprintf (str,  "SUBid", DIS_EADDR32, DIS_I32);

    | ANDid(Eaddr, i32) =>
        sprintf (str,  "ANDid", DIS_EADDR32, DIS_I32);

    | SBBid(Eaddr, i32) =>
        sprintf (str,  "SBBid", DIS_EADDR32, DIS_I32);

    | ADCid(Eaddr, i32) =>
        sprintf (str,  "ADCid", DIS_EADDR32, DIS_I32);

    | ORid(Eaddr, i32) =>
        sprintf (str,  "ORid", DIS_EADDR32, DIS_I32);

    | ADDid(Eaddr, i32) =>
        sprintf (str,  "ADDid", DIS_EADDR32, DIS_I32);

    | CMPiw(Eaddr, i16) =>
        sprintf (str,  "CMPiw", DIS_EADDR16, DIS_I16);

    | XORiw(Eaddr, i16) =>
        sprintf (str,  "XORiw", DIS_EADDR16, DIS_I16);

    | SUBiw(Eaddr, i16) =>
        sprintf (str,  "SUBiw", DIS_EADDR16, DIS_I16);

    | ANDiw(Eaddr, i16) =>
        sprintf (str,  "ANDiw", DIS_EADDR16, DIS_I16);

    | SBBiw(Eaddr, i16) =>
        sprintf (str,  "SBBiw", DIS_EADDR16, DIS_I16);

    | ADCiw(Eaddr, i16) =>
        sprintf (str,  "ADCiw", DIS_EADDR16, DIS_I16);

    | ORiw(Eaddr, i16) =>
        sprintf (str,  "ORiw", DIS_EADDR16, DIS_I16);

    | ADDiw(Eaddr, i16) =>
        sprintf (str,  "ADDiw", DIS_EADDR16, DIS_I16);

    | CMPib(Eaddr, i8) =>
        sprintf (str,  "CMPib", DIS_EADDR8, DIS_I8);

    | XORib(Eaddr, i8) =>
        sprintf (str,  "XORib", DIS_EADDR8, DIS_I8);

    | SUBib(Eaddr, i8) =>
        sprintf (str,  "SUBib", DIS_EADDR8, DIS_I8);

    | ANDib(Eaddr, i8) =>
        sprintf (str,  "ANDib", DIS_EADDR8, DIS_I8);

    | SBBib(Eaddr, i8) =>
        sprintf (str,  "SBBib", DIS_EADDR8, DIS_I8);

    | ADCib(Eaddr, i8) =>
        sprintf (str,  "ADCib", DIS_EADDR8, DIS_I8);

    | ORib(Eaddr, i8) =>
        sprintf (str,  "ORib", DIS_EADDR8, DIS_I8);

    | ADDib(Eaddr, i8) =>
        sprintf (str,  "ADDib", DIS_EADDR8, DIS_I8);

    | CMPiEAX(i32) =>
        sprintf (str,  "CMPiEAX", DIS_I32);

    | XORiEAX(i32) =>
        sprintf (str,  "XORiEAX", DIS_I32);

    | SUBiEAX(i32) =>
        sprintf (str,  "SUBiEAX", DIS_I32);

    | ANDiEAX(i32) =>
        sprintf (str,  "ANDiEAX", DIS_I32);

    | SBBiEAX(i32) =>
        sprintf (str,  "SBBiEAX", DIS_I32);

    | ADCiEAX(i32) =>
        sprintf (str,  "ADCiEAX", DIS_I32);

    | ORiEAX(i32) =>
        sprintf (str,  "ORiEAX", DIS_I32);

    | ADDiEAX(i32) =>
        sprintf (str,  "ADDiEAX", DIS_I32);

    | CMPiAX(i16) =>
        sprintf (str,  "CMPiAX", DIS_I16);

    | XORiAX(i16) =>
        sprintf (str,  "XORiAX", DIS_I16);

    | SUBiAX(i16) =>
        sprintf (str,  "SUBiAX", DIS_I16);

    | ANDiAX(i16) =>
        sprintf (str,  "ANDiAX", DIS_I16);

    | SBBiAX(i16) =>
        sprintf (str,  "SBBiAX", DIS_I16);

    | ADCiAX(i16) =>
        sprintf (str,  "ADCiAX", DIS_I16);

    | ORiAX(i16) =>
        sprintf (str,  "ORiAX", DIS_I16);

    | ADDiAX(i16) =>
        sprintf (str,  "ADDiAX", DIS_I16);

    | CMPiAL(i8) =>
        sprintf (str,  "CMPiAL", DIS_I8);

    | XORiAL(i8) =>
        sprintf (str,  "XORiAL", DIS_I8);

    | SUBiAL(i8) =>
        sprintf (str,  "SUBiAL", DIS_I8);

    | ANDiAL(i8) =>
        sprintf (str,  "ANDiAL", DIS_I8);

    | SBBiAL(i8) =>
        sprintf (str,  "SBBiAL", DIS_I8);

    | ADCiAL(i8) =>
        sprintf (str,  "ADCiAL", DIS_I8);

    | ORiAL(i8) =>
        sprintf (str,  "ORiAL", DIS_I8);

    | ADDiAL(i8) =>
        sprintf (str,  "ADDiAL", DIS_I8);

    | LODSvod() =>
        sprintf (str,  "LODSvod");

    | LODSvow() =>
        sprintf (str,  "LODSvow");

    | LODSB() =>
        sprintf (str,  "LODSB");

    /* Floating point instructions */
    | F2XM1() =>
        sprintf (str,  "F2XM1");

    | FABS() =>
        sprintf (str,  "FABS");

    | FADD.R32(Mem32) =>
        sprintf (str,  "FADD.R32", DIS_MEM32);

    | FADD.R64(Mem64) =>
        sprintf (str,  "FADD.R64", DIS_MEM64);

    | FADD.ST.STi(idx) =>
        sprintf (str,  "FADD.St.STi", DIS_IDX);

    | FADD.STi.ST(idx) =>
        sprintf (str,  "FADD.STi.ST", DIS_IDX);

    | FADDP.STi.ST(idx) =>
        sprintf (str,  "FADDP.STi.ST", DIS_IDX);

    | FIADD.I32(Mem32) =>
        sprintf (str,  "FIADD.I32", DIS_MEM32);

    | FIADD.I16(Mem16) =>
        sprintf (str,  "FIADD.I16", DIS_MEM16);

    | FBLD(Mem80) =>
        sprintf (str,  "FBLD", DIS_MEM80);

    | FBSTP(Mem80) =>
        sprintf (str,  "FBSTP", DIS_MEM80);

    | FCHS() =>
        sprintf (str,  "FCHS");

    | FNCLEX() =>
        sprintf (str,  "FNCLEX");

    | FCOM.R32(Mem32) =>
        sprintf (str,  "FCOM.R32", DIS_MEM32);

    | FCOM.R64(Mem64) =>
        sprintf (str,  "FCOM.R64", DIS_MEM64);

    | FICOM.I32(Mem32) =>
        sprintf (str,  "FICOM.I32", DIS_MEM32);

    | FICOM.I16(Mem16) =>
        sprintf (str,  "FICOM.I16", DIS_MEM16);

    | FCOMP.R32(Mem32) =>
        sprintf (str,  "FCOMP.R32", DIS_MEM32);

    | FCOMP.R64(Mem64) =>
        sprintf (str,  "FCOMP.R64", DIS_MEM64);

    | FCOM.ST.STi(idx) =>
        sprintf (str,  "FCOM.ST.STi", DIS_IDX);

    | FCOMP.ST.STi(idx) =>
        sprintf (str,  "FCOMP.ST.STi", DIS_IDX);

    | FICOMP.I32(Mem32) =>
        sprintf (str,  "FICOMP.I32", DIS_MEM32);

    | FICOMP.I16(Mem16) =>
        sprintf (str,  "FICOMP.I16", DIS_MEM16);

    | FCOMPP() =>
        sprintf (str,  "FCOMPP");

    | FCOMI.ST.STi(idx) [name] =>
        sprintf (str, name, DIS_IDX);

    | FCOMIP.ST.STi(idx) [name] =>
        sprintf (str, name, DIS_IDX);

    | FCOS() =>
        sprintf (str,  "FCOS");

    | FDECSTP() =>
        sprintf (str,  "FDECSTP");

    | FDIV.R32(Mem32) =>
        sprintf (str,  "FDIV.R32", DIS_MEM32);

    | FDIV.R64(Mem64) =>
        sprintf (str,  "FDIV.R64", DIS_MEM64);

    | FDIV.ST.STi(idx) =>
        sprintf (str,  "FDIV.ST.STi", DIS_IDX);

    | FDIV.STi.ST(idx) =>
        sprintf (str,  "FDIV.STi.ST", DIS_IDX);

    | FDIVP.STi.ST(idx) =>
        sprintf (str,  "FDIVP.STi.ST", DIS_IDX);

    | FIDIV.I32(Mem32) =>
        sprintf (str,  "FIDIV.I32", DIS_MEM32);

    | FIDIV.I16(Mem16) =>
        sprintf (str,  "FIDIV.I16", DIS_MEM16);

    | FDIVR.R32(Mem32) =>
        sprintf (str,  "FDIVR.R32", DIS_MEM32);

    | FDIVR.R64(Mem64) =>
        sprintf (str,  "FDIVR.R64", DIS_MEM64);

    | FDIVR.ST.STi(idx) =>
        sprintf (str,  "FDIVR.ST.STi", DIS_IDX);

    | FDIVR.STi.ST(idx) =>
        sprintf (str,  "FDIVR.STi.ST", DIS_IDX);

    | FIDIVR.I32(Mem32) =>
        sprintf (str,  "FIDIVR.I32", DIS_MEM32);

    | FIDIVR.I16(Mem16) =>
        sprintf (str,  "FIDIVR.I16", DIS_MEM16);

    | FDIVRP.STi.ST(idx) =>
        sprintf (str,  "FDIVRP.STi.ST", DIS_IDX);

    | FFREE(idx) =>
        sprintf (str,  "FFREE", DIS_IDX);

    | FILD.lsI16(Mem16) =>
        sprintf (str,  "FILD.lsI16", DIS_MEM16);

    | FILD.lsI32(Mem32) =>
        sprintf (str,  "FILD.lsI32", DIS_MEM32);

    | FILD64(Mem64) =>
        sprintf (str,  "FILD64", DIS_MEM64);

    | FINIT() =>
        sprintf (str,  "FINIT");

    | FIST.lsI16(Mem16) =>
        sprintf (str,  "FIST.lsI16", DIS_MEM16);

    | FIST.lsI32(Mem32) =>
        sprintf (str,  "FIST.lsI32", DIS_MEM32);

    | FISTP.lsI16(Mem16) =>
        sprintf (str,  "FISTP.lsI16", DIS_MEM16);

    | FISTP.lsI32(Mem32) =>
        sprintf (str,  "FISTP.lsI32", DIS_MEM32);

    | FISTP64(Mem64) =>
        sprintf (str,  "FISTP64", DIS_MEM64);

    | FLD.lsR32(Mem32) =>
        sprintf (str,  "FLD.lsR32", DIS_MEM32);

    | FLD.lsR64(Mem64) =>
        sprintf (str,  "FLD.lsR64", DIS_MEM64);

    | FLD80(Mem80) =>
        sprintf (str,  "FLD80", DIS_MEM80);

/* This is a bit tricky. The FPUSH logically comes between the read of STi and
# the write to ST0. In particular, FLD ST0 is supposed to duplicate the TOS.
# This problem only happens with this load instruction, so there is a work
# around here that gives us the SSL a value of i that is one more than in
# the instruction */
    | FLD.STi(idx) =>
        sprintf (str,  "FLD.STi", DIS_IDXP1);

    | FLD1() =>
        sprintf (str,  "FLD1");

    | FLDL2T() =>
        sprintf (str,  "FLDL2T");

    | FLDL2E() =>
        sprintf (str,  "FLDL2E");

    | FLDPI() =>
        sprintf (str,  "FLDPI");

    | FLDLG2() =>
        sprintf (str,  "FLDLG2");

    | FLDLN2() =>
        sprintf (str,  "FLDLN2");

    | FLDZ() =>
        sprintf (str,  "FLDZ");

    | FLDCW(Mem16) =>
        sprintf (str,  "FLDCW", DIS_MEM16);

    | FLDENV(Mem) =>
        sprintf (str,  "FLDENV", DIS_MEM);

    | FMUL.R32(Mem32) =>
        sprintf (str,  "FMUL.R32", DIS_MEM32);

    | FMUL.R64(Mem64) =>
        sprintf (str,  "FMUL.R64", DIS_MEM64);

    | FMUL.ST.STi(idx) =>
        sprintf (str,  "FMUL.ST.STi", DIS_IDX);

    | FMUL.STi.ST(idx) =>
        sprintf (str,  "FMUL.STi.ST", DIS_IDX);

    | FMULP.STi.ST(idx) =>
        sprintf (str,  "FMULP.STi.ST", DIS_IDX);

    | FIMUL.I32(Mem32) =>
        sprintf (str,  "FIMUL.I32", DIS_MEM32);

    | FIMUL.I16(Mem16) =>
        sprintf (str,  "FIMUL.I16", DIS_MEM16);

    | FNOP() =>
        sprintf (str,  "FNOP");

    | FPATAN() =>
        sprintf (str,  "FPATAN");

    | FPREM() =>
        sprintf (str,  "FPREM");

    | FPREM1() =>
        sprintf (str,  "FPREM1");

    | FPTAN() =>
        sprintf (str,  "FPTAN");

    | FRNDINT() =>
        sprintf (str,  "FRNDINT");

    | FRSTOR(Mem) =>
        sprintf (str,  "FRSTOR", DIS_MEM);

    | FNSAVE(Mem) =>
        sprintf (str,  "FNSAVE", DIS_MEM);

    | FSCALE() =>
        sprintf (str,  "FSCALE");

    | FSIN() =>
        sprintf (str,  "FSIN");

    | FSINCOS() =>
        sprintf (str,  "FSINCOS");

    | FSQRT() =>
        sprintf (str,  "FSQRT");

    | FST.lsR32(Mem32) =>
        sprintf (str,  "FST.lsR32", DIS_MEM32);

    | FST.lsR64(Mem64) =>
        sprintf (str,  "FST.lsR64", DIS_MEM64);

    | FSTP.lsR32(Mem32) =>
        sprintf (str,  "FSTP.lsR32", DIS_MEM32);

    | FSTP.lsR64(Mem64) =>
        sprintf (str,  "FSTP.lsR64", DIS_MEM64);

    | FSTP80(Mem80) =>
        sprintf (str,  "FSTP80", DIS_MEM80);

    | FST.st.STi(idx) =>
        sprintf (str,  "FST.st.STi", DIS_IDX);

    | FSTP.st.STi(idx) =>
        sprintf (str,  "FSTP.st.STi", DIS_IDX);

    | FSTCW(Mem16) =>
        sprintf (str,  "FSTCW", DIS_MEM16);

    | FSTENV(Mem) =>
        sprintf (str,  "FSTENV", DIS_MEM);

    | FSTSW(Mem16) =>
        sprintf (str,  "FSTSW", DIS_MEM16);

    | FSTSW.AX() =>
        sprintf (str,  "FSTSW.AX");

    | FSUB.R32(Mem32) =>
        sprintf (str,  "FSUB.R32", DIS_MEM32);

    | FSUB.R64(Mem64) =>
        sprintf (str,  "FSUB.R64", DIS_MEM64);

    | FSUB.ST.STi(idx) =>
        sprintf (str,  "FSUB.ST.STi", DIS_IDX);

    | FSUB.STi.ST(idx) =>
        sprintf (str,  "FSUB.STi.ST", DIS_IDX);

    | FISUB.I32(Mem32) =>
        sprintf (str,  "FISUB.I32", DIS_MEM32);

    | FISUB.I16(Mem16) =>
        sprintf (str,  "FISUB.I16", DIS_MEM16);

    | FSUBP.STi.ST(idx) =>
        sprintf (str,  "FSUBP.STi.ST", DIS_IDX);

    | FSUBR.R32(Mem32) =>
        sprintf (str,  "FSUBR.R32", DIS_MEM32);

    | FSUBR.R64(Mem64) =>
        sprintf (str,  "FSUBR.R64", DIS_MEM64);

    | FSUBR.ST.STi(idx) =>
        sprintf (str,  "FSUBR.ST.STi", DIS_IDX);

    | FSUBR.STi.ST(idx) =>
        sprintf (str,  "FSUBR.STi.ST", DIS_IDX);

    | FISUBR.I32(Mem32) =>
        sprintf (str,  "FISUBR.I32", DIS_MEM32);

    | FISUBR.I16(Mem16) =>
        sprintf (str,  "FISUBR.I16", DIS_MEM16);

    | FSUBRP.STi.ST(idx) =>
        sprintf (str,  "FSUBRP.STi.ST", DIS_IDX);

    | FTST() =>
        sprintf (str,  "FTST");

    | FUCOM(idx) =>
        sprintf (str,  "FUCOM", DIS_IDX);

    | FUCOMP(idx) =>
        sprintf (str,  "FUCOMP", DIS_IDX);

    | FUCOMPP() =>
        sprintf (str,  "FUCOMPP");

    | FUCOMI.ST.STi(idx) [name] =>
        sprintf (str, name, DIS_IDX);

    | FUCOMIP.ST.STi(idx) [name] =>
        sprintf (str, name, DIS_IDX);

    | FXAM() =>
        sprintf (str,  "FXAM");

    | FXCH(idx) =>
        sprintf (str,  "FXCH", DIS_IDX);

    | FXTRACT() =>
        sprintf (str,  "FXTRACT");

    | FYL2X() =>
        sprintf (str,  "FYL2X");

    | FYL2XP1() =>
        sprintf (str,  "FYL2XP1");

//	| inst = n =>
//		sprintf (str, "%0x", n);

    else
		sprintf (str, "%0x", n);

    endmatch

    // return # of bytes parsed
    return (nextPC - hostPC);
}

