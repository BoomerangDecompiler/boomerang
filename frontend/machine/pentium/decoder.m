/*
 * Copyright (C) 1998-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       decoder.m
 * OVERVIEW:   This file contains the high level decoding functionality, for
 *              example matching logues, calls, branches, etc. Ordinary
 *              instructions are processed in decoder_low.m
 *============================================================================*/ 
/*
 * $Revision$
 *
 * 26 Apr 02 - Mike: Changes for boomerang
*/

#include "decoder.h"
#include "prog.h"
#include "exp.h"
#include "rtl.h"
#include "proc.h"

// The following are all dis_Num because the SSL has r[REG32], not just REG32
// Else we will get r[r[24]]
#define DIS_R8    (dis_Num(r8+8))
#define DIS_R16   (dis_Num(r16+0))
#define DIS_R32   (dis_Num(r32+24))
#define DIS_REG8  (dis_Num(reg+8))
#define DIS_REG16 (dis_Num(reg+0))
#define DIS_REG32 (dis_Num(reg+24))
#define DIS_SR16  (dis_Num(sr16+16))
#define DIS_IDX   (dis_Num(idx+32))
#define DIS_IDXP1 (dis_Num((idx+1)%7))

#define DIS_EADDR32 (dis_Eaddr(Eaddr, 32))
#define DIS_EADDR16 (dis_Eaddr(Eaddr, 16))
#define DIS_EADDR8  (dis_Eaddr(Eaddr,  8))
#define DIS_MEM     (dis_Mem(Mem))
#define DIS_MEM16   (dis_Mem(Mem16))    // Probably needs changing
#define DIS_MEM32   (dis_Mem(Mem32))    // Probably needs changing
#define DIS_MEM64   (dis_Mem(Mem64))    // Probably needs changing
#define DIS_MEM80   (dis_Mem(Mem80))    // Probably needs changing

#define DIS_I32     (new Const(i32))
#define DIS_I16     (new Const(i16))
#define DIS_I8      (new Const(i8))
#define DIS_COUNT   (new Const(count))
#define DIS_OFF     (new Const(off))

/**********************************
 * NJMCDecoder methods.
 **********************************/   

/*==============================================================================
 * FUNCTION:       unused
 * OVERVIEW:       A dummy function to suppress "unused local variable" messages
 * PARAMETERS:     x: integer variable to be "used"
 * RETURNS:        Nothing
 *============================================================================*/
void unused(int x)
{}

/*==============================================================================
 * FUNCTION:       NJMCDecoder::decodeInstruction
 * OVERVIEW:       Decodes a machine instruction and returns an RTL instance. In
 *                 most cases a single instruction is decoded. However, if a
 *                 higher level construct that may consist of multiple
 *                 instructions is matched, then there may be a need to return
 *                 more than one RTL. The caller_prologue2 is an example of such
 *                 a construct which encloses an abritary instruction that must
 *                 be decoded into its own RTL.
 * PARAMETERS:     pc - the native address of the pc
 *                 delta - the difference between the above address and the
 *                   host address of the pc (i.e. the address that the pc is at
 *                   in the loaded object file)
 *                 RTLDict - the dictionary of RTL templates used to instantiate
 *                   the RTL for the instruction being decoded
 *                 proc - the enclosing procedure
 * RETURNS:        a DecodeResult structure containing all the information
 *                   gathered during decoding
 *============================================================================*/
DecodeResult& NJMCDecoder::decodeInstruction (ADDRESS pc, int delta)
{
    static DecodeResult result;
    ADDRESS hostPC = pc + delta;

    // Clear the result structure;
    result.reset();

    // The actual list of instantiated Exps
    std::list<Exp*>* Exps = NULL;


    ADDRESS nextPC;
    match [nextPC] hostPC to
    
    | CALL.Evod(Eaddr) =>
        /*
         * Register call
         */
        // Mike: there should probably be a HLNwayCall class for this!
        HLCall* newCall = new HLCall(pc, 0, Exps);
        // Record the fact that this is a computed call
        newCall->setIsComputed();
        // Set the destination expression
        newCall->setDest(DIS_EADDR32);
        result.rtl = newCall;
        // Only one instruction, so size of result is size of this decode
        //result.numBytes = nextPC - hostPC;

    | JMP.Evod(Eaddr) =>
        /*
         * Register jump
         */
        HLNwayJump* newJump = new HLNwayJump(pc, Exps);
        // Record the fact that this is a computed call
        newJump->setIsComputed();
        // Set the destination expression
        newJump->setDest(DIS_EADDR32);
        result.rtl = newJump;
        // Only one instruction, so size of result is size of this decode
        //result.numBytes = nextPC - hostPC;
    
    /*
     * Unconditional branches
     */
    | JMP.Jvod(relocd) [name] =>
        unused((int) name);
        unconditionalJump(name, 5, relocd, delta, pc, Exps, result);
    | JMP.Jvow(relocd) [name] =>
        unused((int) name);
        unconditionalJump(name, 3, relocd, delta, pc, Exps, result);
    | JMP.Jb(relocd) [name] =>
        unused((int) name);
        unconditionalJump(name, 2, relocd, delta, pc, Exps, result);

    /*
     * Conditional branches, 8 bit offset: 7X XX
     */
    | Jb.NLE(relocd) =>
        COND_JUMP("Jb.NLE", 2, relocd, HLJCOND_JSG)
    | Jb.LE(relocd) =>
        COND_JUMP("Jb.LE", 2, relocd, HLJCOND_JSLE)
    | Jb.NL(relocd) =>
        COND_JUMP("Jb.NL", 2, relocd, HLJCOND_JSGE)
    | Jb.L(relocd) =>
        COND_JUMP("Jb.L", 2, relocd, HLJCOND_JSL)
    | Jb.NP(relocd) =>
        COND_JUMP("Jb.NP", 2, relocd, (JCOND_TYPE)0)
    | Jb.P(relocd) =>
        COND_JUMP("Jb.P", 2, relocd, HLJCOND_JPAR)
    | Jb.NS(relocd) =>
        COND_JUMP("Jb.NS", 2, relocd, HLJCOND_JPOS)
    | Jb.S(relocd) =>
        COND_JUMP("Jb.S", 2, relocd, HLJCOND_JMI)
    | Jb.NBE(relocd) =>
        COND_JUMP("Jb.NBE", 2, relocd, HLJCOND_JUG)
    | Jb.BE(relocd) =>
        COND_JUMP("Jb.BE", 2, relocd, HLJCOND_JULE)
    | Jb.NZ(relocd) =>
        COND_JUMP("Jb.NZ", 2, relocd, HLJCOND_JNE)
    | Jb.Z(relocd) =>
        COND_JUMP("Jb.Z", 2, relocd, HLJCOND_JE)
    | Jb.NB(relocd) =>
        COND_JUMP("Jb.NB", 2, relocd, HLJCOND_JUGE)
    | Jb.B(relocd) =>
        COND_JUMP("Jb.B", 2, relocd, HLJCOND_JUL)
    | Jb.NO(relocd) =>
        COND_JUMP("Jb.NO", 2, relocd, (JCOND_TYPE)0)
    | Jb.O(relocd) =>
        COND_JUMP("Jb.O", 2, relocd, (JCOND_TYPE)0)

    /*
     * Conditional branches, 16 bit offset: 66 0F 8X XX XX
     */
    | Jv.NLEow(relocd) =>
        COND_JUMP("Jv.NLEow", 4, relocd, HLJCOND_JSG)
    | Jv.LEow(relocd) =>
        COND_JUMP("Jv.LEow", 4, relocd, HLJCOND_JSLE)
    | Jv.NLow(relocd) =>
        COND_JUMP("Jv.NLow", 4, relocd, HLJCOND_JSGE)
    | Jv.Low(relocd) =>
        COND_JUMP("Jv.Low", 4, relocd, HLJCOND_JSL)
    | Jv.NPow(relocd) =>
        COND_JUMP("Jv.NPow", 4, relocd, (JCOND_TYPE)0)
    | Jv.Pow(relocd) =>
        COND_JUMP("Jv.Pow", 4, relocd, HLJCOND_JPAR)
    | Jv.NSow(relocd) =>
        COND_JUMP("Jv.NSow", 4, relocd, HLJCOND_JPOS)
    | Jv.Sow(relocd) =>
        COND_JUMP("Jv.Sow", 4, relocd, HLJCOND_JMI)
    | Jv.NBEow(relocd) =>
        COND_JUMP("Jv.NBEow", 4, relocd, HLJCOND_JUG)
    | Jv.BEow(relocd) =>
        COND_JUMP("Jv.BEow", 4, relocd, HLJCOND_JULE)
    | Jv.NZow(relocd) =>
        COND_JUMP("Jv.NZow", 4, relocd, HLJCOND_JNE)
    | Jv.Zow(relocd) =>
        COND_JUMP("Jv.Zow", 4, relocd, HLJCOND_JE)
    | Jv.NBow(relocd) =>
        COND_JUMP("Jv.NBow", 4, relocd, HLJCOND_JUGE)
    | Jv.Bow(relocd) =>
        COND_JUMP("Jv.Bow", 4, relocd, HLJCOND_JUL)
    | Jv.NOow(relocd) =>
        COND_JUMP("Jv.NOow", 4, relocd, (JCOND_TYPE)0)
    | Jv.Oow(relocd) =>
        COND_JUMP("Jv.Oow", 4, relocd, (JCOND_TYPE)0)

    /*
     * Conditional branches, 32 bit offset: 0F 8X XX XX XX XX
     */
    | Jv.NLEod(relocd) =>
        COND_JUMP("Jv.NLEod", 6, relocd, HLJCOND_JSG)
    | Jv.LEod(relocd) =>
        COND_JUMP("Jv.LEod", 6, relocd, HLJCOND_JSLE)
    | Jv.NLod(relocd) =>
        COND_JUMP("Jv.NLod", 6, relocd, HLJCOND_JSGE)
    | Jv.Lod(relocd) =>
        COND_JUMP("Jv.Lod", 6, relocd, HLJCOND_JSL)
    | Jv.NPod(relocd) =>
        COND_JUMP("Jv.NPod", 6, relocd, (JCOND_TYPE)0)
    | Jv.Pod(relocd) =>
        COND_JUMP("Jv.Pod", 6, relocd, HLJCOND_JPAR)
    | Jv.NSod(relocd) =>
        COND_JUMP("Jv.NSod", 6, relocd, HLJCOND_JPOS)
    | Jv.Sod(relocd) =>
        COND_JUMP("Jv.Sod", 6, relocd, HLJCOND_JMI)
    | Jv.NBEod(relocd) =>
        COND_JUMP("Jv.NBEod", 6, relocd, HLJCOND_JUG)
    | Jv.BEod(relocd) =>
        COND_JUMP("Jv.BEod", 6, relocd, HLJCOND_JULE)
    | Jv.NZod(relocd) =>
        COND_JUMP("Jv.NZod", 6, relocd, HLJCOND_JNE)
    | Jv.Zod(relocd) =>
        COND_JUMP("Jv.Zod", 6, relocd, HLJCOND_JE)
    | Jv.NBod(relocd) =>
        COND_JUMP("Jv.NBod", 6, relocd, HLJCOND_JUGE)
    | Jv.Bod(relocd) =>
        COND_JUMP("Jv.Bod", 6, relocd, HLJCOND_JUL)
    | Jv.NOod(relocd) =>
        COND_JUMP("Jv.NOod", 6, relocd, (JCOND_TYPE)0)
    | Jv.Ood(relocd) =>
        COND_JUMP("Jv.Ood", 6, relocd, (JCOND_TYPE)0)

    | SETb.NLE(Eaddr) [name] =>
        Exps = instantiate(pc, name, DIS_EADDR8);
        SETS(name, DIS_EADDR8, HLJCOND_JSG)
    | SETb.LE(Eaddr) [name] =>
        Exps = instantiate(pc, name, DIS_EADDR8);
        SETS(name, DIS_EADDR8, HLJCOND_JSLE)
    | SETb.NL(Eaddr) [name] =>
        Exps = instantiate(pc, name, DIS_EADDR8);
        SETS(name, DIS_EADDR8, HLJCOND_JSGE)
    | SETb.L(Eaddr) [name] =>
        Exps = instantiate(pc, name, DIS_EADDR8);
        SETS(name, DIS_EADDR8, HLJCOND_JSL)
    //| SETb.NP(Eaddr) [name] =>
    //  Exps = instantiate(pc, name, DIS_EADDR8);
    //  SETS(name, DIS_EADDR8, HLJCOND_JSG)
    //| SETb.P(Eaddr) [name] =>
    //  Exps = instantiate(pc, name, DIS_EADDR8);
    //  SETS(name, DIS_EADDR8, HLJCOND_JSG)
    | SETb.NS(Eaddr) [name] =>
        Exps = instantiate(pc, name, DIS_EADDR8);
        SETS(name, DIS_EADDR8, HLJCOND_JPOS)
    | SETb.S(Eaddr) [name] =>
        Exps = instantiate(pc, name, DIS_EADDR8);
        SETS(name, DIS_EADDR8, HLJCOND_JMI)
    | SETb.NBE(Eaddr) [name] =>
        Exps = instantiate(pc, name, DIS_EADDR8);
        SETS(name, DIS_EADDR8, HLJCOND_JUG)
    | SETb.BE(Eaddr) [name] =>
        Exps = instantiate(pc, name, DIS_EADDR8);
        SETS(name, DIS_EADDR8, HLJCOND_JULE)
    | SETb.NZ(Eaddr) [name] =>
        Exps = instantiate(pc, name, DIS_EADDR8);
        SETS(name, DIS_EADDR8, HLJCOND_JNE)
    | SETb.Z(Eaddr) [name] =>
        Exps = instantiate(pc, name, DIS_EADDR8);
        SETS(name, DIS_EADDR8, HLJCOND_JE)
    | SETb.NB(Eaddr) [name] =>
        Exps = instantiate(pc, name, DIS_EADDR8);
        SETS(name, DIS_EADDR8, HLJCOND_JUGE)
    | SETb.B(Eaddr) [name] =>
        Exps = instantiate(pc, name, DIS_EADDR8);
        SETS(name, DIS_EADDR8, HLJCOND_JUL)
    //| SETb.NO(Eaddr) [name] =>
    //  Exps = instantiate(pc, name, DIS_EADDR8);
    //  SETS(name, DIS_EADDR8, HLJCOND_JSG)
    //| SETb.O(Eaddr) [name] =>
    //  Exps = instantiate(pc, name, DIS_EADDR8);
    //  SETS(name, DIS_EADDR8, HLJCOND_JSG)

    | XLATB() =>
        Exps = instantiate(pc,  "XLATB");

    | XCHG.Ev.Gvod(Eaddr, reg) =>
        Exps = instantiate(pc,  "XCHG.Ev.Gvod", DIS_EADDR32, DIS_REG32);

    | XCHG.Ev.Gvow(Eaddr, reg) =>
        Exps = instantiate(pc,  "XCHG.Ev.Gvow", DIS_EADDR16, DIS_REG16);

    | XCHG.Eb.Gb(Eaddr, reg) =>
        Exps = instantiate(pc,  "XCHG.Eb.Gb", DIS_EADDR8, DIS_REG8);

    | NOP() =>
        Exps = instantiate(pc,  "NOP");

    | SEG.FS() =>        // For now, treat as a 1 byte NOP
        Exps = instantiate(pc,  "NOP");

    | SEG.GS() =>
        Exps = instantiate(pc,  "NOP");

    | XCHGeAXod(r32) =>
        Exps = instantiate(pc,  "XCHGeAXod", DIS_R32);

    | XCHGeAXow(r32) =>
        Exps = instantiate(pc,  "XCHGeAXow", DIS_R32);

    | XADD.Ev.Gvod(Eaddr, reg) =>
        Exps = instantiate(pc,  "XADD.Ev.Gvod", DIS_EADDR32, DIS_REG32);

    | XADD.Ev.Gvow(Eaddr, reg) =>
        Exps = instantiate(pc,  "XADD.Ev.Gvow", DIS_EADDR16, DIS_REG16);

    | XADD.Eb.Gb(Eaddr, reg) =>
        Exps = instantiate(pc,  "XADD.Eb.Gb", DIS_EADDR8, DIS_REG8);

    | WRMSR() =>
        Exps = instantiate(pc,  "WRMSR");

    | WBINVD() =>
        Exps = instantiate(pc,  "WBINVD");

    | WAIT() =>
        Exps = instantiate(pc,  "WAIT");

    | VERW(Eaddr) =>
        Exps = instantiate(pc,  "VERW", DIS_EADDR32);

    | VERR(Eaddr) =>
        Exps = instantiate(pc,  "VERR", DIS_EADDR32);

    | TEST.Ev.Gvod(Eaddr, reg) =>
        Exps = instantiate(pc,  "TEST.Ev.Gvod", DIS_EADDR32, DIS_REG32);

    | TEST.Ev.Gvow(Eaddr, reg) =>
        Exps = instantiate(pc,  "TEST.Ev.Gvow", DIS_EADDR16, DIS_REG16);

    | TEST.Eb.Gb(Eaddr, reg) =>
        Exps = instantiate(pc,  "TEST.Eb.Gb", DIS_EADDR8, DIS_REG8);

    | TEST.Ed.Id(Eaddr, i32) =>
        Exps = instantiate(pc,  "TEST.Ed.Id", DIS_EADDR32, DIS_I32);

    | TEST.Ew.Iw(Eaddr, i16) =>
        Exps = instantiate(pc,  "TEST.Ew.Iw", DIS_EADDR16, DIS_I16);

    | TEST.Eb.Ib(Eaddr, i8) =>
        Exps = instantiate(pc,  "TEST.Eb.Ib", DIS_EADDR8, DIS_I8);

    | TEST.eAX.Ivod(i32) =>
        Exps = instantiate(pc,  "TEST.eAX.Ivod", DIS_I32);

    | TEST.eAX.Ivow(i16) =>
        Exps = instantiate(pc,  "TEST.eAX.Ivow", DIS_I16);

    | TEST.AL.Ib(i8) =>
        Exps = instantiate(pc,  "TEST.AL.Ib", DIS_I8);

    | STR(Mem) =>
        Exps = instantiate(pc,  "STR", DIS_MEM);

    | STOSvod() =>
        Exps = instantiate(pc,  "STOSvod");

    | STOSvow() =>
        Exps = instantiate(pc,  "STOSvow");

    | STOSB() =>
        Exps = instantiate(pc,  "STOSB");

    | STI() =>
        Exps = instantiate(pc,  "STI");

    | STD() =>
        Exps = instantiate(pc,  "STD");

    | STC() =>
        Exps = instantiate(pc,  "STC");

    | SMSW(Eaddr) =>
        Exps = instantiate(pc,  "SMSW", DIS_EADDR32);

    | SLDT(Eaddr) =>
        Exps = instantiate(pc,  "SLDT", DIS_EADDR32);

    | SHLD.CLod(Eaddr, reg) =>
        Exps = instantiate(pc,  "SHLD.CLod", DIS_EADDR32, DIS_REG32);

    | SHLD.CLow(Eaddr, reg) =>
        Exps = instantiate(pc,  "SHLD.CLow", DIS_EADDR16, DIS_REG16);

    | SHRD.CLod(Eaddr, reg) =>
        Exps = instantiate(pc,  "SHRD.CLod", DIS_EADDR32, DIS_REG32);

    | SHRD.CLow(Eaddr, reg) =>
        Exps = instantiate(pc,  "SHRD.CLow", DIS_EADDR16, DIS_REG16);

    | SHLD.Ibod(Eaddr, reg, count) =>
        Exps = instantiate(pc,  "SHLD.Ibod", DIS_EADDR32, DIS_REG32, DIS_COUNT);

    | SHLD.Ibow(Eaddr, reg, count) =>
        Exps = instantiate(pc,  "SHLD.Ibow", DIS_EADDR16, DIS_REG16, DIS_COUNT);

    | SHRD.Ibod(Eaddr, reg, count) =>
        Exps = instantiate(pc,  "SHRD.Ibod", DIS_EADDR32, DIS_REG32, DIS_COUNT);

    | SHRD.Ibow(Eaddr, reg, count) =>
        Exps = instantiate(pc,  "SHRD.Ibow", DIS_EADDR16, DIS_REG16, DIS_COUNT);

    | SIDT(Mem) =>
        Exps = instantiate(pc,  "SIDT", DIS_MEM);

    | SGDT(Mem) =>
        Exps = instantiate(pc,  "SGDT", DIS_MEM);

    // Sets are now in the high level instructions
    | SCASvod() =>
        Exps = instantiate(pc,  "SCASvod");

    | SCASvow() =>
        Exps = instantiate(pc,  "SCASvow");

    | SCASB() =>
        Exps = instantiate(pc,  "SCASB");

    | SAHF() =>
        Exps = instantiate(pc,  "SAHF");

    | RSM() =>
        Exps = instantiate(pc,  "RSM");

    | RET.far.Iw(i16) =>
        Exps = instantiate(pc,  "RET.far.Iw", DIS_I16);

    | RET.Iw(i16) =>
        Exps = instantiate(pc,  "RET.Iw", DIS_I16);

    | RET.far() =>
        Exps = instantiate(pc,  "RET.far");

    | RET() =>
        Exps = instantiate(pc,  "RET");

//   | REPNE() =>
//      Exps = instantiate(pc,  "REPNE");

//  | REP() =>
//      Exps = instantiate(pc,  "REP");

    | REP.CMPSB() [name] =>
        Exps = instantiate(pc,  name);

    | REP.CMPSvow() [name] =>
        Exps = instantiate(pc,  name);

    | REP.CMPSvod() [name] =>
        Exps = instantiate(pc,  name);

    | REP.LODSB() [name] =>
        Exps = instantiate(pc,  name);

    | REP.LODSvow() [name] =>
        Exps = instantiate(pc,  name);

    | REP.LODSvod() [name] =>
        Exps = instantiate(pc,  name);

    | REP.MOVSB() [name] =>
        Exps = instantiate(pc,  name);

    | REP.MOVSvow() [name] =>
        Exps = instantiate(pc,  name);

    | REP.MOVSvod() [name] =>
        Exps = instantiate(pc,  name);

    | REP.SCASB() [name] =>
        Exps = instantiate(pc,  name);

    | REP.SCASvow() [name] =>
        Exps = instantiate(pc,  name);

    | REP.SCASvod() [name] =>
        Exps = instantiate(pc,  name);

    | REP.STOSB() [name] =>
        Exps = instantiate(pc,  name);

    | REP.STOSvow() [name] =>
        Exps = instantiate(pc,  name);

    | REP.STOSvod() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.CMPSB() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.CMPSvow() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.CMPSvod() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.LODSB() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.LODSvow() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.LODSvod() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.MOVSB() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.MOVSvow() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.MOVSvod() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.SCASB() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.SCASvow() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.SCASvod() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.STOSB() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.STOSvow() [name] =>
        Exps = instantiate(pc,  name);

    | REPNE.STOSvod() [name] =>
        Exps = instantiate(pc,  name);

    | RDMSR() =>
        Exps = instantiate(pc,  "RDMSR");

    | SARB.Ev.Ibod(Eaddr, i8) =>
        Exps = instantiate(pc,  "SARB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | SARB.Ev.Ibow(Eaddr, i8) =>
        Exps = instantiate(pc,  "SARB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | SHRB.Ev.Ibod(Eaddr, i8) =>
        Exps = instantiate(pc,  "SHRB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | SHRB.Ev.Ibow(Eaddr, i8) =>
        Exps = instantiate(pc,  "SHRB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | SHLSALB.Ev.Ibod(Eaddr, i8) =>
        Exps = instantiate(pc,  "SHLSALB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | SHLSALB.Ev.Ibow(Eaddr, i8) =>
        Exps = instantiate(pc,  "SHLSALB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | RCRB.Ev.Ibod(Eaddr, i8) =>
        Exps = instantiate(pc,  "RCRB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | RCRB.Ev.Ibow(Eaddr, i8) =>
        Exps = instantiate(pc,  "RCRB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | RCLB.Ev.Ibod(Eaddr, i8) =>
        Exps = instantiate(pc,  "RCLB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | RCLB.Ev.Ibow(Eaddr, i8) =>
        Exps = instantiate(pc,  "RCLB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | RORB.Ev.Ibod(Eaddr, i8) =>
        Exps = instantiate(pc,  "RORB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | RORB.Ev.Ibow(Eaddr, i8) =>
        Exps = instantiate(pc,  "RORB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | ROLB.Ev.Ibod(Eaddr, i8) =>
        Exps = instantiate(pc,  "ROLB.Ev.Ibod", DIS_EADDR32, DIS_I8);

    | ROLB.Ev.Ibow(Eaddr, i8) =>
        Exps = instantiate(pc,  "ROLB.Ev.Ibow", DIS_EADDR16, DIS_I8);

    | SARB.Eb.Ib(Eaddr, i8) =>
        Exps = instantiate(pc,  "SARB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | SHRB.Eb.Ib(Eaddr, i8) =>
        Exps = instantiate(pc,  "SHRB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | SHLSALB.Eb.Ib(Eaddr, i8) =>
        Exps = instantiate(pc,  "SHLSALB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | RCRB.Eb.Ib(Eaddr, i8) =>
        Exps = instantiate(pc,  "RCRB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | RCLB.Eb.Ib(Eaddr, i8) =>
        Exps = instantiate(pc,  "RCLB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | RORB.Eb.Ib(Eaddr, i8) =>
        Exps = instantiate(pc,  "RORB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | ROLB.Eb.Ib(Eaddr, i8) =>
        Exps = instantiate(pc,  "ROLB.Eb.Ib", DIS_EADDR8, DIS_I8);

    | SARB.Ev.CLod(Eaddr) =>
        Exps = instantiate(pc,  "SARB.Ev.CLod", DIS_EADDR32);

    | SARB.Ev.CLow(Eaddr) =>
        Exps = instantiate(pc,  "SARB.Ev.CLow", DIS_EADDR16);

    | SARB.Ev.1od(Eaddr) =>
        Exps = instantiate(pc,  "SARB.Ev.1od", DIS_EADDR32);

    | SARB.Ev.1ow(Eaddr) =>
        Exps = instantiate(pc,  "SARB.Ev.1ow", DIS_EADDR16);

    | SHRB.Ev.CLod(Eaddr) =>
        Exps = instantiate(pc,  "SHRB.Ev.CLod", DIS_EADDR32);

    | SHRB.Ev.CLow(Eaddr) =>
        Exps = instantiate(pc,  "SHRB.Ev.CLow", DIS_EADDR16);

    | SHRB.Ev.1od(Eaddr) =>
        Exps = instantiate(pc,  "SHRB.Ev.1od", DIS_EADDR32);

    | SHRB.Ev.1ow(Eaddr) =>
        Exps = instantiate(pc,  "SHRB.Ev.1ow", DIS_EADDR16);

    | SHLSALB.Ev.CLod(Eaddr) =>
        Exps = instantiate(pc,  "SHLSALB.Ev.CLod", DIS_EADDR32);

    | SHLSALB.Ev.CLow(Eaddr) =>
        Exps = instantiate(pc,  "SHLSALB.Ev.CLow", DIS_EADDR16);

    | SHLSALB.Ev.1od(Eaddr) =>
        Exps = instantiate(pc,  "SHLSALB.Ev.1od", DIS_EADDR32);

    | SHLSALB.Ev.1ow(Eaddr) =>
        Exps = instantiate(pc,  "SHLSALB.Ev.1ow", DIS_EADDR16);

    | RCRB.Ev.CLod(Eaddr) =>
        Exps = instantiate(pc,  "RCRB.Ev.CLod", DIS_EADDR32);

    | RCRB.Ev.CLow(Eaddr) =>
        Exps = instantiate(pc,  "RCRB.Ev.CLow", DIS_EADDR16);

    | RCRB.Ev.1od(Eaddr) =>
        Exps = instantiate(pc,  "RCRB.Ev.1od", DIS_EADDR32);

    | RCRB.Ev.1ow(Eaddr) =>
        Exps = instantiate(pc,  "RCRB.Ev.1ow", DIS_EADDR16);

    | RCLB.Ev.CLod(Eaddr) =>
        Exps = instantiate(pc,  "RCLB.Ev.CLod", DIS_EADDR32);

    | RCLB.Ev.CLow(Eaddr) =>
        Exps = instantiate(pc,  "RCLB.Ev.CLow", DIS_EADDR16);

    | RCLB.Ev.1od(Eaddr) =>
        Exps = instantiate(pc,  "RCLB.Ev.1od", DIS_EADDR32);

    | RCLB.Ev.1ow(Eaddr) =>
        Exps = instantiate(pc,  "RCLB.Ev.1ow", DIS_EADDR16);

    | RORB.Ev.CLod(Eaddr) =>
        Exps = instantiate(pc,  "RORB.Ev.CLod", DIS_EADDR32);

    | RORB.Ev.CLow(Eaddr) =>
        Exps = instantiate(pc,  "RORB.Ev.CLow", DIS_EADDR16);

    | RORB.Ev.1od(Eaddr) =>
        Exps = instantiate(pc,  "RORB.Ev.1od", DIS_EADDR32);

    | RORB.Ev.1ow(Eaddr) =>
        Exps = instantiate(pc,  "ORB.Ev.1owR", DIS_EADDR16);

    | ROLB.Ev.CLod(Eaddr) =>
        Exps = instantiate(pc,  "ROLB.Ev.CLod", DIS_EADDR32);

    | ROLB.Ev.CLow(Eaddr) =>
        Exps = instantiate(pc,  "ROLB.Ev.CLow", DIS_EADDR16);

    | ROLB.Ev.1od(Eaddr) =>
        Exps = instantiate(pc,  "ROLB.Ev.1od", DIS_EADDR32);

    | ROLB.Ev.1ow(Eaddr) =>
        Exps = instantiate(pc,  "ROLB.Ev.1ow", DIS_EADDR16);

    | SARB.Eb.CL(Eaddr) =>
        Exps = instantiate(pc,  "SARB.Eb.CL", DIS_EADDR32);

    | SARB.Eb.1(Eaddr) =>
        Exps = instantiate(pc,  "SARB.Eb.1", DIS_EADDR16);

    | SHRB.Eb.CL(Eaddr) =>
        Exps = instantiate(pc,  "SHRB.Eb.CL", DIS_EADDR8);

    | SHRB.Eb.1(Eaddr) =>
        Exps = instantiate(pc,  "SHRB.Eb.1", DIS_EADDR8);

    | SHLSALB.Eb.CL(Eaddr) =>
        Exps = instantiate(pc,  "SHLSALB.Eb.CL", DIS_EADDR8);

    | SHLSALB.Eb.1(Eaddr) =>
        Exps = instantiate(pc,  "SHLSALB.Eb.1", DIS_EADDR8);

    | RCRB.Eb.CL(Eaddr) =>
        Exps = instantiate(pc,  "RCRB.Eb.CL", DIS_EADDR8);

    | RCRB.Eb.1(Eaddr) =>
        Exps = instantiate(pc,  "RCRB.Eb.1", DIS_EADDR8);

    | RCLB.Eb.CL(Eaddr) =>
        Exps = instantiate(pc,  "RCLB.Eb.CL", DIS_EADDR8);

    | RCLB.Eb.1(Eaddr) =>
        Exps = instantiate(pc,  "RCLB.Eb.1", DIS_EADDR8);

    | RORB.Eb.CL(Eaddr) =>
        Exps = instantiate(pc,  "RORB.Eb.CL", DIS_EADDR8);

    | RORB.Eb.1(Eaddr) =>
        Exps = instantiate(pc,  "RORB.Eb.1", DIS_EADDR8);

    | ROLB.Eb.CL(Eaddr) =>
        Exps = instantiate(pc,  "ROLB.Eb.CL", DIS_EADDR8);

    | ROLB.Eb.1(Eaddr) =>
        Exps = instantiate(pc,  "ROLB.Eb.1", DIS_EADDR8);

    // There is no SSL for these, so don't call instantiate, it will only
    // cause an assert failure. Also, may as well treat these as invalid instr
//    | PUSHFod() =>
//        Exps = instantiate(pc,  "PUSHFod");

//    | PUSHFow() =>
//        Exps = instantiate(pc,  "PUSHFow");

//    | PUSHAod() =>
//        Exps = instantiate(pc,  "PUSHAod");

//    | PUSHAow() =>
//        Exps = instantiate(pc,  "PUSHAow");

    | PUSH.GS() =>
        Exps = instantiate(pc,  "PUSH.GS");

    | PUSH.FS() =>
        Exps = instantiate(pc,  "PUSH.FS");

    | PUSH.ES() =>
        Exps = instantiate(pc,  "PUSH.ES");

    | PUSH.DS() =>
        Exps = instantiate(pc,  "PUSH.DS");

    | PUSH.SS() =>
        Exps = instantiate(pc,  "PUSH.SS");

    | PUSH.CS() =>
        Exps = instantiate(pc,  "PUSH.CS");

    | PUSH.Ivod(i32) =>
        Exps = instantiate(pc,  "PUSH.Ivod", DIS_I32);

    | PUSH.Ivow(i16) =>
        Exps = instantiate(pc,  "PUSH.Ivow", DIS_I16);

    | PUSH.Ixob(i8) =>
        Exps = instantiate(pc,  "PUSH.Ixob", DIS_I8);

    | PUSH.Ixow(i8) =>
        Exps = instantiate(pc,  "PUSH.Ixow", DIS_I8);

    | PUSHod(r32) =>
        Exps = instantiate(pc,  "PUSHod", DIS_R32);

    | PUSHow(r32) =>
        Exps = instantiate(pc,  "PUSHow", DIS_R32);  // Check!

    | PUSH.Evod(Eaddr) =>
        Exps = instantiate(pc,  "PUSH.Evod", DIS_EADDR32);

    | PUSH.Evow(Eaddr) =>
        Exps = instantiate(pc,  "PUSH.Evow", DIS_EADDR16);

//    | POPFod() =>
//        Exps = instantiate(pc,  "POPFod");

//    | POPFow() =>
//        Exps = instantiate(pc,  "POPFow");

//    | POPAod() =>
//        Exps = instantiate(pc,  "POPAod");

//    | POPAow() =>
//        Exps = instantiate(pc,  "POPAow");

    | POP.GS() =>
        Exps = instantiate(pc,  "POP.GS");

    | POP.FS() =>
        Exps = instantiate(pc,  "POP.FS");

    | POP.DS() =>
        Exps = instantiate(pc,  "POP.DS");

    | POP.SS() =>
        Exps = instantiate(pc,  "POP.SS");

    | POP.ES() =>
        Exps = instantiate(pc,  "POP.ES");

    | POPod(r32) =>
        Exps = instantiate(pc,  "POPod", DIS_R32);

    | POPow(r32) =>
        Exps = instantiate(pc,  "POPow", DIS_R32);   // Check!

    | POP.Evod(Mem) =>
        Exps = instantiate(pc,  "POP.Evod", DIS_MEM);

    | POP.Evow(Mem) =>
        Exps = instantiate(pc,  "POP.Evow", DIS_MEM);

//    | OUTSvod() =>
//        Exps = instantiate(pc,  "OUTSvod");

//    | OUTSvow() =>
//        Exps = instantiate(pc,  "OUTSvow");

//    | OUTSB() =>
//        Exps = instantiate(pc,  "OUTSB");

//    | OUT.DX.eAXod() =>
//        Exps = instantiate(pc,  "OUT.DX.eAXod");

//    | OUT.DX.eAXow() =>
//        Exps = instantiate(pc,  "OUT.DX.eAXow");

//    | OUT.DX.AL() =>
//        Exps = instantiate(pc,  "OUT.DX.AL");

//    | OUT.Ib.eAXod(i8) =>
//        Exps = instantiate(pc,  "OUT.Ib.eAXod", DIS_I8);

//    | OUT.Ib.eAXow(i8) =>
//        Exps = instantiate(pc,  "OUT.Ib.eAXow", DIS_I8);

//    | OUT.Ib.AL(i8) =>
//        Exps = instantiate(pc,  "OUT.Ib.AL", DIS_I8);

    | NOTod(Eaddr) =>
        Exps = instantiate(pc,  "NOTod", DIS_EADDR32);

    | NOTow(Eaddr) =>
        Exps = instantiate(pc,  "NOTow", DIS_EADDR16);

    | NOTb(Eaddr) =>
        Exps = instantiate(pc,  "NOTb", DIS_EADDR8);

    | NEGod(Eaddr) =>
        Exps = instantiate(pc,  "NEGod", DIS_EADDR32);

    | NEGow(Eaddr) =>
        Exps = instantiate(pc,  "NEGow", DIS_EADDR16);

    | NEGb(Eaddr) =>
        Exps = instantiate(pc,  "NEGb", DIS_EADDR8);

    | MUL.AXod(Eaddr) =>
        Exps = instantiate(pc,  "MUL.AXod", DIS_EADDR32);

    | MUL.AXow(Eaddr) =>
        Exps = instantiate(pc,  "MUL.AXow", DIS_EADDR16);

    | MUL.AL(Eaddr) =>
        Exps = instantiate(pc,  "MUL.AL", DIS_EADDR8);

    | MOVZX.Gv.Ew(r32, Eaddr) =>
        Exps = instantiate(pc,  "MOVZX.Gv.Ew", DIS_R32, DIS_EADDR16);

    | MOVZX.Gv.Ebod(r32, Eaddr) =>
        Exps = instantiate(pc,  "MOVZX.Gv.Ebod", DIS_R32, DIS_EADDR8);

    | MOVZX.Gv.Ebow(r16, Eaddr) =>
        Exps = instantiate(pc,  "MOVZX.Gv.Ebow", DIS_R16, DIS_EADDR8);

    | MOVSX.Gv.Ew(r32, Eaddr) =>
        Exps = instantiate(pc,  "MOVSX.Gv.Ew", DIS_R32, DIS_EADDR16);

    | MOVSX.Gv.Ebod(r32, Eaddr) =>
        Exps = instantiate(pc,  "MOVSX.Gv.Ebod", DIS_R32, DIS_EADDR8);

    | MOVSX.Gv.Ebow(r16, Eaddr) =>
        Exps = instantiate(pc,  "MOVZX.Gv.Ebow", DIS_R16, DIS_EADDR8);

    | MOVSvod() =>
        Exps = instantiate(pc,  "MOVSvod");

    | MOVSvow() =>
        Exps = instantiate(pc,  "MOVSvow");

    | MOVSB() =>
        Exps = instantiate(pc,  "MOVSB");

//    | MOV.Rd.Dd(reg, dr) => 
//        unused(reg); unused(dr);
//        Exps = instantiate(pc,  "UNIMP");

//    | MOV.Dd.Rd(dr, reg) =>
//        unused(reg); unused(dr);
//        Exps = instantiate(pc,  "UNIMP");

//    | MOV.Rd.Cd(reg, cr) =>
//        unused(reg); unused(cr);
//        Exps = instantiate(pc,  "UNIMP");

//    | MOV.Cd.Rd(cr, reg) =>
//        unused(reg); unused(cr);
//        Exps = instantiate(pc,  "UNIMP");

    | MOV.Eb.Ivod(Eaddr, i32) =>
        Exps = instantiate(pc,  "MOV.Eb.Ivod", DIS_EADDR32, DIS_I32);

    | MOV.Eb.Ivow(Eaddr, i16) =>
        Exps = instantiate(pc,  "MOV.Eb.Ivow", DIS_EADDR16, DIS_I16);

    | MOV.Eb.Ib(Eaddr, i8) =>
        Exps = instantiate(pc,  "MOV.Eb.Ib", DIS_EADDR8, DIS_I8);

    | MOVid(r32, i32) =>
        Exps = instantiate(pc,  "MOVid", DIS_R32, DIS_I32);

    | MOViw(r16, i16) =>
        Exps = instantiate(pc,  "MOViw", DIS_R16, DIS_I16);  // Check!

    | MOVib(r8, i8) =>
        Exps = instantiate(pc,  "MOVib", DIS_R8, DIS_I8);

    | MOV.Ov.eAXod(off) =>
        Exps = instantiate(pc,  "MOV.Ov.eAXod", DIS_OFF);

    | MOV.Ov.eAXow(off) =>
        Exps = instantiate(pc,  "MOV.Ov.eAXow", DIS_OFF);

    | MOV.Ob.AL(off) =>
        Exps = instantiate(pc,  "MOV.Ob.AL", DIS_OFF);

    | MOV.eAX.Ovod(off) =>
        Exps = instantiate(pc,  "MOV.eAX.Ovod", DIS_OFF);

    | MOV.eAX.Ovow(off) =>
        Exps = instantiate(pc,  "MOV.eAX.Ovow", DIS_OFF);

    | MOV.AL.Ob(off) =>
        Exps = instantiate(pc,  "MOV.AL.Ob", DIS_OFF);

//    | MOV.Sw.Ew(Mem, sr16) =>
//        Exps = instantiate(pc,  "MOV.Sw.Ew", DIS_MEM, DIS_SR16);

//    | MOV.Ew.Sw(Mem, sr16) =>
//        Exps = instantiate(pc,  "MOV.Ew.Sw", DIS_MEM, DIS_SR16);

    | MOVrmod(reg, Eaddr) =>
        Exps = instantiate(pc,  "MOVrmod", DIS_REG32, DIS_EADDR32);

    | MOVrmow(reg, Eaddr) =>
        Exps = instantiate(pc,  "MOVrmow", DIS_REG16, DIS_EADDR16);

    | MOVrmb(reg, Eaddr) =>
        Exps = instantiate(pc,  "MOVrmb", DIS_REG8, DIS_EADDR8);

    | MOVmrod(Eaddr, reg) =>
        Exps = instantiate(pc,  "MOVmrod", DIS_EADDR32, DIS_REG32);

    | MOVmrow(Eaddr, reg) =>
        Exps = instantiate(pc,  "MOVmrow", DIS_EADDR16, DIS_REG16);

    | MOVmrb(Eaddr, reg) =>
        Exps = instantiate(pc,  "MOVmrb", DIS_EADDR8, DIS_REG8);

    | LTR(Eaddr) =>
        Exps = instantiate(pc,  "LTR", DIS_EADDR32);

    | LSS(reg, Mem) =>
        Exps = instantiate(pc,  "LSS", DIS_REG32, DIS_MEM);

    | LSLod(reg, Eaddr) =>
        Exps = instantiate(pc,  "LSLod", DIS_REG32, DIS_EADDR32);

    | LSLow(reg, Eaddr) =>
        Exps = instantiate(pc,  "LSLow", DIS_REG16, DIS_EADDR16);

    | LOOPNE(relocd) =>
        Exps = instantiate(pc,  "LOOPNE", dis_Num(relocd - hostPC - 2));

    | LOOPE(relocd) =>
        Exps = instantiate(pc,  "LOOPE", dis_Num(relocd-hostPC-2));

    | LOOP(relocd) =>
        Exps = instantiate(pc,  "LOOP", dis_Num(relocd-hostPC-2));

    | LGS(reg, Mem) =>
        Exps = instantiate(pc,  "LGS", DIS_REG32, DIS_MEM);

    | LFS(reg, Mem) =>
        Exps = instantiate(pc,  "LFS", DIS_REG32, DIS_MEM);

    | LES(reg, Mem) =>
        Exps = instantiate(pc,  "LES", DIS_REG32, DIS_MEM);

    | LEAVE() =>
        Exps = instantiate(pc,  "LEAVE");

    | LEAod(reg, Mem) =>
        Exps = instantiate(pc,  "LEA.od", DIS_REG32, DIS_MEM);

    | LEAow(reg, Mem) =>
        Exps = instantiate(pc,  "LEA.ow", DIS_REG16, DIS_MEM);

    | LDS(reg, Mem) =>
        Exps = instantiate(pc,  "LDS", DIS_REG32, DIS_MEM);

    | LARod(reg, Eaddr) =>
        Exps = instantiate(pc,  "LAR.od", DIS_REG32, DIS_EADDR32);

    | LARow(reg, Eaddr) =>
        Exps = instantiate(pc,  "LAR.ow", DIS_REG16, DIS_EADDR16);

    | LAHF() =>
        Exps = instantiate(pc,  "LAHF");

    /* Branches have been handled in decodeInstruction() now */
    | IRET() =>
        Exps = instantiate(pc,  "IRET");

    | INVLPG(Mem) =>
        Exps = instantiate(pc,  "INVLPG", DIS_MEM);

    | INVD() =>
        Exps = instantiate(pc,  "INVD");

    | INTO() =>
        Exps = instantiate(pc,  "INTO");

    | INT.Ib(i8) =>
        Exps = instantiate(pc,  "INT.Ib", DIS_I8);

    | INT3() =>
        Exps = instantiate(pc,  "INT3");

//    | INSvod() =>
//        Exps = instantiate(pc,  "INSvod");

//    | INSvow() =>
//        Exps = instantiate(pc,  "INSvow");

//    | INSB() =>
//        Exps = instantiate(pc,  "INSB");

    | INCod(r32) =>
        Exps = instantiate(pc,  "INCod", DIS_R32);

    | INCow(r32) =>
        Exps = instantiate(pc,  "INCow", DIS_R32);

    | INC.Evod(Eaddr) =>
        Exps = instantiate(pc,  "INC.Evod", DIS_EADDR32);

    | INC.Evow(Eaddr) =>
        Exps = instantiate(pc,  "INC.Evow", DIS_EADDR16);

    | INC.Eb(Eaddr) =>
        Exps = instantiate(pc,  "INC.Eb", DIS_EADDR8);

//    | IN.eAX.DXod() =>
//        Exps = instantiate(pc,  "IN.eAX.DXod");

//    | IN.eAX.DXow() =>
//        Exps = instantiate(pc,  "IN.eAX.DXow");

//    | IN.AL.DX() =>
//        Exps = instantiate(pc,  "IN.AL.DX");

//    | IN.eAX.Ibod(i8) =>
//        Exps = instantiate(pc,  "IN.eAX.Ibod", DIS_I8);

//    | IN.eAX.Ibow(i8) =>
//        Exps = instantiate(pc,  "IN.eAX.Ibow", DIS_I8);

//    | IN.AL.Ib(i8) =>
//        Exps = instantiate(pc,  "IN.AL.Ib", DIS_I8);

    | IMUL.Ivd(reg, Eaddr, i32) =>
        Exps = instantiate(pc,  "IMUL.Ivd", DIS_REG32, DIS_EADDR32, DIS_I32);

    | IMUL.Ivw(reg, Eaddr, i16) =>
        Exps = instantiate(pc,  "IMUL.Ivw", DIS_REG16, DIS_EADDR16, DIS_I16);

    | IMUL.Ibod(reg, Eaddr, i8) =>
        Exps = instantiate(pc,  "IMUL.Ibod", DIS_REG32, DIS_EADDR32, DIS_I8);

    | IMUL.Ibow(reg, Eaddr, i8) =>
        Exps = instantiate(pc,  "IMUL.Ibow", DIS_REG16, DIS_EADDR16, DIS_I8);

    | IMULrmod(reg, Eaddr) =>
        Exps = instantiate(pc,  "IMULrmod", DIS_REG32, DIS_EADDR32);

    | IMULrmow(reg, Eaddr) =>
        Exps = instantiate(pc,  "IMULrmow", DIS_REG16, DIS_EADDR16);

    | IMULod(Eaddr) =>
        Exps = instantiate(pc,  "IMULod", DIS_EADDR32);

    | IMULow(Eaddr) =>
        Exps = instantiate(pc,  "IMULow", DIS_EADDR16);

    | IMULb(Eaddr) =>
        Exps = instantiate(pc,  "IMULb", DIS_EADDR8);

    | IDIVeAX(Eaddr) =>
        Exps = instantiate(pc,  "IDIVeAX", DIS_EADDR32);

    | IDIVAX(Eaddr) =>
        Exps = instantiate(pc,  "IDIVAX", DIS_EADDR16);

    | IDIV(Eaddr) =>
        Exps = instantiate(pc,  "IDIV", DIS_EADDR8); /* ?? */

//  | HLT() =>
//      Exps = instantiate(pc,  "HLT");

    | ENTER(i16, i8) =>
        Exps = instantiate(pc,  "ENTER", DIS_I16, DIS_I8);

    | DIVeAX(Eaddr) =>
        Exps = instantiate(pc,  "DIVeAX", DIS_EADDR32);

    | DIVAX(Eaddr) =>
        Exps = instantiate(pc,  "DIVAX", DIS_EADDR16);

    | DIVAL(Eaddr) =>
        Exps = instantiate(pc,  "DIVAL", DIS_EADDR8);

    | DECod(r32) =>
        Exps = instantiate(pc,  "DECod", DIS_R32);

    | DECow(r32) =>
        Exps = instantiate(pc,  "DECow", DIS_R32);

    | DEC.Evod(Eaddr) =>
        Exps = instantiate(pc,  "DEC.Evod", DIS_EADDR32);

    | DEC.Evow(Eaddr) =>
        Exps = instantiate(pc,  "DEC.Evow", DIS_EADDR16);

    | DEC.Eb(Eaddr) =>
        Exps = instantiate(pc,  "DEC.Eb", DIS_EADDR8);

    | DAS() =>
        Exps = instantiate(pc,  "DAS");

    | DAA() =>
        Exps = instantiate(pc,  "DAA");

    | CDQ() =>
        Exps = instantiate(pc,  "CDQ");

    | CWD() =>
        Exps = instantiate(pc,  "CWD");

    | CPUID() =>
        Exps = instantiate(pc,  "CPUID");

    | CMPXCHG8B(Mem) =>
        Exps = instantiate(pc,  "CMPXCHG8B", DIS_MEM);

    | CMPXCHG.Ev.Gvod(Eaddr, reg) =>
        Exps = instantiate(pc,  "CMPXCHG.Ev.Gvod", DIS_EADDR32, DIS_REG32);

    | CMPXCHG.Ev.Gvow(Eaddr, reg) =>
        Exps = instantiate(pc,  "CMPXCHG.Ev.Gvow", DIS_EADDR16, DIS_REG16);

    | CMPXCHG.Eb.Gb(Eaddr, reg) =>
        Exps = instantiate(pc,  "CMPXCHG.Eb.Gb", DIS_EADDR8, DIS_REG8);

    | CMPSvod() =>
        Exps = instantiate(pc,  "CMPSvod");

    | CMPSvow() =>
        Exps = instantiate(pc,  "CMPSvow");

    | CMPSB() =>
        Exps = instantiate(pc,  "CMPSB");

    | CMC() =>
        Exps = instantiate(pc,  "CMC");

    | CLTS() =>
        Exps = instantiate(pc,  "CLTS");

    | CLI() =>
        Exps = instantiate(pc,  "CLI");

    | CLD() =>
        Exps = instantiate(pc,  "CLD");

    | CLC() =>
        Exps = instantiate(pc,  "CLC");

    | CWDE() =>
        Exps = instantiate(pc,  "CWDE");

    | CBW() =>
        Exps = instantiate(pc,  "CBW");

    /* Decode the following as a NOP. We see these in startup code, and anywhere
        that calls the OS (as lcall 7, 0) */
    | CALL.aPod(seg, off) =>
        unused(seg); unused(off);
        Exps = instantiate(pc, "NOP");

    | CALL.Jvod(relocd) [name] =>
        Exps = instantiate(pc,  name, dis_Num(relocd-hostPC-5));

    | CALL.Evod(Eaddr) =>
        Exps = instantiate(pc,  "CALL.Evod", DIS_EADDR32);

    | BTSiod(Eaddr, i8) =>
        Exps = instantiate(pc,  "BTSiod", DIS_I8, DIS_EADDR32);

    | BTSiow(Eaddr, i8) =>
        Exps = instantiate(pc,  "BTSiow", DIS_I8, DIS_EADDR16);

    | BTSod(Eaddr, reg) =>
        Exps = instantiate(pc,  "BTSod", DIS_EADDR32, DIS_REG32);

    | BTSow(Eaddr, reg) =>
        Exps = instantiate(pc,  "BTSow", DIS_EADDR16, DIS_REG16);

    | BTRiod(Eaddr, i8) =>
        Exps = instantiate(pc,  "BTRiod", DIS_EADDR32, DIS_I8);

    | BTRiow(Eaddr, i8) =>
        Exps = instantiate(pc,  "BTRiow", DIS_EADDR16, DIS_I8);

    | BTRod(Eaddr, reg) =>
        Exps = instantiate(pc,  "BTRod", DIS_EADDR32, DIS_REG32);

    | BTRow(Eaddr, reg) =>
        Exps = instantiate(pc,  "BTRow", DIS_EADDR16, DIS_REG16);

    | BTCiod(Eaddr, i8) =>
        Exps = instantiate(pc,  "BTCiod", DIS_EADDR32, DIS_I8);

    | BTCiow(Eaddr, i8) =>
        Exps = instantiate(pc,  "BTCiow", DIS_EADDR16, DIS_I8);

    | BTCod(Eaddr, reg) =>
        Exps = instantiate(pc,  "BTCod", DIS_EADDR32, DIS_REG32);

    | BTCow(Eaddr, reg) =>
        Exps = instantiate(pc,  "BTCow", DIS_EADDR16, DIS_REG16);

    | BTiod(Eaddr, i8) =>
        Exps = instantiate(pc,  "BTiod", DIS_EADDR32, DIS_I8);

    | BTiow(Eaddr, i8) =>
        Exps = instantiate(pc,  "BTiow", DIS_EADDR16, DIS_I8);

    | BTod(Eaddr, reg) =>
        Exps = instantiate(pc,  "BTod", DIS_EADDR32, DIS_REG32);

    | BTow(Eaddr, reg) =>
        Exps = instantiate(pc,  "BTow", DIS_EADDR16, DIS_REG16);

    | BSWAP(r32) =>
        Exps = instantiate(pc,  "BSWAP", DIS_R32);

    | BSRod(reg, Eaddr) =>
        Exps = instantiate(pc,  "BSRod", DIS_REG32, DIS_EADDR32);

    | BSRow(reg, Eaddr) =>
        Exps = instantiate(pc,  "BSRow", DIS_REG16, DIS_EADDR16);

    | BSFod(reg, Eaddr) =>
        Exps = instantiate(pc,  "BSFod", DIS_REG32, DIS_EADDR32);

    | BSFow(reg, Eaddr) =>
        Exps = instantiate(pc,  "BSFow", DIS_REG16, DIS_EADDR16);

    // Not "user" instructions:
//  | BOUNDod(reg, Mem) =>
//      Exps = instantiate(pc,  "BOUNDod", DIS_REG32, DIS_MEM);

//  | BOUNDow(reg, Mem) =>
//      Exps = instantiate(pc,  "BOUNDow", DIS_REG16, DIS_MEM);

//    | ARPL(Eaddr, reg ) =>
//        unused(Eaddr); unused(reg);
//        Exps = instantiate(pc,  "UNIMP");

//    | AAS() =>
//        Exps = instantiate(pc,  "AAS");

//    | AAM() =>
//        Exps = instantiate(pc,  "AAM");

//    | AAD() =>
//        Exps = instantiate(pc,  "AAD");

//    | AAA() =>
//        Exps = instantiate(pc,  "AAA");

    | CMPrmod(reg, Eaddr) =>
        Exps = instantiate(pc,  "CMPrmod", DIS_REG32, DIS_EADDR32);

    | CMPrmow(reg, Eaddr) =>
        Exps = instantiate(pc,  "CMPrmow", DIS_REG16, DIS_EADDR16);

    | XORrmod(reg, Eaddr) =>
        Exps = instantiate(pc,  "XORrmod", DIS_REG32, DIS_EADDR32);

    | XORrmow(reg, Eaddr) =>
        Exps = instantiate(pc,  "XORrmow", DIS_REG16, DIS_EADDR16);

    | SUBrmod(reg, Eaddr) =>
        Exps = instantiate(pc,  "SUBrmod", DIS_REG32, DIS_EADDR32);

    | SUBrmow(reg, Eaddr) =>
        Exps = instantiate(pc,  "SUBrmow", DIS_REG16, DIS_EADDR16);

    | ANDrmod(reg, Eaddr) =>
        Exps = instantiate(pc,  "ANDrmod", DIS_REG32, DIS_EADDR32);

    | ANDrmow(reg, Eaddr) =>
        Exps = instantiate(pc,  "ANDrmow", DIS_REG16, DIS_EADDR16);

    | SBBrmod(reg, Eaddr) =>
        Exps = instantiate(pc,  "SBBrmod", DIS_REG32, DIS_EADDR32);

    | SBBrmow(reg, Eaddr) =>
        Exps = instantiate(pc,  "SBBrmow", DIS_REG16, DIS_EADDR16);

    | ADCrmod(reg, Eaddr) =>
        Exps = instantiate(pc,  "ADCrmod", DIS_REG32, DIS_EADDR32);

    | ADCrmow(reg, Eaddr) =>
        Exps = instantiate(pc,  "ADCrmow", DIS_REG16, DIS_EADDR16);

    | ORrmod(reg, Eaddr) =>
        Exps = instantiate(pc,  "ORrmod", DIS_REG32, DIS_EADDR32);

    | ORrmow(reg, Eaddr) =>
        Exps = instantiate(pc,  "ORrmow", DIS_REG16, DIS_EADDR16);

    | ADDrmod(reg, Eaddr) =>
        Exps = instantiate(pc,  "ADDrmod", DIS_REG32, DIS_EADDR32);

    | ADDrmow(reg, Eaddr) =>
        Exps = instantiate(pc,  "ADDrmow", DIS_REG16, DIS_EADDR16);

    | CMPrmb(r8, Eaddr) =>
        Exps = instantiate(pc,  "CMPrmb", DIS_R8, DIS_EADDR8);

    | XORrmb(r8, Eaddr) =>
        Exps = instantiate(pc,  "XORrmb", DIS_R8, DIS_EADDR8);

    | SUBrmb(r8, Eaddr) =>
        Exps = instantiate(pc,  "SUBrmb", DIS_R8, DIS_EADDR8);

    | ANDrmb(r8, Eaddr) =>
        Exps = instantiate(pc,  "ANDrmb", DIS_R8, DIS_EADDR8);

    | SBBrmb(r8, Eaddr) =>
        Exps = instantiate(pc,  "SBBrmb", DIS_R8, DIS_EADDR8);

    | ADCrmb(r8, Eaddr) =>
        Exps = instantiate(pc,  "ADCrmb", DIS_R8, DIS_EADDR8);

    | ORrmb(r8, Eaddr) =>
        Exps = instantiate(pc,  "ORrmb", DIS_R8, DIS_EADDR8);

    | ADDrmb(r8, Eaddr) =>
        Exps = instantiate(pc,  "ADDrmb", DIS_R8, DIS_EADDR8);

    | CMPmrod(Eaddr, reg) =>
        Exps = instantiate(pc,  "CMPmrod", DIS_EADDR32, DIS_REG32);

    | CMPmrow(Eaddr, reg) =>
        Exps = instantiate(pc,  "CMPmrow", DIS_EADDR16, DIS_REG16);

    | XORmrod(Eaddr, reg) =>
        Exps = instantiate(pc,  "XORmrod", DIS_EADDR32, DIS_REG32);

    | XORmrow(Eaddr, reg) =>
        Exps = instantiate(pc,  "XORmrow", DIS_EADDR16, DIS_REG16);

    | SUBmrod(Eaddr, reg) =>
        Exps = instantiate(pc,  "SUBmrod", DIS_EADDR32, DIS_REG32);

    | SUBmrow(Eaddr, reg) =>
        Exps = instantiate(pc,  "SUBmrow", DIS_EADDR16, DIS_REG16);

    | ANDmrod(Eaddr, reg) =>
        Exps = instantiate(pc,  "ANDmrod", DIS_EADDR32, DIS_REG32);

    | ANDmrow(Eaddr, reg) =>
        Exps = instantiate(pc,  "ANDmrow", DIS_EADDR16, DIS_REG16);

    | SBBmrod(Eaddr, reg) =>
        Exps = instantiate(pc,  "SBBmrod", DIS_EADDR32, DIS_REG32);

    | SBBmrow(Eaddr, reg) =>
        Exps = instantiate(pc,  "SBBmrow", DIS_EADDR16, DIS_REG16);

    | ADCmrod(Eaddr, reg) =>
        Exps = instantiate(pc,  "ADCmrod", DIS_EADDR32, DIS_REG32);

    | ADCmrow(Eaddr, reg) =>
        Exps = instantiate(pc,  "ADCmrow", DIS_EADDR16, DIS_REG16);

    | ORmrod(Eaddr, reg) =>
        Exps = instantiate(pc,  "ORmrod", DIS_EADDR32, DIS_REG32);

    | ORmrow(Eaddr, reg) =>
        Exps = instantiate(pc,  "ORmrow", DIS_EADDR16, DIS_REG16);

    | ADDmrod(Eaddr, reg) =>
        Exps = instantiate(pc,  "ADDmrod", DIS_EADDR32, DIS_REG32);

    | ADDmrow(Eaddr, reg) =>
        Exps = instantiate(pc,  "ADDmrow", DIS_EADDR16, DIS_REG16);

    | CMPmrb(Eaddr, r8) =>
        Exps = instantiate(pc,  "CMPmrb", DIS_EADDR8, DIS_R8);

    | XORmrb(Eaddr, r8) =>
        Exps = instantiate(pc,  "XORmrb", DIS_EADDR8, DIS_R8);

    | SUBmrb(Eaddr, r8) =>
        Exps = instantiate(pc,  "SUBmrb", DIS_EADDR8, DIS_R8);

    | ANDmrb(Eaddr, r8) =>
        Exps = instantiate(pc,  "ANDmrb", DIS_EADDR8, DIS_R8);

    | SBBmrb(Eaddr, r8) =>
        Exps = instantiate(pc,  "SBBmrb", DIS_EADDR8, DIS_R8);

    | ADCmrb(Eaddr, r8) =>
        Exps = instantiate(pc,  "ADCmrb", DIS_EADDR8, DIS_R8);

    | ORmrb(Eaddr, r8) =>
        Exps = instantiate(pc,  "ORmrb", DIS_EADDR8, DIS_R8);

    | ADDmrb(Eaddr, r8) =>
        Exps = instantiate(pc,  "ADDmrb", DIS_EADDR8, DIS_R8);

    | CMPiodb(Eaddr, i8) =>
        Exps = instantiate(pc,  "CMPiodb", DIS_EADDR32, DIS_I8);

    | CMPiowb(Eaddr, i8) =>
        Exps = instantiate(pc,  "CMPiowb", DIS_EADDR16, DIS_I8);

    | XORiodb(Eaddr, i8) =>
        Exps = instantiate(pc,  "XORiodb", DIS_EADDR8, DIS_I8);

    | XORiowb(Eaddr, i8) =>
        Exps = instantiate(pc,  "XORiowb", DIS_EADDR16, DIS_I8);

    | SUBiodb(Eaddr, i8) =>
        Exps = instantiate(pc,  "SUBiodb", DIS_EADDR32, DIS_I8);

    | SUBiowb(Eaddr, i8) =>
        Exps = instantiate(pc,  "SUBiowb", DIS_EADDR16, DIS_I8);

    | ANDiodb(Eaddr, i8) =>
        Exps = instantiate(pc,  "ANDiodb", DIS_EADDR8, DIS_I8);

    | ANDiowb(Eaddr, i8) =>
        Exps = instantiate(pc,  "ANDiowb", DIS_EADDR16, DIS_I8);

    | SBBiodb(Eaddr, i8) =>
        Exps = instantiate(pc,  "SBBiodb", DIS_EADDR32, DIS_I8);

    | SBBiowb(Eaddr, i8) =>
        Exps = instantiate(pc,  "SBBiowb", DIS_EADDR16, DIS_I8);

    | ADCiodb(Eaddr, i8) =>
        Exps = instantiate(pc,  "ADCiodb", DIS_EADDR8, DIS_I8);

    | ADCiowb(Eaddr, i8) =>
        Exps = instantiate(pc,  "ADCiowb", DIS_EADDR16, DIS_I8);

    | ORiodb(Eaddr, i8) =>
        Exps = instantiate(pc,  "ORiodb", DIS_EADDR8, DIS_I8);

    | ORiowb(Eaddr, i8) =>
        Exps = instantiate(pc,  "ORiowb", DIS_EADDR16, DIS_I8);

    | ADDiodb(Eaddr, i8) =>
        Exps = instantiate(pc,  "ADDiodb", DIS_EADDR32, DIS_I8);

    | ADDiowb(Eaddr, i8) =>
        Exps = instantiate(pc,  "ADDiowb", DIS_EADDR16, DIS_I8);

    | CMPid(Eaddr, i32) =>
        Exps = instantiate(pc,  "CMPid", DIS_EADDR32, DIS_I32);

    | XORid(Eaddr, i32) =>
        Exps = instantiate(pc,  "XORid", DIS_EADDR32, DIS_I32);

    | SUBid(Eaddr, i32) =>
        Exps = instantiate(pc,  "SUBid", DIS_EADDR32, DIS_I32);

    | ANDid(Eaddr, i32) =>
        Exps = instantiate(pc,  "ANDid", DIS_EADDR32, DIS_I32);

    | SBBid(Eaddr, i32) =>
        Exps = instantiate(pc,  "SBBid", DIS_EADDR32, DIS_I32);

    | ADCid(Eaddr, i32) =>
        Exps = instantiate(pc,  "ADCid", DIS_EADDR32, DIS_I32);

    | ORid(Eaddr, i32) =>
        Exps = instantiate(pc,  "ORid", DIS_EADDR32, DIS_I32);

    | ADDid(Eaddr, i32) =>
        Exps = instantiate(pc,  "ADDid", DIS_EADDR32, DIS_I32);

    | CMPiw(Eaddr, i16) =>
        Exps = instantiate(pc,  "CMPiw", DIS_EADDR16, DIS_I16);

    | XORiw(Eaddr, i16) =>
        Exps = instantiate(pc,  "XORiw", DIS_EADDR16, DIS_I16);

    | SUBiw(Eaddr, i16) =>
        Exps = instantiate(pc,  "SUBiw", DIS_EADDR16, DIS_I16);

    | ANDiw(Eaddr, i16) =>
        Exps = instantiate(pc,  "ANDiw", DIS_EADDR16, DIS_I16);

    | SBBiw(Eaddr, i16) =>
        Exps = instantiate(pc,  "SBBiw", DIS_EADDR16, DIS_I16);

    | ADCiw(Eaddr, i16) =>
        Exps = instantiate(pc,  "ADCiw", DIS_EADDR16, DIS_I16);

    | ORiw(Eaddr, i16) =>
        Exps = instantiate(pc,  "ORiw", DIS_EADDR16, DIS_I16);

    | ADDiw(Eaddr, i16) =>
        Exps = instantiate(pc,  "ADDiw", DIS_EADDR16, DIS_I16);

    | CMPib(Eaddr, i8) =>
        Exps = instantiate(pc,  "CMPib", DIS_EADDR8, DIS_I8);

    | XORib(Eaddr, i8) =>
        Exps = instantiate(pc,  "XORib", DIS_EADDR8, DIS_I8);

    | SUBib(Eaddr, i8) =>
        Exps = instantiate(pc,  "SUBib", DIS_EADDR8, DIS_I8);

    | ANDib(Eaddr, i8) =>
        Exps = instantiate(pc,  "ANDib", DIS_EADDR8, DIS_I8);

    | SBBib(Eaddr, i8) =>
        Exps = instantiate(pc,  "SBBib", DIS_EADDR8, DIS_I8);

    | ADCib(Eaddr, i8) =>
        Exps = instantiate(pc,  "ADCib", DIS_EADDR8, DIS_I8);

    | ORib(Eaddr, i8) =>
        Exps = instantiate(pc,  "ORib", DIS_EADDR8, DIS_I8);

    | ADDib(Eaddr, i8) =>
        Exps = instantiate(pc,  "ADDib", DIS_EADDR8, DIS_I8);

    | CMPiEAX(i32) =>
        Exps = instantiate(pc,  "CMPiEAX", DIS_I32);

    | XORiEAX(i32) =>
        Exps = instantiate(pc,  "XORiEAX", DIS_I32);

    | SUBiEAX(i32) =>
        Exps = instantiate(pc,  "SUBiEAX", DIS_I32);

    | ANDiEAX(i32) =>
        Exps = instantiate(pc,  "ANDiEAX", DIS_I32);

    | SBBiEAX(i32) =>
        Exps = instantiate(pc,  "SBBiEAX", DIS_I32);

    | ADCiEAX(i32) =>
        Exps = instantiate(pc,  "ADCiEAX", DIS_I32);

    | ORiEAX(i32) =>
        Exps = instantiate(pc,  "ORiEAX", DIS_I32);

    | ADDiEAX(i32) =>
        Exps = instantiate(pc,  "ADDiEAX", DIS_I32);

    | CMPiAX(i16) =>
        Exps = instantiate(pc,  "CMPiAX", DIS_I16);

    | XORiAX(i16) =>
        Exps = instantiate(pc,  "XORiAX", DIS_I16);

    | SUBiAX(i16) =>
        Exps = instantiate(pc,  "SUBiAX", DIS_I16);

    | ANDiAX(i16) =>
        Exps = instantiate(pc,  "ANDiAX", DIS_I16);

    | SBBiAX(i16) =>
        Exps = instantiate(pc,  "SBBiAX", DIS_I16);

    | ADCiAX(i16) =>
        Exps = instantiate(pc,  "ADCiAX", DIS_I16);

    | ORiAX(i16) =>
        Exps = instantiate(pc,  "ORiAX", DIS_I16);

    | ADDiAX(i16) =>
        Exps = instantiate(pc,  "ADDiAX", DIS_I16);

    | CMPiAL(i8) =>
        Exps = instantiate(pc,  "CMPiAL", DIS_I8);

    | XORiAL(i8) =>
        Exps = instantiate(pc,  "XORiAL", DIS_I8);

    | SUBiAL(i8) =>
        Exps = instantiate(pc,  "SUBiAL", DIS_I8);

    | ANDiAL(i8) =>
        Exps = instantiate(pc,  "ANDiAL", DIS_I8);

    | SBBiAL(i8) =>
        Exps = instantiate(pc,  "SBBiAL", DIS_I8);

    | ADCiAL(i8) =>
        Exps = instantiate(pc,  "ADCiAL", DIS_I8);

    | ORiAL(i8) =>
        Exps = instantiate(pc,  "ORiAL", DIS_I8);

    | ADDiAL(i8) =>
        Exps = instantiate(pc,  "ADDiAL", DIS_I8);

    | LODSvod() =>
        Exps = instantiate(pc,  "LODSvod");

    | LODSvow() =>
        Exps = instantiate(pc,  "LODSvow");

    | LODSB() =>
        Exps = instantiate(pc,  "LODSB");

    /* Floating point instructions */
    | F2XM1() =>
        Exps = instantiate(pc,  "F2XM1");

    | FABS() =>
        Exps = instantiate(pc,  "FABS");

    | FADD.R32(Mem32) =>
        Exps = instantiate(pc,  "FADD.R32", DIS_MEM32);

    | FADD.R64(Mem64) =>
        Exps = instantiate(pc,  "FADD.R64", DIS_MEM64);

    | FADD.ST.STi(idx) =>
        Exps = instantiate(pc,  "FADD.St.STi", DIS_IDX);

    | FADD.STi.ST(idx) =>
        Exps = instantiate(pc,  "FADD.STi.ST", DIS_IDX);

    | FADDP.STi.ST(idx) =>
        Exps = instantiate(pc,  "FADDP.STi.ST", DIS_IDX);

    | FIADD.I32(Mem32) =>
        Exps = instantiate(pc,  "FIADD.I32", DIS_MEM32);

    | FIADD.I16(Mem16) =>
        Exps = instantiate(pc,  "FIADD.I16", DIS_MEM16);

    | FBLD(Mem80) =>
        Exps = instantiate(pc,  "FBLD", DIS_MEM80);

    | FBSTP(Mem80) =>
        Exps = instantiate(pc,  "FBSTP", DIS_MEM80);

    | FCHS() =>
        Exps = instantiate(pc,  "FCHS");

    | FNCLEX() =>
        Exps = instantiate(pc,  "FNCLEX");

    | FCOM.R32(Mem32) =>
        Exps = instantiate(pc,  "FCOM.R32", DIS_MEM32);

    | FCOM.R64(Mem64) =>
        Exps = instantiate(pc,  "FCOM.R64", DIS_MEM64);

    | FICOM.I32(Mem32) =>
        Exps = instantiate(pc,  "FICOM.I32", DIS_MEM32);

    | FICOM.I16(Mem16) =>
        Exps = instantiate(pc,  "FICOM.I16", DIS_MEM16);

    | FCOMP.R32(Mem32) =>
        Exps = instantiate(pc,  "FCOMP.R32", DIS_MEM32);

    | FCOMP.R64(Mem64) =>
        Exps = instantiate(pc,  "FCOMP.R64", DIS_MEM64);

    | FCOM.ST.STi(idx) =>
        Exps = instantiate(pc,  "FCOM.ST.STi", DIS_IDX);

    | FCOMP.ST.STi(idx) =>
        Exps = instantiate(pc,  "FCOMP.ST.STi", DIS_IDX);

    | FICOMP.I32(Mem32) =>
        Exps = instantiate(pc,  "FICOMP.I32", DIS_MEM32);

    | FICOMP.I16(Mem16) =>
        Exps = instantiate(pc,  "FICOMP.I16", DIS_MEM16);

    | FCOMPP() =>
        Exps = instantiate(pc,  "FCOMPP");

    | FCOMI.ST.STi(idx) [name] =>
        Exps = instantiate(pc, name, DIS_IDX);

    | FCOMIP.ST.STi(idx) [name] =>
        Exps = instantiate(pc, name, DIS_IDX);

    | FCOS() =>
        Exps = instantiate(pc,  "FCOS");

    | FDECSTP() =>
        Exps = instantiate(pc,  "FDECSTP");

    | FDIV.R32(Mem32) =>
        Exps = instantiate(pc,  "FDIV.R32", DIS_MEM32);

    | FDIV.R64(Mem64) =>
        Exps = instantiate(pc,  "FDIV.R64", DIS_MEM64);

    | FDIV.ST.STi(idx) =>
        Exps = instantiate(pc,  "FDIV.ST.STi", DIS_IDX);

    | FDIV.STi.ST(idx) =>
        Exps = instantiate(pc,  "FDIV.STi.ST", DIS_IDX);

    | FDIVP.STi.ST(idx) =>
        Exps = instantiate(pc,  "FDIVP.STi.ST", DIS_IDX);

    | FIDIV.I32(Mem32) =>
        Exps = instantiate(pc,  "FIDIV.I32", DIS_MEM32);

    | FIDIV.I16(Mem16) =>
        Exps = instantiate(pc,  "FIDIV.I16", DIS_MEM16);

    | FDIVR.R32(Mem32) =>
        Exps = instantiate(pc,  "FDIVR.R32", DIS_MEM32);

    | FDIVR.R64(Mem64) =>
        Exps = instantiate(pc,  "FDIVR.R64", DIS_MEM64);

    | FDIVR.ST.STi(idx) =>
        Exps = instantiate(pc,  "FDIVR.ST.STi", DIS_IDX);

    | FDIVR.STi.ST(idx) =>
        Exps = instantiate(pc,  "FDIVR.STi.ST", DIS_IDX);

    | FIDIVR.I32(Mem32) =>
        Exps = instantiate(pc,  "FIDIVR.I32", DIS_MEM32);

    | FIDIVR.I16(Mem16) =>
        Exps = instantiate(pc,  "FIDIVR.I16", DIS_MEM16);

    | FDIVRP.STi.ST(idx) =>
        Exps = instantiate(pc,  "FDIVRP.STi.ST", DIS_IDX);

    | FFREE(idx) =>
        Exps = instantiate(pc,  "FFREE", DIS_IDX);

    | FILD.lsI16(Mem16) =>
        Exps = instantiate(pc,  "FILD.lsI16", DIS_MEM16);

    | FILD.lsI32(Mem32) =>
        Exps = instantiate(pc,  "FILD.lsI32", DIS_MEM32);

    | FILD64(Mem64) =>
        Exps = instantiate(pc,  "FILD64", DIS_MEM64);

    | FINIT() =>
        Exps = instantiate(pc,  "FINIT");

    | FIST.lsI16(Mem16) =>
        Exps = instantiate(pc,  "FIST.lsI16", DIS_MEM16);

    | FIST.lsI32(Mem32) =>
        Exps = instantiate(pc,  "FIST.lsI32", DIS_MEM32);

    | FISTP.lsI16(Mem16) =>
        Exps = instantiate(pc,  "FISTP.lsI16", DIS_MEM16);

    | FISTP.lsI32(Mem32) =>
        Exps = instantiate(pc,  "FISTP.lsI32", DIS_MEM32);

    | FISTP64(Mem64) =>
        Exps = instantiate(pc,  "FISTP64", DIS_MEM64);

    | FLD.lsR32(Mem32) =>
        Exps = instantiate(pc,  "FLD.lsR32", DIS_MEM32);

    | FLD.lsR64(Mem64) =>
        Exps = instantiate(pc,  "FLD.lsR64", DIS_MEM64);

    | FLD80(Mem80) =>
        Exps = instantiate(pc,  "FLD80", DIS_MEM80);

/* This is a bit tricky. The FPUSH logically comes between the read of STi and
# the write to ST0. In particular, FLD ST0 is supposed to duplicate the TOS.
# This problem only happens with this load instruction, so there is a work
# around here that gives us the SSL a value of i that is one more than in
# the instruction */
    | FLD.STi(idx) =>
        Exps = instantiate(pc,  "FLD.STi", DIS_IDXP1);

    | FLD1() =>
        Exps = instantiate(pc,  "FLD1");

    | FLDL2T() =>
        Exps = instantiate(pc,  "FLDL2T");

    | FLDL2E() =>
        Exps = instantiate(pc,  "FLDL2E");

    | FLDPI() =>
        Exps = instantiate(pc,  "FLDPI");

    | FLDLG2() =>
        Exps = instantiate(pc,  "FLDLG2");

    | FLDLN2() =>
        Exps = instantiate(pc,  "FLDLN2");

    | FLDZ() =>
        Exps = instantiate(pc,  "FLDZ");

    | FLDCW(Mem16) =>
        Exps = instantiate(pc,  "FLDCW", DIS_MEM16);

    | FLDENV(Mem) =>
        Exps = instantiate(pc,  "FLDENV", DIS_MEM);

    | FMUL.R32(Mem32) =>
        Exps = instantiate(pc,  "FMUL.R32", DIS_MEM32);

    | FMUL.R64(Mem64) =>
        Exps = instantiate(pc,  "FMUL.R64", DIS_MEM64);

    | FMUL.ST.STi(idx) =>
        Exps = instantiate(pc,  "FMUL.ST.STi", DIS_IDX);

    | FMUL.STi.ST(idx) =>
        Exps = instantiate(pc,  "FMUL.STi.ST", DIS_IDX);

    | FMULP.STi.ST(idx) =>
        Exps = instantiate(pc,  "FMULP.STi.ST", DIS_IDX);

    | FIMUL.I32(Mem32) =>
        Exps = instantiate(pc,  "FIMUL.I32", DIS_MEM32);

    | FIMUL.I16(Mem16) =>
        Exps = instantiate(pc,  "FIMUL.I16", DIS_MEM16);

    | FNOP() =>
        Exps = instantiate(pc,  "FNOP");

    | FPATAN() =>
        Exps = instantiate(pc,  "FPATAN");

    | FPREM() =>
        Exps = instantiate(pc,  "FPREM");

    | FPREM1() =>
        Exps = instantiate(pc,  "FPREM1");

    | FPTAN() =>
        Exps = instantiate(pc,  "FPTAN");

    | FRNDINT() =>
        Exps = instantiate(pc,  "FRNDINT");

    | FRSTOR(Mem) =>
        Exps = instantiate(pc,  "FRSTOR", DIS_MEM);

    | FNSAVE(Mem) =>
        Exps = instantiate(pc,  "FNSAVE", DIS_MEM);

    | FSCALE() =>
        Exps = instantiate(pc,  "FSCALE");

    | FSIN() =>
        Exps = instantiate(pc,  "FSIN");

    | FSINCOS() =>
        Exps = instantiate(pc,  "FSINCOS");

    | FSQRT() =>
        Exps = instantiate(pc,  "FSQRT");

    | FST.lsR32(Mem32) =>
        Exps = instantiate(pc,  "FST.lsR32", DIS_MEM32);

    | FST.lsR64(Mem64) =>
        Exps = instantiate(pc,  "FST.lsR64", DIS_MEM64);

    | FSTP.lsR32(Mem32) =>
        Exps = instantiate(pc,  "FSTP.lsR32", DIS_MEM32);

    | FSTP.lsR64(Mem64) =>
        Exps = instantiate(pc,  "FSTP.lsR64", DIS_MEM64);

    | FSTP80(Mem80) =>
        Exps = instantiate(pc,  "FSTP80", DIS_MEM80);

    | FST.st.STi(idx) =>
        Exps = instantiate(pc,  "FST.st.STi", DIS_IDX);

    | FSTP.st.STi(idx) =>
        Exps = instantiate(pc,  "FSTP.st.STi", DIS_IDX);

    | FSTCW(Mem16) =>
        Exps = instantiate(pc,  "FSTCW", DIS_MEM16);

    | FSTENV(Mem) =>
        Exps = instantiate(pc,  "FSTENV", DIS_MEM);

    | FSTSW(Mem16) =>
        Exps = instantiate(pc,  "FSTSW", DIS_MEM16);

    | FSTSW.AX() =>
        Exps = instantiate(pc,  "FSTSW.AX");

    | FSUB.R32(Mem32) =>
        Exps = instantiate(pc,  "FSUB.R32", DIS_MEM32);

    | FSUB.R64(Mem64) =>
        Exps = instantiate(pc,  "FSUB.R64", DIS_MEM64);

    | FSUB.ST.STi(idx) =>
        Exps = instantiate(pc,  "FSUB.ST.STi", DIS_IDX);

    | FSUB.STi.ST(idx) =>
        Exps = instantiate(pc,  "FSUB.STi.ST", DIS_IDX);

    | FISUB.I32(Mem32) =>
        Exps = instantiate(pc,  "FISUB.I32", DIS_MEM32);

    | FISUB.I16(Mem16) =>
        Exps = instantiate(pc,  "FISUB.I16", DIS_MEM16);

    | FSUBP.STi.ST(idx) =>
        Exps = instantiate(pc,  "FSUBP.STi.ST", DIS_IDX);

    | FSUBR.R32(Mem32) =>
        Exps = instantiate(pc,  "FSUBR.R32", DIS_MEM32);

    | FSUBR.R64(Mem64) =>
        Exps = instantiate(pc,  "FSUBR.R64", DIS_MEM64);

    | FSUBR.ST.STi(idx) =>
        Exps = instantiate(pc,  "FSUBR.ST.STi", DIS_IDX);

    | FSUBR.STi.ST(idx) =>
        Exps = instantiate(pc,  "FSUBR.STi.ST", DIS_IDX);

    | FISUBR.I32(Mem32) =>
        Exps = instantiate(pc,  "FISUBR.I32", DIS_MEM32);

    | FISUBR.I16(Mem16) =>
        Exps = instantiate(pc,  "FISUBR.I16", DIS_MEM16);

    | FSUBRP.STi.ST(idx) =>
        Exps = instantiate(pc,  "FSUBRP.STi.ST", DIS_IDX);

    | FTST() =>
        Exps = instantiate(pc,  "FTST");

    | FUCOM(idx) =>
        Exps = instantiate(pc,  "FUCOM", DIS_IDX);

    | FUCOMP(idx) =>
        Exps = instantiate(pc,  "FUCOMP", DIS_IDX);

    | FUCOMPP() =>
        Exps = instantiate(pc,  "FUCOMPP");

    | FUCOMI.ST.STi(idx) [name] =>
        Exps = instantiate(pc, name, DIS_IDX);

    | FUCOMIP.ST.STi(idx) [name] =>
        Exps = instantiate(pc, name, DIS_IDX);

    | FXAM() =>
        Exps = instantiate(pc,  "FXAM");

    | FXCH(idx) =>
        Exps = instantiate(pc,  "FXCH", DIS_IDX);

    | FXTRACT() =>
        Exps = instantiate(pc,  "FXTRACT");

    | FYL2X() =>
        Exps = instantiate(pc,  "FYL2X");

    | FYL2XP1() =>
        Exps = instantiate(pc,  "FYL2XP1");

    else
        result.valid = false;       // Invalid instruction
        result.rtl = NULL;
        result.numBytes = 0;
        return result;
    endmatch

    if (result.rtl == 0)
        result.rtl = new RTL(pc, Exps);
    result.numBytes = nextPC - hostPC;
    return result;
}

/*==============================================================================
 * These are machine specific functions used to decode instruction
 * operands into Exp*s.
 *============================================================================*/

/*==============================================================================
 * FUNCTION:        dis_Mem
 * OVERVIEW:        Converts a dynamic address to a Exp* expression.
 *                  E.g. [1000] --> m[, 1000
 * PARAMETERS:      pc - the address of the Eaddr part of the instr
 *                  expr - the expression that will be built
 * RETURNS:         the Exp* representation of the given Eaddr
 *============================================================================*/
Exp* NJMCDecoder::dis_Mem(ADDRESS pc)
{
    Exp* expr;

    match pc to 
    | Abs32 (a) =>
            // [a]
            expr = new Unary(opMemOf, new Const(a));
    | Disp32 (d, base) => 
            // m[ r[ base] + d]
            expr = new Unary(opMemOf,
                new Binary(opPlus,
                    dis_Reg(24+base),
                    new Const(d)));
    | Disp8 (d, r32) => 
            // m[ r[ r32] + d]
            expr = new Unary(opMemOf,
                new Binary(opPlus,
                    dis_Reg(24+r32),
                    new Const(d)));
    | Index (base, index, ss) =>
            // m[ r[base] + r[index] * ss]
            expr = new Unary(opMemOf,
                new Binary(opPlus,
                    dis_Reg(24+base),
                    new Binary(opMult,
                        dis_Reg(24+index),
                        new Const(1<<ss))));
    | Base (base) =>
            // m[ r[base] ]
            expr = new Unary(opMemOf,
                dis_Reg(24+base));
    | Index32 (d, base, index, ss) =>
            // m[ r[ base ] + r[ index ] * ss + d ]
            expr = new Unary(opMemOf,
                new Binary(opPlus,
                    dis_Reg(24+base),
                    new Binary(opPlus,
                        new Binary(opMult,
                            dis_Reg(24+index),
                            new Const(1<<ss)),
                        new Const(d))));
    | Base32 (d, base) =>
            // m[ r[ base] + d ]
            expr = new Unary(opMemOf,
                new Binary(opPlus,
                    dis_Reg(24+base),
                    new Const(d)));
    | Index8 (d, base, index, ss) =>
            // m[ r[ base ] + r[ index ] * ss + d ]
            expr = new Unary(opMemOf,
                new Binary(opPlus,
                    dis_Reg(24+base),
                    new Binary(opPlus,
                        new Binary(opMult,
                            dis_Reg(24+index),
                            new Const(1<<ss)),
                        new Const(d))));
    | Base8 (d, base) =>
            // m[ r[ base] + d ]
            expr = new Unary(opMemOf,
                new Binary(opPlus,
                    dis_Reg(24+base),
                    new Const(d)));
    | Indir (base) => 
            // m[ r[base] ]
            expr = new Unary(opMemOf,
                dis_Reg(24+base));
    | ShortIndex (d, index, ss) =>
            // m[ r[index] * ss + d ]
            expr = new Unary(opMemOf,
                new Binary(opPlus,
                    new Binary(opMult,
                        dis_Reg(24+index),
                        new Const(1<<ss)),
                    new Const(d)));
    | IndirMem (d) =>
            // [d] (Same as Abs32 using SIB)
            expr = new Unary(opMemOf, new Const(d));
    endmatch
    return expr;
}

/*==============================================================================
 * FUNCTION:        dis_Eaddr
 * OVERVIEW:        Converts a dynamic address to a Exp* expression.
 *                  E.g. %ecx --> r[ 25 ]
 * CALLED FROM:     Macros DIS_EADDR32, DIS_EADDR16 and DIS_EADDR8
 * PARAMETERS:      pc - the instruction stream address of the dynamic
 *                    address
 *                  size - size of the operand (important if a register)
 * RETURNS:         the Exp* representation of the given Eaddr
 *============================================================================*/
Exp* NJMCDecoder::dis_Eaddr(ADDRESS pc, int size)
{
    match pc to
    | E (mem) =>
        return dis_Mem (mem);
    | Reg (reg) =>
        Exp* e;
        switch(size) {
            case 32: e = dis_Reg(24+reg); break;
            case 16: e = dis_Reg(0+reg); break;
            case  8: e = dis_Reg(8+reg); break;
        }
        return e;
    endmatch
}

/*==============================================================================
 * FUNCTION:      isFuncPrologue()
 * OVERVIEW:      Check to see if the instructions at the given offset match
 *                  any callee prologue, i.e. does it look like this offset
 *                  is a pointer to a function?
 * PARAMETERS:    hostPC - pointer to the code in question (native address)
 * RETURNS:       True if a match found
 *============================================================================*/
bool isFuncPrologue(ADDRESS hostPC)
{
#if 0
    int locals, regs;
    if ((InstructionPatterns::frameless_pro(prog.csrSrc, hostPC, locals, regs))
        != NULL)
            return true;
    if ((InstructionPatterns::struct_ptr(prog.csrSrc, hostPC, locals, regs))
        != NULL)
            return true;
    if ((InstructionPatterns::std_entry(prog.csrSrc, hostPC, locals, regs))
        != NULL)
            return true;
#endif
    return false;
}


/**********************************
 * These are the fetch routines.
 **********************************/   

/*==============================================================================
 * FUNCTION:        getWord
 * OVERVIEW:        Returns the word starting at the given address.
 * PARAMETERS:      lc - address at which to decode the double
 * RETURNS:         the decoded double
 *============================================================================*/
Byte getByte (unsigned lc)
/* getByte - returns next byte from image pointed to by lc.  */
{
    return *(Byte *)lc;
}

/*==============================================================================
 * FUNCTION:        getWord
 * OVERVIEW:        Returns the word starting at the given address.
 * PARAMETERS:      lc - address at which to decode the double
 * RETURNS:         the decoded double
 *============================================================================*/
SWord getWord (unsigned lc)
/* get2Bytes - returns next 2-Byte from image pointed to by lc.  */
{
    return (SWord)(*(Byte *)lc + (*(Byte *)(lc+1) << 8));
}

/*==============================================================================
 * FUNCTION:        getDword
 * OVERVIEW:        Returns the double starting at the given address.
 * PARAMETERS:      lc - address at which to decode the double
 * RETURNS:         the decoded double
 *============================================================================*/
DWord getDword (unsigned lc)
/* get4Bytes - returns the next 4-Byte word from image pointed to by lc. */
{
    return (DWord)(*(Byte *)lc + (*(Byte *)(lc+1) << 8) +
        (*(Byte *)(lc+2) << 16) + (*(Byte *)(lc+3) << 24));
}


/*==============================================================================
 * FUNCTION:       NJMCDecoder::NJMCDecoder
 * OVERVIEW:       Constructor. The code won't work without this (not sure why
 *                  the default constructor won't do...)
 * PARAMETERS:     None
 * RETURNS:        N/A
 *============================================================================*/
NJMCDecoder::NJMCDecoder()
{}

// For now...
int NJMCDecoder::decodeAssemblyInstruction(unsigned, int)
{ return 0; }

