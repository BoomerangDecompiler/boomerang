/*
 * Copyright (C) 1996-2001, The University of Queensland
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
 * 24 Jan 00 - Mike: Changes to ensure that double and quad registers get the
 *              correct names (e.g. %f2to3, %f4to7)
 * 20 Jul 00 - Cristina: The [numBytes] notation in a match statement in 
 *				version Apr 5 14:56:28 EDT 2000 of the toolkit (mltk)
 *				now returns a pointer to the next instruction to decode 
 *				rather than the number of bytes decoded.  results.numBytes
 *				was fixed accordingly, and numBytes was renamed nextPC. 
 * 03 Apr 01 - Mike: Added DIS_ADDR to "trap" arm (was segfaulting whenever a
 *              trap instruction was speculatively decoded)
 * 12 Apr 01 - Mike: Added FSQRTd and FSQRTq instructions
 * 24 Oct 01 - Mike: Suppressed some "unused variable" warnings
*/

#include "global.h"
#include "decoder.h"
#include "rtl.h"
#include "sparc-names.h"

void unused(int);

/*==============================================================================
 * FUNCTION:       NJMCDecoder::decodeLowLevelInstruction
 * OVERVIEW:       Decodes a machine instruction and returns an instantiated
 *                 list of RTs.
 * PARAMETERS:     hostPC - the address of the pc in the loaded Elf object
 *                 pc - the virtual address of the pc
 *                 result - a reference parameter that has a fields for the
 *                  number of bytes decoded, their validity, etc
 * RETURNS:        the instantiated list of RTs
 *============================================================================*/
list<RT*>* NJMCDecoder::decodeLowLevelInstruction (ADDRESS hostPC, ADDRESS pc,
	DecodeResult& result)
{

	// The list of instantiated RTs.
	list<RT*>* RTs = NULL;

	ADDRESS nextPC;
	match [nextPC] hostPC to
 
	| NOP [name] =>
		result.type = NOP;
		RTs = instantiate(pc,  name);

	| sethi(imm22, rd) => 
		RTs = instantiate(pc,  "sethi", dis_Num(imm22), DIS_RD);

	| restore_() [name] =>
		RTs = instantiate(pc,  name, dis_Reg("%g0"), dis_Reg("%g0"),
			dis_Reg("%g0"));

	| save_ () [name] =>
		RTs = instantiate(pc,  name, dis_Reg("%g0"), dis_Reg("%g0"),
			dis_Reg("%g0"));

	| load_greg(addr, rd) [name] => 
		RTs = instantiate(pc,  name, DIS_ADDR, DIS_RD);

	| LDF (addr, fds) [name] => 
		RTs = instantiate(pc,  name, DIS_ADDR, DIS_FDS);

	| LDDF (addr, fdd) [name] => 
		RTs = instantiate(pc,  name, DIS_ADDR, DIS_FDD);

	| load_creg(addr, cd) [name] => 
		RTs = instantiate(pc,  name, DIS_ADDR, DIS_CD);
		
	| load_asi (addr, asi, rd) [name] => 
        unused(asi);            // Note: this could be serious!
		RTs = instantiate(pc,  name, DIS_RD, DIS_ADDR);

	| sto_greg(rd, addr) [name] => 
		RTs = instantiate(pc,  name, DIS_RD, DIS_ADDR);

	| STF (fds, addr) [name] => 
		RTs = instantiate(pc,  name, DIS_FDS, DIS_ADDR);

	| STDF (fdd, addr) [name] => 
		RTs = instantiate(pc,  name, DIS_FDD, DIS_ADDR);

	| sto_creg(cd, addr) [name] => 
		RTs = instantiate(pc,  name, DIS_CD, DIS_ADDR);

	| sto_asi (rd, addr, asi) [name] => 
        unused(asi);            // Note: this could be serious!
		RTs = instantiate(pc,  name, DIS_RD, DIS_ADDR);

	| LDFSR(addr) [name] => 
		RTs = instantiate(pc,  name, DIS_ADDR);

	| LDCSR(addr) [name] => 
		RTs = instantiate(pc,  name, DIS_ADDR);

	| STFSR(addr) [name] => 
		RTs = instantiate(pc,  name, DIS_ADDR);

	| STCSR(addr) [name] => 
		RTs = instantiate(pc,  name, DIS_ADDR);

	| STDFQ(addr) [name] => 
		RTs = instantiate(pc,  name, DIS_ADDR);

	| STDCQ(addr) [name] => 
		RTs = instantiate(pc,  name, DIS_ADDR);

	| RDY(rd) [name] => 
		RTs = instantiate(pc,  name, DIS_RD);

	| RDPSR(rd) [name] => 
		RTs = instantiate(pc,  name, DIS_RD);

	| RDWIM(rd) [name] => 
		RTs = instantiate(pc,  name, DIS_RD);

	| RDTBR(rd) [name]	=> 
		RTs = instantiate(pc,  name, DIS_RD);

	| WRY(rs1,roi) [name]	=> 
		RTs = instantiate(pc,  name, DIS_RS1, DIS_ROI);

	| WRPSR(rs1, roi) [name] => 
		RTs = instantiate(pc,  name, DIS_RS1, DIS_ROI);

	| WRWIM(rs1, roi) [name] => 
		RTs = instantiate(pc,  name, DIS_RS1, DIS_ROI);

	| WRTBR(rs1, roi) [name] => 
		RTs = instantiate(pc,  name, DIS_RS1, DIS_ROI);

	| alu (rs1, roi, rd) [name] => 
		RTs = instantiate(pc,  name, DIS_RS1, DIS_ROI, DIS_RD);

	| branch^",a" (tgt) [name] => 
/*
		char shortname[8];
		strcpy(shortname, name);
		shortname[strlen(name)-2] = '\0';		// Remove the ",a"
*/
              RTs = instantiate(pc,  name, dis_Num((tgt-hostPC)>>2) );

	| branch (tgt) [name] => 

              RTs = instantiate(pc,  name, dis_Num((int)((tgt-hostPC))>>2) );

	| call__ (tgt) =>
		result.type = SD;
		RTs = instantiate(pc,  "call__", dis_Num(((int)(tgt-hostPC))>>2));

	| float2s (fs2s, fds) [name] => 
		RTs = instantiate(pc,  name, DIS_FS2S, DIS_FDS);

	| float3s (fs1s, fs2s, fds) [name] => 
		RTs = instantiate(pc,  name, DIS_FS1S, DIS_FS2S, DIS_FDS);
 
	| float3d (fs1d, fs2d, fdd) [name] => 
		RTs = instantiate(pc,  name, DIS_FS1D, DIS_FS2D, DIS_FDD);
 
	| float3q (fs1q, fs2q, fdq) [name] => 
		RTs = instantiate(pc,  name, DIS_FS1Q, DIS_FS2Q, DIS_FDQ);
 
	| fcompares (fs1s, fs2s) [name] => 
		RTs = instantiate(pc,  name, DIS_FS1S, DIS_FS2S);

	| fcompared (fs1d, fs2d) [name] => 
		RTs = instantiate(pc,  name, DIS_FS1D, DIS_FS2D);

	| fcompareq (fs1q, fs2q) [name] => 
		RTs = instantiate(pc,  name, DIS_FS1Q, DIS_FS2Q);

    | FTOs (fs2s, fds) [name] =>
        RTs = instantiate(pc, name, DIS_FS2S, DIS_FDS);

    // Note: itod and dtoi have different sized registers
    | FiTOd (fs2s, fdd) [name] =>
        RTs = instantiate(pc, name, DIS_FS2S, DIS_FDD);
    | FdTOi (fs2d, fds) [name] =>
        RTs = instantiate(pc, name, DIS_FS2D, DIS_FDS);

    | FiTOq (fs2s, fdq) [name] =>
        RTs = instantiate(pc, name, DIS_FS2S, DIS_FDQ);
    | FqTOi (fs2q, fds) [name] =>
        RTs = instantiate(pc, name, DIS_FS2Q, DIS_FDS);

    | FsTOd (fs2s, fdd) [name] =>
        RTs = instantiate(pc, name, DIS_FS2S, DIS_FDD);
    | FdTOs (fs2d, fds) [name] =>
        RTs = instantiate(pc, name, DIS_FS2D, DIS_FDS);

    | FsTOq (fs2s, fdq) [name] =>
        RTs = instantiate(pc, name, DIS_FS2S, DIS_FDQ);
    | FqTOs (fs2q, fds) [name] =>
        RTs = instantiate(pc, name, DIS_FS2Q, DIS_FDS);

    | FdTOq (fs2d, fdq) [name] =>
        RTs = instantiate(pc, name, DIS_FS2D, DIS_FDQ);
    | FqTOd (fs2q, fdd) [name] =>
        RTs = instantiate(pc, name, DIS_FS2Q, DIS_FDD);


    | FSQRTd (fs2d, fdd) [name] =>
        RTs = instantiate(pc, name, DIS_FS2D, DIS_FDD);

    | FSQRTq (fs2q, fdq) [name] =>
        RTs = instantiate(pc, name, DIS_FS2Q, DIS_FDQ);


	| JMPL (addr, rd) [name] => 
		result.type = DD;
		RTs = instantiate(pc,  name, DIS_ADDR, DIS_RD);

	| RETT (addr) [name] => 
        unused(addr);
		RTs = instantiate(pc,  name);

	| trap (addr) [name] => 
		RTs = instantiate(pc,  name, DIS_ADDR);

	| UNIMP (n) => 
        unused(n);
		RTs = NULL;
        result.valid = false;

	| inst = n => 
        // What does this mean?
        unused(n);
        result.valid = false;
		RTs = NULL;

    else
		RTs = NULL;
        result.valid = false;
		result.numBytes = 0;

	endmatch

	result.numBytes = (nextPC - hostPC);
	return RTs;
}

