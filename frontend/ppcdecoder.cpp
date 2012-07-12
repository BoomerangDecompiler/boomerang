#define sign_extend(N,SIZE) (((int)((N) << (sizeof(unsigned)*8-(SIZE)))) >> (sizeof(unsigned)*8-(SIZE)))
#include <assert.h>

#line 0 "frontend/machine/ppc/decoder.m"
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

/* $Revision$	// 1.24.2.1
 *
 * 23/Nov/04 - Jay Sweeney and Alajandro Dubrovsky: Created
 * 26/Sep/05 - Mike: Added Xsab_, Xsax_; DIS_INDEX uses RAZ not RA now; A2c_ -> Ac_ (does single as well as double prec)
 **/

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#include <cstring>
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
#define DIS_RELADDR	(new Const(reladdr - delta))
#define DIS_CRBD	(crBit(crbD))
#define DIS_CRBA	(crBit(crbA))
#define DIS_CRBB	(crBit(crbB))
#define DIS_DISP	(new Binary(opPlus, dis_RAmbz(ra), new Const(d)))
#define DIS_INDEX	(new Binary(opPlus, DIS_RAZ, DIS_NZRB))
#define DIS_BICR	(new Const(BIcr))
#define DIS_RS_NUM	(new Const(rs))
#define DIS_RD_NUM	(new Const(rd))
#define DIS_BEG		(new Const(beg))
#define DIS_END		(new Const(end))
#define DIS_FD		(dis_Reg(fd+32))
#define DIS_FS		(dis_Reg(fs+32))
#define DIS_FA		(dis_Reg(fa+32))
#define DIS_FB		(dis_Reg(fb+32))

#define PPC_COND_JUMP(name, size, relocd, cond, BIcr) \
	result.rtl = new RTL(pc, stmts); \
	BranchStatement* jump = new BranchStatement; \
	result.rtl->appendStmt(jump); \
	result.numBytes = size; \
	jump->setDest(relocd-delta); \
	jump->setCondType(cond); \
	SHOW_ASM(name<<" "<<BIcr<<", 0x"<<std::hex<<relocd-delta)

/*==============================================================================
 * FUNCTION:	   unused
 * OVERVIEW:	   A dummy function to suppress "unused local variable" messages
 * PARAMETERS:	   x: integer variable to be "used"
 * RETURNS:		   Nothing
 *============================================================================*/
void PPCDecoder::unused(int x)
{}
void unused(const char* x) {}


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



#line 119 "frontend/machine/ppc/decoder.m"
{ 
  dword MATCH_p = 
    
#line 119 "frontend/machine/ppc/decoder.m"
    hostPC
    ;
  const char *MATCH_name;
  static const char *MATCH_name_OPCD_0[] = {
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, "mulli", "subfic", (const char *)0, (const char *)0, (const char *)0, "addic", 
    "addicq", "addi", "addis", (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, "ori", "oris", "xori", 
    "xoris", "andiq", "andisq", (const char *)0, (const char *)0, "lwz", "lwzu", "lbz", 
    "lbzu", "stw", "stwu", "stb", "stbu", "lhz", "lhzu", "lha", "lhau", 
    "sth", "sthu", "lmw", "stmw", "lfs", "lfsu", "lfd", "lfdu", "stfs", 
    "stfsu", "stfd", "stfdu", 
  };
  static const char *MATCH_name_BO4_3[] = {
    (const char *)0, (const char *)0, "bge", (const char *)0, (const char *)0, (const char *)0, "blt", 
  };
  static const char *MATCH_name_BO4_4[] = {
    (const char *)0, (const char *)0, "ble", (const char *)0, (const char *)0, (const char *)0, "bgt", 
  };
  static const char *MATCH_name_BO4_5[] = {
    (const char *)0, (const char *)0, "bne", (const char *)0, (const char *)0, (const char *)0, "beq", 
  };
  static const char *MATCH_name_BO4_6[] = {
    (const char *)0, (const char *)0, "bns", (const char *)0, (const char *)0, (const char *)0, "bso", 
  };
  static const char *MATCH_name_LK_8[] = {"crnor", "bl", };
  static const char *MATCH_name_BO4_10[] = {
    (const char *)0, (const char *)0, "bgelr", (const char *)0, (const char *)0, (const char *)0, "bltlr", 
  };
  static const char *MATCH_name_BO4_11[] = {
    (const char *)0, (const char *)0, "blelr", (const char *)0, (const char *)0, (const char *)0, "bgtlr", 
  };
  static const char *MATCH_name_BO4_12[] = {
    (const char *)0, (const char *)0, "bnelr", (const char *)0, (const char *)0, (const char *)0, "beqlr", 
  };
  static const char *MATCH_name_BO4_13[] = {
    (const char *)0, (const char *)0, "bnslr", (const char *)0, (const char *)0, (const char *)0, "bsolr", 
  };
  static const char *MATCH_name_LK_14[] = {"crandc", "balctrl", };
  static const char *MATCH_name_Rc_22[] = {"rlwimi", "rlwimiq", };
  static const char *MATCH_name_Rc_23[] = {"rlwinm", "rlwinmq", };
  static const char *MATCH_name_Xo1_26[] = {
    "fcmpu", (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "frsp", 
    (const char *)0, "fctiw", "fctiwz", (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    "lwarx", "ldx", (const char *)0, "lwzx", "slw", (const char *)0, "cntlzw", "sld", 
    "and", (const char *)0, (const char *)0, (const char *)0, "fcmpo", (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "fneg", (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "ldux", (const char *)0, 
    "lwzux", (const char *)0, (const char *)0, "cntlzd", (const char *)0, "andc", (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, "fmr", (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, "ldarx", (const char *)0, (const char *)0, "lbzx", 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, "lbzux", (const char *)0, (const char *)0, (const char *)0, (const char *)0, "nor", 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "fnabs", 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "stdx", 
    "stwcxq", "stwx", (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    "stdux", (const char *)0, "stwux", (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, "stdcxq", "stbx", (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, "stbux", (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, "fabs", (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "lhzx", (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, "eqv", (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, "eciwx", "lhzux", (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, "xor", (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, "mfspr", (const char *)0, "lwax", (const char *)0, "lhax", (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, "lwaux", (const char *)0, "lhaux", 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, "sthx", (const char *)0, (const char *)0, (const char *)0, (const char *)0, "orc", 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, "ecowx", "sthux", (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    "or", (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, "mtspr", (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, "nand", (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, "lswx", "lwbrx", "lfsx", "srw", 
    (const char *)0, (const char *)0, "srd", (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "lfsux", 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, "lfdx", (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, "lfdux", (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, "stswx", "stwbrx", "stfsx", (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "stfsux", 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, "stfdx", (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, "stfdux", (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, "lhbrx", (const char *)0, "sraw", 
    (const char *)0, "srad", (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, "fctid", "fctidz", (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "srawi", 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, "fcfid", (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, "sthbrx", (const char *)0, (const char *)0, 
    (const char *)0, "extsh", (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, "extsb", (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, "stfiwx", (const char *)0, (const char *)0, "extsw", 
  };
  static const char *MATCH_name_Xo9_29[] = {
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, "subfc", (const char *)0, "addc", (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, "subf", (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "neg", 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, "subfe", (const char *)0, "adde", (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, "subfze", (const char *)0, "addze", 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "subfme", "mulld", 
    "addme", "mullw", (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, "add", (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "divdu", 
    (const char *)0, "divwu", (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, "divd", (const char *)0, "divw", 
  };
  static const char *MATCH_name_Xo1_30[] = {
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    "frspq", (const char *)0, "fctiwq", "fctiwzq", (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "slwq", (const char *)0, 
    (const char *)0, "sldq", "andq", (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, "fnegq", (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, "andcq", (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, "fmrq", (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "norq", 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "fnabsq", 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, "fabsq", (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, "eqvq", (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, "xorq", (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, "orcq", (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, "orq", (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "nandq", 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "srwq", (const char *)0, 
    (const char *)0, "srdq", (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, "srawq", (const char *)0, "sradq", (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, "fctidq", "fctidzq", 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, "srawiq", (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    "fcfidq", 
  };
  static const char *MATCH_name_Rc_36[] = {"fdivs", "fdivsq", };
  static const char *MATCH_name_Rc_37[] = {"fsubs", "fsubsq", };
  static const char *MATCH_name_Rc_38[] = {"fadds", "faddsq", };
  static const char *MATCH_name_Xo5_40[] = {
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, 
    (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, (const char *)0, "fdiv", 
    (const char *)0, "fsub", "fadd", 
  };
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    
      switch((MATCH_w_32_0 >> 26 & 0x3f) /* OPCD at 0 */) {
        case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 9: 
        case 17: case 22: case 23: case 30: case 56: case 57: case 58: 
        case 60: case 61: case 62: 
          goto MATCH_label_a0; break;
        case 7: case 8: case 12: case 13: case 14: case 15: 
          MATCH_name = 
            MATCH_name_OPCD_0[(MATCH_w_32_0 >> 26 & 0x3f) /* OPCD at 0 */]; 
          { 
            const char *name = MATCH_name;
            unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
            unsigned rd = (MATCH_w_32_0 >> 21 & 0x1f) /* D at 0 */;
            int /* [~32768..32767] */ simm = 
              sign_extend((MATCH_w_32_0 & 0xffff) /* SIMM at 0 */, 16);
            nextPC = 4 + MATCH_p; 
            
#line 139 "frontend/machine/ppc/decoder.m"
            

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
              const char *name = MATCH_name;
              unsigned crfd = (MATCH_w_32_0 >> 23 & 0x7) /* crfD at 0 */;
              unsigned l = (MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */;
              unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
              unsigned uimm = (MATCH_w_32_0 & 0xffff) /* UIMM at 0 */;
              nextPC = 4 + MATCH_p; 
              
#line 239 "frontend/machine/ppc/decoder.m"
              

              		stmts = instantiate(pc, name, DIS_CRFD, DIS_NZRA, DIS_UIMM);

              		unused(l);

              

              
              
              
            }
            
          } /*opt-block*/
          else 
            goto MATCH_label_a0;  /*opt-block+*/
          
          break;
        case 11: 
          if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
            MATCH_name = "cmpi"; 
            { 
              const char *name = MATCH_name;
              unsigned crfd = (MATCH_w_32_0 >> 23 & 0x7) /* crfD at 0 */;
              unsigned l = (MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */;
              unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
              int /* [~32768..32767] */ simm = 
                sign_extend((MATCH_w_32_0 & 0xffff) /* SIMM at 0 */, 16);
              nextPC = 4 + MATCH_p; 
              
#line 236 "frontend/machine/ppc/decoder.m"
              

              		stmts = instantiate(pc, name, DIS_CRFD, DIS_NZRA, DIS_SIMM);

              		unused(l);

              
              
              
            }
            
          } /*opt-block*/
          else 
            goto MATCH_label_a0;  /*opt-block+*/
          
          break;
        case 16: 
          if ((MATCH_w_32_0 >> 1 & 0x1) /* AA at 0 */ == 1) 
            goto MATCH_label_a0;  /*opt-block+*/
          else 
            if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 1) 
              if ((MATCH_w_32_0 >> 21 & 0x1f) /* BO at 0 */ == 20) { 
                MATCH_name = "ball"; 
                { 
                  const char *name = MATCH_name;
                  unsigned BIcr = (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                  unsigned reladdr = 
                    4 * (MATCH_w_32_0 >> 2 & 0x3fff) /* BD at 0 */ + 
                    addressToPC(MATCH_p);
                  nextPC = 4 + MATCH_p; 
                  
#line 210 "frontend/machine/ppc/decoder.m"
                  		// Always "conditional" branch with link, test/OSX/hello has this

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
              else 
                goto MATCH_label_a0;  /*opt-block+*/ /*opt-block+*/
            else 
              
                switch((MATCH_w_32_0 >> 16 & 0x3) /* BIcc at 0 */) {
                  case 0: 
                    
                      switch((MATCH_w_32_0 >> 22 & 0xf) /* BO4 at 0 */) {
                        case 0: case 1: case 3: case 4: case 5: case 7: 
                        case 8: case 9: case 10: case 11: case 12: case 13: 
                        case 14: case 15: 
                          if ((MATCH_w_32_0 >> 21 & 0x1f) 
                                  /* BO at 0 */ == 20) 
                            goto MATCH_label_a1;  /*opt-block+*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/
                          
                          break;
                        case 2: 
                          MATCH_name = 
                            MATCH_name_BO4_3[(MATCH_w_32_0 >> 22 & 0xf) 
                                /* BO4 at 0 */]; 
                          { 
                            const char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 275 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSGE, BIcr);

                            
                            
                            
                          }
                          
                          break;
                        case 6: 
                          MATCH_name = 
                            MATCH_name_BO4_3[(MATCH_w_32_0 >> 22 & 0xf) 
                                /* BO4 at 0 */]; 
                          { 
                            const char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 269 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSL, BIcr);

                            
                            
                            
                          }
                          
                          break;
                        default: assert(0);
                      } /* (MATCH_w_32_0 >> 22 & 0xf) -- BO4 at 0 --*/ 
                    break;
                  case 1: 
                    
                      switch((MATCH_w_32_0 >> 22 & 0xf) /* BO4 at 0 */) {
                        case 0: case 1: case 3: case 4: case 5: case 7: 
                        case 8: case 9: case 10: case 11: case 12: case 13: 
                        case 14: case 15: 
                          if ((MATCH_w_32_0 >> 21 & 0x1f) 
                                  /* BO at 0 */ == 20) 
                            goto MATCH_label_a1;  /*opt-block+*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/
                          
                          break;
                        case 2: 
                          MATCH_name = 
                            MATCH_name_BO4_4[(MATCH_w_32_0 >> 22 & 0xf) 
                                /* BO4 at 0 */]; 
                          { 
                            const char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 271 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSLE, BIcr);

                            
                            
                            
                          }
                          
                          break;
                        case 6: 
                          MATCH_name = 
                            MATCH_name_BO4_4[(MATCH_w_32_0 >> 22 & 0xf) 
                                /* BO4 at 0 */]; 
                          { 
                            const char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 278 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSG, BIcr);

                            //	| bnl(BIcr, reladdr) [name] =>								// bnl same as bge

                            //		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSGE, BIcr);

                            
                            
                            
                          }
                          
                          break;
                        default: assert(0);
                      } /* (MATCH_w_32_0 >> 22 & 0xf) -- BO4 at 0 --*/ 
                    break;
                  case 2: 
                    
                      switch((MATCH_w_32_0 >> 22 & 0xf) /* BO4 at 0 */) {
                        case 0: case 1: case 3: case 4: case 5: case 7: 
                        case 8: case 9: case 10: case 11: case 12: case 13: 
                        case 14: case 15: 
                          if ((MATCH_w_32_0 >> 21 & 0x1f) 
                                  /* BO at 0 */ == 20) 
                            goto MATCH_label_a1;  /*opt-block+*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/
                          
                          break;
                        case 2: 
                          MATCH_name = 
                            MATCH_name_BO4_5[(MATCH_w_32_0 >> 22 & 0xf) 
                                /* BO4 at 0 */]; 
                          { 
                            const char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 282 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JNE, BIcr);

                            //	| bng(BIcr, reladdr) [name] =>								// bng same as blt

                            //		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSLE, BIcr);

                            
                            
                            
                          }
                          
                          break;
                        case 6: 
                          MATCH_name = 
                            MATCH_name_BO4_5[(MATCH_w_32_0 >> 22 & 0xf) 
                                /* BO4 at 0 */]; 
                          { 
                            const char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 273 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JE, BIcr);

                            
                            
                            
                          }
                          
                          break;
                        default: assert(0);
                      } /* (MATCH_w_32_0 >> 22 & 0xf) -- BO4 at 0 --*/ 
                    break;
                  case 3: 
                    
                      switch((MATCH_w_32_0 >> 22 & 0xf) /* BO4 at 0 */) {
                        case 0: case 1: case 3: case 4: case 5: case 7: 
                        case 8: case 9: case 10: case 11: case 12: case 13: 
                        case 14: case 15: 
                          if ((MATCH_w_32_0 >> 21 & 0x1f) 
                                  /* BO at 0 */ == 20) 
                            goto MATCH_label_a1;  /*opt-block+*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/
                          
                          break;
                        case 2: 
                          MATCH_name = 
                            MATCH_name_BO4_6[(MATCH_w_32_0 >> 22 & 0xf) 
                                /* BO4 at 0 */]; 
                          { 
                            const char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 288 "frontend/machine/ppc/decoder.m"
                            

                            		PPC_COND_JUMP(name, 4, reladdr, (BRANCH_TYPE)0, BIcr);

                            //	| bun(BIcr, reladdr) [name] =>

                            //		PPC_COND_JUMP(name, 4, reladdr, (BRANCH_TYPE)0, BIcr);

                            //	| bnu(BIcr, reladdr) [name] =>

                            //		PPC_COND_JUMP(name, 4, reladdr, (BRANCH_TYPE)0, BIcr);

                            

                            
                            
                            
                          }
                          
                          break;
                        case 6: 
                          MATCH_name = 
                            MATCH_name_BO4_6[(MATCH_w_32_0 >> 22 & 0xf) 
                                /* BO4 at 0 */]; 
                          { 
                            const char *name = MATCH_name;
                            unsigned BIcr = 
                              (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                            unsigned reladdr = 
                              4 * (MATCH_w_32_0 >> 2 & 0x3fff) 
                                    /* BD at 0 */ + addressToPC(MATCH_p);
                            nextPC = 4 + MATCH_p; 
                            
#line 285 "frontend/machine/ppc/decoder.m"
                            								// Branch on summary overflow

                            		PPC_COND_JUMP(name, 4, reladdr, (BRANCH_TYPE)0, BIcr);	// MVE: Don't know these last 4 yet

                            
                            
                            
                          }
                          
                          break;
                        default: assert(0);
                      } /* (MATCH_w_32_0 >> 22 & 0xf) -- BO4 at 0 --*/ 
                    break;
                  default: assert(0);
                } /* (MATCH_w_32_0 >> 16 & 0x3) -- BIcc at 0 --*/   
          break;
        case 18: 
          if ((MATCH_w_32_0 >> 1 & 0x1) /* AA at 0 */ == 1) 
            goto MATCH_label_a0;  /*opt-block+*/
          else 
            if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 1) { 
              MATCH_name = 
                MATCH_name_LK_8[(MATCH_w_32_0 & 0x1) /* LK at 0 */]; 
              { 
                const char *name = MATCH_name;
                unsigned reladdr = 
                  4 * sign_extend(
                              (MATCH_w_32_0 >> 2 & 0xffffff) /* LI at 0 */, 
                              24) + addressToPC(MATCH_p);
                nextPC = 4 + MATCH_p; 
                
#line 193 "frontend/machine/ppc/decoder.m"
                

                		Exp* dest = DIS_RELADDR;

                		stmts = instantiate(pc, name, dest);

                		CallStatement* newCall = new CallStatement;

                		// Record the fact that this is not a computed call

                		newCall->setIsComputed(false);

                		// Set the destination expression

                		newCall->setDest(dest);

                		result.rtl = new RTL(pc, stmts);

                		result.rtl->appendStmt(newCall);

                		Proc* destProc = prog->setNewProc(reladdr-delta);

                		if (destProc == (Proc*)-1) destProc = NULL;

                		newCall->setDestProc(destProc);

                

                
                
                
              }
              
            } /*opt-block*/
            else { 
              unsigned reladdr = 
                4 * sign_extend((MATCH_w_32_0 >> 2 & 0xffffff) /* LI at 0 */, 
                            24) + addressToPC(MATCH_p);
              nextPC = 4 + MATCH_p; 
              
#line 207 "frontend/machine/ppc/decoder.m"
              

              		unconditionalJump("b", 4, reladdr, delta, pc, stmts, result);

              

              
              
              
            } /*opt-block*//*opt-block+*/ /*opt-block+*/
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
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 1) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else 
                  if ((MATCH_w_32_0 >> 11 & 0x1f) /* crbB at 0 */ == 0) 
                    
                      switch((MATCH_w_32_0 >> 16 & 0x3) /* BIcc at 0 */) {
                        case 0: 
                          
                            switch((MATCH_w_32_0 >> 22 & 0xf) 
                                  /* BO4 at 0 */) {
                              case 0: case 1: case 3: case 4: case 5: case 7: 
                              case 8: case 9: case 10: case 11: case 12: 
                              case 13: case 14: case 15: 
                                if ((MATCH_w_32_0 >> 21 & 0x1f) 
                                        /* BO at 0 */ == 20) { 
                                  MATCH_name = "ballr"; 
                                  goto MATCH_label_a2; 
                                  
                                } /*opt-block*/
                                else 
                                  goto MATCH_label_a0;  /*opt-block+*/
                                
                                break;
                              case 2: 
                                MATCH_name = 
                                  MATCH_name_BO4_10[(MATCH_w_32_0 >> 22 & 0xf) 
                                      /* BO4 at 0 */]; 
                                { 
                                  const char *name = MATCH_name;
                                  unsigned BIcr = 
                                    (MATCH_w_32_0 >> 18 & 0x7) 
                                          /* BIcr at 0 */;
                                  nextPC = 4 + MATCH_p; 
                                  
#line 321 "frontend/machine/ppc/decoder.m"
                                  

                                  		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JSL, BIcr);

                                  		result.rtl->appendStmt(new ReturnStatement);

                                  

                                  
                                  
                                  
                                }
                                
                                break;
                              case 6: 
                                MATCH_name = 
                                  MATCH_name_BO4_10[(MATCH_w_32_0 >> 22 & 0xf) 
                                      /* BO4 at 0 */]; 
                                { 
                                  const char *name = MATCH_name;
                                  unsigned BIcr = 
                                    (MATCH_w_32_0 >> 18 & 0x7) 
                                          /* BIcr at 0 */;
                                  nextPC = 4 + MATCH_p; 
                                  
#line 309 "frontend/machine/ppc/decoder.m"
                                  

                                  		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JSGE, BIcr);

                                  		result.rtl->appendStmt(new ReturnStatement);

                                  

                                  
                                  
                                  
                                }
                                
                                break;
                              default: assert(0);
                            } /* (MATCH_w_32_0 >> 22 & 0xf) -- BO4 at 0 --*/ 
                          break;
                        case 1: 
                          
                            switch((MATCH_w_32_0 >> 22 & 0xf) 
                                  /* BO4 at 0 */) {
                              case 0: case 1: case 3: case 4: case 5: case 7: 
                              case 8: case 9: case 10: case 11: case 12: 
                              case 13: case 14: case 15: 
                                if ((MATCH_w_32_0 >> 21 & 0x1f) 
                                        /* BO at 0 */ == 20) { 
                                  MATCH_name = "ballr"; 
                                  goto MATCH_label_a2; 
                                  
                                } /*opt-block*/
                                else 
                                  goto MATCH_label_a0;  /*opt-block+*/
                                
                                break;
                              case 2: 
                                MATCH_name = 
                                  MATCH_name_BO4_11[(MATCH_w_32_0 >> 22 & 0xf) 
                                      /* BO4 at 0 */]; 
                                { 
                                  const char *name = MATCH_name;
                                  unsigned BIcr = 
                                    (MATCH_w_32_0 >> 18 & 0x7) 
                                          /* BIcr at 0 */;
                                  nextPC = 4 + MATCH_p; 
                                  
#line 313 "frontend/machine/ppc/decoder.m"
                                  

                                  		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JSG, BIcr);

                                  		result.rtl->appendStmt(new ReturnStatement);

                                  

                                  
                                  
                                  
                                }
                                
                                break;
                              case 6: 
                                MATCH_name = 
                                  MATCH_name_BO4_11[(MATCH_w_32_0 >> 22 & 0xf) 
                                      /* BO4 at 0 */]; 
                                { 
                                  const char *name = MATCH_name;
                                  unsigned BIcr = 
                                    (MATCH_w_32_0 >> 18 & 0x7) 
                                          /* BIcr at 0 */;
                                  nextPC = 4 + MATCH_p; 
                                  
#line 325 "frontend/machine/ppc/decoder.m"
                                  

                                  		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JSLE, BIcr);

                                  		result.rtl->appendStmt(new ReturnStatement);

                                  

                                  
                                  
                                  
                                }
                                
                                break;
                              default: assert(0);
                            } /* (MATCH_w_32_0 >> 22 & 0xf) -- BO4 at 0 --*/ 
                          break;
                        case 2: 
                          
                            switch((MATCH_w_32_0 >> 22 & 0xf) 
                                  /* BO4 at 0 */) {
                              case 0: case 1: case 3: case 4: case 5: case 7: 
                              case 8: case 9: case 10: case 11: case 12: 
                              case 13: case 14: case 15: 
                                if ((MATCH_w_32_0 >> 21 & 0x1f) 
                                        /* BO at 0 */ == 20) { 
                                  MATCH_name = "ballr"; 
                                  goto MATCH_label_a2; 
                                  
                                } /*opt-block*/
                                else 
                                  goto MATCH_label_a0;  /*opt-block+*/
                                
                                break;
                              case 2: 
                                MATCH_name = 
                                  MATCH_name_BO4_12[(MATCH_w_32_0 >> 22 & 0xf) 
                                      /* BO4 at 0 */]; 
                                { 
                                  const char *name = MATCH_name;
                                  unsigned BIcr = 
                                    (MATCH_w_32_0 >> 18 & 0x7) 
                                          /* BIcr at 0 */;
                                  nextPC = 4 + MATCH_p; 
                                  
#line 329 "frontend/machine/ppc/decoder.m"
                                  

                                  		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JE, BIcr);

                                  		result.rtl->appendStmt(new ReturnStatement);

                                  

                                  
                                  
                                  
                                }
                                
                                break;
                              case 6: 
                                MATCH_name = 
                                  MATCH_name_BO4_12[(MATCH_w_32_0 >> 22 & 0xf) 
                                      /* BO4 at 0 */]; 
                                { 
                                  const char *name = MATCH_name;
                                  unsigned BIcr = 
                                    (MATCH_w_32_0 >> 18 & 0x7) 
                                          /* BIcr at 0 */;
                                  nextPC = 4 + MATCH_p; 
                                  
#line 317 "frontend/machine/ppc/decoder.m"
                                  

                                  		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JNE, BIcr);

                                  		result.rtl->appendStmt(new ReturnStatement);

                                  

                                  
                                  
                                  
                                }
                                
                                break;
                              default: assert(0);
                            } /* (MATCH_w_32_0 >> 22 & 0xf) -- BO4 at 0 --*/ 
                          break;
                        case 3: 
                          
                            switch((MATCH_w_32_0 >> 22 & 0xf) 
                                  /* BO4 at 0 */) {
                              case 0: case 1: case 3: case 4: case 5: case 7: 
                              case 8: case 9: case 10: case 11: case 12: 
                              case 13: case 14: case 15: 
                                if ((MATCH_w_32_0 >> 21 & 0x1f) 
                                        /* BO at 0 */ == 20) { 
                                  MATCH_name = "ballr"; 
                                  goto MATCH_label_a2; 
                                  
                                } /*opt-block*/
                                else 
                                  goto MATCH_label_a0;  /*opt-block+*/
                                
                                break;
                              case 2: 
                                MATCH_name = 
                                  MATCH_name_BO4_13[(MATCH_w_32_0 >> 22 & 0xf) 
                                      /* BO4 at 0 */]; 
                                { 
                                  const char *name = MATCH_name;
                                  unsigned BIcr = 
                                    (MATCH_w_32_0 >> 18 & 0x7) 
                                          /* BIcr at 0 */;
                                  nextPC = 4 + MATCH_p; 
                                  
#line 337 "frontend/machine/ppc/decoder.m"
                                  

                                  		PPC_COND_JUMP(name, 4, hostPC+4, (BRANCH_TYPE)0, BIcr);

                                  		result.rtl->appendStmt(new ReturnStatement);

                                  

                                  
                                  
                                  
                                }
                                
                                break;
                              case 6: 
                                MATCH_name = 
                                  MATCH_name_BO4_13[(MATCH_w_32_0 >> 22 & 0xf) 
                                      /* BO4 at 0 */]; 
                                { 
                                  const char *name = MATCH_name;
                                  unsigned BIcr = 
                                    (MATCH_w_32_0 >> 18 & 0x7) 
                                          /* BIcr at 0 */;
                                  nextPC = 4 + MATCH_p; 
                                  
#line 333 "frontend/machine/ppc/decoder.m"
                                  

                                  		PPC_COND_JUMP(name, 4, hostPC+4, (BRANCH_TYPE)0, BIcr);

                                  		result.rtl->appendStmt(new ReturnStatement);

                                  

                                  
                                  
                                  
                                }
                                
                                break;
                              default: assert(0);
                            } /* (MATCH_w_32_0 >> 22 & 0xf) -- BO4 at 0 --*/ 
                          break;
                        default: assert(0);
                      } /* (MATCH_w_32_0 >> 16 & 0x3) -- BIcc at 0 --*/  
                  else 
                    goto MATCH_label_a0;  /*opt-block+*/ 
                break;
              case 33: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_LK_8[(MATCH_w_32_0 & 0x1) /* LK at 0 */]; 
                  goto MATCH_label_a3; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 129: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_LK_14[(MATCH_w_32_0 & 0x1) /* LK at 0 */]; 
                  goto MATCH_label_a3; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 193: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = "crxor"; 
                  goto MATCH_label_a3; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 225: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = "crnand"; 
                  goto MATCH_label_a3; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 257: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = "crand"; 
                  goto MATCH_label_a3; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 289: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = "creqv"; 
                  goto MATCH_label_a3; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 417: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = "crorc"; 
                  goto MATCH_label_a3; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 449: 
                if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 0) { 
                  MATCH_name = "cror"; 
                  goto MATCH_label_a3; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 528: 
                if ((MATCH_w_32_0 >> 21 & 0x1f) /* BO at 0 */ == 20) 
                  if ((MATCH_w_32_0 >> 11 & 0x1f) /* crbB at 0 */ == 0) 
                    if ((MATCH_w_32_0 & 0x1) /* LK at 0 */ == 1) { 
                      MATCH_name = 
                        MATCH_name_LK_14[(MATCH_w_32_0 & 0x1) /* LK at 0 */]; 
                      { 
                        const char *name = MATCH_name;
                        unsigned BIcr = 
                          (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 299 "frontend/machine/ppc/decoder.m"
                        

                        		computedCall(name, 4, new Unary(opMachFtr, new Const("%CTR")), pc, stmts, result);

                        		unused(BIcr);

                        		

                        
                        
                        
                      }
                      
                    } /*opt-block*/
                    else { 
                      MATCH_name = "balctr"; 
                      { 
                        const char *name = MATCH_name;
                        unsigned BIcr = 
                          (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 295 "frontend/machine/ppc/decoder.m"
                        

                        		computedJump(name, 4, new Unary(opMachFtr, new Const("%CTR")), pc, stmts, result);

                        		unused(BIcr);

                        		

                        
                        
                        
                      }
                      
                    } /*opt-block*/ /*opt-block+*/
                  else 
                    goto MATCH_label_a0;  /*opt-block+*/ 
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/ 
          break;
        case 20: 
          MATCH_name = MATCH_name_Rc_22[(MATCH_w_32_0 & 0x1) /* Rc at 0 */]; 
          goto MATCH_label_a4; 
          
          break;
        case 21: 
          MATCH_name = MATCH_name_Rc_23[(MATCH_w_32_0 & 0x1) /* Rc at 0 */]; 
          goto MATCH_label_a4; 
          
          break;
        case 24: case 25: case 26: case 27: case 28: case 29: 
          MATCH_name = 
            MATCH_name_OPCD_0[(MATCH_w_32_0 >> 26 & 0x3f) /* OPCD at 0 */]; 
          { 
            const char *name = MATCH_name;
            unsigned ra = (MATCH_w_32_0 >> 21 & 0x1f) /* S at 0 */;
            unsigned rd = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
            unsigned uimm = (MATCH_w_32_0 & 0xffff) /* UIMM at 0 */;
            nextPC = 4 + MATCH_p; 
            
#line 136 "frontend/machine/ppc/decoder.m"
            

            		stmts = instantiate(pc, name, DIS_RD, DIS_RA, DIS_UIMM);

            
            
            
          }
          
          break;
        case 31: 
          if ((MATCH_w_32_0 & 0x1) /* Rc at 0 */ == 1) 
            if (61 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
              (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 124 || 
              151 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
              (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 214 || 
              215 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
              (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 284 || 
              317 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
              (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 412 || 
              477 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
              (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 536 || 
              540 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
              (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 792 || 
              825 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
              (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 922 || 
              987 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
              (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
              goto MATCH_label_a0;  /*opt-block+*/
            else 
              switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                case 0: case 1: case 2: case 3: case 4: case 5: case 6: 
                case 7: case 8: case 9: case 10: case 11: case 12: case 13: 
                case 14: case 15: case 16: case 17: case 18: case 19: 
                case 20: case 21: case 22: case 23: case 25: case 29: 
                case 30: case 31: case 32: case 33: case 34: case 35: 
                case 36: case 37: case 38: case 39: case 40: case 41: 
                case 42: case 43: case 44: case 45: case 46: case 47: 
                case 48: case 49: case 50: case 51: case 52: case 53: 
                case 54: case 55: case 56: case 57: case 59: case 125: 
                case 126: case 127: case 128: case 129: case 130: case 131: 
                case 132: case 133: case 134: case 135: case 136: case 137: 
                case 138: case 139: case 140: case 141: case 142: case 143: 
                case 144: case 145: case 146: case 147: case 148: case 149: 
                case 285: case 286: case 287: case 288: case 289: case 290: 
                case 291: case 292: case 293: case 294: case 295: case 296: 
                case 297: case 298: case 299: case 300: case 301: case 302: 
                case 303: case 304: case 305: case 306: case 307: case 308: 
                case 309: case 310: case 311: case 312: case 313: case 314: 
                case 315: case 413: case 414: case 415: case 416: case 417: 
                case 418: case 419: case 420: case 421: case 422: case 423: 
                case 424: case 425: case 426: case 427: case 428: case 429: 
                case 430: case 431: case 432: case 433: case 434: case 435: 
                case 436: case 437: case 438: case 439: case 440: case 441: 
                case 442: case 443: case 445: case 446: case 447: case 448: 
                case 449: case 450: case 451: case 452: case 453: case 454: 
                case 455: case 456: case 457: case 458: case 459: case 460: 
                case 461: case 462: case 463: case 464: case 465: case 466: 
                case 467: case 468: case 469: case 470: case 471: case 472: 
                case 473: case 474: case 475: case 537: case 538: case 793: 
                case 795: case 796: case 797: case 798: case 799: case 800: 
                case 801: case 802: case 803: case 804: case 805: case 806: 
                case 807: case 808: case 809: case 810: case 811: case 812: 
                case 813: case 814: case 815: case 816: case 817: case 818: 
                case 819: case 820: case 821: case 822: case 823: case 923: 
                case 924: case 925: case 926: case 927: case 928: case 929: 
                case 930: case 931: case 932: case 933: case 934: case 935: 
                case 936: case 937: case 938: case 939: case 940: case 941: 
                case 942: case 943: case 944: case 945: case 946: case 947: 
                case 948: case 949: case 950: case 951: case 952: case 953: 
                case 955: case 956: case 957: case 958: case 959: case 960: 
                case 961: case 962: case 963: case 964: case 965: case 966: 
                case 967: case 968: case 969: case 970: case 971: case 972: 
                case 973: case 974: case 975: case 976: case 977: case 978: 
                case 979: case 980: case 981: case 982: case 983: case 984: 
                case 985: 
                  goto MATCH_label_a0; break;
                case 24: case 27: case 28: case 60: case 124: case 284: 
                case 316: case 412: case 444: case 476: case 536: case 539: 
                case 792: case 794: 
                  MATCH_name = MATCH_name_Xo1_30[(MATCH_w_32_0 >> 1 & 0x3ff) 
                        /* Xo1 at 0 */]; 
                  goto MATCH_label_a8; 
                  
                  break;
                case 26: 
                  if ((MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */ == 0) { 
                    MATCH_name = "cntlzwq"; 
                    goto MATCH_label_a9; 
                    
                  } /*opt-block*/
                  else 
                    goto MATCH_label_a0;  /*opt-block+*/
                  
                  break;
                case 58: 
                  if ((MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */ == 0) { 
                    MATCH_name = "cntlzdq"; 
                    goto MATCH_label_a9; 
                    
                  } /*opt-block*/
                  else 
                    goto MATCH_label_a0;  /*opt-block+*/
                  
                  break;
                case 150: case 214: 
                  MATCH_name = MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                        /* Xo1 at 0 */]; 
                  goto MATCH_label_a10; 
                  
                  break;
                case 824: 
                  MATCH_name = MATCH_name_Xo1_30[(MATCH_w_32_0 >> 1 & 0x3ff) 
                        /* Xo1 at 0 */]; 
                  { 
                    const char *name = MATCH_name;
                    unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
                    unsigned rs = (MATCH_w_32_0 >> 21 & 0x1f) /* S at 0 */;
                    unsigned uimm = (MATCH_w_32_0 >> 11 & 0x1f) /* SH at 0 */;
                    nextPC = 4 + MATCH_p; 
                    
#line 350 "frontend/machine/ppc/decoder.m"
                    

                    		stmts = instantiate(pc,	 name, DIS_RA, DIS_RS, DIS_UIMM);

                    		

                    
                    
                    
                  }
                  
                  break;
                case 922: 
                  if ((MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */ == 0) { 
                    MATCH_name = "extshq"; 
                    goto MATCH_label_a9; 
                    
                  } /*opt-block*/
                  else 
                    goto MATCH_label_a0;  /*opt-block+*/
                  
                  break;
                case 954: 
                  if ((MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */ == 0) { 
                    MATCH_name = "extsbq"; 
                    goto MATCH_label_a9; 
                    
                  } /*opt-block*/
                  else 
                    goto MATCH_label_a0;  /*opt-block+*/
                  
                  break;
                case 986: 
                  if ((MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */ == 0) { 
                    MATCH_name = "extswq"; 
                    goto MATCH_label_a9; 
                    
                  } /*opt-block*/
                  else 
                    goto MATCH_label_a0;  /*opt-block+*/
                  
                  break;
                default: assert(0);
              } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/  
          else 
            if ((MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */ == 0) 
              if ((MATCH_w_32_0 >> 10 & 0x1) /* OE at 0 */ == 1) 
                if (477 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 533 || 
                  825 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 918 || 
                  987 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else 
                  switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                    case 0: 
                      if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
                        MATCH_name = "cmp"; 
                        goto MATCH_label_a5; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    case 1: case 2: case 3: case 4: case 5: case 6: case 7: 
                    case 8: case 9: case 10: case 11: case 12: case 13: 
                    case 14: case 15: case 16: case 17: case 18: case 22: 
                    case 25: case 29: case 30: case 31: case 33: case 34: 
                    case 35: case 36: case 37: case 38: case 39: case 40: 
                    case 41: case 42: case 43: case 44: case 45: case 46: 
                    case 47: case 48: case 49: case 50: case 51: case 52: 
                    case 54: case 56: case 57: case 59: case 61: case 62: 
                    case 63: case 64: case 65: case 66: case 67: case 68: 
                    case 69: case 70: case 71: case 72: case 73: case 74: 
                    case 75: case 76: case 77: case 78: case 79: case 80: 
                    case 81: case 82: case 85: case 86: case 88: case 89: 
                    case 90: case 91: case 92: case 93: case 94: case 95: 
                    case 96: case 97: case 98: case 99: case 100: case 101: 
                    case 102: case 103: case 104: case 105: case 106: 
                    case 107: case 108: case 109: case 110: case 111: 
                    case 112: case 113: case 114: case 115: case 116: 
                    case 117: case 118: case 120: case 121: case 122: 
                    case 123: case 125: case 126: case 127: case 128: 
                    case 129: case 130: case 131: case 132: case 133: 
                    case 134: case 135: case 136: case 137: case 138: 
                    case 139: case 140: case 141: case 142: case 143: 
                    case 144: case 145: case 146: case 147: case 148: 
                    case 150: case 152: case 153: case 154: case 155: 
                    case 156: case 157: case 158: case 159: case 160: 
                    case 161: case 162: case 163: case 164: case 165: 
                    case 166: case 167: case 168: case 169: case 170: 
                    case 171: case 172: case 173: case 174: case 175: 
                    case 176: case 177: case 178: case 179: case 180: 
                    case 182: case 184: case 185: case 186: case 187: 
                    case 188: case 189: case 190: case 191: case 192: 
                    case 193: case 194: case 195: case 196: case 197: 
                    case 198: case 199: case 200: case 201: case 202: 
                    case 203: case 204: case 205: case 206: case 207: 
                    case 208: case 209: case 210: case 211: case 212: 
                    case 213: case 214: case 216: case 217: case 218: 
                    case 219: case 220: case 221: case 222: case 223: 
                    case 224: case 225: case 226: case 227: case 228: 
                    case 229: case 230: case 231: case 232: case 233: 
                    case 234: case 235: case 236: case 237: case 238: 
                    case 239: case 240: case 241: case 242: case 243: 
                    case 244: case 245: case 246: case 248: case 249: 
                    case 250: case 251: case 252: case 253: case 254: 
                    case 255: case 256: case 257: case 258: case 259: 
                    case 260: case 261: case 262: case 263: case 264: 
                    case 265: case 266: case 267: case 268: case 269: 
                    case 270: case 271: case 272: case 273: case 274: 
                    case 275: case 276: case 277: case 278: case 280: 
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
                    case 374: case 376: case 377: case 378: case 379: 
                    case 380: case 381: case 382: case 383: case 384: 
                    case 385: case 386: case 387: case 388: case 389: 
                    case 390: case 391: case 392: case 393: case 394: 
                    case 395: case 396: case 397: case 398: case 399: 
                    case 400: case 401: case 402: case 403: case 404: 
                    case 405: case 406: case 408: case 409: case 410: 
                    case 411: case 413: case 414: case 415: case 416: 
                    case 417: case 418: case 419: case 420: case 421: 
                    case 422: case 423: case 424: case 425: case 426: 
                    case 427: case 428: case 429: case 430: case 431: 
                    case 432: case 433: case 434: case 435: case 436: 
                    case 437: case 440: case 441: case 442: case 443: 
                    case 445: case 446: case 447: case 448: case 449: 
                    case 450: case 451: case 452: case 453: case 454: 
                    case 455: case 456: case 457: case 458: case 459: 
                    case 460: case 461: case 462: case 463: case 464: 
                    case 465: case 466: case 468: case 469: case 470: 
                    case 471: case 472: case 473: case 474: case 475: 
                    case 537: case 538: case 540: case 541: case 542: 
                    case 543: case 544: case 545: case 546: case 547: 
                    case 548: case 549: case 550: case 551: case 552: 
                    case 553: case 554: case 555: case 556: case 557: 
                    case 558: case 559: case 560: case 561: case 562: 
                    case 563: case 564: case 565: case 566: case 568: 
                    case 569: case 570: case 571: case 572: case 573: 
                    case 574: case 575: case 576: case 577: case 578: 
                    case 579: case 580: case 581: case 582: case 583: 
                    case 584: case 585: case 586: case 587: case 588: 
                    case 589: case 590: case 591: case 592: case 593: 
                    case 594: case 595: case 596: case 597: case 598: 
                    case 600: case 601: case 602: case 603: case 604: 
                    case 605: case 606: case 607: case 608: case 609: 
                    case 610: case 611: case 612: case 613: case 614: 
                    case 615: case 616: case 617: case 618: case 619: 
                    case 620: case 621: case 622: case 623: case 624: 
                    case 625: case 626: case 627: case 628: case 629: 
                    case 630: case 632: case 633: case 634: case 635: 
                    case 636: case 637: case 638: case 639: case 640: 
                    case 641: case 642: case 643: case 644: case 645: 
                    case 646: case 647: case 648: case 649: case 650: 
                    case 651: case 652: case 653: case 654: case 655: 
                    case 656: case 657: case 658: case 659: case 660: 
                    case 664: case 665: case 666: case 667: case 668: 
                    case 669: case 670: case 671: case 672: case 673: 
                    case 674: case 675: case 676: case 677: case 678: 
                    case 679: case 680: case 681: case 682: case 683: 
                    case 684: case 685: case 686: case 687: case 688: 
                    case 689: case 690: case 691: case 692: case 693: 
                    case 694: case 696: case 697: case 698: case 699: 
                    case 700: case 701: case 702: case 703: case 704: 
                    case 705: case 706: case 707: case 708: case 709: 
                    case 710: case 711: case 712: case 713: case 714: 
                    case 715: case 716: case 717: case 718: case 719: 
                    case 720: case 721: case 722: case 723: case 724: 
                    case 725: case 726: case 728: case 729: case 730: 
                    case 731: case 732: case 733: case 734: case 735: 
                    case 736: case 737: case 738: case 739: case 740: 
                    case 741: case 742: case 743: case 744: case 745: 
                    case 746: case 747: case 748: case 749: case 750: 
                    case 751: case 752: case 753: case 754: case 755: 
                    case 756: case 757: case 758: case 760: case 761: 
                    case 762: case 763: case 764: case 765: case 766: 
                    case 767: case 768: case 769: case 770: case 771: 
                    case 772: case 773: case 774: case 775: case 776: 
                    case 777: case 778: case 779: case 780: case 781: 
                    case 782: case 783: case 784: case 785: case 786: 
                    case 787: case 788: case 789: case 791: case 793: 
                    case 795: case 796: case 797: case 798: case 799: 
                    case 800: case 801: case 802: case 803: case 804: 
                    case 805: case 806: case 807: case 808: case 809: 
                    case 810: case 811: case 812: case 813: case 814: 
                    case 815: case 816: case 817: case 818: case 819: 
                    case 820: case 821: case 822: case 823: case 919: 
                    case 920: case 921: case 923: case 924: case 925: 
                    case 926: case 927: case 928: case 929: case 930: 
                    case 931: case 932: case 933: case 934: case 935: 
                    case 936: case 937: case 938: case 939: case 940: 
                    case 941: case 942: case 943: case 944: case 945: 
                    case 946: case 947: case 948: case 949: case 950: 
                    case 951: case 952: case 953: case 955: case 956: 
                    case 957: case 958: case 959: case 960: case 961: 
                    case 962: case 963: case 964: case 965: case 966: 
                    case 967: case 968: case 969: case 970: case 971: 
                    case 972: case 973: case 974: case 975: case 976: 
                    case 977: case 978: case 979: case 980: case 981: 
                    case 982: case 984: case 985: 
                      goto MATCH_label_a0; break;
                    case 19: 
                      if ((MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */ == 0) { 
                        MATCH_name = "mfcr"; 
                        goto MATCH_label_a6; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    case 20: case 21: case 23: case 53: case 55: case 84: 
                    case 87: case 119: case 279: case 310: case 311: 
                    case 341: case 343: case 373: case 375: case 533: 
                    case 534: case 790: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a7; 
                      
                      break;
                    case 24: case 27: case 28: case 60: case 124: case 284: 
                    case 316: case 412: case 444: case 476: case 536: 
                    case 539: case 792: case 794: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a8; 
                      
                      break;
                    case 26: case 58: case 922: case 954: case 986: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a9; 
                      
                      break;
                    case 32: 
                      if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
                        MATCH_name = "cmpl"; 
                        goto MATCH_label_a5; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    case 83: 
                      if ((MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */ == 0) { 
                        MATCH_name = "mfmsr"; 
                        goto MATCH_label_a6; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    case 149: case 151: case 181: case 183: case 215: 
                    case 247: case 407: case 438: case 439: case 661: 
                    case 662: case 918: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a10; 
                      
                      break;
                    case 339: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a11; 
                      
                      break;
                    case 467: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a12; 
                      
                      break;
                    case 535: case 567: case 599: case 631: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a13; 
                      
                      break;
                    case 663: case 695: case 727: case 759: case 983: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a14; 
                      
                      break;
                    case 824: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a15; 
                      
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
                  if (477 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                    (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 533 || 
                    825 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                    (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 918 || 
                    987 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                    (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                    goto MATCH_label_a0;  /*opt-block+*/
                  else 
                    switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                      case 0: 
                        if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
                          MATCH_name = "cmp"; 
                          goto MATCH_label_a5; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a0;  /*opt-block+*/
                        
                        break;
                      case 1: case 2: case 3: case 4: case 5: case 6: case 7: 
                      case 8: case 9: case 10: case 11: case 12: case 13: 
                      case 14: case 15: case 16: case 17: case 18: case 22: 
                      case 25: case 29: case 30: case 31: case 33: case 34: 
                      case 35: case 36: case 37: case 38: case 39: case 40: 
                      case 41: case 42: case 43: case 44: case 45: case 46: 
                      case 47: case 48: case 49: case 50: case 51: case 52: 
                      case 54: case 56: case 57: case 59: case 61: case 62: 
                      case 63: case 64: case 65: case 66: case 67: case 68: 
                      case 69: case 70: case 71: case 72: case 73: case 74: 
                      case 75: case 76: case 77: case 78: case 79: case 80: 
                      case 81: case 82: case 85: case 86: case 88: case 89: 
                      case 90: case 91: case 92: case 93: case 94: case 95: 
                      case 96: case 97: case 98: case 99: case 100: case 101: 
                      case 102: case 103: case 104: case 105: case 106: 
                      case 107: case 108: case 109: case 110: case 111: 
                      case 112: case 113: case 114: case 115: case 116: 
                      case 117: case 118: case 120: case 121: case 122: 
                      case 123: case 125: case 126: case 127: case 128: 
                      case 129: case 130: case 131: case 132: case 133: 
                      case 134: case 135: case 136: case 137: case 138: 
                      case 139: case 140: case 141: case 142: case 143: 
                      case 144: case 145: case 146: case 147: case 148: 
                      case 150: case 152: case 153: case 154: case 155: 
                      case 156: case 157: case 158: case 159: case 160: 
                      case 161: case 162: case 163: case 164: case 165: 
                      case 166: case 167: case 168: case 169: case 170: 
                      case 171: case 172: case 173: case 174: case 175: 
                      case 176: case 177: case 178: case 179: case 180: 
                      case 182: case 184: case 185: case 186: case 187: 
                      case 188: case 189: case 190: case 191: case 192: 
                      case 193: case 194: case 195: case 196: case 197: 
                      case 198: case 199: case 200: case 201: case 202: 
                      case 203: case 204: case 205: case 206: case 207: 
                      case 208: case 209: case 210: case 211: case 212: 
                      case 213: case 214: case 216: case 217: case 218: 
                      case 219: case 220: case 221: case 222: case 223: 
                      case 224: case 225: case 226: case 227: case 228: 
                      case 229: case 230: case 231: case 232: case 233: 
                      case 234: case 235: case 236: case 237: case 238: 
                      case 239: case 240: case 241: case 242: case 243: 
                      case 244: case 245: case 246: case 248: case 249: 
                      case 250: case 251: case 252: case 253: case 254: 
                      case 255: case 256: case 257: case 258: case 259: 
                      case 260: case 261: case 262: case 263: case 264: 
                      case 265: case 266: case 267: case 268: case 269: 
                      case 270: case 271: case 272: case 273: case 274: 
                      case 275: case 276: case 277: case 278: case 280: 
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
                      case 374: case 376: case 377: case 378: case 379: 
                      case 380: case 381: case 382: case 383: case 384: 
                      case 385: case 386: case 387: case 388: case 389: 
                      case 390: case 391: case 392: case 393: case 394: 
                      case 395: case 396: case 397: case 398: case 399: 
                      case 400: case 401: case 402: case 403: case 404: 
                      case 405: case 406: case 408: case 409: case 410: 
                      case 411: case 413: case 414: case 415: case 416: 
                      case 417: case 418: case 419: case 420: case 421: 
                      case 422: case 423: case 424: case 425: case 426: 
                      case 427: case 428: case 429: case 430: case 431: 
                      case 432: case 433: case 434: case 435: case 436: 
                      case 437: case 440: case 441: case 442: case 443: 
                      case 445: case 446: case 447: case 448: case 449: 
                      case 450: case 451: case 452: case 453: case 454: 
                      case 455: case 456: case 457: case 458: case 459: 
                      case 460: case 461: case 462: case 463: case 464: 
                      case 465: case 466: case 468: case 469: case 470: 
                      case 471: case 472: case 473: case 474: case 475: 
                      case 537: case 538: case 540: case 541: case 542: 
                      case 543: case 544: case 545: case 546: case 547: 
                      case 548: case 549: case 550: case 551: case 552: 
                      case 553: case 554: case 555: case 556: case 557: 
                      case 558: case 559: case 560: case 561: case 562: 
                      case 563: case 564: case 565: case 566: case 568: 
                      case 569: case 570: case 571: case 572: case 573: 
                      case 574: case 575: case 576: case 577: case 578: 
                      case 579: case 580: case 581: case 582: case 583: 
                      case 584: case 585: case 586: case 587: case 588: 
                      case 589: case 590: case 591: case 592: case 593: 
                      case 594: case 595: case 596: case 597: case 598: 
                      case 600: case 601: case 602: case 603: case 604: 
                      case 605: case 606: case 607: case 608: case 609: 
                      case 610: case 611: case 612: case 613: case 614: 
                      case 615: case 616: case 617: case 618: case 619: 
                      case 620: case 621: case 622: case 623: case 624: 
                      case 625: case 626: case 627: case 628: case 629: 
                      case 630: case 632: case 633: case 634: case 635: 
                      case 636: case 637: case 638: case 639: case 640: 
                      case 641: case 642: case 643: case 644: case 645: 
                      case 646: case 647: case 648: case 649: case 650: 
                      case 651: case 652: case 653: case 654: case 655: 
                      case 656: case 657: case 658: case 659: case 660: 
                      case 664: case 665: case 666: case 667: case 668: 
                      case 669: case 670: case 671: case 672: case 673: 
                      case 674: case 675: case 676: case 677: case 678: 
                      case 679: case 680: case 681: case 682: case 683: 
                      case 684: case 685: case 686: case 687: case 688: 
                      case 689: case 690: case 691: case 692: case 693: 
                      case 694: case 696: case 697: case 698: case 699: 
                      case 700: case 701: case 702: case 703: case 704: 
                      case 705: case 706: case 707: case 708: case 709: 
                      case 710: case 711: case 712: case 713: case 714: 
                      case 715: case 716: case 717: case 718: case 719: 
                      case 720: case 721: case 722: case 723: case 724: 
                      case 725: case 726: case 728: case 729: case 730: 
                      case 731: case 732: case 733: case 734: case 735: 
                      case 736: case 737: case 738: case 739: case 740: 
                      case 741: case 742: case 743: case 744: case 745: 
                      case 746: case 747: case 748: case 749: case 750: 
                      case 751: case 752: case 753: case 754: case 755: 
                      case 756: case 757: case 758: case 760: case 761: 
                      case 762: case 763: case 764: case 765: case 766: 
                      case 767: case 768: case 769: case 770: case 771: 
                      case 772: case 773: case 774: case 775: case 776: 
                      case 777: case 778: case 779: case 780: case 781: 
                      case 782: case 783: case 784: case 785: case 786: 
                      case 787: case 788: case 789: case 791: case 793: 
                      case 795: case 796: case 797: case 798: case 799: 
                      case 800: case 801: case 802: case 803: case 804: 
                      case 805: case 806: case 807: case 808: case 809: 
                      case 810: case 811: case 812: case 813: case 814: 
                      case 815: case 816: case 817: case 818: case 819: 
                      case 820: case 821: case 822: case 823: case 919: 
                      case 920: case 921: case 923: case 924: case 925: 
                      case 926: case 927: case 928: case 929: case 930: 
                      case 931: case 932: case 933: case 934: case 935: 
                      case 936: case 937: case 938: case 939: case 940: 
                      case 941: case 942: case 943: case 944: case 945: 
                      case 946: case 947: case 948: case 949: case 950: 
                      case 951: case 952: case 953: case 955: case 956: 
                      case 957: case 958: case 959: case 960: case 961: 
                      case 962: case 963: case 964: case 965: case 966: 
                      case 967: case 968: case 969: case 970: case 971: 
                      case 972: case 973: case 974: case 975: case 976: 
                      case 977: case 978: case 979: case 980: case 981: 
                      case 982: case 984: case 985: 
                        goto MATCH_label_a0; break;
                      case 19: 
                        if ((MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */ == 0) { 
                          MATCH_name = "mfcr"; 
                          goto MATCH_label_a6; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a0;  /*opt-block+*/
                        
                        break;
                      case 20: case 21: case 23: case 53: case 55: case 84: 
                      case 87: case 119: case 279: case 310: case 311: 
                      case 341: case 343: case 373: case 375: case 533: 
                      case 534: case 790: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a7; 
                        
                        break;
                      case 24: case 27: case 28: case 60: case 124: case 284: 
                      case 316: case 412: case 444: case 476: case 536: 
                      case 539: case 792: case 794: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a8; 
                        
                        break;
                      case 26: case 58: case 922: case 954: case 986: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a9; 
                        
                        break;
                      case 32: 
                        if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
                          MATCH_name = "cmpl"; 
                          goto MATCH_label_a5; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a0;  /*opt-block+*/
                        
                        break;
                      case 83: 
                        if ((MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */ == 0) { 
                          MATCH_name = "mfmsr"; 
                          goto MATCH_label_a6; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a0;  /*opt-block+*/
                        
                        break;
                      case 149: case 151: case 181: case 183: case 215: 
                      case 247: case 407: case 438: case 439: case 661: 
                      case 662: case 918: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a10; 
                        
                        break;
                      case 339: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a11; 
                        
                        break;
                      case 467: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a12; 
                        
                        break;
                      case 535: case 567: case 599: case 631: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a13; 
                        
                        break;
                      case 663: case 695: case 727: case 759: case 983: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a14; 
                        
                        break;
                      case 824: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a15; 
                        
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
                    case 39: case 105: case 106: case 107: case 108: 
                    case 109: case 110: case 111: case 112: case 113: 
                    case 114: case 115: case 116: case 117: case 118: 
                    case 119: case 120: case 121: case 122: case 123: 
                    case 124: case 125: case 126: case 127: case 128: 
                    case 129: case 130: case 131: case 132: case 133: 
                    case 134: case 135: case 137: case 201: case 203: 
                    case 204: case 205: case 206: case 207: case 208: 
                    case 209: case 210: case 211: case 212: case 213: 
                    case 214: case 215: case 216: case 217: case 218: 
                    case 219: case 220: case 221: case 222: case 223: 
                    case 224: case 225: case 226: case 227: case 228: 
                    case 229: case 230: case 231: case 236: case 237: 
                    case 238: case 239: case 240: case 241: case 242: 
                    case 243: case 244: case 245: case 246: case 247: 
                    case 248: case 249: case 250: case 251: case 252: 
                    case 253: case 254: case 255: case 256: case 257: 
                    case 258: case 259: case 260: case 261: case 262: 
                    case 263: case 264: case 265: case 458: case 460: 
                    case 461: case 462: case 463: case 464: case 465: 
                    case 466: case 467: case 468: case 469: case 470: 
                    case 471: case 472: case 473: case 474: case 475: 
                    case 476: case 477: case 478: case 479: case 480: 
                    case 481: case 482: case 483: case 484: case 485: 
                    case 486: case 487: case 488: case 490: case 492: 
                    case 493: case 494: case 495: case 496: case 497: 
                    case 498: case 499: case 500: case 501: case 502: 
                    case 503: case 504: case 505: case 506: case 507: 
                    case 508: case 509: case 510: case 511: 
                      if (477 <= (MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 533 || 
                        825 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 918 || 
                        987 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                        goto MATCH_label_a0;  /*opt-block+*/
                      else 
                        switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                          case 0: 
                            if ((MATCH_w_32_0 >> 22 & 0x1) 
                                    /* Lz at 0 */ == 0) { 
                              MATCH_name = "cmp"; 
                              goto MATCH_label_a5; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 1: case 2: case 3: case 4: case 5: case 6: 
                          case 7: case 8: case 9: case 10: case 11: case 12: 
                          case 13: case 14: case 15: case 16: case 17: 
                          case 18: case 22: case 25: case 29: case 30: 
                          case 31: case 33: case 34: case 35: case 36: 
                          case 37: case 38: case 39: case 40: case 41: 
                          case 42: case 43: case 44: case 45: case 46: 
                          case 47: case 48: case 49: case 50: case 51: 
                          case 52: case 54: case 56: case 57: case 59: 
                          case 61: case 62: case 63: case 64: case 65: 
                          case 66: case 67: case 68: case 69: case 70: 
                          case 71: case 72: case 73: case 74: case 75: 
                          case 76: case 77: case 78: case 79: case 80: 
                          case 81: case 82: case 85: case 86: case 88: 
                          case 89: case 90: case 91: case 92: case 93: 
                          case 94: case 95: case 96: case 97: case 98: 
                          case 99: case 100: case 101: case 102: case 103: 
                          case 104: case 105: case 106: case 107: case 108: 
                          case 109: case 110: case 111: case 112: case 113: 
                          case 114: case 115: case 116: case 117: case 118: 
                          case 120: case 121: case 122: case 123: case 125: 
                          case 126: case 127: case 128: case 129: case 130: 
                          case 131: case 132: case 133: case 134: case 135: 
                          case 136: case 137: case 138: case 139: case 140: 
                          case 141: case 142: case 143: case 144: case 145: 
                          case 146: case 147: case 148: case 150: case 152: 
                          case 153: case 154: case 155: case 156: case 157: 
                          case 158: case 159: case 160: case 161: case 162: 
                          case 163: case 164: case 165: case 166: case 167: 
                          case 168: case 169: case 170: case 171: case 172: 
                          case 173: case 174: case 175: case 176: case 177: 
                          case 178: case 179: case 180: case 182: case 184: 
                          case 185: case 186: case 187: case 188: case 189: 
                          case 190: case 191: case 192: case 193: case 194: 
                          case 195: case 196: case 197: case 198: case 199: 
                          case 200: case 201: case 202: case 203: case 204: 
                          case 205: case 206: case 207: case 208: case 209: 
                          case 210: case 211: case 212: case 213: case 214: 
                          case 216: case 217: case 218: case 219: case 220: 
                          case 221: case 222: case 223: case 224: case 225: 
                          case 226: case 227: case 228: case 229: case 230: 
                          case 231: case 232: case 233: case 234: case 235: 
                          case 236: case 237: case 238: case 239: case 240: 
                          case 241: case 242: case 243: case 244: case 245: 
                          case 246: case 248: case 249: case 250: case 251: 
                          case 252: case 253: case 254: case 255: case 256: 
                          case 257: case 258: case 259: case 260: case 261: 
                          case 262: case 263: case 264: case 265: case 266: 
                          case 267: case 268: case 269: case 270: case 271: 
                          case 272: case 273: case 274: case 275: case 276: 
                          case 277: case 278: case 280: case 281: case 282: 
                          case 283: case 285: case 286: case 287: case 288: 
                          case 289: case 290: case 291: case 292: case 293: 
                          case 294: case 295: case 296: case 297: case 298: 
                          case 299: case 300: case 301: case 302: case 303: 
                          case 304: case 305: case 306: case 307: case 308: 
                          case 309: case 312: case 313: case 314: case 315: 
                          case 317: case 318: case 319: case 320: case 321: 
                          case 322: case 323: case 324: case 325: case 326: 
                          case 327: case 328: case 329: case 330: case 331: 
                          case 332: case 333: case 334: case 335: case 336: 
                          case 337: case 338: case 340: case 342: case 344: 
                          case 345: case 346: case 347: case 348: case 349: 
                          case 350: case 351: case 352: case 353: case 354: 
                          case 355: case 356: case 357: case 358: case 359: 
                          case 360: case 361: case 362: case 363: case 364: 
                          case 365: case 366: case 367: case 368: case 369: 
                          case 370: case 371: case 372: case 374: case 376: 
                          case 377: case 378: case 379: case 380: case 381: 
                          case 382: case 383: case 384: case 385: case 386: 
                          case 387: case 388: case 389: case 390: case 391: 
                          case 392: case 393: case 394: case 395: case 396: 
                          case 397: case 398: case 399: case 400: case 401: 
                          case 402: case 403: case 404: case 405: case 406: 
                          case 408: case 409: case 410: case 411: case 413: 
                          case 414: case 415: case 416: case 417: case 418: 
                          case 419: case 420: case 421: case 422: case 423: 
                          case 424: case 425: case 426: case 427: case 428: 
                          case 429: case 430: case 431: case 432: case 433: 
                          case 434: case 435: case 436: case 437: case 440: 
                          case 441: case 442: case 443: case 445: case 446: 
                          case 447: case 448: case 449: case 450: case 451: 
                          case 452: case 453: case 454: case 455: case 456: 
                          case 457: case 458: case 459: case 460: case 461: 
                          case 462: case 463: case 464: case 465: case 466: 
                          case 468: case 469: case 470: case 471: case 472: 
                          case 473: case 474: case 475: case 537: case 538: 
                          case 540: case 541: case 542: case 543: case 544: 
                          case 545: case 546: case 547: case 548: case 549: 
                          case 550: case 551: case 552: case 553: case 554: 
                          case 555: case 556: case 557: case 558: case 559: 
                          case 560: case 561: case 562: case 563: case 564: 
                          case 565: case 566: case 568: case 569: case 570: 
                          case 571: case 572: case 573: case 574: case 575: 
                          case 576: case 577: case 578: case 579: case 580: 
                          case 581: case 582: case 583: case 584: case 585: 
                          case 586: case 587: case 588: case 589: case 590: 
                          case 591: case 592: case 593: case 594: case 595: 
                          case 596: case 597: case 598: case 600: case 601: 
                          case 602: case 603: case 604: case 605: case 606: 
                          case 607: case 608: case 609: case 610: case 611: 
                          case 612: case 613: case 614: case 615: case 616: 
                          case 617: case 618: case 619: case 620: case 621: 
                          case 622: case 623: case 624: case 625: case 626: 
                          case 627: case 628: case 629: case 630: case 632: 
                          case 633: case 634: case 635: case 636: case 637: 
                          case 638: case 639: case 640: case 641: case 642: 
                          case 643: case 644: case 645: case 646: case 647: 
                          case 648: case 649: case 650: case 651: case 652: 
                          case 653: case 654: case 655: case 656: case 657: 
                          case 658: case 659: case 660: case 664: case 665: 
                          case 666: case 667: case 668: case 669: case 670: 
                          case 671: case 672: case 673: case 674: case 675: 
                          case 676: case 677: case 678: case 679: case 680: 
                          case 681: case 682: case 683: case 684: case 685: 
                          case 686: case 687: case 688: case 689: case 690: 
                          case 691: case 692: case 693: case 694: case 696: 
                          case 697: case 698: case 699: case 700: case 701: 
                          case 702: case 703: case 704: case 705: case 706: 
                          case 707: case 708: case 709: case 710: case 711: 
                          case 712: case 713: case 714: case 715: case 716: 
                          case 717: case 718: case 719: case 720: case 721: 
                          case 722: case 723: case 724: case 725: case 726: 
                          case 728: case 729: case 730: case 731: case 732: 
                          case 733: case 734: case 735: case 736: case 737: 
                          case 738: case 739: case 740: case 741: case 742: 
                          case 743: case 744: case 745: case 746: case 747: 
                          case 748: case 749: case 750: case 751: case 752: 
                          case 753: case 754: case 755: case 756: case 757: 
                          case 758: case 760: case 761: case 762: case 763: 
                          case 764: case 765: case 766: case 767: case 768: 
                          case 769: case 770: case 771: case 772: case 773: 
                          case 774: case 775: case 776: case 777: case 778: 
                          case 779: case 780: case 781: case 782: case 783: 
                          case 784: case 785: case 786: case 787: case 788: 
                          case 789: case 791: case 793: case 795: case 796: 
                          case 797: case 798: case 799: case 800: case 801: 
                          case 802: case 803: case 804: case 805: case 806: 
                          case 807: case 808: case 809: case 810: case 811: 
                          case 812: case 813: case 814: case 815: case 816: 
                          case 817: case 818: case 819: case 820: case 821: 
                          case 822: case 823: case 919: case 920: case 921: 
                          case 923: case 924: case 925: case 926: case 927: 
                          case 928: case 929: case 930: case 931: case 932: 
                          case 933: case 934: case 935: case 936: case 937: 
                          case 938: case 939: case 940: case 941: case 942: 
                          case 943: case 944: case 945: case 946: case 947: 
                          case 948: case 949: case 950: case 951: case 952: 
                          case 953: case 955: case 956: case 957: case 958: 
                          case 959: case 960: case 961: case 962: case 963: 
                          case 964: case 965: case 966: case 967: case 968: 
                          case 969: case 970: case 971: case 972: case 973: 
                          case 974: case 975: case 976: case 977: case 978: 
                          case 979: case 980: case 981: case 982: case 984: 
                          case 985: 
                            goto MATCH_label_a0; break;
                          case 19: 
                            if ((MATCH_w_32_0 >> 16 & 0x1f) 
                                    /* A at 0 */ == 0) { 
                              MATCH_name = "mfcr"; 
                              goto MATCH_label_a6; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 20: case 21: case 23: case 53: case 55: 
                          case 84: case 87: case 119: case 279: case 310: 
                          case 311: case 341: case 343: case 373: case 375: 
                          case 533: case 534: case 790: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a7; 
                            
                            break;
                          case 24: case 27: case 28: case 60: case 124: 
                          case 284: case 316: case 412: case 444: case 476: 
                          case 536: case 539: case 792: case 794: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a8; 
                            
                            break;
                          case 26: case 58: case 922: case 954: case 986: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a9; 
                            
                            break;
                          case 32: 
                            if ((MATCH_w_32_0 >> 22 & 0x1) 
                                    /* Lz at 0 */ == 0) { 
                              MATCH_name = "cmpl"; 
                              goto MATCH_label_a5; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 83: 
                            if ((MATCH_w_32_0 >> 16 & 0x1f) 
                                    /* A at 0 */ == 0) { 
                              MATCH_name = "mfmsr"; 
                              goto MATCH_label_a6; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 149: case 151: case 181: case 183: case 215: 
                          case 247: case 407: case 438: case 439: case 661: 
                          case 662: case 918: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a10; 
                            
                            break;
                          case 339: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a11; 
                            
                            break;
                          case 467: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a12; 
                            
                            break;
                          case 535: case 567: case 599: case 631: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a13; 
                            
                            break;
                          case 663: case 695: case 727: case 759: case 983: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a14; 
                            
                            break;
                          case 824: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a15; 
                            
                            break;
                          default: assert(0);
                        } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/ 
                      break;
                    case 8: case 10: case 40: case 136: case 138: case 233: 
                    case 235: case 266: case 457: case 459: case 489: 
                    case 491: 
                      MATCH_name = 
                        MATCH_name_Xo9_29[(MATCH_w_32_0 >> 1 & 0x1ff) 
                            /* Xo9 at 0 */]; 
                      goto MATCH_label_a16; 
                      
                      break;
                    case 104: case 200: case 202: case 232: case 234: 
                      MATCH_name = 
                        MATCH_name_Xo9_29[(MATCH_w_32_0 >> 1 & 0x1ff) 
                            /* Xo9 at 0 */]; 
                      { 
                        const char *name = MATCH_name;
                        unsigned ra = 
                          (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
                        unsigned rd = 
                          (MATCH_w_32_0 >> 21 & 0x1f) /* D at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 122 "frontend/machine/ppc/decoder.m"
                        

                        		stmts = instantiate(pc, name, DIS_RD, DIS_RA);

                        
                        
                        
                      }
                      
                      break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 1 & 0x1ff) -- Xo9 at 0 --*/   
            else 
              if ((MATCH_w_32_0 >> 10 & 0x1) /* OE at 0 */ == 1) 
                if (477 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 533 || 
                  825 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 918 || 
                  919 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 983 || 
                  984 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else 
                  switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                    case 0: 
                      if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
                        MATCH_name = "cmp"; 
                        goto MATCH_label_a5; 
                        
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
                    case 120: case 121: case 122: case 123: case 125: 
                    case 126: case 127: case 128: case 129: case 130: 
                    case 131: case 132: case 133: case 134: case 135: 
                    case 136: case 137: case 138: case 139: case 140: 
                    case 141: case 142: case 143: case 144: case 145: 
                    case 146: case 147: case 148: case 150: case 152: 
                    case 153: case 154: case 155: case 156: case 157: 
                    case 158: case 159: case 160: case 161: case 162: 
                    case 163: case 164: case 165: case 166: case 167: 
                    case 168: case 169: case 170: case 171: case 172: 
                    case 173: case 174: case 175: case 176: case 177: 
                    case 178: case 179: case 180: case 182: case 184: 
                    case 185: case 186: case 187: case 188: case 189: 
                    case 190: case 191: case 192: case 193: case 194: 
                    case 195: case 196: case 197: case 198: case 199: 
                    case 200: case 201: case 202: case 203: case 204: 
                    case 205: case 206: case 207: case 208: case 209: 
                    case 210: case 211: case 212: case 213: case 214: 
                    case 216: case 217: case 218: case 219: case 220: 
                    case 221: case 222: case 223: case 224: case 225: 
                    case 226: case 227: case 228: case 229: case 230: 
                    case 231: case 232: case 233: case 234: case 235: 
                    case 236: case 237: case 238: case 239: case 240: 
                    case 241: case 242: case 243: case 244: case 245: 
                    case 246: case 248: case 249: case 250: case 251: 
                    case 252: case 253: case 254: case 255: case 256: 
                    case 257: case 258: case 259: case 260: case 261: 
                    case 262: case 263: case 264: case 265: case 266: 
                    case 267: case 268: case 269: case 270: case 271: 
                    case 272: case 273: case 274: case 275: case 276: 
                    case 277: case 278: case 280: case 281: case 282: 
                    case 283: case 285: case 286: case 287: case 288: 
                    case 289: case 290: case 291: case 292: case 293: 
                    case 294: case 295: case 296: case 297: case 298: 
                    case 299: case 300: case 301: case 302: case 303: 
                    case 304: case 305: case 306: case 307: case 308: 
                    case 309: case 312: case 313: case 314: case 315: 
                    case 317: case 318: case 319: case 320: case 321: 
                    case 322: case 323: case 324: case 325: case 326: 
                    case 327: case 328: case 329: case 330: case 331: 
                    case 332: case 333: case 334: case 335: case 336: 
                    case 337: case 338: case 340: case 342: case 344: 
                    case 345: case 346: case 347: case 348: case 349: 
                    case 350: case 351: case 352: case 353: case 354: 
                    case 355: case 356: case 357: case 358: case 359: 
                    case 360: case 361: case 362: case 363: case 364: 
                    case 365: case 366: case 367: case 368: case 369: 
                    case 370: case 371: case 372: case 374: case 376: 
                    case 377: case 378: case 379: case 380: case 381: 
                    case 382: case 383: case 384: case 385: case 386: 
                    case 387: case 388: case 389: case 390: case 391: 
                    case 392: case 393: case 394: case 395: case 396: 
                    case 397: case 398: case 399: case 400: case 401: 
                    case 402: case 403: case 404: case 405: case 406: 
                    case 408: case 409: case 410: case 411: case 413: 
                    case 414: case 415: case 416: case 417: case 418: 
                    case 419: case 420: case 421: case 422: case 423: 
                    case 424: case 425: case 426: case 427: case 428: 
                    case 429: case 430: case 431: case 432: case 433: 
                    case 434: case 435: case 436: case 437: case 440: 
                    case 441: case 442: case 443: case 445: case 446: 
                    case 447: case 448: case 449: case 450: case 451: 
                    case 452: case 453: case 454: case 455: case 456: 
                    case 457: case 458: case 459: case 460: case 461: 
                    case 462: case 463: case 464: case 465: case 466: 
                    case 468: case 469: case 470: case 471: case 472: 
                    case 473: case 474: case 475: case 537: case 538: 
                    case 540: case 541: case 542: case 543: case 544: 
                    case 545: case 546: case 547: case 548: case 549: 
                    case 550: case 551: case 552: case 553: case 554: 
                    case 555: case 556: case 557: case 558: case 559: 
                    case 560: case 561: case 562: case 563: case 564: 
                    case 565: case 566: case 568: case 569: case 570: 
                    case 571: case 572: case 573: case 574: case 575: 
                    case 576: case 577: case 578: case 579: case 580: 
                    case 581: case 582: case 583: case 584: case 585: 
                    case 586: case 587: case 588: case 589: case 590: 
                    case 591: case 592: case 593: case 594: case 595: 
                    case 596: case 597: case 598: case 600: case 601: 
                    case 602: case 603: case 604: case 605: case 606: 
                    case 607: case 608: case 609: case 610: case 611: 
                    case 612: case 613: case 614: case 615: case 616: 
                    case 617: case 618: case 619: case 620: case 621: 
                    case 622: case 623: case 624: case 625: case 626: 
                    case 627: case 628: case 629: case 630: case 632: 
                    case 633: case 634: case 635: case 636: case 637: 
                    case 638: case 639: case 640: case 641: case 642: 
                    case 643: case 644: case 645: case 646: case 647: 
                    case 648: case 649: case 650: case 651: case 652: 
                    case 653: case 654: case 655: case 656: case 657: 
                    case 658: case 659: case 660: case 664: case 665: 
                    case 666: case 667: case 668: case 669: case 670: 
                    case 671: case 672: case 673: case 674: case 675: 
                    case 676: case 677: case 678: case 679: case 680: 
                    case 681: case 682: case 683: case 684: case 685: 
                    case 686: case 687: case 688: case 689: case 690: 
                    case 691: case 692: case 693: case 694: case 696: 
                    case 697: case 698: case 699: case 700: case 701: 
                    case 702: case 703: case 704: case 705: case 706: 
                    case 707: case 708: case 709: case 710: case 711: 
                    case 712: case 713: case 714: case 715: case 716: 
                    case 717: case 718: case 719: case 720: case 721: 
                    case 722: case 723: case 724: case 725: case 726: 
                    case 728: case 729: case 730: case 731: case 732: 
                    case 733: case 734: case 735: case 736: case 737: 
                    case 738: case 739: case 740: case 741: case 742: 
                    case 743: case 744: case 745: case 746: case 747: 
                    case 748: case 749: case 750: case 751: case 752: 
                    case 753: case 754: case 755: case 756: case 757: 
                    case 758: case 760: case 761: case 762: case 763: 
                    case 764: case 765: case 766: case 767: case 768: 
                    case 769: case 770: case 771: case 772: case 773: 
                    case 774: case 775: case 776: case 777: case 778: 
                    case 779: case 780: case 781: case 782: case 783: 
                    case 784: case 785: case 786: case 787: case 788: 
                    case 789: case 791: case 793: case 795: case 796: 
                    case 797: case 798: case 799: case 800: case 801: 
                    case 802: case 803: case 804: case 805: case 806: 
                    case 807: case 808: case 809: case 810: case 811: 
                    case 812: case 813: case 814: case 815: case 816: 
                    case 817: case 818: case 819: case 820: case 821: 
                    case 822: case 823: 
                      goto MATCH_label_a0; break;
                    case 20: case 21: case 23: case 53: case 55: case 84: 
                    case 87: case 119: case 279: case 310: case 311: 
                    case 341: case 343: case 373: case 375: case 533: 
                    case 534: case 790: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a7; 
                      
                      break;
                    case 24: case 27: case 28: case 60: case 124: case 284: 
                    case 316: case 412: case 444: case 476: case 536: 
                    case 539: case 792: case 794: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a8; 
                      
                      break;
                    case 32: 
                      if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
                        MATCH_name = "cmpl"; 
                        goto MATCH_label_a5; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    case 149: case 151: case 181: case 183: case 215: 
                    case 247: case 407: case 438: case 439: case 661: 
                    case 662: case 918: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a10; 
                      
                      break;
                    case 339: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a11; 
                      
                      break;
                    case 467: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a12; 
                      
                      break;
                    case 535: case 567: case 599: case 631: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a13; 
                      
                      break;
                    case 663: case 695: case 727: case 759: case 983: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a14; 
                      
                      break;
                    case 824: 
                      MATCH_name = 
                        MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                            /* Xo1 at 0 */]; 
                      goto MATCH_label_a15; 
                      
                      break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/  
              else 
                if (41 <= (MATCH_w_32_0 >> 1 & 0x1ff) /* Xo9 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x1ff) /* Xo9 at 0 */ < 136 || 
                  139 <= (MATCH_w_32_0 >> 1 & 0x1ff) /* Xo9 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x1ff) /* Xo9 at 0 */ < 233 || 
                  267 <= (MATCH_w_32_0 >> 1 & 0x1ff) /* Xo9 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x1ff) /* Xo9 at 0 */ < 457) 
                  if (477 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                    (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 533 || 
                    825 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                    (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 918 || 
                    919 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                    (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 983 || 
                    984 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                    (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                    goto MATCH_label_a0;  /*opt-block+*/
                  else 
                    switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                      case 0: 
                        if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
                          MATCH_name = "cmp"; 
                          goto MATCH_label_a5; 
                          
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
                      case 98: case 99: case 100: case 101: case 102: 
                      case 103: case 104: case 105: case 106: case 107: 
                      case 108: case 109: case 110: case 111: case 112: 
                      case 113: case 114: case 115: case 116: case 117: 
                      case 118: case 120: case 121: case 122: case 123: 
                      case 125: case 126: case 127: case 128: case 129: 
                      case 130: case 131: case 132: case 133: case 134: 
                      case 135: case 136: case 137: case 138: case 139: 
                      case 140: case 141: case 142: case 143: case 144: 
                      case 145: case 146: case 147: case 148: case 150: 
                      case 152: case 153: case 154: case 155: case 156: 
                      case 157: case 158: case 159: case 160: case 161: 
                      case 162: case 163: case 164: case 165: case 166: 
                      case 167: case 168: case 169: case 170: case 171: 
                      case 172: case 173: case 174: case 175: case 176: 
                      case 177: case 178: case 179: case 180: case 182: 
                      case 184: case 185: case 186: case 187: case 188: 
                      case 189: case 190: case 191: case 192: case 193: 
                      case 194: case 195: case 196: case 197: case 198: 
                      case 199: case 200: case 201: case 202: case 203: 
                      case 204: case 205: case 206: case 207: case 208: 
                      case 209: case 210: case 211: case 212: case 213: 
                      case 214: case 216: case 217: case 218: case 219: 
                      case 220: case 221: case 222: case 223: case 224: 
                      case 225: case 226: case 227: case 228: case 229: 
                      case 230: case 231: case 232: case 233: case 234: 
                      case 235: case 236: case 237: case 238: case 239: 
                      case 240: case 241: case 242: case 243: case 244: 
                      case 245: case 246: case 248: case 249: case 250: 
                      case 251: case 252: case 253: case 254: case 255: 
                      case 256: case 257: case 258: case 259: case 260: 
                      case 261: case 262: case 263: case 264: case 265: 
                      case 266: case 267: case 268: case 269: case 270: 
                      case 271: case 272: case 273: case 274: case 275: 
                      case 276: case 277: case 278: case 280: case 281: 
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
                      case 376: case 377: case 378: case 379: case 380: 
                      case 381: case 382: case 383: case 384: case 385: 
                      case 386: case 387: case 388: case 389: case 390: 
                      case 391: case 392: case 393: case 394: case 395: 
                      case 396: case 397: case 398: case 399: case 400: 
                      case 401: case 402: case 403: case 404: case 405: 
                      case 406: case 408: case 409: case 410: case 411: 
                      case 413: case 414: case 415: case 416: case 417: 
                      case 418: case 419: case 420: case 421: case 422: 
                      case 423: case 424: case 425: case 426: case 427: 
                      case 428: case 429: case 430: case 431: case 432: 
                      case 433: case 434: case 435: case 436: case 437: 
                      case 440: case 441: case 442: case 443: case 445: 
                      case 446: case 447: case 448: case 449: case 450: 
                      case 451: case 452: case 453: case 454: case 455: 
                      case 456: case 457: case 458: case 459: case 460: 
                      case 461: case 462: case 463: case 464: case 465: 
                      case 466: case 468: case 469: case 470: case 471: 
                      case 472: case 473: case 474: case 475: case 537: 
                      case 538: case 540: case 541: case 542: case 543: 
                      case 544: case 545: case 546: case 547: case 548: 
                      case 549: case 550: case 551: case 552: case 553: 
                      case 554: case 555: case 556: case 557: case 558: 
                      case 559: case 560: case 561: case 562: case 563: 
                      case 564: case 565: case 566: case 568: case 569: 
                      case 570: case 571: case 572: case 573: case 574: 
                      case 575: case 576: case 577: case 578: case 579: 
                      case 580: case 581: case 582: case 583: case 584: 
                      case 585: case 586: case 587: case 588: case 589: 
                      case 590: case 591: case 592: case 593: case 594: 
                      case 595: case 596: case 597: case 598: case 600: 
                      case 601: case 602: case 603: case 604: case 605: 
                      case 606: case 607: case 608: case 609: case 610: 
                      case 611: case 612: case 613: case 614: case 615: 
                      case 616: case 617: case 618: case 619: case 620: 
                      case 621: case 622: case 623: case 624: case 625: 
                      case 626: case 627: case 628: case 629: case 630: 
                      case 632: case 633: case 634: case 635: case 636: 
                      case 637: case 638: case 639: case 640: case 641: 
                      case 642: case 643: case 644: case 645: case 646: 
                      case 647: case 648: case 649: case 650: case 651: 
                      case 652: case 653: case 654: case 655: case 656: 
                      case 657: case 658: case 659: case 660: case 664: 
                      case 665: case 666: case 667: case 668: case 669: 
                      case 670: case 671: case 672: case 673: case 674: 
                      case 675: case 676: case 677: case 678: case 679: 
                      case 680: case 681: case 682: case 683: case 684: 
                      case 685: case 686: case 687: case 688: case 689: 
                      case 690: case 691: case 692: case 693: case 694: 
                      case 696: case 697: case 698: case 699: case 700: 
                      case 701: case 702: case 703: case 704: case 705: 
                      case 706: case 707: case 708: case 709: case 710: 
                      case 711: case 712: case 713: case 714: case 715: 
                      case 716: case 717: case 718: case 719: case 720: 
                      case 721: case 722: case 723: case 724: case 725: 
                      case 726: case 728: case 729: case 730: case 731: 
                      case 732: case 733: case 734: case 735: case 736: 
                      case 737: case 738: case 739: case 740: case 741: 
                      case 742: case 743: case 744: case 745: case 746: 
                      case 747: case 748: case 749: case 750: case 751: 
                      case 752: case 753: case 754: case 755: case 756: 
                      case 757: case 758: case 760: case 761: case 762: 
                      case 763: case 764: case 765: case 766: case 767: 
                      case 768: case 769: case 770: case 771: case 772: 
                      case 773: case 774: case 775: case 776: case 777: 
                      case 778: case 779: case 780: case 781: case 782: 
                      case 783: case 784: case 785: case 786: case 787: 
                      case 788: case 789: case 791: case 793: case 795: 
                      case 796: case 797: case 798: case 799: case 800: 
                      case 801: case 802: case 803: case 804: case 805: 
                      case 806: case 807: case 808: case 809: case 810: 
                      case 811: case 812: case 813: case 814: case 815: 
                      case 816: case 817: case 818: case 819: case 820: 
                      case 821: case 822: case 823: 
                        goto MATCH_label_a0; break;
                      case 20: case 21: case 23: case 53: case 55: case 84: 
                      case 87: case 119: case 279: case 310: case 311: 
                      case 341: case 343: case 373: case 375: case 533: 
                      case 534: case 790: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a7; 
                        
                        break;
                      case 24: case 27: case 28: case 60: case 124: case 284: 
                      case 316: case 412: case 444: case 476: case 536: 
                      case 539: case 792: case 794: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a8; 
                        
                        break;
                      case 32: 
                        if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 0) { 
                          MATCH_name = "cmpl"; 
                          goto MATCH_label_a5; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a0;  /*opt-block+*/
                        
                        break;
                      case 149: case 151: case 181: case 183: case 215: 
                      case 247: case 407: case 438: case 439: case 661: 
                      case 662: case 918: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a10; 
                        
                        break;
                      case 339: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a11; 
                        
                        break;
                      case 467: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a12; 
                        
                        break;
                      case 535: case 567: case 599: case 631: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a13; 
                        
                        break;
                      case 663: case 695: case 727: case 759: case 983: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a14; 
                        
                        break;
                      case 824: 
                        MATCH_name = 
                          MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */]; 
                        goto MATCH_label_a15; 
                        
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
                    case 39: case 137: case 234: case 236: case 237: 
                    case 238: case 239: case 240: case 241: case 242: 
                    case 243: case 244: case 245: case 246: case 247: 
                    case 248: case 249: case 250: case 251: case 252: 
                    case 253: case 254: case 255: case 256: case 257: 
                    case 258: case 259: case 260: case 261: case 262: 
                    case 263: case 264: case 265: case 458: case 460: 
                    case 461: case 462: case 463: case 464: case 465: 
                    case 466: case 467: case 468: case 469: case 470: 
                    case 471: case 472: case 473: case 474: case 475: 
                    case 476: case 477: case 478: case 479: case 480: 
                    case 481: case 482: case 483: case 484: case 485: 
                    case 486: case 487: case 488: case 490: case 492: 
                    case 493: case 494: case 495: case 496: case 497: 
                    case 498: case 499: case 500: case 501: case 502: 
                    case 503: case 504: case 505: case 506: case 507: 
                    case 508: case 509: case 510: case 511: 
                      if (477 <= (MATCH_w_32_0 >> 1 & 0x3ff) 
                              /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 533 || 
                        825 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 918 || 
                        919 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 983 || 
                        984 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                        goto MATCH_label_a0;  /*opt-block+*/
                      else 
                        switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                          case 0: 
                            if ((MATCH_w_32_0 >> 22 & 0x1) 
                                    /* Lz at 0 */ == 0) { 
                              MATCH_name = "cmp"; 
                              goto MATCH_label_a5; 
                              
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
                          case 121: case 122: case 123: case 125: case 126: 
                          case 127: case 128: case 129: case 130: case 131: 
                          case 132: case 133: case 134: case 135: case 136: 
                          case 137: case 138: case 139: case 140: case 141: 
                          case 142: case 143: case 144: case 145: case 146: 
                          case 147: case 148: case 150: case 152: case 153: 
                          case 154: case 155: case 156: case 157: case 158: 
                          case 159: case 160: case 161: case 162: case 163: 
                          case 164: case 165: case 166: case 167: case 168: 
                          case 169: case 170: case 171: case 172: case 173: 
                          case 174: case 175: case 176: case 177: case 178: 
                          case 179: case 180: case 182: case 184: case 185: 
                          case 186: case 187: case 188: case 189: case 190: 
                          case 191: case 192: case 193: case 194: case 195: 
                          case 196: case 197: case 198: case 199: case 200: 
                          case 201: case 202: case 203: case 204: case 205: 
                          case 206: case 207: case 208: case 209: case 210: 
                          case 211: case 212: case 213: case 214: case 216: 
                          case 217: case 218: case 219: case 220: case 221: 
                          case 222: case 223: case 224: case 225: case 226: 
                          case 227: case 228: case 229: case 230: case 231: 
                          case 232: case 233: case 234: case 235: case 236: 
                          case 237: case 238: case 239: case 240: case 241: 
                          case 242: case 243: case 244: case 245: case 246: 
                          case 248: case 249: case 250: case 251: case 252: 
                          case 253: case 254: case 255: case 256: case 257: 
                          case 258: case 259: case 260: case 261: case 262: 
                          case 263: case 264: case 265: case 266: case 267: 
                          case 268: case 269: case 270: case 271: case 272: 
                          case 273: case 274: case 275: case 276: case 277: 
                          case 278: case 280: case 281: case 282: case 283: 
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
                          case 371: case 372: case 374: case 376: case 377: 
                          case 378: case 379: case 380: case 381: case 382: 
                          case 383: case 384: case 385: case 386: case 387: 
                          case 388: case 389: case 390: case 391: case 392: 
                          case 393: case 394: case 395: case 396: case 397: 
                          case 398: case 399: case 400: case 401: case 402: 
                          case 403: case 404: case 405: case 406: case 408: 
                          case 409: case 410: case 411: case 413: case 414: 
                          case 415: case 416: case 417: case 418: case 419: 
                          case 420: case 421: case 422: case 423: case 424: 
                          case 425: case 426: case 427: case 428: case 429: 
                          case 430: case 431: case 432: case 433: case 434: 
                          case 435: case 436: case 437: case 440: case 441: 
                          case 442: case 443: case 445: case 446: case 447: 
                          case 448: case 449: case 450: case 451: case 452: 
                          case 453: case 454: case 455: case 456: case 457: 
                          case 458: case 459: case 460: case 461: case 462: 
                          case 463: case 464: case 465: case 466: case 468: 
                          case 469: case 470: case 471: case 472: case 473: 
                          case 474: case 475: case 537: case 538: case 540: 
                          case 541: case 542: case 543: case 544: case 545: 
                          case 546: case 547: case 548: case 549: case 550: 
                          case 551: case 552: case 553: case 554: case 555: 
                          case 556: case 557: case 558: case 559: case 560: 
                          case 561: case 562: case 563: case 564: case 565: 
                          case 566: case 568: case 569: case 570: case 571: 
                          case 572: case 573: case 574: case 575: case 576: 
                          case 577: case 578: case 579: case 580: case 581: 
                          case 582: case 583: case 584: case 585: case 586: 
                          case 587: case 588: case 589: case 590: case 591: 
                          case 592: case 593: case 594: case 595: case 596: 
                          case 597: case 598: case 600: case 601: case 602: 
                          case 603: case 604: case 605: case 606: case 607: 
                          case 608: case 609: case 610: case 611: case 612: 
                          case 613: case 614: case 615: case 616: case 617: 
                          case 618: case 619: case 620: case 621: case 622: 
                          case 623: case 624: case 625: case 626: case 627: 
                          case 628: case 629: case 630: case 632: case 633: 
                          case 634: case 635: case 636: case 637: case 638: 
                          case 639: case 640: case 641: case 642: case 643: 
                          case 644: case 645: case 646: case 647: case 648: 
                          case 649: case 650: case 651: case 652: case 653: 
                          case 654: case 655: case 656: case 657: case 658: 
                          case 659: case 660: case 664: case 665: case 666: 
                          case 667: case 668: case 669: case 670: case 671: 
                          case 672: case 673: case 674: case 675: case 676: 
                          case 677: case 678: case 679: case 680: case 681: 
                          case 682: case 683: case 684: case 685: case 686: 
                          case 687: case 688: case 689: case 690: case 691: 
                          case 692: case 693: case 694: case 696: case 697: 
                          case 698: case 699: case 700: case 701: case 702: 
                          case 703: case 704: case 705: case 706: case 707: 
                          case 708: case 709: case 710: case 711: case 712: 
                          case 713: case 714: case 715: case 716: case 717: 
                          case 718: case 719: case 720: case 721: case 722: 
                          case 723: case 724: case 725: case 726: case 728: 
                          case 729: case 730: case 731: case 732: case 733: 
                          case 734: case 735: case 736: case 737: case 738: 
                          case 739: case 740: case 741: case 742: case 743: 
                          case 744: case 745: case 746: case 747: case 748: 
                          case 749: case 750: case 751: case 752: case 753: 
                          case 754: case 755: case 756: case 757: case 758: 
                          case 760: case 761: case 762: case 763: case 764: 
                          case 765: case 766: case 767: case 768: case 769: 
                          case 770: case 771: case 772: case 773: case 774: 
                          case 775: case 776: case 777: case 778: case 779: 
                          case 780: case 781: case 782: case 783: case 784: 
                          case 785: case 786: case 787: case 788: case 789: 
                          case 791: case 793: case 795: case 796: case 797: 
                          case 798: case 799: case 800: case 801: case 802: 
                          case 803: case 804: case 805: case 806: case 807: 
                          case 808: case 809: case 810: case 811: case 812: 
                          case 813: case 814: case 815: case 816: case 817: 
                          case 818: case 819: case 820: case 821: case 822: 
                          case 823: 
                            goto MATCH_label_a0; break;
                          case 20: case 21: case 23: case 53: case 55: 
                          case 84: case 87: case 119: case 279: case 310: 
                          case 311: case 341: case 343: case 373: case 375: 
                          case 533: case 534: case 790: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a7; 
                            
                            break;
                          case 24: case 27: case 28: case 60: case 124: 
                          case 284: case 316: case 412: case 444: case 476: 
                          case 536: case 539: case 792: case 794: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a8; 
                            
                            break;
                          case 32: 
                            if ((MATCH_w_32_0 >> 22 & 0x1) 
                                    /* Lz at 0 */ == 0) { 
                              MATCH_name = "cmpl"; 
                              goto MATCH_label_a5; 
                              
                            } /*opt-block*/
                            else 
                              goto MATCH_label_a0;  /*opt-block+*/
                            
                            break;
                          case 149: case 151: case 181: case 183: case 215: 
                          case 247: case 407: case 438: case 439: case 661: 
                          case 662: case 918: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a10; 
                            
                            break;
                          case 339: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a11; 
                            
                            break;
                          case 467: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a12; 
                            
                            break;
                          case 535: case 567: case 599: case 631: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a13; 
                            
                            break;
                          case 663: case 695: case 727: case 759: case 983: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a14; 
                            
                            break;
                          case 824: 
                            MATCH_name = 
                              MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                  /* Xo1 at 0 */]; 
                            goto MATCH_label_a15; 
                            
                            break;
                          default: assert(0);
                        } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/ 
                      break;
                    case 8: case 10: case 40: case 136: case 138: case 233: 
                    case 235: case 266: case 457: case 459: case 489: 
                    case 491: 
                      MATCH_name = 
                        MATCH_name_Xo9_29[(MATCH_w_32_0 >> 1 & 0x1ff) 
                            /* Xo9 at 0 */]; 
                      goto MATCH_label_a16; 
                      
                      break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 1 & 0x1ff) -- Xo9 at 0 --*/    
          break;
        case 32: case 33: case 34: case 35: case 40: case 41: case 42: 
        case 43: case 46: 
          MATCH_name = 
            MATCH_name_OPCD_0[(MATCH_w_32_0 >> 26 & 0x3f) /* OPCD at 0 */]; 
          { 
            const char *name = MATCH_name;
            int /* [~32768..32767] */ d = 
              sign_extend((MATCH_w_32_0 & 0xffff) /* d at 0 */, 16);
            unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
            unsigned rd = (MATCH_w_32_0 >> 21 & 0x1f) /* D at 0 */;
            nextPC = 4 + MATCH_p; 
            
#line 152 "frontend/machine/ppc/decoder.m"
            

            		if (strcmp(name, "lmw") == 0) {

            			// Needs the third param d, which is the register number from rd

            			stmts = instantiate(pc, name, DIS_RD, DIS_DISP, DIS_RD_NUM);

            		} else

            			stmts = instantiate(pc, name, DIS_RD, DIS_DISP, DIS_NZRA);

            //	| XLb_ (b0, b1) [name] =>

            #if BCCTR_LONG	// Prefer to see bltctr instead of bcctr 12,0

            				// But also affects return instructions (bclr)

            		/*FIXME: since this is used for returns, do a jump to LR instead (ie ignoring control registers) */

            		stmts = instantiate(pc,	 name);

            		result.rtl = new RTL(pc, stmts);

            		result.rtl->appendStmt(new ReturnStatement);

            		unused(b0);

            		unused(b1);

            #endif

            
            
            
          }
          
          break;
        case 36: case 37: case 38: case 39: case 44: case 45: case 47: 
          MATCH_name = 
            MATCH_name_OPCD_0[(MATCH_w_32_0 >> 26 & 0x3f) /* OPCD at 0 */]; 
          { 
            const char *name = MATCH_name;
            int /* [~32768..32767] */ d = 
              sign_extend((MATCH_w_32_0 & 0xffff) /* d at 0 */, 16);
            unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
            unsigned rs = (MATCH_w_32_0 >> 21 & 0x1f) /* S at 0 */;
            nextPC = 4 + MATCH_p; 
            
#line 130 "frontend/machine/ppc/decoder.m"
            

            		if (strcmp(name, "stmw") == 0) {

            			// Needs the last param s, which is the register number from rs

            			stmts = instantiate(pc, name, DIS_RS, DIS_DISP, DIS_RS_NUM);

            		} else

            			stmts = instantiate(pc, name, DIS_RS, DIS_DISP, DIS_NZRA);

            		

            
            
            
          }
          
          break;
        case 48: case 49: case 50: case 51: 
          MATCH_name = 
            MATCH_name_OPCD_0[(MATCH_w_32_0 >> 26 & 0x3f) /* OPCD at 0 */]; 
          { 
            const char *name = MATCH_name;
            int /* [~32768..32767] */ d = 
              sign_extend((MATCH_w_32_0 & 0xffff) /* d at 0 */, 16);
            unsigned fd = (MATCH_w_32_0 >> 21 & 0x1f) /* fD at 0 */;
            unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
            nextPC = 4 + MATCH_p; 
            
#line 243 "frontend/machine/ppc/decoder.m"
            									// Floating point loads (non indexed)

            		stmts = instantiate(pc, name, DIS_FD, DIS_DISP, DIS_RA);	// Pass RA twice (needed for update)

            

            
            
            
          }
          
          break;
        case 52: case 53: case 54: case 55: 
          MATCH_name = 
            MATCH_name_OPCD_0[(MATCH_w_32_0 >> 26 & 0x3f) /* OPCD at 0 */]; 
          { 
            const char *name = MATCH_name;
            int /* [~32768..32767] */ d = 
              sign_extend((MATCH_w_32_0 & 0xffff) /* d at 0 */, 16);
            unsigned fs = (MATCH_w_32_0 >> 21 & 0x1f) /* fS at 0 */;
            unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
            nextPC = 4 + MATCH_p; 
            
#line 249 "frontend/machine/ppc/decoder.m"
            									// Floating point stores (non indexed)

            		stmts = instantiate(pc, name, DIS_FS, DIS_DISP, DIS_RA);	// Pass RA twice (needed for update)

            

            
            
            
          }
          
          break;
        case 59: 
          
            switch((MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */) {
              case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: 
              case 8: case 9: case 10: case 11: case 12: case 13: case 14: 
              case 15: case 16: case 17: case 19: case 22: case 23: case 24: 
              case 25: case 26: case 27: case 28: case 29: case 30: case 31: 
                goto MATCH_label_a0; break;
              case 18: 
                if (1 <= (MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ && 
                  (MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ < 32) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else { 
                  MATCH_name = 
                    MATCH_name_Rc_36[(MATCH_w_32_0 & 0x1) /* Rc at 0 */]; 
                  goto MATCH_label_a17; 
                  
                } /*opt-block*/
                
                break;
              case 20: 
                if (1 <= (MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ && 
                  (MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ < 32) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else { 
                  MATCH_name = 
                    MATCH_name_Rc_37[(MATCH_w_32_0 & 0x1) /* Rc at 0 */]; 
                  goto MATCH_label_a17; 
                  
                } /*opt-block*/
                
                break;
              case 21: 
                if (1 <= (MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ && 
                  (MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ < 32) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else { 
                  MATCH_name = 
                    MATCH_name_Rc_38[(MATCH_w_32_0 & 0x1) /* Rc at 0 */]; 
                  goto MATCH_label_a17; 
                  
                } /*opt-block*/
                
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 1 & 0x1f) -- Xo5 at 0 --*/ 
          break;
        case 63: 
          if ((MATCH_w_32_0 & 0x1) /* Rc at 0 */ == 1) 
            if ((MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */ == 0) 
              if (73 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 136 || 
                137 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 264 || 
                265 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 814 || 
                847 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                
                  switch((MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */) {
                    case 0: case 1: case 2: case 3: case 4: case 5: case 6: 
                    case 7: case 8: case 9: case 10: case 11: case 12: 
                    case 13: case 14: case 15: case 16: case 17: case 19: 
                    case 22: case 23: case 24: case 25: case 26: case 27: 
                    case 28: case 29: case 30: case 31: 
                      goto MATCH_label_a0; break;
                    case 18: 
                      if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                        MATCH_name = "fdivq"; 
                        goto MATCH_label_a17; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    case 20: 
                      if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                        MATCH_name = "fsubq"; 
                        goto MATCH_label_a17; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    case 21: 
                      if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                        MATCH_name = "faddq"; 
                        goto MATCH_label_a17; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 1 & 0x1f) -- Xo5 at 0 --*/  
              else 
                switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                  case 0: case 1: case 2: case 3: case 4: case 5: case 6: 
                  case 7: case 8: case 9: case 10: case 11: case 13: case 16: 
                  case 17: case 18: case 19: case 20: case 21: case 22: 
                  case 23: case 24: case 25: case 26: case 27: case 28: 
                  case 29: case 30: case 31: case 32: case 33: case 34: 
                  case 35: case 36: case 37: case 38: case 39: case 41: 
                  case 42: case 43: case 44: case 45: case 46: case 47: 
                  case 48: case 49: case 50: case 51: case 52: case 53: 
                  case 54: case 55: case 56: case 57: case 58: case 59: 
                  case 60: case 61: case 62: case 63: case 64: case 65: 
                  case 66: case 67: case 68: case 69: case 70: case 71: 
                  case 816: case 817: case 818: case 819: case 820: case 821: 
                  case 822: case 823: case 824: case 825: case 826: case 827: 
                  case 828: case 829: case 830: case 831: case 832: case 833: 
                  case 834: case 835: case 836: case 837: case 838: case 839: 
                  case 840: case 841: case 842: case 843: case 844: case 845: 
                    
                      switch((MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */) {
                        case 0: case 1: case 2: case 3: case 4: case 5: 
                        case 6: case 7: case 8: case 9: case 10: case 11: 
                        case 12: case 13: case 14: case 15: case 16: case 17: 
                        case 19: case 22: case 23: case 24: case 25: case 26: 
                        case 27: case 28: case 29: case 30: case 31: 
                          goto MATCH_label_a0; break;
                        case 18: 
                          if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                            MATCH_name = "fdivq"; 
                            goto MATCH_label_a17; 
                            
                          } /*opt-block*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/
                          
                          break;
                        case 20: 
                          if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                            MATCH_name = "fsubq"; 
                            goto MATCH_label_a17; 
                            
                          } /*opt-block*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/
                          
                          break;
                        case 21: 
                          if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                            MATCH_name = "faddq"; 
                            goto MATCH_label_a17; 
                            
                          } /*opt-block*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/
                          
                          break;
                        default: assert(0);
                      } /* (MATCH_w_32_0 >> 1 & 0x1f) -- Xo5 at 0 --*/ 
                    break;
                  case 12: case 14: case 15: case 40: case 72: case 136: 
                  case 264: case 814: case 815: case 846: 
                    MATCH_name = 
                      MATCH_name_Xo1_30[(MATCH_w_32_0 >> 1 & 0x3ff) 
                          /* Xo1 at 0 */]; 
                    goto MATCH_label_a19; 
                    
                    break;
                  default: assert(0);
                } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/  
            else 
              
                switch((MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */) {
                  case 0: case 1: case 2: case 3: case 4: case 5: case 6: 
                  case 7: case 8: case 9: case 10: case 11: case 12: case 13: 
                  case 14: case 15: case 16: case 17: case 19: case 22: 
                  case 23: case 24: case 25: case 26: case 27: case 28: 
                  case 29: case 30: case 31: 
                    goto MATCH_label_a0; break;
                  case 18: 
                    if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                      MATCH_name = "fdivq"; 
                      goto MATCH_label_a17; 
                      
                    } /*opt-block*/
                    else 
                      goto MATCH_label_a0;  /*opt-block+*/
                    
                    break;
                  case 20: 
                    if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                      MATCH_name = "fsubq"; 
                      goto MATCH_label_a17; 
                      
                    } /*opt-block*/
                    else 
                      goto MATCH_label_a0;  /*opt-block+*/
                    
                    break;
                  case 21: 
                    if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                      MATCH_name = "faddq"; 
                      goto MATCH_label_a17; 
                      
                    } /*opt-block*/
                    else 
                      goto MATCH_label_a0;  /*opt-block+*/
                    
                    break;
                  default: assert(0);
                } /* (MATCH_w_32_0 >> 1 & 0x1f) -- Xo5 at 0 --*/   
          else 
            if ((MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */ == 0) 
              if (73 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 136 || 
                137 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 264 || 
                265 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 814 || 
                847 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                
                  switch((MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */) {
                    case 0: case 1: case 2: case 3: case 4: case 5: case 6: 
                    case 7: case 8: case 9: case 10: case 11: case 12: 
                    case 13: case 14: case 15: case 16: case 17: case 19: 
                    case 22: case 23: case 24: case 25: case 26: case 27: 
                    case 28: case 29: case 30: case 31: 
                      goto MATCH_label_a0; break;
                    case 18: 
                      if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                        MATCH_name = "fdiv"; 
                        goto MATCH_label_a17; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    case 20: 
                      if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                        MATCH_name = "fsub"; 
                        goto MATCH_label_a17; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    case 21: 
                      if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                        MATCH_name = "fadd"; 
                        goto MATCH_label_a17; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 1 & 0x1f) -- Xo5 at 0 --*/  
              else 
                switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                  case 0: 
                    if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) 
                      if ((MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */ == 1) 
                        if ((MATCH_w_32_0 >> 1 & 0x1f) 
                                /* Xo5 at 0 */ == 18 || 
                          20 <= (MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ && 
                          (MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ < 22) { 
                          MATCH_name = 
                            MATCH_name_Xo5_40[(MATCH_w_32_0 >> 1 & 0x1f) 
                                /* Xo5 at 0 */]; 
                          goto MATCH_label_a17; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a0;  /*opt-block+*/ /*opt-block+*/
                      else 
                        if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 1) 
                          if ((MATCH_w_32_0 >> 1 & 0x1f) 
                                  /* Xo5 at 0 */ == 18 || 
                            20 <= (MATCH_w_32_0 >> 1 & 0x1f) 
                                  /* Xo5 at 0 */ && 
                            (MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ < 22) { 
                            MATCH_name = 
                              MATCH_name_Xo5_40[(MATCH_w_32_0 >> 1 & 0x1f) 
                                  /* Xo5 at 0 */]; 
                            goto MATCH_label_a17; 
                            
                          } /*opt-block*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/ /*opt-block+*/
                        else { 
                          MATCH_name = "fcmpu"; 
                          goto MATCH_label_a18; 
                          
                        } /*opt-block*/  
                    else 
                      if ((MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */ == 0 && 
                        (MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 1 || 
                        (MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */ == 1) 
                        goto MATCH_label_a0;  /*opt-block+*/
                      else { 
                        MATCH_name = "fcmpu"; 
                        goto MATCH_label_a18; 
                        
                      } /*opt-block*/ /*opt-block+*/
                    break;
                  case 1: case 2: case 3: case 4: case 5: case 6: case 7: 
                  case 8: case 9: case 10: case 11: case 13: case 16: 
                  case 17: case 18: case 19: case 20: case 21: case 22: 
                  case 23: case 24: case 25: case 26: case 27: case 28: 
                  case 29: case 30: case 31: case 33: case 34: case 35: 
                  case 36: case 37: case 38: case 39: case 41: case 42: 
                  case 43: case 44: case 45: case 46: case 47: case 48: 
                  case 49: case 50: case 51: case 52: case 53: case 54: 
                  case 55: case 56: case 57: case 58: case 59: case 60: 
                  case 61: case 62: case 63: case 64: case 65: case 66: 
                  case 67: case 68: case 69: case 70: case 71: case 816: 
                  case 817: case 818: case 819: case 820: case 821: case 822: 
                  case 823: case 824: case 825: case 826: case 827: case 828: 
                  case 829: case 830: case 831: case 832: case 833: case 834: 
                  case 835: case 836: case 837: case 838: case 839: case 840: 
                  case 841: case 842: case 843: case 844: case 845: 
                    
                      switch((MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */) {
                        case 0: case 1: case 2: case 3: case 4: case 5: 
                        case 6: case 7: case 8: case 9: case 10: case 11: 
                        case 12: case 13: case 14: case 15: case 16: case 17: 
                        case 19: case 22: case 23: case 24: case 25: case 26: 
                        case 27: case 28: case 29: case 30: case 31: 
                          goto MATCH_label_a0; break;
                        case 18: 
                          if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                            MATCH_name = "fdiv"; 
                            goto MATCH_label_a17; 
                            
                          } /*opt-block*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/
                          
                          break;
                        case 20: 
                          if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                            MATCH_name = "fsub"; 
                            goto MATCH_label_a17; 
                            
                          } /*opt-block*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/
                          
                          break;
                        case 21: 
                          if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) { 
                            MATCH_name = "fadd"; 
                            goto MATCH_label_a17; 
                            
                          } /*opt-block*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/
                          
                          break;
                        default: assert(0);
                      } /* (MATCH_w_32_0 >> 1 & 0x1f) -- Xo5 at 0 --*/ 
                    break;
                  case 12: case 14: case 15: case 40: case 72: case 136: 
                  case 264: case 814: case 815: case 846: 
                    MATCH_name = 
                      MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                          /* Xo1 at 0 */]; 
                    goto MATCH_label_a19; 
                    
                    break;
                  case 32: 
                    if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) 
                      if ((MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */ == 1) 
                        if ((MATCH_w_32_0 >> 1 & 0x1f) 
                                /* Xo5 at 0 */ == 18 || 
                          20 <= (MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ && 
                          (MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ < 22) { 
                          MATCH_name = 
                            MATCH_name_Xo5_40[(MATCH_w_32_0 >> 1 & 0x1f) 
                                /* Xo5 at 0 */]; 
                          goto MATCH_label_a17; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a0;  /*opt-block+*/ /*opt-block+*/
                      else 
                        if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 1) 
                          if ((MATCH_w_32_0 >> 1 & 0x1f) 
                                  /* Xo5 at 0 */ == 18 || 
                            20 <= (MATCH_w_32_0 >> 1 & 0x1f) 
                                  /* Xo5 at 0 */ && 
                            (MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ < 22) { 
                            MATCH_name = 
                              MATCH_name_Xo5_40[(MATCH_w_32_0 >> 1 & 0x1f) 
                                  /* Xo5 at 0 */]; 
                            goto MATCH_label_a17; 
                            
                          } /*opt-block*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/ /*opt-block+*/
                        else { 
                          MATCH_name = "fcmpo"; 
                          goto MATCH_label_a18; 
                          
                        } /*opt-block*/  
                    else 
                      if ((MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */ == 0 && 
                        (MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 1 || 
                        (MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */ == 1) 
                        goto MATCH_label_a0;  /*opt-block+*/
                      else { 
                        MATCH_name = "fcmpo"; 
                        goto MATCH_label_a18; 
                        
                      } /*opt-block*/ /*opt-block+*/
                    break;
                  default: assert(0);
                } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/  
            else 
              if ((MATCH_w_32_0 >> 6 & 0x1f) /* C at 0 */ == 0) 
                if ((MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */ == 1) 
                  if ((MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ == 18 || 
                    20 <= (MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ && 
                    (MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ < 22) { 
                    MATCH_name = MATCH_name_Xo5_40[(MATCH_w_32_0 >> 1 & 0x1f) 
                          /* Xo5 at 0 */]; 
                    goto MATCH_label_a17; 
                    
                  } /*opt-block*/
                  else 
                    goto MATCH_label_a0;  /*opt-block+*/ /*opt-block+*/
                else 
                  if ((MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 1) 
                    if ((MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ == 18 || 
                      20 <= (MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ && 
                      (MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ < 22) { 
                      MATCH_name = 
                        MATCH_name_Xo5_40[(MATCH_w_32_0 >> 1 & 0x1f) 
                            /* Xo5 at 0 */]; 
                      goto MATCH_label_a17; 
                      
                    } /*opt-block*/
                    else 
                      goto MATCH_label_a0;  /*opt-block+*/ /*opt-block+*/
                  else 
                    if (33 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                      (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                      if ((MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ == 18 || 
                        20 <= (MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ && 
                        (MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ < 22) { 
                        MATCH_name = 
                          MATCH_name_Xo5_40[(MATCH_w_32_0 >> 1 & 0x1f) 
                              /* Xo5 at 0 */]; 
                        goto MATCH_label_a17; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/ /*opt-block+*/
                    else 
                      switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                        case 0: case 32: 
                          MATCH_name = 
                            MATCH_name_Xo1_26[(MATCH_w_32_0 >> 1 & 0x3ff) 
                                /* Xo1 at 0 */]; 
                          goto MATCH_label_a18; 
                          
                          break;
                        case 1: case 2: case 3: case 4: case 5: case 6: 
                        case 7: case 8: case 9: case 10: case 11: case 12: 
                        case 13: case 14: case 15: case 16: case 17: case 18: 
                        case 19: case 20: case 21: case 22: case 23: case 24: 
                        case 25: case 26: case 27: case 28: case 29: case 30: 
                        case 31: 
                          if ((MATCH_w_32_0 >> 1 & 0x1f) 
                                  /* Xo5 at 0 */ == 18 || 
                            20 <= (MATCH_w_32_0 >> 1 & 0x1f) 
                                  /* Xo5 at 0 */ && 
                            (MATCH_w_32_0 >> 1 & 0x1f) /* Xo5 at 0 */ < 22) { 
                            MATCH_name = 
                              MATCH_name_Xo5_40[(MATCH_w_32_0 >> 1 & 0x1f) 
                                  /* Xo5 at 0 */]; 
                            goto MATCH_label_a17; 
                            
                          } /*opt-block*/
                          else 
                            goto MATCH_label_a0;  /*opt-block+*/
                          
                          break;
                        default: assert(0);
                      } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/    
              else 
                if (33 <= (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */ < 1024) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else 
                  switch((MATCH_w_32_0 >> 1 & 0x3ff) /* Xo1 at 0 */) {
                    case 0: 
                      if ((MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */ == 0 && 
                        (MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 1 || 
                        (MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */ == 1) 
                        goto MATCH_label_a0;  /*opt-block+*/
                      else { 
                        MATCH_name = "fcmpu"; 
                        goto MATCH_label_a18; 
                        
                      } /*opt-block*/
                      
                      break;
                    case 1: case 2: case 3: case 4: case 5: case 6: case 7: 
                    case 8: case 9: case 10: case 11: case 12: case 13: 
                    case 14: case 15: case 16: case 17: case 18: case 19: 
                    case 20: case 21: case 22: case 23: case 24: case 25: 
                    case 26: case 27: case 28: case 29: case 30: case 31: 
                      goto MATCH_label_a0; break;
                    case 32: 
                      if ((MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */ == 0 && 
                        (MATCH_w_32_0 >> 22 & 0x1) /* Lz at 0 */ == 1 || 
                        (MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */ == 1) 
                        goto MATCH_label_a0;  /*opt-block+*/
                      else { 
                        MATCH_name = "fcmpo"; 
                        goto MATCH_label_a18; 
                        
                      } /*opt-block*/
                      
                      break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 1 & 0x3ff) -- Xo1 at 0 --*/    
          break;
        default: assert(0);
      } /* (MATCH_w_32_0 >> 26 & 0x3f) -- OPCD at 0 --*/ 
    
  }goto MATCH_finished_a; 
  
  MATCH_label_a0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
#line 353 "frontend/machine/ppc/decoder.m"
      
      		stmts = NULL;

      		result.valid = false;

      		result.numBytes = 4;	  

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a1: (void)0; /*placeholder for label*/ 
    { 
      unsigned BIcr = (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
      unsigned reladdr = 
        4 * (MATCH_w_32_0 >> 2 & 0x3fff) /* BD at 0 */ + addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
#line 303 "frontend/machine/ppc/decoder.m"
      

      		unconditionalJump("bal", 4, reladdr, delta, pc, stmts, result);

      		unused(BIcr);

      

      	// b<cond>lr: Branch conditionally to the link register. Model this as a conditional branch around a return

      	// statement.

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a2: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned BIcr = (MATCH_w_32_0 >> 18 & 0x7) /* BIcr at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 341 "frontend/machine/ppc/decoder.m"
      

      		result.rtl = new RTL(pc, stmts);

      		result.rtl->appendStmt(new ReturnStatement);

      		SHOW_ASM(name<<"\n");

      		unused(BIcr);

      

      	// Shift right arithmetic

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a3: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned crbA = (MATCH_w_32_0 >> 16 & 0x1f) /* crbA at 0 */;
      unsigned crbB = (MATCH_w_32_0 >> 11 & 0x1f) /* crbB at 0 */;
      unsigned crbD = (MATCH_w_32_0 >> 21 & 0x1f) /* crbD at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 168 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_CRBD, DIS_CRBA, DIS_CRBB);

      		

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a4: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned beg = (MATCH_w_32_0 >> 6 & 0x1f) /* MB at 0 */;
      unsigned end = (MATCH_w_32_0 >> 1 & 0x1f) /* ME at 0 */;
      unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
      unsigned rs = (MATCH_w_32_0 >> 21 & 0x1f) /* S at 0 */;
      unsigned uimm = (MATCH_w_32_0 >> 11 & 0x1f) /* SH at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 189 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_RA, DIS_RS, DIS_UIMM, DIS_BEG, DIS_END);

      

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a5: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned crfd = (MATCH_w_32_0 >> 23 & 0x7) /* crfD at 0 */;
      unsigned l = (MATCH_w_32_0 >> 21 & 0x1) /* L at 0 */;
      unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
      unsigned rb = (MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 233 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_CRFD, DIS_NZRA, DIS_NZRB);

      		unused(l);

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a6: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned rd = (MATCH_w_32_0 >> 21 & 0x1f) /* D at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 186 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_RD);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a7: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
      unsigned rb = (MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */;
      unsigned rd = (MATCH_w_32_0 >> 21 & 0x1f) /* D at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 146 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_RD, DIS_INDEX);

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a8: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned ra = (MATCH_w_32_0 >> 21 & 0x1f) /* S at 0 */;
      unsigned rb = (MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */;
      unsigned rd = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 144 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_RD, DIS_RA, DIS_RB);

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a9: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned ra = (MATCH_w_32_0 >> 21 & 0x1f) /* S at 0 */;
      unsigned rd = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 125 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_RD, DIS_RA);

      	// The number of parameters in these matcher arms has to agree with the number in core.spec

      	// The number of parameters passed to instantiate() after pc and name has to agree with ppc.ssl

      	// Stores and loads pass rA to instantiate twice: as part of DIS_DISP, and separately as DIS_NZRA

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a10: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
      unsigned rb = (MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */;
      unsigned rd = (MATCH_w_32_0 >> 21 & 0x1f) /* S at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 149 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_RD, DIS_INDEX);

      	// Load instructions

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a11: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned rd = (MATCH_w_32_0 >> 21 & 0x1f) /* D at 0 */;
      unsigned uimm = 
        ((MATCH_w_32_0 >> 11 & 0x1f) /* sprH at 0 */ << 5) + 
        (MATCH_w_32_0 >> 16 & 0x1f) /* sprL at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 170 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc, name, DIS_RD, DIS_UIMM);

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a12: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned rs = (MATCH_w_32_0 >> 21 & 0x1f) /* S at 0 */;
      unsigned uimm = 
        ((MATCH_w_32_0 >> 11 & 0x1f) /* sprH at 0 */ << 5) + 
        (MATCH_w_32_0 >> 16 & 0x1f) /* sprL at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 173 "frontend/machine/ppc/decoder.m"
      

      		switch (uimm) {

      			case 1:

      				stmts = instantiate(pc, "MTXER" , DIS_RS); break;

      			case 8:

      				stmts = instantiate(pc, "MTLR" , DIS_RS); break;

      			case 9:

      				stmts = instantiate(pc, "MTCTR" , DIS_RS); break;

      			default:

      				std::cerr << "ERROR: MTSPR instruction with invalid S field: " << uimm << "\n";

      		}

      		::unused(name);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a13: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned fd = (MATCH_w_32_0 >> 21 & 0x1f) /* fD at 0 */;
      unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
      unsigned rb = (MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 246 "frontend/machine/ppc/decoder.m"
      									// Floating point loads (indexed)

      		stmts = instantiate(pc, name, DIS_FD, DIS_INDEX, DIS_RA);	// Pass RA twice (needed for update)

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a14: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned fs = (MATCH_w_32_0 >> 21 & 0x1f) /* fS at 0 */;
      unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
      unsigned rb = (MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 252 "frontend/machine/ppc/decoder.m"
      									// Floating point stores (indexed)

      		stmts = instantiate(pc, name, DIS_FS, DIS_INDEX, DIS_RA);	// Pass RA twice (needed for update)

      

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a15: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
      unsigned rs = (MATCH_w_32_0 >> 21 & 0x1f) /* S at 0 */;
      unsigned uimm = (MATCH_w_32_0 >> 11 & 0x1f) /* SH at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 347 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc,	 name, DIS_RA, DIS_RS, DIS_UIMM);

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a16: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned ra = (MATCH_w_32_0 >> 16 & 0x1f) /* A at 0 */;
      unsigned rb = (MATCH_w_32_0 >> 11 & 0x1f) /* B at 0 */;
      unsigned rd = (MATCH_w_32_0 >> 21 & 0x1f) /* D at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 120 "frontend/machine/ppc/decoder.m"
      

      		stmts = instantiate(pc,	 name, DIS_RD, DIS_RA, DIS_RB);

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a17: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned fa = (MATCH_w_32_0 >> 16 & 0x1f) /* fA at 0 */;
      unsigned fb = (MATCH_w_32_0 >> 11 & 0x1f) /* fB at 0 */;
      unsigned fd = (MATCH_w_32_0 >> 21 & 0x1f) /* fD at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 262 "frontend/machine/ppc/decoder.m"
      									// Floating point binary

      		stmts = instantiate(pc, name, DIS_FD, DIS_FA, DIS_FB);

      

      

      		

      

      	// Conditional branches

      	// bcc_ is blt | ble | beq | bge | bgt | bnl | bne | bng | bso | bns | bun | bnu | bal (branch always)

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a18: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned crfd = (MATCH_w_32_0 >> 23 & 0x7) /* crfD at 0 */;
      unsigned fa = (MATCH_w_32_0 >> 16 & 0x1f) /* fA at 0 */;
      unsigned fb = (MATCH_w_32_0 >> 11 & 0x1f) /* fB at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 256 "frontend/machine/ppc/decoder.m"
      									// Floating point compare

      		stmts = instantiate(pc, name, DIS_CRFD, DIS_FA, DIS_FB);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a19: (void)0; /*placeholder for label*/ 
    { 
      const char *name = MATCH_name;
      unsigned fb = (MATCH_w_32_0 >> 11 & 0x1f) /* fB at 0 */;
      unsigned fd = (MATCH_w_32_0 >> 21 & 0x1f) /* fD at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 259 "frontend/machine/ppc/decoder.m"
      									// Floating point unary

      		stmts = instantiate(pc, name, DIS_FD, DIS_FB);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_finished_a: (void)0; /*placeholder for label*/
  
}

#line 358 "frontend/machine/ppc/decoder.m"

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
PPCDecoder::PPCDecoder(Prog* prog) : NJMCDecoder(prog)
{
  std::string file = Boomerang::get()->getProgPath() + "frontend/machine/ppc/ppc.ssl";
  RTLDict.readSSLFile(file.c_str());
}

// For now...
int PPCDecoder::decodeAssemblyInstruction(ADDRESS, int)
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



