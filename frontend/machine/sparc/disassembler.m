/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       disassembler.m
 * OVERVIEW:   Skeleton file for a disassembler of SPARC instructions. 
 *============================================================================*/

/*
 * $Revision$
 *
 *    Apr 01 - Cristina: Created
 * 18 May 01 - Mike: Slight mods to accomodate other disassemblers; moved
 *              default NJMCDecoder constructor to disasm.cc
 */

#include "global.h"
#include "decoder.h"
#include "sparc-names.h"
#include "BinaryFile.h"         // For SymbolByAddress()

// Globals in driver disasm.cc file
extern  char _assembly[81];


// Need to provide the fetch routines for this machine (32-bits only
// on SPARC)

/*
 * FUNCTION:        getDword
 * OVERVIEW:        Returns the double starting at the given address.
 * PARAMETERS:      lc - address at which to decode the double
 * RETURNS:         the decoded double
 */

DWord getDword(ADDRESS lc)
{
  Byte* p = (Byte*)lc;
  return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
}



/* 
 * FUNCTION: 		dis_RegImm
 * OVERVIEW:		decodes a register or an immediate value
 * PARAMETERS: 		address pointer to be decoded
 * RETURNS: 		string with information about register or immediate 
 */

char *NJMCDecoder::dis_RegImm (ADDRESS pc)
{
	static char _buffer[11]; 

    match pc to
    | imode(i) =>	sprintf (_buffer, "%d", i); 
    | rmode(rs2) => sprintf (_buffer, "%s", DIS_RS2); 
    endmatch
	return _buffer;
}


/* 
 * FUNCTION: 		dis_Eaddr
 * OVERVIEW:		decodes an effective address
 * PARAMETERS: 		address pointer to be decoded
 * RETURNS: 		string with effective address in assembly format
 */

char* NJMCDecoder::dis_Eaddr (ADDRESS pc)
{
	static char _buffer[21]; 

    match pc to
    | indirectA(rs1) =>	sprintf (_buffer, "[%s]", DIS_RS1);
                        strcat(constrName, "indirectA ");
    | indexA(rs1, rs2) =>	sprintf (_buffer, "%s[%s]", DIS_RS1, DIS_RS2); 
                        strcat(constrName, "indexA ");
    | absoluteA(i) =>	sprintf (_buffer, "[0x%x]", i); 
                        strcat(constrName, "absoluteA ");
    | dispA(rs1,i) =>	sprintf (_buffer, "[%s+%d]", DIS_RS1, i); 
                        strcat(constrName, "dispA ");
    endmatch
	return _buffer;
}

/*==============================================================================
 * FUNCTION:       NJMCDecoder::decodeAssemblyInstruction
 * OVERVIEW:       Decodes a machine instruction and displays its assembly
 *                 representation onto the external array _assembly[].
 * PARAMETERS:     pc - the native address of the pc
 *				   delta - the difference between the native address and 
 *					the host address of the pc
 * RETURNS: 	   number of bytes taken up by the decoded instruction 
 *					(i.e. number of bytes processed)
 *============================================================================*/

int NJMCDecoder::decodeAssemblyInstruction (ADDRESS pc, int delta)
{
	ADDRESS hostPC = pc + delta; 
	ADDRESS nextPC;

    sprintf(_assembly, "%X: %08X  ", pc, getDword(hostPC) );
    char* str = _assembly + strlen(_assembly);

	match [nextPC] hostPC to
 
	| NOP =>
		sprintf (str, "NOP");

	| sethi(imm22, rd) => 
		sprintf (str, "%s 0x%X,%s", "sethi", (imm22), DIS_RD);

	| restore_() =>
		sprintf (str, "restore");

    | ret() =>
        sprintf (str, "ret");

    | retl() =>
        sprintf (str, "retl");

	| save_ () [name] =>
		sprintf (str, "%s %s,%s", name, "%g0", "%g0", "%g0");

	| load_greg(addr, rd) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_ADDR, DIS_RD);

	| LDF (addr, fds) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_ADDR, DIS_FDS);

	| LDDF (addr, fdd) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_ADDR, DIS_FDD);

	| load_creg(addr, cd) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_ADDR, DIS_CD);
		
	| load_asi (addr, asi, rd) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_RD, DIS_ADDR);

	| sto_greg(rd, addr) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_RD, DIS_ADDR);

	| STF (fds, addr) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_FDS, DIS_ADDR);

	| STDF (fdd, addr) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_FDD, DIS_ADDR);

	| sto_creg(cd, addr) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_CD, DIS_ADDR);

	| sto_asi (rd, addr, asi) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_RD, DIS_ADDR);

	| LDFSR(addr) [name] => 
		sprintf (str, "%s %s", name, DIS_ADDR);

	| LDCSR(addr) [name] => 
		sprintf (str, "%s %s", name, DIS_ADDR);

	| STFSR(addr) [name] => 
		sprintf (str, "%s %s", name, DIS_ADDR);

	| STCSR(addr) [name] => 
		sprintf (str, "%s %s", name, DIS_ADDR);

	| STDFQ(addr) [name] => 
		sprintf (str, "%s %s", name, DIS_ADDR);

	| STDCQ(addr) [name] => 
		sprintf (str, "%s %s", name, DIS_ADDR);

	| RDY(rd) [name] => 
		sprintf (str, "%s %s", name, DIS_RD);

	| RDPSR(rd) [name] => 
		sprintf (str, "%s %s", name, DIS_RD);

	| RDWIM(rd) [name] => 
		sprintf (str, "%s %s", name, DIS_RD);

	| RDTBR(rd) [name]	=> 
		sprintf (str, "%s %s", name, DIS_RD);

	| WRY(rs1,roi) [name]	=> 
		sprintf (str, "%s %s,%s", name, DIS_RS1, DIS_ROI);

	| WRPSR(rs1, roi) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_RS1, DIS_ROI);

	| WRWIM(rs1, roi) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_RS1, DIS_ROI);

	| WRTBR(rs1, roi) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_RS1, DIS_ROI);

	| alu (rs1, roi, rd) [name] => 
		sprintf (str, "%s %s,%s,%s", name, DIS_RS1, DIS_ROI, DIS_RD);

	| branch^",a" (tgt) [name] => 
		sprintf (str, "%s %X", name, tgt-delta);

	| branch (tgt) [name] => 
		sprintf (str, "%s %X", name, tgt-delta);

	| call__ (tgt) => {
        // Get the actual destination
        ADDRESS dest = tgt - delta;
        // Get a symbol for it, if possible
        const char* dsym = pBF->SymbolByAddress(dest);
        char hexsym[128];
        if (dsym == 0)
            sprintf(hexsym, "0x%x", dest);
     	sprintf (str, "%s %s", "call", (dsym ? dsym : hexsym));
    }

	| float2s (fs2s, fds) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_FS2S, DIS_FDS);

	| float3s (fs1s, fs2s, fds) [name] => 
		sprintf (str, "%s %s,%s,%s", name, DIS_FS1S, DIS_FS2S, DIS_FDS);
 
	| float3d (fs1d, fs2d, fdd) [name] => 
		sprintf (str, "%s %s,%s,%s", name, DIS_FS1D, DIS_FS2D, DIS_FDD);
 
	| float3q (fs1q, fs2q, fdq) [name] => 
		sprintf (str, "%s %s,%s,%s", name, DIS_FS1Q, DIS_FS2Q, DIS_FDQ);
 
	| fcompares (fs1s, fs2s) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_FS1S, DIS_FS2S);

	| fcompared (fs1d, fs2d) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_FS1D, DIS_FS2D);

	| fcompareq (fs1q, fs2q) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_FS1Q, DIS_FS2Q);

    | FTOs (fs2s, fds) [name] =>
        sprintf (str, "%s %s,%s", name, DIS_FS2S, DIS_FDS);

    | FiTOd (fs2s, fdd) [name] =>
        sprintf (str, "%s %s,%s", name, DIS_FS2S, DIS_FDD);
    | FdTOi (fs2d, fds) [name] =>
        sprintf (str, "%s %s,%s", name, DIS_FS2D, DIS_FDS);

    | FiTOq (fs2s, fdq) [name] =>
        sprintf (str, "%s %s,%s", name, DIS_FS2S, DIS_FDQ);
    | FqTOi (fs2q, fds) [name] =>
        sprintf (str, "%s %s,%s", name, DIS_FS2Q, DIS_FDS);

    | FsTOd (fs2s, fdd) [name] =>
        sprintf (str, "%s %s,%s", name, DIS_FS2S, DIS_FDD);
    | FdTOs (fs2d, fds) [name] =>
        sprintf (str, "%s %s,%s", name, DIS_FS2D, DIS_FDS);

    | FsTOq (fs2s, fdq) [name] =>
        sprintf (str, "%s %s,%s", name, DIS_FS2S, DIS_FDQ);
    | FqTOs (fs2q, fds) [name] =>
        sprintf (str, "%s %s,%s", name, DIS_FS2Q, DIS_FDS);

    | FdTOq (fs2d, fdq) [name] =>
        sprintf (str, "%s %s,%s", name, DIS_FS2D, DIS_FDQ);
    | FqTOd (fs2q, fdd) [name] =>
        sprintf (str, "%s %s,%s", name, DIS_FS2Q, DIS_FDD);

	| JMPL (addr, rd) [name] => 
		sprintf (str, "%s %s,%s", name, DIS_ADDR, DIS_RD);

	| RETT (addr) [name] => 
		sprintf (str, "%s", name);

	| trap (addr) [name] => 
		sprintf (str, "%s %s", name, DIS_ADDR);

	| UNIMP (n) [name] => 
		sprintf (str, "%s %d", name, n);

	| inst = n => 
        // What does this mean?
		NULL;

    else
		NULL;

	endmatch

	return (nextPC - hostPC);
}

