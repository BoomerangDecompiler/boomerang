#define sign_extend(N,SIZE) (((int)((N) << (sizeof(unsigned)*8-(SIZE)))) >> (sizeof(unsigned)*8-(SIZE)))
#include <assert.h>

#line 1 "frontend/machine/ppc/decoder.m"
/*
 * Copyright (C) 2004, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   decoder.m
 * OVERVIEW:   Implementation of the PPC specific parts of the PPCDecoder class.
 *============================================================================*/

/* $Revision$
 *
 * 23/Nov/04 - Jay Sweeney and Alajandro Dubrovsky: Created
 **/

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1100
#include "signature.h"
#endif

#include "exp.h"
#include "prog.h"
#include "proc.h"
#include "decoder.h"
#include "ppcdecoder.h"
#include "rtl.h"
#include "BinaryFile.h"		// For SymbolByAddress()
#include "boomerang.h"
#include <iostream>

Exp*	crBit(int bitNum);	// Get an expression for a CR bit access

#define DIS_UIMM    (new Const(uimm))
#define DIS_SIMM    (new Const(simm))
#define DIS_RS		(dis_Reg(rs))
#define DIS_RD		(dis_Reg(rd))
//#define DIS_CRFD	(dis_Reg(64/* condition registers start*/ + crfd))
#define DIS_CRFD	(new Const(crfd))
#define DIS_RDR		(dis_Reg(rd))
#define DIS_RA		(dis_Reg(ra))
#define DIS_RAZ     (dis_RAmbz(ra))		// As above, but May Be constant Zero
#define DIS_RB		(dis_Reg(rb))
#define DIS_D		(new Const(d))
#define DIS_NZRA	(dis_Reg(ra))
#define DIS_NZRB	(dis_Reg(rb))
#define DIS_ADDR	(new Const(addr))
#define DIS_RELADDR (new Const(reladdr - delta))
#define DIS_CRBD	(crBit(crbD))
#define DIS_CRBA	(crBit(crbA))
#define DIS_CRBB	(crBit(crbB))
#define DIS_INDEX   (new Binary(opPlus, dis_RAmbz(ra), new Const(d)))
#define DIS_DISP    (new Binary(opPlus, DIS_RA, DIS_NZRB))
#define DIS_BICR	(new Const(BIcr))
#define DIS_S		(new Const(rs))

#define PPC_COND_JUMP(name, size, relocd, cond, BIcr) \
	result.rtl = new RTL(pc, stmts); \
	BranchStatement* jump = new BranchStatement; \
	result.rtl->appendStmt(jump); \
	result.numBytes = size; \
	jump->setDest(relocd-delta); \
	jump->setCondType(cond); \
	SHOW_ASM(name<<" "<<BIcr<<", "<<std::hex<<relocd-delta)

/*==============================================================================
 * FUNCTION:	   unused
 * OVERVIEW:	   A dummy function to suppress "unused local variable" messages
 * PARAMETERS:	   x: integer variable to be "used"
 * RETURNS:		   Nothing
 *============================================================================*/
void PPCDecoder::unused(int x)
{}
void unused(char* x) {}


/*==============================================================================
 * FUNCTION:	   PPCDecoder::decodeInstruction
 * OVERVIEW:	   Attempt to decode the high level instruction at a given
 *				   address and return the corresponding HL type (e.g. CallStatement,
 *				   GotoStatement etc). If no high level instruction exists at the
 *				   given address, then simply return the RTL for the low level
 *				   instruction at this address. There is an option to also
 *				   include the low level statements for a HL instruction.
 * PARAMETERS:	   pc - the native address of the pc
 *				   delta - the difference between the above address and the
 *					 host address of the pc (i.e. the address that the pc is at
 *					 in the loaded object file)
 *				   proc - the enclosing procedure. This can be NULL for
 *					 those of us who are using this method in an interpreter
 * RETURNS:		   a DecodeResult structure containing all the information
 *					 gathered during decoding
 *============================================================================*/
DecodeResult& PPCDecoder::decodeInstruction (ADDRESS pc, int delta) { 
	static DecodeResult result;
	ADDRESS hostPC = pc+delta;

	// Clear the result structure;
	result.reset();

	// The actual list of instantiated statements
	std::list<Statement*>* stmts = NULL;

	ADDRESS nextPC = NO_ADDRESS;



#line 112 "frontend/machine/ppc/decoder.m"
{ 
  dword MATCH_p = 
    
#line 112 "frontend/machine/ppc/decoder.m"
    hostPC
    ;
  char *MATCH_name;
  char *MATCH_name_OPCD_0[] = {
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, "mulli", "subfic", (char *)0, (char *)0, (char *)0, "addic", 
    "addicq", "addi", "addis", (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, "ori", "oris", "xori", 
    "xoris", "andiq", "andisq", (char *)0, (char *)0, "lwz", "lwzu", "lbz", 
    "lbzu", "stw", "stwu", "stb", "stbu", "lhz", "lhzu", "lha", "lhau", 
    "sth", "sthu", "lmw", "stmw", 
  };
  char *MATCH_name_BIcc_3[] = {"bge", "ble", "bne", "bns", };
  char *MATCH_name_BIcc_4[] = {"bgel", "blel", "bnel", "bnsl", };
  char *MATCH_name_BIcc_5[] = {"blt", "bgt", "beq", "bso", };
  char *MATCH_name_BIcc_6[] = {"bltl", "bgtl", "beql", "bsol", };
  char *MATCH_name_LK_7[] = {"crnor", "buul", };
  char *MATCH_name_LK_8[] = {"crandc", "bl", };
  char *MATCH_name_Xo1_18[] = {
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, "lwarx", "ldx", (char *)0, "lwzx", "slw", 
    (char *)0, (char *)0, "sld", "and", (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, "ldux", (char *)0, "lwzux", (char *)0, 
    (char *)0, (char *)0, (char *)0, "andc", (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, "ldarx", (char *)0, (char *)0, "lbzx", (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    "lbzux", (char *)0, (char *)0, (char *)0, (char *)0, "nor", (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, "lhzx", (char *)0, (char *)0, (char *)0, 
    (char *)0, "eqv", (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, "eciwx", "lhzux", (char *)0, (char *)0, (char *)0, 
    (char *)0, "xor", (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, "mfspr", 
    (char *)0, "lwax", (char *)0, "lhax", (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, "lwaux", (char *)0, "lhaux", (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, "orc", (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, "or", (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, "mtspr", (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, "nand", 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, "lswx", "lwbrx", (char *)0, "srw", (char *)0, 
    (char *)0, "srd", (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, "lhbrx", 
    (char *)0, "sraw", (char *)0, "srad", 
  };
  char *MATCH_name_Xo9_20[] = {
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, "subfc", (char *)0, "addc", (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, "subf", (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, "subfe", (char *)0, "adde", (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, "mulld", (char *)0, "mullw", (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, "add", (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, "divdu", (char *)0, "divwu", (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, "divd", (char *)0, "divw", 
  };
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    
      switch((MATCH_w_32_0 >> 26 & 0x3f) /* OPCD at 0 */) {
        case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 9: 
        case 17: case 20: case 21: case 22: case 23: case 30: case 48: 
        case 49: case 50: case 51: case 52: case 53: case 54: case 55: 
        case 56: case 57: case 58: case 59: case 60: case 61: case 62: 
        case 63: 
          goto MATCH_label_a0; break;
        case 7: case 8: case 12: case 13: case 14: case 15: 
          MATCH_name = 
            MATCH_name_OPCD_0[(MATCH_w_32_0 >> 26 & 0x3f) /* OPCD at 0 */]; 
          { 
            char *name = MATCH_name;
            unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
            unsigned rd = (MATCH_w_32_0 >> 21 & 0x1f) /* D at 0 */;
            int /* [~32768..32767] */ simm = 
              sign_extend((MATCH_w_32_0 & 0xffff) /* SIMM at 0 */, 16);
            nextPC = 4 + MATCH_p; 
            
#line 127 "frontend/machine/ppc/decoder.m"
            

            		if (strcmp(name, "addi") == 0 || strcmp(name, "addis") == 0) {

            			// Note the DIS_RAZ, since rA could be constant zero

            			stmts = instantiate(pc, name, DIS_RD, DIS_RAZ, DIS_SIMM);

            		} else

            			stmts = instantiate(pc, name, DIS_RD, DIS_RA , DIS_SIMM);

            
            
            
          }
          
          break;
        case 10: 
          if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
            MATCH_name = "cmpli"; 
            { 
              char *name = MATCH_name;
              unsigned crfd = (MATCH_w_32_0 >> 23 & 0x7) /* crfD at 0 */;
              unsigned l = (MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */;
              unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
              unsigned uimm = (MATCH_w_32_0 & 0xffff) /* UIMM at 0 */;
              nextPC = 4 + MATCH_p; 
              
#line 203 "frontend/machine/ppc/decoder.m"
              

              		stmts = instantiate(pc, name, DIS_CRFD, DIS_NZRA, DIS_UIMM);

              		unused(l);

              

              	// Conditional branches

              	// bcc_ is blt | ble | beq | bge | bgt | bnl | bne | bng | bso | bns | bun | bnu

              
              
              
            }
            
          } /*opt-block*/
          else 
            goto MATCH_label_a0;  /*opt-block+*/
          
          break;
        case 11: 
          if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
            MATCH_name = "cmpi"; 
            { 
              char *name = MATCH_name;
              unsigned crfd = (MATCH_w_32_0 >> 23 & 0x7) /* crfD at 0 */;
              unsigned l = (MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */;
              unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
              int /* [~32768..32767] */ simm = 
                sign_extend((MATCH_w_32_0 & 0xffff) /* SIMM at 0 */, 16);
              nextPC = 4 + MATCH_p; 
              
#line 200 "frontend/machine/ppc/decoder.m"
              

              		stmts = instantiate(pc, name, DIS_CRFD, DIS_NZRA, DIS_SIMM);

              		unused(l);

              
              
              
            }
            
          } /*opt-block*/
          else 
            goto MATCH_label_a0;  /*opt-block+*/
          
          break;
        case 16: 
          
            switch((MATCH_w_32_0 >> 21 & 0x1f) /* BO at 0 */) {
              case 0: case 1: case 2: case 3: case 5: case 6: case 7: case 8: 
              case 9: case 10: case 11: case 13: case 14: case 15: case 16: 
              case 17: case 18: case 19: case 21: case 22: case 23: case 24: 
              case 25: case 26: case 27: case 28: case 29: case 30: case 31: 
                goto MATCH_label_a0; break;
              case 4: 
                if ((MATCH_w_32_0 >> 1 & 0x1) /* AA at 0 */ == 1) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else 
                  if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 1) 
                    
                      switch((MATCH_w_32_0 >> 16 & 0x3) /* BIcc at 0 */) {
                        case 0: 
                          MATCH_name = 
                            MATCH_name_BIcc_4[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 241 "frontend/machine/ppc/decoder.m"
                            

                            		std::cerr << "HACK bgel\n";

                            
                            
                            
                          }
                          
                          break;
                        case 1: 
                          MATCH_name = 
                            MATCH_name_BIcc_4[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 237 "frontend/machine/ppc/decoder.m"
                            

                            		std::cerr << "HACK blel\n";

                            
                            
                            
                          }
                          
                          break;
                        case 2: 
                          MATCH_name = 
                            MATCH_name_BIcc_4[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 247 "frontend/machine/ppc/decoder.m"
                            

                            		std::cerr << "HACK bnel\n";

                            
                            
                            
                          }
                          
                          break;
                        case 3: 
                          MATCH_name = 
                            MATCH_name_BIcc_4[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 253 "frontend/machine/ppc/decoder.m"
                            

                            		std::cerr << "HACK bnsl\n";

                            
                            
                            
                          }
                          
                          break;
                        default: assert(0);
                      } /* (MATCH_w_32_0 >> 16 & 0x3) -- BIcc at 0 --*/  
                  else 
                    
                      switch((MATCH_w_32_0 >> 16 & 0x3) /* BIcc at 0 */) {
                        case 0: 
                          MATCH_name = 
                            MATCH_name_BIcc_3[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 214 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSGE, BIcr);

                            
                            
                            
                          }
                          
                          break;
                        case 1: 
                          MATCH_name = 
                            MATCH_name_BIcc_3[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 210 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSLE, BIcr);

                            
                            
                            
                          }
                          
                          break;
                        case 2: 
                          MATCH_name = 
                            MATCH_name_BIcc_3[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 220 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JNE, BIcr);

                            
                            
                            
                          }
                          
                          break;
                        case 3: 
                          MATCH_name = 
                            MATCH_name_BIcc_3[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 226 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, (BRANCH_TYPE)0, BIcr);

                            
                            
                            
                          }
                          
                          break;
                        default: assert(0);
                      } /* (MATCH_w_32_0 >> 16 & 0x3) -- BIcc at 0 --*/   
                break;
              case 12: 
                if ((MATCH_w_32_0 >> 1 & 0x1) /* AA at 0 */ == 1) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else 
                  if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 1) 
                    
                      switch((MATCH_w_32_0 >> 16 & 0x3) /* BIcc at 0 */) {
                        case 0: 
                          MATCH_name = 
                            MATCH_name_BIcc_6[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 235 "frontend/machine/ppc/decoder.m"
                            

                            		std::cerr << "HACK bltl\n";

                            
                            
                            
                          }
                          
                          break;
                        case 1: 
                          MATCH_name = 
                            MATCH_name_BIcc_6[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 243 "frontend/machine/ppc/decoder.m"
                            

                            		std::cerr << "HACK bgtl\n";

                            
                            
                            
                          }
                          
                          break;
                        case 2: 
                          MATCH_name = 
                            MATCH_name_BIcc_6[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 239 "frontend/machine/ppc/decoder.m"
                            

                            		std::cerr << "HACK beql\n";

                            
                            
                            
                          }
                          
                          break;
                        case 3: 
                          MATCH_name = 
                            MATCH_name_BIcc_6[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 251 "frontend/machine/ppc/decoder.m"
                            

                            		std::cerr << "HACK bsol\n";

                            
                            
                            
                          }
                          
                          break;
                        default: assert(0);
                      } /* (MATCH_w_32_0 >> 16 & 0x3) -- BIcc at 0 --*/  
                  else 
                    
                      switch((MATCH_w_32_0 >> 16 & 0x3) /* BIcc at 0 */) {
                        case 0: 
                          MATCH_name = 
                            MATCH_name_BIcc_5[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 208 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSL, BIcr);

                            
                            
                            
                          }
                          
                          break;
                        case 1: 
                          MATCH_name = 
                            MATCH_name_BIcc_5[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 216 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSG, BIcr);

                            
                            
                            
                          }
                          
                          break;
                        case 2: 
                          MATCH_name = 
                            MATCH_name_BIcc_5[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 212 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JE, BIcr);

                            
                            
                            
                          }
                          
                          break;
                        case 3: 
                          MATCH_name = 
                            MATCH_name_BIcc_5[(MATCH_w_32_0 >> 16 & 0x3) 
                                /* BIcc at 0 */]; 
                          { 
                            char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 224 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, (BRANCH_TYPE)0, BIcr);	// MVE: Don't know these last 4 yet

                            
                            
                            
                          }
                          
                          break;
                        default: assert(0);
                      } /* (MATCH_w_32_0 >> 16 & 0x3) -- BIcc at 0 --*/   
                break;
              case 20: 
                if ((MATCH_w_32_0 >> 1 & 0x1) /* AA at 0 */ == 0 && 
                  (0 <= (MATCH_w_32_0 >> 16 & 0x3) /* BIcc at 0 */ && 
                  (MATCH_w_32_0 >> 16 & 0x3) /* BIcc at 0 */ < 3) || 
                  (MATCH_w_32_0 >> 1 & 0x1) /* AA at 0 */ == 0 && 
                  (MATCH_w_32_0 >> 16 & 0x3) /* BIcc at 0 */ == 3 && 
                  (MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0 || 
                  (MATCH_w_32_0 >> 1 & 0x1) /* AA at 0 */ == 1) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else { 
                  MATCH_name = 
                    MATCH_name_LK_7[(MATCH_w_32_0 & 0x1) /* LK at 0 */]; 
                  { 
                    char *name = MATCH_name;
                    unsigned BIcr = 
                      (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                    unsigned reladdr = 
                      4 * (MATCH_w_32_0 >> 2 & 0x3fff) /* BD at 0 */ + 
                      addressToPC(MATCH_p);
                    nextPC = 4 + MATCH_p; 
                    
#line 174 "frontend/machine/ppc/decoder.m"
                    		// Unconditional "conditional" branch with link, test/OSX/hello has this

                    		if (reladdr - delta - pc == 4) {	// Branch to next instr?

                    			// Effectively %LR = %pc+4, but give the actual value for %pc

                    			Assign* as = new Assign(

                    				new IntegerType,

                    				new Unary(opMachFtr, new Const("%LR")),

                    				new Const(pc+4));

                    			stmts = new std::list<Statement*>;

                    			stmts->push_back(as);

                    			SHOW_ASM(name<<" "<<BIcr<<", .+4"<<" %LR = %pc+4")

                    		} else {

                    			Exp* dest = DIS_RELADDR;

                    			stmts = instantiate(pc, name, dest);

                    			CallStatement* newCall = new CallStatement;

                    			// Record the fact that this is not a computed call

                    			newCall->setIsComputed(false);

                    			// Set the destination expression

                    			newCall->setDest(dest);

                    			result.rtl = new RTL(pc, stmts);

                    			result.rtl->appendStmt(newCall);

                    		}

                    		unused(BIcr);

                    

                    
                    
                    
                  }
                  
                } /*opt-block*/
                
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 21 & 0x1f) -- BO at 0 --*/ 
          break;
        case 18: 
          if ((MATCH_w_32_0 >> 1 & 0x1) /* AA at 0 */ == 0 && 
            (MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0 || 
            (MATCH_w_32_0 >> 1 & 0x1) /* AA at 0 */ == 1) 
            goto MATCH_label_a0;  /*opt-block+*/
          else { 
            MATCH_name = MATCH_name_LK_8[(MATCH_w_32_0 & 0x1) /* LK at 0 */]; 
            { 
              char *name = MATCH_name;
              unsigned reladdr = 
                4 * sign_extend((MATCH_w_32_0 >> 2 & 0xffffff) /* LI at 0 */, 
                            24) + addressToPC(MATCH_p);
              nextPC = 4 + MATCH_p; 
              
#line 163 "frontend/machine/ppc/decoder.m"
              

              		Exp* dest = DIS_RELADDR;

              		stmts = instantiate(pc, name, dest);

              		CallStatement* newCall = new CallStatement;

              		// Record the fact that this is not a computed call

              		newCall->setIsComputed(false);

              		// Set the destination expression

              		newCall->setDest(dest);

              		result.rtl = new RTL(pc, stmts);

              		result.rtl->appendStmt(newCall);

              

              
              
              
            }
            
          } /*opt-block*/
          
          break;
        case 19: 
          if (34 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
            (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 129 || 
            130 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
            (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 193 || 
            290 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
            (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 417 || 
            450 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
            (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 528 || 
            529 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
            (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
            goto MATCH_label_a0;  /*opt-block+*/
          else 
            switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
              case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: 
              case 8: case 9: case 10: case 11: case 12: case 13: case 14: 
              case 15: case 17: case 18: case 19: case 20: case 21: case 22: 
              case 23: case 24: case 25: case 26: case 27: case 28: case 29: 
              case 30: case 31: case 32: case 194: case 195: case 196: 
              case 197: case 198: case 199: case 200: case 201: case 202: 
              case 203: case 204: case 205: case 206: case 207: case 208: 
              case 209: case 210: case 211: case 212: case 213: case 214: 
              case 215: case 216: case 217: case 218: case 219: case 220: 
              case 221: case 222: case 223: case 224: case 226: case 227: 
              case 228: case 229: case 230: case 231: case 232: case 233: 
              case 234: case 235: case 236: case 237: case 238: case 239: 
              case 240: case 241: case 242: case 243: case 244: case 245: 
              case 246: case 247: case 248: case 249: case 250: case 251: 
              case 252: case 253: case 254: case 255: case 256: case 258: 
              case 259: case 260: case 261: case 262: case 263: case 264: 
              case 265: case 266: case 267: case 268: case 269: case 270: 
              case 271: case 272: case 273: case 274: case 275: case 276: 
              case 277: case 278: case 279: case 280: case 281: case 282: 
              case 283: case 284: case 285: case 286: case 287: case 288: 
              case 418: case 419: case 420: case 421: case 422: case 423: 
              case 424: case 425: case 426: case 427: case 428: case 429: 
              case 430: case 431: case 432: case 433: case 434: case 435: 
              case 436: case 437: case 438: case 439: case 440: case 441: 
              case 442: case 443: case 444: case 445: case 446: case 447: 
              case 448: 
                goto MATCH_label_a0; break;
              case 16: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0 && 
                  (MATCH_w_32_0 >> 11 & 0x1f) /* crbB at 0 */ == 0) { 
                  MATCH_name = "bclr"; 
                  goto MATCH_label_a1; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 33: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_LK_7[(MATCH_w_32_0 & 0x1) /* LK at 0 */]; 
                  goto MATCH_label_a2; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 129: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_LK_8[(MATCH_w_32_0 & 0x1) /* LK at 0 */]; 
                  goto MATCH_label_a2; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 193: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = "crxor"; 
                  goto MATCH_label_a2; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 225: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = "crnand"; 
                  goto MATCH_label_a2; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 257: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = "crand"; 
                  goto MATCH_label_a2; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 289: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = "creqv"; 
                  goto MATCH_label_a2; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 417: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = "crorc"; 
                  goto MATCH_label_a2; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 449: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = "cror"; 
                  goto MATCH_label_a2; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 528: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0 && 
                  (MATCH_w_32_0 >> 11 & 0x1f) /* crbB at 0 */ == 0) { 
                  MATCH_name = "bcctr"; 
                  goto MATCH_label_a1; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/ 
          break;
        case 24: case 25: case 26: case 27: case 28: case 29: 
          MATCH_name = 
            MATCH_name_OPCD_0[(MATCH_w_32_0 >> 26 & 0x3f) /* OPCD at 0 */]; 
          { 
            char *name = MATCH_name;
            unsigned ra = (MATCH_w_32_0 >> 21 & 0x1f) /* S at 0 */;
            unsigned rd = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
            unsigned uimm = (MATCH_w_32_0 & 0xffff) /* UIMM at 0 */;
            nextPC = 4 + MATCH_p; 
            
#line 124 "frontend/machine/ppc/decoder.m"
            

            		stmts = instantiate(pc, name, DIS_RD, DIS_RA, DIS_UIMM);

            
            
            
          }
          
          break;
        case 31: 
          if ((MATCH_w_32_0 & 0x1) /* Rc at 0 */ == 1) 
            goto MATCH_label_a0;  /*opt-block+*/
          else 
            if ((MATCH_w_32_0 >> 10 & 0x1) /* OE at 0 */ == 1) 
              if (125 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 279 || 
                376 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 412 || 
                477 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 533 || 
                540 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 790 || 
                795 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                goto MATCH_label_a0;  /*opt-block+*/
              else 
                switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                  case 0: 
                    if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
                      MATCH_name = "cmp"; 
                      goto MATCH_label_a3; 
                      
                    } /*opt-block*/
                    else 
                      goto MATCH_label_a0;  /*opt-block+*/
                    
                    break;
                  case 1: case 2: case 3: case 4: case 5: case 6: case 7: 
                  case 8: case 9: case 10: case 11: case 12: case 13: 
                  case 14: case 15: case 16: case 17: case 18: case 19: 
                  case 22: case 25: case 26: case 29: case 30: case 31: 
                  case 33: case 34: case 35: case 36: case 37: case 38: 
                  case 39: case 40: case 41: case 42: case 43: case 44: 
                  case 45: case 46: case 47: case 48: case 49: case 50: 
                  case 51: case 52: case 54: case 56: case 57: case 58: 
                  case 59: case 61: case 62: case 63: case 64: case 65: 
                  case 66: case 67: case 68: case 69: case 70: case 71: 
                  case 72: case 73: case 74: case 75: case 76: case 77: 
                  case 78: case 79: case 80: case 81: case 82: case 83: 
                  case 85: case 86: case 88: case 89: case 90: case 91: 
                  case 92: case 93: case 94: case 95: case 96: case 97: 
                  case 98: case 99: case 100: case 101: case 102: case 103: 
                  case 104: case 105: case 106: case 107: case 108: case 109: 
                  case 110: case 111: case 112: case 113: case 114: case 115: 
                  case 116: case 117: case 118: case 120: case 121: case 122: 
                  case 123: case 280: case 281: case 282: case 283: case 285: 
                  case 286: case 287: case 288: case 289: case 290: case 291: 
                  case 292: case 293: case 294: case 295: case 296: case 297: 
                  case 298: case 299: case 300: case 301: case 302: case 303: 
                  case 304: case 305: case 306: case 307: case 308: case 309: 
                  case 312: case 313: case 314: case 315: case 317: case 318: 
                  case 319: case 320: case 321: case 322: case 323: case 324: 
                  case 325: case 326: case 327: case 328: case 329: case 330: 
                  case 331: case 332: case 333: case 334: case 335: case 336: 
                  case 337: case 338: case 340: case 342: case 344: case 345: 
                  case 346: case 347: case 348: case 349: case 350: case 351: 
                  case 352: case 353: case 354: case 355: case 356: case 357: 
                  case 358: case 359: case 360: case 361: case 362: case 363: 
                  case 364: case 365: case 366: case 367: case 368: case 369: 
                  case 370: case 371: case 372: case 374: case 413: case 414: 
                  case 415: case 416: case 417: case 418: case 419: case 420: 
                  case 421: case 422: case 423: case 424: case 425: case 426: 
                  case 427: case 428: case 429: case 430: case 431: case 432: 
                  case 433: case 434: case 435: case 436: case 437: case 438: 
                  case 439: case 440: case 441: case 442: case 443: case 445: 
                  case 446: case 447: case 448: case 449: case 450: case 451: 
                  case 452: case 453: case 454: case 455: case 456: case 457: 
                  case 458: case 459: case 460: case 461: case 462: case 463: 
                  case 464: case 465: case 466: case 468: case 469: case 470: 
                  case 471: case 472: case 473: case 474: case 475: case 535: 
                  case 537: case 538: case 791: case 793: 
                    goto MATCH_label_a0; break;
                  case 20: case 21: case 23: case 53: case 55: case 84: 
                  case 87: case 119: case 279: case 310: case 311: case 341: 
                  case 343: case 373: case 375: case 533: case 534: case 790: 
                    MATCH_name = 
                      MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                          /* Xo1 at 0 */]; 
                    goto MATCH_label_a4; 
                    
                    break;
                  case 24: case 27: case 28: case 60: case 124: case 284: 
                  case 316: case 412: case 444: case 476: case 536: case 539: 
                  case 792: case 794: 
                    MATCH_name = 
                      MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                          /* Xo1 at 0 */]; 
                    goto MATCH_label_a5; 
                    
                    break;
                  case 32: 
                    if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
                      MATCH_name = "cmpl"; 
                      goto MATCH_label_a3; 
                      
                    } /*opt-block*/
                    else 
                      goto MATCH_label_a0;  /*opt-block+*/
                    
                    break;
                  case 339: 
                    MATCH_name = 
                      MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                          /* Xo1 at 0 */]; 
                    goto MATCH_label_a6; 
                    
                    break;
                  case 467: 
                    MATCH_name = 
                      MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                          /* Xo1 at 0 */]; 
                    goto MATCH_label_a7; 
                    
                    break;
                  default: assert(0);
                } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/  
            else 
              if (41 <= (MATCH_w_32_0 >> 1 & 0x1ff) /* Xo9 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x1ff) /* Xo9 at 0 */ < 104 || 
                139 <= (MATCH_w_32_0 >> 1 & 0x1ff) /* Xo9 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x1ff) /* Xo9 at 0 */ < 200 || 
                267 <= (MATCH_w_32_0 >> 1 & 0x1ff) /* Xo9 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x1ff) /* Xo9 at 0 */ < 457) 
                if (125 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 279 || 
                  376 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 412 || 
                  477 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 533 || 
                  540 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 790 || 
                  795 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else 
                  switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                    case 0: 
                      if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
                        MATCH_name = "cmp"; 
                        goto MATCH_label_a3; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    case 1: case 2: case 3: case 4: case 5: case 6: case 7: 
                    case 8: case 9: case 10: case 11: case 12: case 13: 
                    case 14: case 15: case 16: case 17: case 18: case 19: 
                    case 22: case 25: case 26: case 29: case 30: case 31: 
                    case 33: case 34: case 35: case 36: case 37: case 38: 
                    case 39: case 40: case 41: case 42: case 43: case 44: 
                    case 45: case 46: case 47: case 48: case 49: case 50: 
                    case 51: case 52: case 54: case 56: case 57: case 58: 
                    case 59: case 61: case 62: case 63: case 64: case 65: 
                    case 66: case 67: case 68: case 69: case 70: case 71: 
                    case 72: case 73: case 74: case 75: case 76: case 77: 
                    case 78: case 79: case 80: case 81: case 82: case 83: 
                    case 85: case 86: case 88: case 89: case 90: case 91: 
                    case 92: case 93: case 94: case 95: case 96: case 97: 
                    case 98: case 99: case 100: case 101: case 102: case 103: 
                    case 104: case 105: case 106: case 107: case 108: 
                    case 109: case 110: case 111: case 112: case 113: 
                    case 114: case 115: case 116: case 117: case 118: 
                    case 120: case 121: case 122: case 123: case 280: 
                    case 281: case 282: case 283: case 285: case 286: 
                    case 287: case 288: case 289: case 290: case 291: 
                    case 292: case 293: case 294: case 295: case 296: 
                    case 297: case 298: case 299: case 300: case 301: 
                    case 302: case 303: case 304: case 305: case 306: 
                    case 307: case 308: case 309: case 312: case 313: 
                    case 314: case 315: case 317: case 318: case 319: 
                    case 320: case 321: case 322: case 323: case 324: 
                    case 325: case 326: case 327: case 328: case 329: 
                    case 330: case 331: case 332: case 333: case 334: 
                    case 335: case 336: case 337: case 338: case 340: 
                    case 342: case 344: case 345: case 346: case 347: 
                    case 348: case 349: case 350: case 351: case 352: 
                    case 353: case 354: case 355: case 356: case 357: 
                    case 358: case 359: case 360: case 361: case 362: 
                    case 363: case 364: case 365: case 366: case 367: 
                    case 368: case 369: case 370: case 371: case 372: 
                    case 374: case 413: case 414: case 415: case 416: 
                    case 417: case 418: case 419: case 420: case 421: 
                    case 422: case 423: case 424: case 425: case 426: 
                    case 427: case 428: case 429: case 430: case 431: 
                    case 432: case 433: case 434: case 435: case 436: 
                    case 437: case 438: case 439: case 440: case 441: 
                    case 442: case 443: case 445: case 446: case 447: 
                    case 448: case 449: case 450: case 451: case 452: 
                    case 453: case 454: case 455: case 456: case 457: 
                    case 458: case 459: case 460: case 461: case 462: 
                    case 463: case 464: case 465: case 466: case 468: 
                    case 469: case 470: case 471: case 472: case 473: 
                    case 474: case 475: case 535: case 537: case 538: 
                    case 791: case 793: 
                      goto MATCH_label_a0; break;
                    case 20: case 21: case 23: case 53: case 55: case 84: 
                    case 87: case 119: case 279: case 310: case 311: 
                    case 341: case 343: case 373: case 375: case 533: 
                    case 534: case 790: 
                      MATCH_name = 
                        MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a4; 
                      
                      break;
                    case 24: case 27: case 28: case 60: case 124: case 284: 
                    case 316: case 412: case 444: case 476: case 536: 
                    case 539: case 792: case 794: 
                      MATCH_name = 
                        MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a5; 
                      
                      break;
                    case 32: 
                      if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
                        MATCH_name = "cmpl"; 
                        goto MATCH_label_a3; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    case 339: 
                      MATCH_name = 
                        MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a6; 
                      
                      break;
                    case 467: 
                      MATCH_name = 
                        MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a7; 
                      
                      break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/  
              else 
                switch((MATCH_w_32_0 >> 1 & 0x1ff) /* Xo9 at 0 */) {
                  case 0: case 1: case 2: case 3: case 4: case 5: case 6: 
                  case 7: case 9: case 11: case 12: case 13: case 14: 
                  case 15: case 16: case 17: case 18: case 19: case 20: 
                  case 21: case 22: case 23: case 24: case 25: case 26: 
                  case 27: case 28: case 29: case 30: case 31: case 32: 
                  case 33: case 34: case 35: case 36: case 37: case 38: 
                  case 39: case 105: case 106: case 107: case 108: case 109: 
                  case 110: case 111: case 112: case 113: case 114: case 115: 
                  case 116: case 117: case 118: case 119: case 120: case 121: 
                  case 122: case 123: case 124: case 125: case 126: case 127: 
                  case 128: case 129: case 130: case 131: case 132: case 133: 
                  case 134: case 135: case 137: case 201: case 203: case 204: 
                  case 205: case 206: case 207: case 208: case 209: case 210: 
                  case 211: case 212: case 213: case 214: case 215: case 216: 
                  case 217: case 218: case 219: case 220: case 221: case 222: 
                  case 223: case 224: case 225: case 226: case 227: case 228: 
                  case 229: case 230: case 231: case 236: case 237: case 238: 
                  case 239: case 240: case 241: case 242: case 243: case 244: 
                  case 245: case 246: case 247: case 248: case 249: case 250: 
                  case 251: case 252: case 253: case 254: case 255: case 256: 
                  case 257: case 258: case 259: case 260: case 261: case 262: 
                  case 263: case 264: case 265: case 458: case 460: case 461: 
                  case 462: case 463: case 464: case 465: case 466: case 467: 
                  case 468: case 469: case 470: case 471: case 472: case 473: 
                  case 474: case 475: case 476: case 477: case 478: case 479: 
                  case 480: case 481: case 482: case 483: case 484: case 485: 
                  case 486: case 487: case 488: case 490: case 492: case 493: 
                  case 494: case 495: case 496: case 497: case 498: case 499: 
                  case 500: case 501: case 502: case 503: case 504: case 505: 
                  case 506: case 507: case 508: case 509: case 510: case 511: 
                    if (125 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                      (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 279 || 
                      376 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                      (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 412 || 
                      477 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                      (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 533 || 
                      540 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                      (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 790 || 
                      795 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                      (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                      goto MATCH_label_a0;  /*opt-block+*/
                    else 
                      switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                        case 0: 
                          if ((MATCH_w_32_0 >> 22 & 0x1) 
                                  /* Lz at 0 */ == 0) { 
                            MATCH_name = "cmp"; 
                            goto MATCH_label_a3; 
                            
                          } /*opt-block*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/
                          
                          break;
                        case 1: case 2: case 3: case 4: case 5: case 6: 
                        case 7: case 8: case 9: case 10: case 11: case 12: 
                        case 13: case 14: case 15: case 16: case 17: case 18: 
                        case 19: case 22: case 25: case 26: case 29: case 30: 
                        case 31: case 33: case 34: case 35: case 36: case 37: 
                        case 38: case 39: case 40: case 41: case 42: case 43: 
                        case 44: case 45: case 46: case 47: case 48: case 49: 
                        case 50: case 51: case 52: case 54: case 56: case 57: 
                        case 58: case 59: case 61: case 62: case 63: case 64: 
                        case 65: case 66: case 67: case 68: case 69: case 70: 
                        case 71: case 72: case 73: case 74: case 75: case 76: 
                        case 77: case 78: case 79: case 80: case 81: case 82: 
                        case 83: case 85: case 86: case 88: case 89: case 90: 
                        case 91: case 92: case 93: case 94: case 95: case 96: 
                        case 97: case 98: case 99: case 100: case 101: 
                        case 102: case 103: case 104: case 105: case 106: 
                        case 107: case 108: case 109: case 110: case 111: 
                        case 112: case 113: case 114: case 115: case 116: 
                        case 117: case 118: case 120: case 121: case 122: 
                        case 123: case 280: case 281: case 282: case 283: 
                        case 285: case 286: case 287: case 288: case 289: 
                        case 290: case 291: case 292: case 293: case 294: 
                        case 295: case 296: case 297: case 298: case 299: 
                        case 300: case 301: case 302: case 303: case 304: 
                        case 305: case 306: case 307: case 308: case 309: 
                        case 312: case 313: case 314: case 315: case 317: 
                        case 318: case 319: case 320: case 321: case 322: 
                        case 323: case 324: case 325: case 326: case 327: 
                        case 328: case 329: case 330: case 331: case 332: 
                        case 333: case 334: case 335: case 336: case 337: 
                        case 338: case 340: case 342: case 344: case 345: 
                        case 346: case 347: case 348: case 349: case 350: 
                        case 351: case 352: case 353: case 354: case 355: 
                        case 356: case 357: case 358: case 359: case 360: 
                        case 361: case 362: case 363: case 364: case 365: 
                        case 366: case 367: case 368: case 369: case 370: 
                        case 371: case 372: case 374: case 413: case 414: 
                        case 415: case 416: case 417: case 418: case 419: 
                        case 420: case 421: case 422: case 423: case 424: 
                        case 425: case 426: case 427: case 428: case 429: 
                        case 430: case 431: case 432: case 433: case 434: 
                        case 435: case 436: case 437: case 438: case 439: 
                        case 440: case 441: case 442: case 443: case 445: 
                        case 446: case 447: case 448: case 449: case 450: 
                        case 451: case 452: case 453: case 454: case 455: 
                        case 456: case 457: case 458: case 459: case 460: 
                        case 461: case 462: case 463: case 464: case 465: 
                        case 466: case 468: case 469: case 470: case 471: 
                        case 472: case 473: case 474: case 475: case 535: 
                        case 537: case 538: case 791: case 793: 
                          goto MATCH_label_a0; break;
                        case 20: case 21: case 23: case 53: case 55: case 84: 
                        case 87: case 119: case 279: case 310: case 311: 
                        case 341: case 343: case 373: case 375: case 533: 
                        case 534: case 790: 
                          MATCH_name = 
                            MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                /* Xo1 at 0 */]; 
                          goto MATCH_label_a4; 
                          
                          break;
                        case 24: case 27: case 28: case 60: case 124: 
                        case 284: case 316: case 412: case 444: case 476: 
                        case 536: case 539: case 792: case 794: 
                          MATCH_name = 
                            MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                /* Xo1 at 0 */]; 
                          goto MATCH_label_a5; 
                          
                          break;
                        case 32: 
                          if ((MATCH_w_32_0 >> 22 & 0x1) 
                                  /* Lz at 0 */ == 0) { 
                            MATCH_name = "cmpl"; 
                            goto MATCH_label_a3; 
                            
                          } /*opt-block*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/
                          
                          break;
                        case 339: 
                          MATCH_name = 
                            MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                /* Xo1 at 0 */]; 
                          goto MATCH_label_a6; 
                          
                          break;
                        case 467: 
                          MATCH_name = 
                            MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                /* Xo1 at 0 */]; 
                          goto MATCH_label_a7; 
                          
                          break;
                        default: assert(0);
                      } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/ 
                    break;
                  case 8: case 10: case 40: case 136: case 138: case 233: 
                  case 235: case 266: case 457: case 459: case 489: case 491: 
                    MATCH_name = 
                      MATCH_name_Xo9_20[(MATCH_w_32_0 >> 1 & 0x1ff) 
                          /* Xo9 at 0 */]; 
                    { 
                      char *name = MATCH_name;
                      unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
                      unsigned rb = (MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */;
                      unsigned rd = (MATCH_w_32_0 >> 21 & 0x1f) /* D at 0 */;
                      nextPC = 4 + MATCH_p; 
                      
#line 113 "frontend/machine/ppc/decoder.m"
                      

                      		stmts = instantiate(pc,	 name, DIS_RD, DIS_RA, DIS_RB);

                      
                      
                      
                    }
                    
                    break;
                  case 104: 
                    if ((MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */ == 0) { 
                      MATCH_name = "neg"; 
                      goto MATCH_label_a8; 
                      
                    } /*opt-block*/
                    else 
                      if (125 <= (MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 279 || 
                        376 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 412 || 
                        477 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 533 || 
                        540 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 790 || 
                        795 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                        goto MATCH_label_a0;  /*opt-block+*/
                      else 
                        switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                          case 0: 
                            if ((MATCH_w_32_0 >> 22 & 0x1) 
                                    /* Lz at 0 */ == 0) { 
                              MATCH_name = "cmp"; 
                              goto MATCH_label_a3; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 1: case 2: case 3: case 4: case 5: case 6: 
                          case 7: case 8: case 9: case 10: case 11: case 12: 
                          case 13: case 14: case 15: case 16: case 17: 
                          case 18: case 19: case 22: case 25: case 26: 
                          case 29: case 30: case 31: case 33: case 34: 
                          case 35: case 36: case 37: case 38: case 39: 
                          case 40: case 41: case 42: case 43: case 44: 
                          case 45: case 46: case 47: case 48: case 49: 
                          case 50: case 51: case 52: case 54: case 56: 
                          case 57: case 58: case 59: case 61: case 62: 
                          case 63: case 64: case 65: case 66: case 67: 
                          case 68: case 69: case 70: case 71: case 72: 
                          case 73: case 74: case 75: case 76: case 77: 
                          case 78: case 79: case 80: case 81: case 82: 
                          case 83: case 85: case 86: case 88: case 89: 
                          case 90: case 91: case 92: case 93: case 94: 
                          case 95: case 96: case 97: case 98: case 99: 
                          case 100: case 101: case 102: case 103: case 104: 
                          case 105: case 106: case 107: case 108: case 109: 
                          case 110: case 111: case 112: case 113: case 114: 
                          case 115: case 116: case 117: case 118: case 120: 
                          case 121: case 122: case 123: case 280: case 281: 
                          case 282: case 283: case 285: case 286: case 287: 
                          case 288: case 289: case 290: case 291: case 292: 
                          case 293: case 294: case 295: case 296: case 297: 
                          case 298: case 299: case 300: case 301: case 302: 
                          case 303: case 304: case 305: case 306: case 307: 
                          case 308: case 309: case 312: case 313: case 314: 
                          case 315: case 317: case 318: case 319: case 320: 
                          case 321: case 322: case 323: case 324: case 325: 
                          case 326: case 327: case 328: case 329: case 330: 
                          case 331: case 332: case 333: case 334: case 335: 
                          case 336: case 337: case 338: case 340: case 342: 
                          case 344: case 345: case 346: case 347: case 348: 
                          case 349: case 350: case 351: case 352: case 353: 
                          case 354: case 355: case 356: case 357: case 358: 
                          case 359: case 360: case 361: case 362: case 363: 
                          case 364: case 365: case 366: case 367: case 368: 
                          case 369: case 370: case 371: case 372: case 374: 
                          case 413: case 414: case 415: case 416: case 417: 
                          case 418: case 419: case 420: case 421: case 422: 
                          case 423: case 424: case 425: case 426: case 427: 
                          case 428: case 429: case 430: case 431: case 432: 
                          case 433: case 434: case 435: case 436: case 437: 
                          case 438: case 439: case 440: case 441: case 442: 
                          case 443: case 445: case 446: case 447: case 448: 
                          case 449: case 450: case 451: case 452: case 453: 
                          case 454: case 455: case 456: case 457: case 458: 
                          case 459: case 460: case 461: case 462: case 463: 
                          case 464: case 465: case 466: case 468: case 469: 
                          case 470: case 471: case 472: case 473: case 474: 
                          case 475: case 535: case 537: case 538: case 791: 
                          case 793: 
                            goto MATCH_label_a0; break;
                          case 20: case 21: case 23: case 53: case 55: 
                          case 84: case 87: case 119: case 279: case 310: 
                          case 311: case 341: case 343: case 373: case 375: 
                          case 533: case 534: case 790: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a4; 
                            
                            break;
                          case 24: case 27: case 28: case 60: case 124: 
                          case 284: case 316: case 412: case 444: case 476: 
                          case 536: case 539: case 792: case 794: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a5; 
                            
                            break;
                          case 32: 
                            if ((MATCH_w_32_0 >> 22 & 0x1) 
                                    /* Lz at 0 */ == 0) { 
                              MATCH_name = "cmpl"; 
                              goto MATCH_label_a3; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 339: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a6; 
                            
                            break;
                          case 467: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a7; 
                            
                            break;
                          default: assert(0);
                        } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/  
                    break;
                  case 200: 
                    if ((MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */ == 0) { 
                      MATCH_name = "subfze"; 
                      goto MATCH_label_a8; 
                      
                    } /*opt-block*/
                    else 
                      if (125 <= (MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 279 || 
                        376 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 412 || 
                        477 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 533 || 
                        540 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 790 || 
                        795 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                        goto MATCH_label_a0;  /*opt-block+*/
                      else 
                        switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                          case 0: 
                            if ((MATCH_w_32_0 >> 22 & 0x1) 
                                    /* Lz at 0 */ == 0) { 
                              MATCH_name = "cmp"; 
                              goto MATCH_label_a3; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 1: case 2: case 3: case 4: case 5: case 6: 
                          case 7: case 8: case 9: case 10: case 11: case 12: 
                          case 13: case 14: case 15: case 16: case 17: 
                          case 18: case 19: case 22: case 25: case 26: 
                          case 29: case 30: case 31: case 33: case 34: 
                          case 35: case 36: case 37: case 38: case 39: 
                          case 40: case 41: case 42: case 43: case 44: 
                          case 45: case 46: case 47: case 48: case 49: 
                          case 50: case 51: case 52: case 54: case 56: 
                          case 57: case 58: case 59: case 61: case 62: 
                          case 63: case 64: case 65: case 66: case 67: 
                          case 68: case 69: case 70: case 71: case 72: 
                          case 73: case 74: case 75: case 76: case 77: 
                          case 78: case 79: case 80: case 81: case 82: 
                          case 83: case 85: case 86: case 88: case 89: 
                          case 90: case 91: case 92: case 93: case 94: 
                          case 95: case 96: case 97: case 98: case 99: 
                          case 100: case 101: case 102: case 103: case 104: 
                          case 105: case 106: case 107: case 108: case 109: 
                          case 110: case 111: case 112: case 113: case 114: 
                          case 115: case 116: case 117: case 118: case 120: 
                          case 121: case 122: case 123: case 280: case 281: 
                          case 282: case 283: case 285: case 286: case 287: 
                          case 288: case 289: case 290: case 291: case 292: 
                          case 293: case 294: case 295: case 296: case 297: 
                          case 298: case 299: case 300: case 301: case 302: 
                          case 303: case 304: case 305: case 306: case 307: 
                          case 308: case 309: case 312: case 313: case 314: 
                          case 315: case 317: case 318: case 319: case 320: 
                          case 321: case 322: case 323: case 324: case 325: 
                          case 326: case 327: case 328: case 329: case 330: 
                          case 331: case 332: case 333: case 334: case 335: 
                          case 336: case 337: case 338: case 340: case 342: 
                          case 344: case 345: case 346: case 347: case 348: 
                          case 349: case 350: case 351: case 352: case 353: 
                          case 354: case 355: case 356: case 357: case 358: 
                          case 359: case 360: case 361: case 362: case 363: 
                          case 364: case 365: case 366: case 367: case 368: 
                          case 369: case 370: case 371: case 372: case 374: 
                          case 413: case 414: case 415: case 416: case 417: 
                          case 418: case 419: case 420: case 421: case 422: 
                          case 423: case 424: case 425: case 426: case 427: 
                          case 428: case 429: case 430: case 431: case 432: 
                          case 433: case 434: case 435: case 436: case 437: 
                          case 438: case 439: case 440: case 441: case 442: 
                          case 443: case 445: case 446: case 447: case 448: 
                          case 449: case 450: case 451: case 452: case 453: 
                          case 454: case 455: case 456: case 457: case 458: 
                          case 459: case 460: case 461: case 462: case 463: 
                          case 464: case 465: case 466: case 468: case 469: 
                          case 470: case 471: case 472: case 473: case 474: 
                          case 475: case 535: case 537: case 538: case 791: 
                          case 793: 
                            goto MATCH_label_a0; break;
                          case 20: case 21: case 23: case 53: case 55: 
                          case 84: case 87: case 119: case 279: case 310: 
                          case 311: case 341: case 343: case 373: case 375: 
                          case 533: case 534: case 790: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a4; 
                            
                            break;
                          case 24: case 27: case 28: case 60: case 124: 
                          case 284: case 316: case 412: case 444: case 476: 
                          case 536: case 539: case 792: case 794: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a5; 
                            
                            break;
                          case 32: 
                            if ((MATCH_w_32_0 >> 22 & 0x1) 
                                    /* Lz at 0 */ == 0) { 
                              MATCH_name = "cmpl"; 
                              goto MATCH_label_a3; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 339: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a6; 
                            
                            break;
                          case 467: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a7; 
                            
                            break;
                          default: assert(0);
                        } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/  
                    break;
                  case 202: 
                    if ((MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */ == 0) { 
                      MATCH_name = "addze"; 
                      goto MATCH_label_a8; 
                      
                    } /*opt-block*/
                    else 
                      if (125 <= (MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 279 || 
                        376 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 412 || 
                        477 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 533 || 
                        540 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 790 || 
                        795 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                        goto MATCH_label_a0;  /*opt-block+*/
                      else 
                        switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                          case 0: 
                            if ((MATCH_w_32_0 >> 22 & 0x1) 
                                    /* Lz at 0 */ == 0) { 
                              MATCH_name = "cmp"; 
                              goto MATCH_label_a3; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 1: case 2: case 3: case 4: case 5: case 6: 
                          case 7: case 8: case 9: case 10: case 11: case 12: 
                          case 13: case 14: case 15: case 16: case 17: 
                          case 18: case 19: case 22: case 25: case 26: 
                          case 29: case 30: case 31: case 33: case 34: 
                          case 35: case 36: case 37: case 38: case 39: 
                          case 40: case 41: case 42: case 43: case 44: 
                          case 45: case 46: case 47: case 48: case 49: 
                          case 50: case 51: case 52: case 54: case 56: 
                          case 57: case 58: case 59: case 61: case 62: 
                          case 63: case 64: case 65: case 66: case 67: 
                          case 68: case 69: case 70: case 71: case 72: 
                          case 73: case 74: case 75: case 76: case 77: 
                          case 78: case 79: case 80: case 81: case 82: 
                          case 83: case 85: case 86: case 88: case 89: 
                          case 90: case 91: case 92: case 93: case 94: 
                          case 95: case 96: case 97: case 98: case 99: 
                          case 100: case 101: case 102: case 103: case 104: 
                          case 105: case 106: case 107: case 108: case 109: 
                          case 110: case 111: case 112: case 113: case 114: 
                          case 115: case 116: case 117: case 118: case 120: 
                          case 121: case 122: case 123: case 280: case 281: 
                          case 282: case 283: case 285: case 286: case 287: 
                          case 288: case 289: case 290: case 291: case 292: 
                          case 293: case 294: case 295: case 296: case 297: 
                          case 298: case 299: case 300: case 301: case 302: 
                          case 303: case 304: case 305: case 306: case 307: 
                          case 308: case 309: case 312: case 313: case 314: 
                          case 315: case 317: case 318: case 319: case 320: 
                          case 321: case 322: case 323: case 324: case 325: 
                          case 326: case 327: case 328: case 329: case 330: 
                          case 331: case 332: case 333: case 334: case 335: 
                          case 336: case 337: case 338: case 340: case 342: 
                          case 344: case 345: case 346: case 347: case 348: 
                          case 349: case 350: case 351: case 352: case 353: 
                          case 354: case 355: case 356: case 357: case 358: 
                          case 359: case 360: case 361: case 362: case 363: 
                          case 364: case 365: case 366: case 367: case 368: 
                          case 369: case 370: case 371: case 372: case 374: 
                          case 413: case 414: case 415: case 416: case 417: 
                          case 418: case 419: case 420: case 421: case 422: 
                          case 423: case 424: case 425: case 426: case 427: 
                          case 428: case 429: case 430: case 431: case 432: 
                          case 433: case 434: case 435: case 436: case 437: 
                          case 438: case 439: case 440: case 441: case 442: 
                          case 443: case 445: case 446: case 447: case 448: 
                          case 449: case 450: case 451: case 452: case 453: 
                          case 454: case 455: case 456: case 457: case 458: 
                          case 459: case 460: case 461: case 462: case 463: 
                          case 464: case 465: case 466: case 468: case 469: 
                          case 470: case 471: case 472: case 473: case 474: 
                          case 475: case 535: case 537: case 538: case 791: 
                          case 793: 
                            goto MATCH_label_a0; break;
                          case 20: case 21: case 23: case 53: case 55: 
                          case 84: case 87: case 119: case 279: case 310: 
                          case 311: case 341: case 343: case 373: case 375: 
                          case 533: case 534: case 790: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a4; 
                            
                            break;
                          case 24: case 27: case 28: case 60: case 124: 
                          case 284: case 316: case 412: case 444: case 476: 
                          case 536: case 539: case 792: case 794: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a5; 
                            
                            break;
                          case 32: 
                            if ((MATCH_w_32_0 >> 22 & 0x1) 
                                    /* Lz at 0 */ == 0) { 
                              MATCH_name = "cmpl"; 
                              goto MATCH_label_a3; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 339: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a6; 
                            
                            break;
                          case 467: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a7; 
                            
                            break;
                          default: assert(0);
                        } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/  
                    break;
                  case 232: 
                    if ((MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */ == 0) { 
                      MATCH_name = "subfme"; 
                      goto MATCH_label_a8; 
                      
                    } /*opt-block*/
                    else 
                      if (125 <= (MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 279 || 
                        376 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 412 || 
                        477 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 533 || 
                        540 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 790 || 
                        795 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                        goto MATCH_label_a0;  /*opt-block+*/
                      else 
                        switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                          case 0: 
                            if ((MATCH_w_32_0 >> 22 & 0x1) 
                                    /* Lz at 0 */ == 0) { 
                              MATCH_name = "cmp"; 
                              goto MATCH_label_a3; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 1: case 2: case 3: case 4: case 5: case 6: 
                          case 7: case 8: case 9: case 10: case 11: case 12: 
                          case 13: case 14: case 15: case 16: case 17: 
                          case 18: case 19: case 22: case 25: case 26: 
                          case 29: case 30: case 31: case 33: case 34: 
                          case 35: case 36: case 37: case 38: case 39: 
                          case 40: case 41: case 42: case 43: case 44: 
                          case 45: case 46: case 47: case 48: case 49: 
                          case 50: case 51: case 52: case 54: case 56: 
                          case 57: case 58: case 59: case 61: case 62: 
                          case 63: case 64: case 65: case 66: case 67: 
                          case 68: case 69: case 70: case 71: case 72: 
                          case 73: case 74: case 75: case 76: case 77: 
                          case 78: case 79: case 80: case 81: case 82: 
                          case 83: case 85: case 86: case 88: case 89: 
                          case 90: case 91: case 92: case 93: case 94: 
                          case 95: case 96: case 97: case 98: case 99: 
                          case 100: case 101: case 102: case 103: case 104: 
                          case 105: case 106: case 107: case 108: case 109: 
                          case 110: case 111: case 112: case 113: case 114: 
                          case 115: case 116: case 117: case 118: case 120: 
                          case 121: case 122: case 123: case 280: case 281: 
                          case 282: case 283: case 285: case 286: case 287: 
                          case 288: case 289: case 290: case 291: case 292: 
                          case 293: case 294: case 295: case 296: case 297: 
                          case 298: case 299: case 300: case 301: case 302: 
                          case 303: case 304: case 305: case 306: case 307: 
                          case 308: case 309: case 312: case 313: case 314: 
                          case 315: case 317: case 318: case 319: case 320: 
                          case 321: case 322: case 323: case 324: case 325: 
                          case 326: case 327: case 328: case 329: case 330: 
                          case 331: case 332: case 333: case 334: case 335: 
                          case 336: case 337: case 338: case 340: case 342: 
                          case 344: case 345: case 346: case 347: case 348: 
                          case 349: case 350: case 351: case 352: case 353: 
                          case 354: case 355: case 356: case 357: case 358: 
                          case 359: case 360: case 361: case 362: case 363: 
                          case 364: case 365: case 366: case 367: case 368: 
                          case 369: case 370: case 371: case 372: case 374: 
                          case 413: case 414: case 415: case 416: case 417: 
                          case 418: case 419: case 420: case 421: case 422: 
                          case 423: case 424: case 425: case 426: case 427: 
                          case 428: case 429: case 430: case 431: case 432: 
                          case 433: case 434: case 435: case 436: case 437: 
                          case 438: case 439: case 440: case 441: case 442: 
                          case 443: case 445: case 446: case 447: case 448: 
                          case 449: case 450: case 451: case 452: case 453: 
                          case 454: case 455: case 456: case 457: case 458: 
                          case 459: case 460: case 461: case 462: case 463: 
                          case 464: case 465: case 466: case 468: case 469: 
                          case 470: case 471: case 472: case 473: case 474: 
                          case 475: case 535: case 537: case 538: case 791: 
                          case 793: 
                            goto MATCH_label_a0; break;
                          case 20: case 21: case 23: case 53: case 55: 
                          case 84: case 87: case 119: case 279: case 310: 
                          case 311: case 341: case 343: case 373: case 375: 
                          case 533: case 534: case 790: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a4; 
                            
                            break;
                          case 24: case 27: case 28: case 60: case 124: 
                          case 284: case 316: case 412: case 444: case 476: 
                          case 536: case 539: case 792: case 794: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a5; 
                            
                            break;
                          case 32: 
                            if ((MATCH_w_32_0 >> 22 & 0x1) 
                                    /* Lz at 0 */ == 0) { 
                              MATCH_name = "cmpl"; 
                              goto MATCH_label_a3; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 339: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a6; 
                            
                            break;
                          case 467: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a7; 
                            
                            break;
                          default: assert(0);
                        } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/  
                    break;
                  case 234: 
                    if ((MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */ == 0) { 
                      MATCH_name = "addme"; 
                      goto MATCH_label_a8; 
                      
                    } /*opt-block*/
                    else 
                      if (125 <= (MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 279 || 
                        376 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 412 || 
                        477 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 533 || 
                        540 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 790 || 
                        795 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                        goto MATCH_label_a0;  /*opt-block+*/
                      else 
                        switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                          case 0: 
                            if ((MATCH_w_32_0 >> 22 & 0x1) 
                                    /* Lz at 0 */ == 0) { 
                              MATCH_name = "cmp"; 
                              goto MATCH_label_a3; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 1: case 2: case 3: case 4: case 5: case 6: 
                          case 7: case 8: case 9: case 10: case 11: case 12: 
                          case 13: case 14: case 15: case 16: case 17: 
                          case 18: case 19: case 22: case 25: case 26: 
                          case 29: case 30: case 31: case 33: case 34: 
                          case 35: case 36: case 37: case 38: case 39: 
                          case 40: case 41: case 42: case 43: case 44: 
                          case 45: case 46: case 47: case 48: case 49: 
                          case 50: case 51: case 52: case 54: case 56: 
                          case 57: case 58: case 59: case 61: case 62: 
                          case 63: case 64: case 65: case 66: case 67: 
                          case 68: case 69: case 70: case 71: case 72: 
                          case 73: case 74: case 75: case 76: case 77: 
                          case 78: case 79: case 80: case 81: case 82: 
                          case 83: case 85: case 86: case 88: case 89: 
                          case 90: case 91: case 92: case 93: case 94: 
                          case 95: case 96: case 97: case 98: case 99: 
                          case 100: case 101: case 102: case 103: case 104: 
                          case 105: case 106: case 107: case 108: case 109: 
                          case 110: case 111: case 112: case 113: case 114: 
                          case 115: case 116: case 117: case 118: case 120: 
                          case 121: case 122: case 123: case 280: case 281: 
                          case 282: case 283: case 285: case 286: case 287: 
                          case 288: case 289: case 290: case 291: case 292: 
                          case 293: case 294: case 295: case 296: case 297: 
                          case 298: case 299: case 300: case 301: case 302: 
                          case 303: case 304: case 305: case 306: case 307: 
                          case 308: case 309: case 312: case 313: case 314: 
                          case 315: case 317: case 318: case 319: case 320: 
                          case 321: case 322: case 323: case 324: case 325: 
                          case 326: case 327: case 328: case 329: case 330: 
                          case 331: case 332: case 333: case 334: case 335: 
                          case 336: case 337: case 338: case 340: case 342: 
                          case 344: case 345: case 346: case 347: case 348: 
                          case 349: case 350: case 351: case 352: case 353: 
                          case 354: case 355: case 356: case 357: case 358: 
                          case 359: case 360: case 361: case 362: case 363: 
                          case 364: case 365: case 366: case 367: case 368: 
                          case 369: case 370: case 371: case 372: case 374: 
                          case 413: case 414: case 415: case 416: case 417: 
                          case 418: case 419: case 420: case 421: case 422: 
                          case 423: case 424: case 425: case 426: case 427: 
                          case 428: case 429: case 430: case 431: case 432: 
                          case 433: case 434: case 435: case 436: case 437: 
                          case 438: case 439: case 440: case 441: case 442: 
                          case 443: case 445: case 446: case 447: case 448: 
                          case 449: case 450: case 451: case 452: case 453: 
                          case 454: case 455: case 456: case 457: case 458: 
                          case 459: case 460: case 461: case 462: case 463: 
                          case 464: case 465: case 466: case 468: case 469: 
                          case 470: case 471: case 472: case 473: case 474: 
                          case 475: case 535: case 537: case 538: case 791: 
                          case 793: 
                            goto MATCH_label_a0; break;
                          case 20: case 21: case 23: case 53: case 55: 
                          case 84: case 87: case 119: case 279: case 310: 
                          case 311: case 341: case 343: case 373: case 375: 
                          case 533: case 534: case 790: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a4; 
                            
                            break;
                          case 24: case 27: case 28: case 60: case 124: 
                          case 284: case 316: case 412: case 444: case 476: 
                          case 536: case 539: case 792: case 794: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a5; 
                            
                            break;
                          case 32: 
                            if ((MATCH_w_32_0 >> 22 & 0x1) 
                                    /* Lz at 0 */ == 0) { 
                              MATCH_name = "cmpl"; 
                              goto MATCH_label_a3; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 339: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a6; 
                            
                            break;
                          case 467: 
                            MATCH_name = 
                              MATCH_name_Xo1_18[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a7; 
                            
                            break;
                          default: assert(0);
                        } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/  
                    break;
                  default: assert(0);
                } /* (MATCH_w_32_0 >> 1 & 0x1ff) -- Xo9 at 0 --*/   
          break;
        case 32: case 33: case 34: case 35: case 40: case 41: case 42: 
        case 43: case 46: 
          MATCH_name = 
            MATCH_name_OPCD_0[(MATCH_w_32_0 >> 26 & 0x3f) /* OPCD at 0 */]; 
          { 
            char *name = MATCH_name;
            int /* [~32768..32767] */ d = 
              sign_extend((MATCH_w_32_0 & 0xffff) /* d at 0 */, 16);
            unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
            unsigned rd = (MATCH_w_32_0 >> 21 & 0x1f) /* D at 0 */;
            nextPC = 4 + MATCH_p; 
            
#line 137 "frontend/machine/ppc/decoder.m"
            

            		stmts = instantiate(pc, name, DIS_RD, DIS_INDEX);

            
            
            
          }
          
          break;
        case 36: case 37: case 38: case 39: case 44: case 45: case 47: 
          MATCH_name = 
            MATCH_name_OPCD_0[(MATCH_w_32_0 >> 26 & 0x3f) /* OPCD at 0 */]; 
          { 
            char *name = MATCH_name;
            int /* [~32768..32767] */ d = 
              sign_extend((MATCH_w_32_0 & 0xffff) /* d at 0 */, 16);
            unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
            unsigned rs = (MATCH_w_32_0 >> 21 & 0x1f) /* S at 0 */;
            nextPC = 4 + MATCH_p; 
            
#line 118 "frontend/machine/ppc/decoder.m"
            

            		if (strcmp(name, "stmw") == 0) {

            			// Needs the fourth param s, which is the register number from rs

            			stmts = instantiate(pc, name, DIS_RS, DIS_D, DIS_RA, DIS_S);

            		} else

            			stmts = instantiate(pc, name, DIS_RS, DIS_D, DIS_RA);

            		

            
            
            
          }
          
          break;
        default: assert(0);
      } /* (MATCH_w_32_0 >> 26 & 0x3f) -- OPCD at 0 --*/ 
    
  }goto MATCH_finished_a; 
  
  MATCH_label_a0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
#line 260 "frontend/machine/ppc/decoder.m"
      
      		stmts = NULL;

      		result.valid = false;

      		result.numBytes = 4;	  

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a1: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned b0 = (MATCH_w_32_0 >> 21 & 0x1f) /* BO at 0 */;
      unsigned b1 = (MATCH_w_32_0 >> 16 & 0x1f) /* BI at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 140 "frontend/machine/ppc/decoder.m"
      

      		/*FIXME: since this is used for returns, do a jump to LR instead (ie ignoring control registers) */

      		stmts = instantiate(pc,	 name);

      		result.rtl = new RTL(pc, stmts);

      		result.rtl->appendStmt(new ReturnStatement);

      		unused(b0);

      		unused(b1);

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a2: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned crbA = (MATCH_w_32_0 >> 16 & 0x1f) /* crbA at 0 */;
      unsigned crbB = (MATCH_w_32_0 >> 11 & 0x1f) /* crbB at 0 */;
      unsigned crbD = (MATCH_w_32_0 >> 21 & 0x1f) /* crbD at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 147 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_CRBD, DIS_CRBA, DIS_CRBB);

      		

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a3: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned crfd = (MATCH_w_32_0 >> 23 & 0x7) /* crfD at 0 */;
      unsigned l = (MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */;
      unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
      unsigned rb = (MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 197 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_CRFD, DIS_NZRA, DIS_NZRB);

      		unused(l);

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a4: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
      unsigned rb = (MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */;
      unsigned rd = (MATCH_w_32_0 >> 21 & 0x1f) /* D at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 135 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_RD, DIS_DISP);

      	// Load instructions

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a5: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ra = (MATCH_w_32_0 >> 21 & 0x1f) /* S at 0 */;
      unsigned rb = (MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */;
      unsigned rd = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 132 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_RD, DIS_RA, DIS_RB);

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a6: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned rd = (MATCH_w_32_0 >> 21 & 0x1f) /* D at 0 */;
      unsigned uimm = 
        ((MATCH_w_32_0 >> 11 & 0x1f) /* sprH at 0 */ << 5) + 
        (MATCH_w_32_0 >> 16 & 0x1f) /* sprL at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 149 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_RD, DIS_UIMM);

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a7: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned rs = (MATCH_w_32_0 >> 21 & 0x1f) /* S at 0 */;
      unsigned uimm = 
        ((MATCH_w_32_0 >> 11 & 0x1f) /* sprH at 0 */ << 5) + 
        (MATCH_w_32_0 >> 16 & 0x1f) /* sprL at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 152 "frontend/machine/ppc/decoder.m"
      

      		if ((uimm >> 5) &1) {		// FIXME: Fix shift amounts

      		  if ((uimm >> 8) & 1) {

      			stmts = instantiate(pc, "MTCTR" , DIS_RS);

      		  } else {

      			stmts = instantiate(pc, "MTXER" , DIS_RS);

      		  }

      		} else {

      			stmts = instantiate(pc, "MTLR" , DIS_RS);

      		}

      		::unused(name);

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a8: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
      unsigned rd = (MATCH_w_32_0 >> 21 & 0x1f) /* D at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 115 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_RD, DIS_RA);

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_finished_a: (void)0; /*placeholder for label*/
  
}

#line 265 "frontend/machine/ppc/decoder.m"

	result.numBytes = nextPC - hostPC;
	if (result.valid && result.rtl == 0)	// Don't override higher level res
		result.rtl = new RTL(pc, stmts);

	return result;
}


/***********************************************************************
 * These are functions used to decode instruction operands into
 * expressions (Exp*s).
 **********************************************************************/

/*==============================================================================
 * FUNCTION:		PPCDecoder::dis_Reg
 * OVERVIEW:		Decode the register
 * PARAMETERS:		r - register (0-31)
 * RETURNS:			the expression representing the register
 *============================================================================*/
Exp* PPCDecoder::dis_Reg(unsigned r)
{
	return Location::regOf(r);
}

/*==============================================================================
 * FUNCTION:		PPCDecoder::dis_RAmbz
 * OVERVIEW:		Decode the register rA when rA represents constant 0 if r == 0
 * PARAMETERS:		r - register (0-31)
 * RETURNS:			the expression representing the register
 *============================================================================*/
Exp* PPCDecoder::dis_RAmbz(unsigned r)
{
	if (r == 0)
		return new Const(0);
	return Location::regOf(r);
}


/*==============================================================================
 * FUNCTION:	  isFuncPrologue()
 * OVERVIEW:	  Check to see if the instructions at the given offset match
 *					any callee prologue, i.e. does it look like this offset
 *					is a pointer to a function?
 * PARAMETERS:	  hostPC - pointer to the code in question (host address)
 * RETURNS:		  True if a match found
 *============================================================================*/
bool PPCDecoder::isFuncPrologue(ADDRESS hostPC)
{

	return false;
}


 /**********************************
 * These are the fetch routines.
 **********************************/

/*==============================================================================
 * FUNCTION:		getDword
 * OVERVIEW:		Returns the double starting at the given address.
 * PARAMETERS:		lc - address at which to decode the double
 * RETURNS:			the decoded double
 *============================================================================*/
DWord PPCDecoder::getDword(ADDRESS lc)
{
  Byte* p = (Byte*)lc;
  return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
}

/*==============================================================================
 * FUNCTION:	   PPCDecoder::PPCDecoder
 * OVERVIEW:	   
 * PARAMETERS:	   None
 * RETURNS:		   N/A
 *============================================================================*/
PPCDecoder::PPCDecoder() : NJMCDecoder()
{
  std::string file = Boomerang::get()->getProgPath() + "frontend/machine/ppc/ppc.ssl";
  RTLDict.readSSLFile(file.c_str());
}

// For now...
int PPCDecoder::decodeAssemblyInstruction(unsigned, int)
{ return 0; }

// Get an expression for a CR bit. For example, if bitNum is 6, return r65@[2:2]
// (r64 .. r71 are the %cr0 .. %cr7 flag sets)
Exp* crBit(int bitNum) {
	int		crNum = bitNum / 4;
	bitNum = bitNum & 3;
	return new Ternary(opAt,
		Location::regOf(64 + crNum),
		new Const(bitNum),
		new Const(bitNum));
}



