/*
 * Copyright (C) 1996, Princeton University or Owen Braun ?????
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       decoder.m
 * OVERVIEW:   Implementation of the higher level mc68000 specific parts of the
 *             NJMCDecoder class.
 *
 * (C) 2000 The University of Queensland, BT Group & Sun Microsystems, Inc.
 *============================================================================*/

/* $Revision$
 * $Id$
 * Created by Mike 15 Feb 2000
 * 21 Feb 2000 - Mike: support for -(an) and (an)+ (bump and bumpr)
 * 21 Mar 2000 - Mike: pPcDisp generates expressions with idCodeAddr
 * 24 Mar 2000 - Mike: Converted sizes to bits; initialise size of SemStr
 *  1 Aug 00 - Cristina: upgraded to support mltk version Apr 5 14:56:28
 *              EDT 2000.  [numBytes] renamed to nextPC as semantics of
 *              [numBytes] has changed.  Assignment of (nextPC - hostPC). 
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Added new derived class
 *               RTL for low-level HRTLs.
 */

#include "global.h"
#include "decoder.h"
#include "prog.h"
#include "ss.h"
#include "rtl.h"
#include "proc.h"
#include "csr.h"
#include "mc68k.pat.h"

/**********************************
 * NJMCDecoder methods.
 **********************************/   

/*==============================================================================
 * FUNCTION:       NJMCDecoder::decodeInstruction
 * OVERVIEW:       Decodes a machine instruction and returns an RTL instance. In
 *                 most cases a single instruction is decoded. However, if a
 *                 higher level construct that may consist of multiple
 *                 instructions is matched, then there may be a need to return
 *                 more than one RTL. The caller_prologue2 is an example of such
 *                 a construct which encloses an abritary instruction that must
 *                 be decoded into its own RTL (386 example)
 * PARAMETERS:     pc - the native address of the pc
 *                 delta - the difference between the above address and the
 *                   host address of the pc (i.e. the address that the pc is at
 *                   in the loaded object file)
 *                 RTLDict - the dictionary of RTL templates used to instantiate
 *                   the RTL for the instruction being decoded
 *                 proc - the enclosing procedure
 *                 pProc - the enclosing procedure
 * RETURNS:        a DecodeResult structure containing all the information
 *                   gathered during decoding
 *============================================================================*/
DecodeResult& NJMCDecoder::decodeInstruction (ADDRESS pc, int delta,
	UserProc* proc = NULL)
{
	static DecodeResult result;
	ADDRESS hostPC = pc + delta;

	// Clear the result structure;
	result.reset();

	// The actual list of instantiated RTs
	list<RT*>* RTs = NULL;

	// Try matching a logue first
	int addr, locals, stackSize, d16, d32, reg;
	ADDRESS saveHostPC = hostPC;
	Logue* logue;
	if ((logue = InstructionPatterns::std_call(csr, hostPC, addr)) != NULL) {
		/*
		 * Direct call
		 */
		HLCall* newCall = new HLCall(pc, 0, RTs);
		result.rtl = newCall;
		result.numBytes = hostPC - saveHostPC;

		// Set the destination expression
		newCall->setDest(addr - delta);
		newCall->setPrologue(logue);

		// Save RTL for the latest call
		//lastCall = newCall;
		SHOW_ASM("std_call "<<addr)
	}

	else if ((logue = InstructionPatterns::near_call(csr, hostPC, addr))
        != NULL) {
		/*
		 * Call with short displacement (16 bit instruction)
		 */
		HLCall* newCall = new HLCall(pc, 0, RTs);
		result.rtl = newCall;
		result.numBytes = hostPC - saveHostPC;

		// Set the destination expression
		newCall->setDest(addr - delta);
		newCall->setPrologue(logue);

		// Save RTL for the latest call
		//lastCall = newCall;
		SHOW_ASM("near_call " << addr)
	}

	else if ((logue = InstructionPatterns::pea_pea_add_rts(csr, hostPC, d32))
        != NULL) {
		/*
		 * pea E(pc) pea 4(pc) / addil #d32, (a7) / rts
         * Handle as a call
		 */
		HLCall* newCall = new HLCall(pc, 0, RTs);
		result.rtl = newCall;
		result.numBytes = hostPC - saveHostPC;

		// Set the destination expression. It's d32 past the address of the
        // d32 itself, which is pc+10
		newCall->setDest(pc + 10 + d32);
		newCall->setPrologue(logue);

		// Save RTL for the latest call
		//lastCall = newCall;
		SHOW_ASM("pea/pea/add/rts " << pc+10+d32)
	}

	else if ((logue = InstructionPatterns::pea_add_rts(csr, hostPC, d32))
        != NULL) {
		/*
		 * pea 4(pc) / addil #d32, (a7) / rts
         * Handle as a call followed by a return
		 */
		HLCall* newCall = new HLCall(pc, 0, RTs);
		result.rtl = newCall;
		result.numBytes = hostPC - saveHostPC;

		// Set the destination expression. It's d32 past the address of the
        // d32 itself, which is pc+6
		newCall->setDest(pc + 6 + d32);
		newCall->setPrologue(logue);

        // This call effectively is followed by a return
        newCall->setReturnAfterCall(true);

		// Save RTL for the latest call
		//lastCall = newCall;
		SHOW_ASM("pea/add/rts " << pc+6+d32)
	}

	else if ((logue = InstructionPatterns::trap_syscall(csr, hostPC, d16))
        != NULL) {
		/*
		 * trap / AXXX  (d16 set to the XXX)
         * Handle as a library call
		 */
		HLCall* newCall = new HLCall(pc, 0, RTs);
		result.rtl = newCall;
		result.numBytes = hostPC - saveHostPC;

		// Set the destination expression. For now, we put AAAAA000+d16 there
		newCall->setDest(0xAAAAA000 + d16);
		newCall->setPrologue(logue);

		SHOW_ASM("trap/syscall " << hex << 0xA000 + d16)
	}

/*
 * CALLEE PROLOGUES
 */
	else if ((logue = InstructionPatterns::link_save(csr, hostPC,
		locals, d16)) != NULL)
	{
		/*
		 * Standard link with save of registers using movem
		 */
		if (proc != NULL) {

			// Record the prologue of this callee
			assert(logue->getType() == Logue::CALLEE_PROLOGUE);
			proc->setPrologue((CalleePrologue*)logue);
		}
		result.rtl = new RTL(pc, RTs);
		result.numBytes = hostPC - saveHostPC;
		SHOW_ASM("link_save " << locals)
	}

	else if ((logue = InstructionPatterns::link_save1(csr, hostPC,
		locals, reg)) != NULL)
	{
		/*
		 * Standard link with save of 1 D register using move dn,-(a7)
		 */
		if (proc != NULL) {

			// Record the prologue of this callee
			assert(logue->getType() == Logue::CALLEE_PROLOGUE);
			proc->setPrologue((CalleePrologue*)logue);
		}
		result.rtl = new RTL(pc, RTs);
		result.numBytes = hostPC - saveHostPC;
		SHOW_ASM("link_save1 " << locals)
	}

	else if ((logue = InstructionPatterns::push_lea(csr, hostPC,
		locals, reg)) != NULL)
	{
		/*
		 * Just save of 1 D register using move dn,-(a7);
         * then an lea d16(a7), a7 to allocate the stack
		 */
        //locals = 0;             // No locals for this prologue
		if (proc != NULL) {

			// Record the prologue of this callee
			assert(logue->getType() == Logue::CALLEE_PROLOGUE);
			proc->setPrologue((CalleePrologue*)logue);
		}
		result.rtl = new RTL(pc, RTs);
		result.numBytes = hostPC - saveHostPC;
		SHOW_ASM("push_lea " << locals)
	}

	else if ((logue = InstructionPatterns::std_link(csr, hostPC,
		locals)) != NULL)
	{
		/*
		 * Standard link
		 */
		if (proc != NULL) {

			// Record the prologue of this callee
			assert(logue->getType() == Logue::CALLEE_PROLOGUE);
			proc->setPrologue((CalleePrologue*)logue);
		}
		result.rtl = new RTL(pc, RTs);
		result.numBytes = hostPC - saveHostPC;
		SHOW_ASM("std_link "<< locals)
	}

	else if ((logue = InstructionPatterns::bare_ret(csr, hostPC))
        != NULL)
	{
		/*
		 * Just a bare rts instruction
		 */
		if (proc != NULL) {

			// Record the prologue of this callee
			assert(logue->getType() == Logue::CALLEE_PROLOGUE);
			proc->setPrologue((CalleePrologue*)logue);
            proc->setEpilogue(new CalleeEpilogue("__dummy",list<string>()));
		}
		result.rtl = new HLReturn(pc, RTs);
		result.numBytes = hostPC - saveHostPC;
		SHOW_ASM("bare_ret")
	}

	else if ((logue = InstructionPatterns::std_ret(csr, hostPC)) != NULL) {
		/*
		 * An unlink and return
		 */
		if (proc!= NULL) {

			// Record the epilogue of this callee
			assert(logue->getType() == Logue::CALLEE_EPILOGUE);
			proc->setEpilogue((CalleeEpilogue*)logue);
		}

		result.rtl = new HLReturn(pc, RTs);
		result.numBytes = hostPC - saveHostPC;
		SHOW_ASM("std_ret")
	}

	else if ((logue = InstructionPatterns::rest_ret(csr, hostPC, d16)) != NULL)
    {
		/*
		 * A restore (movem stack to registers) then return
		 */
		if (proc!= NULL) {

			// Record the epilogue of this callee
			assert(logue->getType() == Logue::CALLEE_EPILOGUE);
			proc->setEpilogue((CalleeEpilogue*)logue);
		}

		result.rtl = new HLReturn(pc, RTs);
		result.numBytes = hostPC - saveHostPC;
		SHOW_ASM("rest_ret")
	}

	else if ((logue = InstructionPatterns::rest1_ret(csr, hostPC, reg)) != NULL)
    {
		/*
		 * A pop (move (a7)+ to one D register) then unlink and return
		 */
		if (proc!= NULL) {

			// Record the epilogue of this callee
			assert(logue->getType() == Logue::CALLEE_EPILOGUE);
			proc->setEpilogue((CalleeEpilogue*)logue);
		}

		result.rtl = new HLReturn(pc, RTs);
		result.numBytes = hostPC - saveHostPC;
		SHOW_ASM("rest1_ret")
	}

	else if ((logue = InstructionPatterns::pop_ret(csr, hostPC, reg)) != NULL)
    {
		/*
		 * A pop (move (a7)+ to one D register) then just return
		 */
		if (proc!= NULL) {

			// Record the epilogue of this callee
			assert(logue->getType() == Logue::CALLEE_EPILOGUE);
			proc->setEpilogue((CalleeEpilogue*)logue);
		}

		result.rtl = new HLReturn(pc, RTs);
		result.numBytes = hostPC - saveHostPC;
		SHOW_ASM("pop_ret")
	}

    else if ((logue = InstructionPatterns::clear_stack(csr, hostPC, stackSize))
        != NULL)
    {
        /*
         * Remove parameters from the stack
         */
        RTs = instantiate(pc, "clear_stack", dis_Num(stackSize));

        result.rtl = new RTL(pc, RTs);
        result.numBytes = hostPC - saveHostPC;
    }

	else {

		ADDRESS nextPC;
        int bump = 0, bumpr;
        SemStr* ss;

		match [nextPC] hostPC to
		
		| regCall(ea) =>
			/*
			 * Register call
			 */
			// Mike: there should probably be a HLNwayCall class for this!
			HLCall* newCall = new HLCall(pc, 0, RTs);
			// Record the fact that this is a computed call
			newCall->setIsComputed();
			// Set the destination expression
			newCall->setDest(cEA(ea, pc, 32));
			result.rtl = newCall;
			// Only one instruction, so size of result is size of this decode
			result.numBytes = nextPC - hostPC;
	
		| regJmp(ea) =>
			/*
			 * Register jump
			 */
			HLNwayJump* newJump = new HLNwayJump(pc, RTs);
			// Record the fact that this is a computed call
			newJump->setIsComputed();
			// Set the destination expression
			newJump->setDest(cEA(ea, pc, 32));
			result.rtl = newJump;
			// Only one instruction, so size of result is size of this decode
			result.numBytes = nextPC - hostPC;
		
		/*
		 * Unconditional branches
		 */
        | bra(d) [name]         =>
			ss = BTA(d, result, pc);
            UNCOND_JUMP(name, nextPC - hostPC, ss);
	
		/*
		 * Conditional branches
		 */
		| bgt(d) [name] =>
            ss = BTA(d, result, pc);
			COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JSG)
		| ble(d) [name] =>
            ss = BTA(d, result, pc);
			COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JSLE)
		| bge(d) [name] =>
            ss = BTA(d, result, pc);
			COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JSGE)
		| blt(d) [name] =>
            ss = BTA(d, result, pc);
			COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JSL)
		| bpl(d) [name] =>
            ss = BTA(d, result, pc);
			COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JPOS)
		| bmi(d) [name] =>
            ss = BTA(d, result, pc);
			COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JMI)
		| bhi(d) [name] =>
            ss = BTA(d, result, pc);
			COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JUG)
		| bls(d) [name] =>
            ss = BTA(d, result, pc);
			COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JULE)
		| bne(d) [name] =>
            ss = BTA(d, result, pc);
			COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JNE)
		| beq(d) [name] =>
            ss = BTA(d, result, pc);
			COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JE)
		| bcc(d) [name] =>
            ss = BTA(d, result, pc);
			COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JUGE)
		| bcs(d) [name] =>
            ss = BTA(d, result, pc);
			COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JUL)
		| bvc(d) [name] =>
            ss = BTA(d, result, pc);
			COND_JUMP(name, nextPC - hostPC, ss, (JCOND_TYPE)0)
		| bvs(d) [name] =>
            ss = BTA(d, result, pc);
			COND_JUMP(name, nextPC - hostPC, ss, (JCOND_TYPE)0)
	
	
        // MVE: I'm assuming that we won't ever see shi(-(a7)) or the like.
        // This would unbalance the stack, although it would be legal for
        // address registers other than a7. For now, we ignore the possibility
        // of having to bump a register
		| shi(ea) [name] =>
            ss = daEA(ea, pc, bump, bumpr, 1);
			RTs = instantiate(pc, name, ss);
			SETS(name, ss, HLJCOND_JSG)
		| sls(ea) [name] =>
            ss = daEA(ea, pc, bump, bumpr, 1);
			RTs = instantiate(pc, name, ss);
			SETS(name, ss, HLJCOND_JULE)
		| scc(ea) [name] =>
            ss = daEA(ea, pc, bump, bumpr, 1);
			RTs = instantiate(pc, name, ss);
			SETS(name, ss, HLJCOND_JUGE)
		| scs(ea) [name] =>
            ss = daEA(ea, pc, bump, bumpr, 1);
			RTs = instantiate(pc, name, ss);
			SETS(name, ss, HLJCOND_JUL)
		| sne(ea) [name] =>
            ss = daEA(ea, pc, bump, bumpr, 1);
			RTs = instantiate(pc, name, ss);
			SETS(name, ss, HLJCOND_JNE)
		| seq(ea) [name] =>
            ss = daEA(ea, pc, bump, bumpr, 1);
			RTs = instantiate(pc, name, ss);
			SETS(name, ss, HLJCOND_JE)
		//| svc(ea) [name] =>
        //  ss = daEA(ea, pc, bump, bumpr, 1);
		//	RTs = instantiate(pc, name, ss);
		//	SETS(name, ss, HLJCOND_)
		//| svs(ea) [name] =>
        //  ss = daEA(ea, pc, bump, bumpr, 1);
		//	RTs = instantiate(pc, name, ss);
		//	SETS(name, ss, HLJCOND_)
		| spl(ea) [name] =>
            ss = daEA(ea, pc, bump, bumpr, 1);
			RTs = instantiate(pc, name, ss);
			SETS(name, ss, HLJCOND_JPOS)
		| smi(ea) [name] =>
            ss = daEA(ea, pc, bump, bumpr, 1);
			RTs = instantiate(pc, name, ss);
			SETS(name, ss, HLJCOND_JMI)
		| sge(ea) [name] =>
            ss = daEA(ea, pc, bump, bumpr, 1);
			RTs = instantiate(pc, name, ss);
			SETS(name, ss, HLJCOND_JSGE)
		| slt(ea) [name] =>
            ss = daEA(ea, pc, bump, bumpr, 1);
			RTs = instantiate(pc, name, ss);
			SETS(name, ss, HLJCOND_JSL)
		| sgt(ea) [name] =>
            ss = daEA(ea, pc, bump, bumpr, 1);
			RTs = instantiate(pc, name, ss);
			SETS(name, ss, HLJCOND_JSG)
		| sle(ea) [name] =>
            ss = daEA(ea, pc, bump, bumpr, 1);
			RTs = instantiate(pc, name, ss);
			SETS(name, ss, HLJCOND_JSLE)
// HACK: Still need to do .ex versions of set, jsr, jmp
	
	else
			result.rtl = new RTL(pc,
				decodeLowLevelInstruction(hostPC, pc, result));
		endmatch
	}
	return result;
}

/*==============================================================================
 * These are machine specific functions used to decode instruction
 * operands into SemStrs.
 *============================================================================*/

/*
 * Modified from the original: 
 *  m68k_ea.m
 *  written by Owen Braun
 *  ocbraun@princeton.edu
 *  created 4/7/96 11:27 pm
 * 
 * 4 Feb 2000 - Cristina, removed ObjFile reference for integration w/UQBT 
 * 7,9 Feb 2000 - Cristina, changed display format of addressing modes to
 *      comply with the Motorola assembly format.
 * 8 Feb 2000 - Note that the ML version of the toolkit generates
 *      repeated label names for procedures that have more than one
 *      matching statement.  Mike's program formats replaces repeated
 *      names by unique names.
 * 9 Feb 00 - Cristina.  Note that I have *not* tested the indexed 
 *		addressing modes as the mc68328 does not support *any* of
 *		such addressing modes.  The mc68000 does however. 
 * 20 Feb 00 - Cristina: fixed branches
 * 21 Feb 00 - Mike: Removed redundant delta from BTA()
 */


// Branch target
SemStr* NJMCDecoder::BTA(ADDRESS d, DecodeResult& result, ADDRESS pc)
{

  SemStr* ret = new SemStr(32);
  ret->push(idIntConst);

  match d to

  | wordOffset (dsp16) => {
                ret->push(pc+2 + dsp16);
                result.numBytes += 2;
            }

  | byteOffset (dsp8) =>  {
                // Casts needed to work around MLTK bug
                ret->push(pc+2 + (int)(char)dsp8);
            }
  endmatch
    
  return ret;
}


void NJMCDecoder::pIllegalMode(ADDRESS pc)
{
    ostrstream ost;
    ost << "Illegal addressing mode at " << hex << pc;
    error(str(ost));
}

SemStr* NJMCDecoder::pDDirect(int r2, int size)
{
    SemStr* ret = new SemStr(size);
    ret->push(idRegOf);
    ret->push(idIntConst);
    ret->push(r2);
    return ret;
}

SemStr* NJMCDecoder::pADirect(int r2, int size)
{
    SemStr* ret = new SemStr(size);
    ret->push(idRegOf);
    ret->push(idIntConst);
    ret->push(r2 + 8);          // First A register is r[8]
    return ret;
}

SemStr* NJMCDecoder::pIndirect(int r2, int size)
{
    SemStr* ret = new SemStr(size);
    ret->push(idMemOf);
    ret->push(idRegOf);
    ret->push(idIntConst);
    ret->push(r2 + 8);          // First A register is r[8]
    return ret;
}

SemStr* NJMCDecoder::pPostInc(int r2, int& bump, int& bumpr, int size)
{
    // Treat this as (an), set bump to size, and bumpr to r2+8
    // Note special semantics when r2 == 7 (stack pointer): if size == 1, then
    // the system will change it to 2 to keep the stack word aligned
    if ((r2 == 7) && (size == 8)) size = 16;
    SemStr* ret = new SemStr(size/8);
    ret->push(idMemOf);
    ret->push(idRegOf);
    ret->push(idIntConst);
    ret->push(r2 + 8);          // First A register is r[8]
    bump = size/8;              // Amount to bump register by
    bumpr = r2 + 8;             // Register to bump
    return ret;
}

SemStr* NJMCDecoder::pPreDec(int r2, int& bump, int& bumpr, int size)
{
    // We treat this as -size(an), set bump to -size, and bumpr to r2+8
    // Note special semantics when r2 == 7 (stack pointer): if size == 1, then
    // the system will change it to 2 to keep the stack word aligned
    if ((r2 == 7) && (size == 8)) size = 16;
    SemStr* ret = new SemStr(size);
    ret->push(idMemOf); ret->push(idPlus);
    ret->push(idRegOf);
    ret->push(idIntConst);
    ret->push(r2 + 8);          // First A register is r[8]
    ret->push(idIntConst); ret->push(-size/8);
    bump = -size/8;             // Amount to bump register by
    bumpr = r2 + 8;             // Register to bump
    return ret;
}

SemStr* NJMCDecoder::alEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);
  match ea to
    | alDDirect (reg2)  => ret = pDDirect(reg2, size);
    | alADirect (reg2)  => ret = pADirect(reg2, size);
    | alIndirect (reg2) => ret = pIndirect(reg2, size);
    | alPostInc (reg2)  => ret = pPostInc(reg2, bump, bumpr, size);
    | alPreDec (reg2)   => ret = pPreDec(reg2, bump, bumpr, size);
    else pIllegalMode(pc);
  endmatch
  return ret;
}

SemStr* NJMCDecoder::amEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);
  match ea to
    | amDDirect (reg2)  => ret = pDDirect(reg2, size);
    | amADirect (reg2)  => ret = pADirect(reg2, size);
    | amIndirect (reg2) => ret = pIndirect(reg2, size);
    | amPostInc (reg2)  => ret = pPostInc(reg2, bump, bumpr, size);
    | amPreDec (reg2)   => ret = pPreDec(reg2, bump, bumpr, size);
    else pIllegalMode(pc);
  endmatch
  return ret;
}

SemStr* NJMCDecoder::awlEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);
  match ea to
    | awlDDirect (reg2)  => ret = pDDirect(reg2, size);
    | awlADirect (reg2)  => ret = pADirect(reg2, size);
    | awlIndirect (reg2) => ret = pIndirect(reg2, size);
    | awlPostInc (reg2)  => ret = pPostInc(reg2, bump, bumpr, size);
    | awlPreDec (reg2)   => ret = pPreDec(reg2, bump, bumpr, size);
    else pIllegalMode(pc);
  endmatch
  return ret;
}

SemStr* NJMCDecoder::cEA(ADDRESS ea, ADDRESS pc, int size)
{
  SemStr* ret = new SemStr(size);
  match ea to
    | cIndirect (reg2) => ret = pIndirect(reg2, size);
    else pIllegalMode(pc);
  endmatch
  return ret;
}

SemStr* NJMCDecoder::dEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);
  match ea to
    | dDDirect (reg2)  => ret = pDDirect(reg2, size);
    | dIndirect (reg2) => ret = pIndirect(reg2, size);
    | dPostInc (reg2)  => ret = pPostInc(reg2, bump, bumpr, size);
    | dPreDec (reg2)   => ret = pPreDec(reg2, bump, bumpr, size);
    else pIllegalMode(pc);
  endmatch
  return ret;
}

SemStr* NJMCDecoder::daEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);
  match ea to
    | daDDirect (reg2)  => ret = pDDirect(reg2, size);
    | daIndirect (reg2) => ret = pIndirect(reg2, size);
    | daPostInc (reg2)  => ret = pPostInc(reg2, bump, bumpr, size);
    | daPreDec (reg2)   => ret = pPreDec(reg2, bump, bumpr, size);
    else pIllegalMode(pc);
  endmatch
  return ret;
}

SemStr* NJMCDecoder::dBEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);
  match ea to
    | dBDDirect (reg2)  => ret = pDDirect(reg2, size);
    | dBIndirect (reg2) => ret = pIndirect(reg2, size);
    | dBPostInc (reg2)  => ret = pPostInc(reg2, bump, bumpr, size);
    | dBPreDec (reg2)   => ret = pPreDec(reg2, bump, bumpr, size);
    else pIllegalMode(pc);
  endmatch
  return ret;
}

SemStr* NJMCDecoder::dWEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);
  match ea to
    | dWDDirect (reg2)  => ret = pDDirect(reg2, size);
    | dWIndirect (reg2) => ret = pIndirect(reg2, size);
    | dWPostInc (reg2)  => ret = pPostInc(reg2, bump, bumpr, size);
    | dWPreDec (reg2)   => ret = pPreDec(reg2, bump, bumpr, size);
    else pIllegalMode(pc);
  endmatch
  return ret;
}

SemStr* NJMCDecoder::maEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);
  match ea to
    | maIndirect (reg2) => ret = pIndirect(reg2, size);
    | maPostInc (reg2)  => ret = pPostInc(reg2, bump, bumpr, size);
    | maPreDec (reg2)   => ret = pPreDec(reg2, bump, bumpr, size);
    else pIllegalMode(pc);
  endmatch
  return ret;
}

SemStr* NJMCDecoder::msEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);
  match ea to
    | msDDirect (reg2)  => ret = pDDirect(reg2, size);
    | msADirect (reg2)  => ret = pADirect(reg2, size);
    | msIndirect (reg2) => ret = pIndirect(reg2, size);
    | msPostInc (reg2)  => ret = pPostInc(reg2, bump, bumpr, size);
    | msPreDec (reg2)   => ret = pPreDec(reg2, bump, bumpr, size);
    else pIllegalMode(pc);
  endmatch
  return ret;
}

SemStr* NJMCDecoder::mdEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);
  match ea to
    | mdDDirect (reg1)  => ret = pDDirect(reg1, size);
    | mdADirect (reg1)  => ret = pADirect(reg1, size);
    | mdIndirect (reg1) => ret = pIndirect(reg1, size);
    | mdPostInc (reg1)  => ret = pPostInc(reg1, bump, bumpr, size);
    | mdPreDec (reg1)   => ret = pPreDec(reg1, bump, bumpr, size);
    else pIllegalMode(pc);
  endmatch
  return ret;
}

SemStr* NJMCDecoder::mrEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);
  match ea to
    | mrIndirect (reg2) => ret = pIndirect(reg2, size);
    | mrPostInc (reg2)  => ret = pPostInc(reg2, bump, bumpr, size);
    else pIllegalMode(pc);
  endmatch
  return ret;
}

SemStr* NJMCDecoder::rmEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);
  match ea to
    | rmIndirect (reg2) => ret = pIndirect(reg2, size);
    | rmPreDec (reg2)   => ret = pPreDec(reg2, bump, bumpr, size);
    else pIllegalMode(pc);
  endmatch
  return ret;
}


SemStr* NJMCDecoder::pADisp(int d16, int r, int size)
{
  SemStr* ret = new SemStr(size);
  // d16(Ar) -> m[ + r[ int r+8 ] int d16]
  ret->push(idMemOf); ret->push(idPlus); ret->push(idRegOf);
  ret->push(idIntConst); ret->push(r+8);
  ret->push(idIntConst); ret->push(d16);
  return ret;
}

SemStr* NJMCDecoder::pAIndex(int d8, int r, int iT, int iR, int iS, int size)
{
    SemStr* ret = new SemStr(size);
    // d8(Ar, A/Di.[wl] ->
    //   m[ + + r[ int r+8 ] ! size[ 16/32 r[ int iT<<3+iR ]] int i8 ]
    ret->push(idMemOf); ret->push(idPlus); ret->push(idPlus);
    ret->push(idRegOf); ret->push(idIntConst); ret->push(r+8);
    ret->push(idSignExt);
    ret->push(idSize);  ret->push(iS == 0 ? 16 : 32);
    ret->push(idRegOf); ret->push(idIntConst); ret->push((iT<<3) + iR);
    ret->push(idIntConst); ret->push(d8);
    return ret;
}

SemStr* NJMCDecoder::pPcDisp(ADDRESS label, int delta, int size)
{
    // Note: label is in the host address space, so need to subtract delta
    SemStr* ret = new SemStr(size);
    // d16(pc) -> m[ code pc+d16 ]
    // Note: we use "code" instead of "int" to flag the fact that this address
    // is relative to the pc, and hence needs translation before use in the
    // target machine
    ret->push(idMemOf); ret->push(idCodeAddr);
    ret->push(label - delta);
    return ret;
}

SemStr* NJMCDecoder::pPcIndex(int d8, int iT, int iR, int iS, ADDRESS nextPc, int size)
{
    // Note: nextPc is expected to have +2 or +4 etc already added to it!
    SemStr* ret = new SemStr(size);
    // d8(pc, A/Di.[wl] ->
    //   m[ + pc+i8 ! size[ 16/32 r[ int iT<<3+iR ]]]
    ret->push(idMemOf); ret->push(idPlus);
    ret->push(idIntConst); ret->push(nextPc+d8);
    ret->push(idSignExt);
    ret->push(idSize);  ret->push(iS == 0 ? 16 : 32);
    ret->push(idRegOf); ret->push(idIntConst); ret->push((iT<<3) + iR);
    return ret;
}

SemStr* NJMCDecoder::pAbsW(int d16, int size)
{
  // (d16).w  ->  size[ ss m[ int d16 ]]
  // Note: d16 should already have been sign extended to 32 bits
  SemStr* ret = new SemStr(size);
  ret->push(idSize); ret->push(size);
  ret->push(idMemOf); ret->push(idIntConst); ret->push(d16);
  return ret;
}

SemStr* NJMCDecoder::pAbsL(int d32, int size)
{
  // (d32).w  ->  size[ ss m[ int d32 ]]
  SemStr* ret = new SemStr(size);
  ret->push(idSize); ret->push(size);
  ret->push(idMemOf); ret->push(idIntConst); ret->push(d32);
  return ret;
}

SemStr* NJMCDecoder::pImmB(int d8)
{
  // #d8 -> int d8
  // Should already be sign extended to 32 bits
  SemStr* ret = new SemStr(8);
  ret->push(idIntConst); ret->push(d8);
  return ret;
}

SemStr* NJMCDecoder::pImmW(int d16)
{
  // #d16 -> int d16
  // Should already be sign extended to 32 bits
  SemStr* ret = new SemStr(16);
  ret->push(idIntConst); ret->push(d16);
  return ret;
}

SemStr* NJMCDecoder::pImmL(int d32)
{
  SemStr* ret = new SemStr(32);
  ret->push(idIntConst); ret->push(d32);
  return ret;
}

void NJMCDecoder::pNonzeroByte(ADDRESS pc)
{
    ostrstream ost;
    ost << "Non zero upper byte at " << hex << pc;
    error(str(ost));
}


SemStr* NJMCDecoder::alEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int size)
{
  SemStr* ret;
  int reg2, mode;

  match eax to
    | alADisp (r2)  => { mode = 0; reg2 = r2; result.numBytes += 2;}
    | alAIndex (r2) => { mode = 1; reg2 = r2; result.numBytes += 2;}
    | alAbs.w ()    => { mode = 4; result.numBytes += 2;}
    | alAbs.l ()    => { mode = 5; result.numBytes += 4;}
    else pIllegalMode(pc);
  endmatch

  switch (mode) {
    case 0 : {
      match x to  
	| alADisp.x (d16) => ret = pADisp(d16, reg2, size);
      endmatch
      break;
    }
    case 1 : {
      match x to
	| alAIndex.x (iT, iR, iS, d8) => ret = pAIndex(d8, reg2, iT, iR, iS, size);
      endmatch
      break;
    }
    case 4 : {
      match x to
	| alAbs.w.x (d16) => ret = pAbsW(d16, size);
      endmatch
      break;
    }
    case 5 : {
      match x to
	| alAbs.l.x (d32) => ret = pAbsL(d32, size);
      endmatch
      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::amEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;

  match eax to
    | amADisp (r2)  => { mode = 0; reg2 = r2; result.numBytes += 2;}
    | amAIndex (r2) => { mode = 1; reg2 = r2; result.numBytes += 2;}
    | amPcDisp ()   => { mode = 2; result.numBytes += 2;}
    | amPcIndex ()  => { mode = 3; result.numBytes += 2;}
    | amAbs.w ()    => { mode = 4; result.numBytes += 2;}
    | amAbs.l ()    => { mode = 5; result.numBytes += 4;}
    | amImm.b ()    => { mode = 6; result.numBytes += 2;}
    | amImm.w ()    => { mode = 7; result.numBytes += 2;}
    | amImm.l ()    => { mode = 8; result.numBytes += 4;}
    else pIllegalMode(pc);
  endmatch

  switch (mode) {
    case 0 : {
      match x to  
	| amADisp.x (d16) => ret = pADisp(d16, reg2, size);
      endmatch
      break;
    }
    case 1 : {
      match x to
	| amAIndex.x (iT, iR, iS, d8) => ret = pAIndex(d8, reg2, iT, iR, iS, size);
      endmatch
      break;
    }
    case 2 : {
      match x to
	| amPcDisp.x (d16) => ret = pPcDisp(d16, delta, size);
      endmatch
      break;
    }
    case 3 : {
      match x to
	| amPcIndex.x (iT, iR, iS, d8) =>
        ret = pPcIndex(d8, iT, iR, iS, pc+2, size);
      endmatch
      break;
    }
    case 4 : {
      match x to
	| amAbs.w.x (d16) => ret = pAbsW(d16, size);
      endmatch
      break;
    }
    case 5 : {
      match x to
	| amAbs.l.x (d32) => ret = pAbsL(d32, size);
      endmatch
      break;
    }
    case 6 : {
      match x to
	| amImm.b.x (d8) => ret = pImmB(d8);
        else pNonzeroByte(pc);
      endmatch
      break;
    }
    case 7 : {
      match x to
	| amImm.w.x (d16) => ret = pImmW(d16);
      endmatch
      break;
    }
    case 8 : {
      match x to
	| amImm.l.x (d32) => ret = pImmL(d32);
      endmatch
      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::awlEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;

  match eax to
    | awlADisp (r2)  => { mode = 0; reg2 = r2; result.numBytes += 2;}
    | awlAIndex (r2) => { mode = 1; reg2 = r2; result.numBytes += 2;}
    | awlPcDisp ()   => { mode = 2; result.numBytes += 2;}
    | awlPcIndex ()  => { mode = 3; result.numBytes += 2;}
    | awlAbs.w ()    => { mode = 4; result.numBytes += 2;}
    | awlAbs.l ()    => { mode = 5; result.numBytes += 4;}
    | awlImm.w ()    => { mode = 7; result.numBytes += 2;}
    | awlImm.l ()    => { mode = 8; result.numBytes += 4;}
    else pIllegalMode(pc);
  endmatch

  switch (mode) {
    case 0 : {
      match x to  
	| awlADisp.x (d16) => ret = pADisp(d16, reg2, size);
      endmatch
      break;
    }
    case 1 : {
      match x to
	| awlAIndex.x (iT, iR, iS, d8) => ret = pAIndex(d8, reg2, iT, iR, iS, size);
      endmatch
      break;
    }
    case 2 : {
      match x to
	| awlPcDisp.x (d16) => ret = pPcDisp(d16, delta, size);
      endmatch
      break;
    }
    case 3 : {
      match x to
	| awlPcIndex.x (iT, iR, iS, d8) =>
        ret = pPcIndex(d8, iT, iR, iS, pc+2, size);
      endmatch
      break;
    }
    case 4 : {
      match x to
	| awlAbs.w.x (d16) => ret = pAbsW(d16, size);
      endmatch
      break;
    }
    case 5 : {
      match x to
	| awlAbs.l.x (d32) => ret = pAbsL(d32, size);
      endmatch
      break;
    }
    case 7 : {
      match x to
	| awlImm.w.x (d16) => ret = pImmW(d16);
      endmatch
      break;
    }
    case 8 : {
      match x to
	| awlImm.l.x (d32) => ret = pImmL(d32);
      endmatch
      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::cEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;

  match eax to
    | cADisp (r2)  => { mode = 0; reg2 = r2; result.numBytes += 2;}
    | cAIndex (r2) => { mode = 1; reg2 = r2; result.numBytes += 2;}
    | cPcDisp ()   => { mode = 2; result.numBytes += 2;}
    | cPcIndex ()  => { mode = 3; result.numBytes += 2;}
    | cAbs.w ()    => { mode = 4; result.numBytes += 2;}
    | cAbs.l ()    => { mode = 5; result.numBytes += 4;}
    else pIllegalMode(pc);
  endmatch

  switch (mode) {
    case 0 : {
      match x to  
	| cADisp.x (d16) => ret = pADisp(d16, reg2, size);
      endmatch
      break;
    }
    case 1 : {
      match x to
	| cAIndex.x (iT, iR, iS, d8) => ret = pAIndex(d8, reg2, iT, iR, iS, size);
      endmatch
      break;
    }
    case 2 : {
      match x to
	| cPcDisp.x (label) => ret = pPcDisp(label, delta, size);
      endmatch
      break;
    }
    case 3 : {
      match x to
	| cPcIndex.x (iT, iR, iS, d8) => ret = pPcIndex(d8, iT, iR, iS, pc+2, size);
      endmatch
      break;
    }
    case 4 : {
      match x to
	| cAbs.w.x (d16) => ret = pAbsW(d16, size);
      endmatch
      break;
    }
    case 5 : {
      match x to
	| cAbs.l.x (d32) => ret = pAbsL(d32, size);
      endmatch
      break;
    }
    default : pIllegalMode(pc); break;
  }  
  return ret;
}


SemStr* NJMCDecoder::dEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;

  match eax to
    | dADisp (r2)  => { mode = 0; reg2 = r2; result.numBytes += 2;}
    | dAIndex (r2) => { mode = 1; reg2 = r2; result.numBytes += 2;}
    | dPcDisp ()   => { mode = 2; result.numBytes += 2;}
    | dPcIndex ()  => { mode = 3; result.numBytes += 2;}
    | dAbs.w ()    => { mode = 4; result.numBytes += 2;}
    | dAbs.l ()    => { mode = 5; result.numBytes += 4;}
    | dImm.b ()    => { mode = 6; result.numBytes += 2;}
    | dImm.w ()    => { mode = 7; result.numBytes += 2;}
    | dImm.l ()    => { mode = 8; result.numBytes += 4;}
    else pIllegalMode(pc);
  endmatch

  switch (mode) {
    case 0 : {
      match x to  
	| dADisp.x (d16) => ret = pADisp(d16, reg2, size);
      endmatch
      break;
    }
    case 1 : {
      match x to
	| dAIndex.x (iT, iR, iS, d8) => ret = pAIndex(d8, reg2, iT, iR, iS, size);
      endmatch
      break;
    }
    case 2 : {
      match x to
	| dPcDisp.x (label) => ret = pPcDisp(label, delta, size);
      endmatch
      break;
    }
    case 3 : {
      match x to
	| dPcIndex.x (iT, iR, iS, d8) => ret = pPcIndex(d8, iT, iR, iS, pc+2, size);
      endmatch
      break;
    }
    case 4 : {
      match x to
	| dAbs.w.x (d16) => ret = pAbsW(d16, size);
      endmatch
      break;
    }
    case 5 : {
      match x to
	| dAbs.l.x (d32) => ret = pAbsL(d32, size);
      endmatch
      break;
    }
    case 6 : {
      match x to
	| dImm.b.x (d8) => ret = pImmB(d8);
        else pNonzeroByte(pc);
      endmatch
      break;
    }
    case 7 : {
      match x to
	| dImm.w.x (d16) => ret = pImmW(d16);
      endmatch
      break;
    }
    case 8 : {
      match x to
	| dImm.l.x (d32) => ret = pImmL(d32);
      endmatch
      break;
    }
    default : pIllegalMode(pc); break;
  }  
  return ret;
}


SemStr* NJMCDecoder::daEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int size)
{
  SemStr* ret;
  int reg2, mode;

  match eax to
    | daADisp (r2)  => { mode = 0; reg2 = r2; result.numBytes += 2;}
    | daAIndex (r2) => { mode = 1; reg2 = r2; result.numBytes += 2;}
    | daAbs.w ()    => { mode = 4; result.numBytes += 2;}
    | daAbs.l ()    => { mode = 5; result.numBytes += 4;}
    else pIllegalMode(pc);
  endmatch

  switch (mode) {
    case 0 : {
      match x to  
	| daADisp.x (d16) => ret = pADisp(d16, reg2, size);
      endmatch
      break;
    }
    case 1 : {
      match x to
	| daAIndex.x (iT, iR, iS, d8) => ret = pAIndex(d8, reg2, iT, iR, iS, size);
      endmatch
      break;
    }
    case 4 : {
      match x to
	| daAbs.w.x (d16) => ret = pAbsW(d16, size);
      endmatch
      break;
    }
    case 5 : {
      match x to
	| daAbs.l.x (d32) => ret = pAbsL(d32, size);
      endmatch
      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::dBEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;

  match eax to
    | dBADisp (r2)  => { mode = 0; reg2 = r2; result.numBytes += 2;}
    | dBAIndex (r2) => { mode = 1; reg2 = r2; result.numBytes += 2;}
    | dBPcDisp ()   => { mode = 2; result.numBytes += 2;}
    | dBPcIndex ()  => { mode = 3; result.numBytes += 2;}
    | dBAbs.w ()    => { mode = 4; result.numBytes += 2;}
    | dBAbs.l ()    => { mode = 5; result.numBytes += 4;}
    | dBImm.b ()    => { mode = 6; result.numBytes += 2;}
    else pIllegalMode(pc);
  endmatch

  switch (mode) {
    case 0 : {
      match x to  
	| dBADisp.x (d16) => ret = pADisp(d16, reg2, size);
      endmatch
      break;
    }
    case 1 : {
      match x to
	| dBAIndex.x (iT, iR, iS, d8) => ret = pAIndex(d8, reg2, iT, iR, iS, size);
      endmatch
      break;
    }
    case 2 : {
      match x to
	| dBPcDisp.x (label) => ret = pPcDisp(label, delta, size);
      endmatch
      break;
    }
    case 3 : {
      match x to
	| dBPcIndex.x (iT, iR, iS, d8) =>
        ret = pPcIndex(d8, iT, iR, iS, pc+2, size);
      endmatch
      break;
    }
    case 4 : {
      match x to
	| dBAbs.w.x (d16) => ret = pAbsW(d16, size);
      endmatch
      break;
    }
    case 5 : {
      match x to
	| dBAbs.l.x (d32) => ret = pAbsL(d32, size);
      endmatch
      break;
    }
    case 6 : {
      match x to
	| dImm.b.x (d8) => ret = pImmB(d8);
        else pNonzeroByte(pc);
      endmatch
      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::dWEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;

  match eax to
    | dWADisp (r2)  => { mode = 0; reg2 = r2; result.numBytes += 2;}
    | dWAIndex (r2) => { mode = 1; reg2 = r2; result.numBytes += 2;}
    | dWPcDisp ()   => { mode = 2; result.numBytes += 2;}
    | dWPcIndex ()  => { mode = 3; result.numBytes += 2;}
    | dWAbs.w ()    => { mode = 4; result.numBytes += 2;}
    | dWAbs.l ()    => { mode = 5; result.numBytes += 4;}
    | dWImm.w ()    => { mode = 7; result.numBytes += 2;}
    else pIllegalMode(pc);
  endmatch

  switch (mode) {
    case 0 : {
      match x to  
	| dWADisp.x (d16) => ret = pADisp(d16, reg2, size);
      endmatch
      break;
    }
    case 1 : {
      match x to
	| dWAIndex.x (iT, iR, iS, d8) => ret = pAIndex(d8, reg2, iT, iR, iS, size);
      endmatch
      break;
    }
    case 2 : {
      match x to
	| dWPcDisp.x (label) => ret = pPcDisp(label, delta, size);
      endmatch
      break;
    }
    case 3 : {
      match x to
	| dWPcIndex.x (iT, iR, iS, d8) =>
        ret = pPcIndex(d8, iT, iR, iS, pc+2, size);
      endmatch
      break;
    }
    case 4 : {
      match x to
	| dWAbs.w.x (d16) => ret = pAbsW(d16, size);
      endmatch
      break;
    }
    case 5 : {
      match x to
	| dWAbs.l.x (d32) => ret = pAbsL(d32, size);
      endmatch
      break;
    }
    case 7 : {
      match x to
	| dWImm.w.x (d16) => ret = pImmW(d16);
      endmatch
      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::maEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int size)
{
  SemStr* ret;
  int reg2, mode;

  match eax to
    | maADisp (r2)  => { mode = 0; reg2 = r2; result.numBytes += 2;}
    | maAIndex (r2) => { mode = 1; reg2 = r2; result.numBytes += 2;}
    | maAbs.w ()    => { mode = 4; result.numBytes += 2;}
    | maAbs.l ()    => { mode = 5; result.numBytes += 4;}
    else pIllegalMode(pc);
  endmatch

  switch (mode) {
    case 0 : {
      match x to  
	| maADisp.x (d16) => ret = pADisp(d16, reg2, size);
      endmatch
      break;
    }
    case 1 : {
      match x to
	| maAIndex.x (iT, iR, iS, d8) => ret = pAIndex(d8, reg2, iT, iR, iS, size);
      endmatch
      break;
    }
    case 4 : {
      match x to
	| maAbs.w.x (d16) => ret = pAbsW(d16, size);
      endmatch
      break;
    }
    case 5 : {
      match x to
	| maAbs.l.x (d32) => ret = pAbsL(d32, size);
      endmatch
      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::msEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;

  match eax to
    | msADisp (r2)  => { mode = 0; reg2 = r2; result.numBytes += 2;}
    | msAIndex (r2) => { mode = 1; reg2 = r2; result.numBytes += 2;}
    | msPcDisp ()   => { mode = 2; result.numBytes += 2;}
    | msPcIndex ()  => { mode = 3; result.numBytes += 2;}
    | msAbs.w ()    => { mode = 4; result.numBytes += 2;}
    | msImm.b ()    => { mode = 6; result.numBytes += 2;}
    | msImm.w ()    => { mode = 7; result.numBytes += 2;}
    else pIllegalMode(pc);
  endmatch

  switch (mode) {
    case 0 : {
      match x to  
	| msADisp.x (d16) => ret = pADisp(d16, reg2, size);
      endmatch
      break;
    }
    case 1 : {
      match x to
	| msAIndex.x (iT, iR, iS, d8) => ret = pAIndex(d8, reg2, iT, iR, iS, size);
      endmatch
      break;
    }
    case 2 : {
      match x to
	| msPcDisp.x (label) => ret = pPcDisp(label, delta, size);
      endmatch
      break;
    }
    case 3 : {
      match x to
	| msPcIndex.x (iT, iR, iS, d8) =>
        ret = pPcIndex(d8, iT, iR, iS, pc+2, size);
      endmatch
      break;
    }
    case 4 : {
      match x to
	| msAbs.w.x (d16) => ret = pAbsW(d16, size);
      endmatch
      break;
    }
    case 6 : {
      match x to
	| msImm.b.x (d8) => ret = pImmB(d8);
        else pNonzeroByte(pc);
      endmatch
      break;
    }
    case 7 : {
      match x to
	| msImm.w.x (d16) => ret = pImmW(d16);
      endmatch
      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::msEAXL(ADDRESS eaxl, int d32, DecodeResult& result,
    ADDRESS pc, int size)
{
  SemStr* ret;
  int reg2, mode;

  match eaxl to
    | msAbs.l ()    => { mode = 5; result.numBytes += 4;}
    | msImm.l ()    => { mode = 8; result.numBytes += 4;}
    else pIllegalMode(pc);
  endmatch

  switch (mode) {
    case 5 : {
      ret = pAbsL(d32, size);
      break;
    }
    case 8 : {
      ret = pImmL(d32);
      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::mdEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int size)
{
  SemStr* ret;
  int reg1, mode;

  match eax to
    | mdADisp (r1)  => { mode = 0; reg1 = r1; result.numBytes += 2;}
    | mdAIndex (r1) => { mode = 1; reg1 = r1; result.numBytes += 2;}
    | mdAbs.w ()    => { mode = 4; result.numBytes += 2;}
    | mdAbs.l ()    => { mode = 5; result.numBytes += 4;}
    else pIllegalMode(pc);
  endmatch

  switch (mode) {
    case 0 : {
      match x to  
	| mdADisp.x (d16) => ret = pADisp(d16, reg1, size);
      endmatch
      break;
    }
    case 1 : {
      match x to
	| mdAIndex.x (iT, iR, iS, d8) => ret = pAIndex(d8, reg1, iT, iR, iS, size);
	endmatch
      break;
    }
    case 4 : {
      match x to
	| mdAbs.w.x (d16) => ret = pAbsW(d16, size);
      endmatch
      break;
    }
    case 5 : {
      match x to
	| mdAbs.l.x (d32) => ret = pAbsL(d32, size);
      endmatch
      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::mrEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;

  match eax to
    | mrADisp (r2)  => { mode = 0; reg2 = r2; result.numBytes += 2;}
    | mrAIndex (r2) => { mode = 1; reg2 = r2; result.numBytes += 2;}
    | mrPcDisp ()   => { mode = 2; result.numBytes += 2;}
    | mrPcIndex ()  => { mode = 3; result.numBytes += 2;}
    | mrAbs.w ()    => { mode = 4; result.numBytes += 2;}
    | mrAbs.l ()    => { mode = 5; result.numBytes += 4;}
    else pIllegalMode(pc);
  endmatch

  switch (mode) {
    case 0 : {
      match x to  
	| mrADisp.x (d16) => ret = pADisp(d16, reg2, size);
      endmatch
      break;
    }
    case 1 : {
      match x to
	| mrAIndex.x (iT, iR, iS, d8) => ret = pAIndex(d8, reg2, iT, iR, iS, size);
      endmatch
      break;
    }
    case 2 : {
      match x to
	| mrPcDisp.x (label) => ret = pPcDisp(label, delta, size);
      endmatch
      break;
    }
    case 3 : {
      match x to
	| mrPcIndex.x (iT, iR, iS, d8) =>
        ret = pPcIndex(d8, iT, iR, iS, pc+2, size);
      endmatch
      break;
    }
    case 4 : {
      match x to
	| mrAbs.w.x (d16) => ret = pAbsW(d16, size);
      endmatch
      break;
    }
    case 5 : {
      match x to
	| mrAbs.l.x (d32) => ret = pAbsL(d32, size);
      endmatch
      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::rmEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int size)
{
  SemStr* ret;
  int reg2, mode;

  match eax to
    | rmADisp (r2)  => { mode = 0; reg2 = r2; result.numBytes += 2;}
    | rmAIndex (r2) => { mode = 1; reg2 = r2; result.numBytes += 2;}
    | rmAbs.w ()    => { mode = 4; result.numBytes += 2;}
    | rmAbs.l ()    => { mode = 5; result.numBytes += 4;}
    else pIllegalMode(pc);
  endmatch

  switch (mode) {
    case 0 : {
      match x to  
	| rmADisp.x (d16) => ret = pADisp(d16, reg2, size);
      endmatch
      break;
    }
    case 1 : {
      match x to
	| rmAIndex.x (iT, iR, iS, d8) => ret = pAIndex(d8, reg2, iT, iR, iS, size);
      endmatch
      break;
    }
    case 4 : {
      match x to
	| rmAbs.w.x (d16) => ret = pAbsW(d16, size);
      endmatch
      break;
    }
    case 5 : {
      match x to
	| rmAbs.l.x (d32) => ret = pAbsL(d32, size);
      endmatch
      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
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
    int locals, reg, d16;

    if ((InstructionPatterns::link_save(prog.csrSrc, hostPC, locals, d16))
        != NULL)
            return true;
    if ((InstructionPatterns::link_save1(prog.csrSrc, hostPC, locals, reg))
        != NULL)
            return true;
    if ((InstructionPatterns::push_lea(prog.csrSrc, hostPC, locals, reg))
        != NULL)
            return true;
    if ((InstructionPatterns::std_link(prog.csrSrc, hostPC, locals)) != NULL)
        return true;

    return false;
}

