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
#define DIS_DISP   (new Binary(opPlus, dis_RAmbz(ra), new Const(d)))
#define DIS_INDEX    (new Binary(opPlus, DIS_RA, DIS_NZRB))
#define DIS_BICR	(new Const(BIcr))
#define DIS_RS_NUM	(new Const(rs))
#define DIS_RD_NUM	(new Const(rd))

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

	match [nextPC] hostPC to
	| XO_ ( rd, ra, rb) [name] =>
		stmts = instantiate(pc,	 name, DIS_RD, DIS_RA, DIS_RB);
	| XOb_ ( rd, ra) [name] =>
		stmts = instantiate(pc, name, DIS_RD, DIS_RA);
	// The number of parameters in these matcher arms has to agree with the number in core.spec
	// The number of parameters passed to instantiate() after pc and name has to agree with ppc.ssl
	// Stores and loads pass rA to instantiate twice: as part of DIS_DISP, and separately as DIS_NZRA
	| Dsad_ (rs, d, ra) [name] =>
		if (strcmp(name, "stmw") == 0) {
			// Needs the last param s, which is the register number from rs
			stmts = instantiate(pc, name, DIS_RS, DIS_DISP, DIS_RS_NUM);
		} else
			stmts = instantiate(pc, name, DIS_RS, DIS_DISP, DIS_NZRA);
		
	| Dsaui_ (rd, ra, uimm) [name] =>
		stmts = instantiate(pc, name, DIS_RD, DIS_RA, DIS_UIMM);
	| Ddasi_ (rd, ra, simm) [name] =>
		if (strcmp(name, "addi") == 0 || strcmp(name, "addis") == 0) {
			// Note the DIS_RAZ, since rA could be constant zero
			stmts = instantiate(pc, name, DIS_RD, DIS_RAZ, DIS_SIMM);
		} else
			stmts = instantiate(pc, name, DIS_RD, DIS_RA , DIS_SIMM);
	| Xsabx_ (rd, ra, rb) [name] =>
		stmts = instantiate(pc, name, DIS_RD, DIS_RA, DIS_RB);
	| Xdab_ (rd, ra, rb) [name] =>
		stmts = instantiate(pc, name, DIS_RD, DIS_INDEX);
	// Load instructions
	| Ddad_ (rd, d, ra) [name] =>
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
	| XLc_ (crbD, crbA, crbB) [name] =>
		stmts = instantiate(pc, name, DIS_CRBD, DIS_CRBA, DIS_CRBB);
		
	| mfspr (rd, uimm) [name] =>
		stmts = instantiate(pc, name, DIS_RD, DIS_UIMM);
	| mtspr (uimm, rs) [name] =>
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
	| bl (reladdr) [name] =>
		Exp* dest = DIS_RELADDR;
		stmts = instantiate(pc, name, dest);
		CallStatement* newCall = new CallStatement;
		// Record the fact that this is not a computed call
		newCall->setIsComputed(false);
		// Set the destination expression
		newCall->setDest(dest);
		result.rtl = new RTL(pc, stmts);
		result.rtl->appendStmt(newCall);

	| b (reladdr) =>
		unconditionalJump("b", 4, reladdr, delta, pc, stmts, result);

	| ball (BIcr, reladdr) [name] =>		// Always "conditional" branch with link, test/OSX/hello has this
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

	| Xcmp_ (crfd, l, ra, rb) [name] =>
		stmts = instantiate(pc, name, DIS_CRFD, DIS_NZRA, DIS_NZRB);
		unused(l);
	| cmpi (crfd, l, ra, simm) [name] =>
		stmts = instantiate(pc, name, DIS_CRFD, DIS_NZRA, DIS_SIMM);
		unused(l);
	| cmpli (crfd, l, ra, uimm) [name] =>
		stmts = instantiate(pc, name, DIS_CRFD, DIS_NZRA, DIS_UIMM);
		unused(l);

	// Conditional branches
	// bcc_ is blt | ble | beq | bge | bgt | bnl | bne | bng | bso | bns | bun | bnu | bal (branch always)
	| blt(BIcr, reladdr) [name] =>
		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSL, BIcr);
	| ble(BIcr, reladdr) [name] =>
		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSLE, BIcr);
	| beq(BIcr, reladdr) [name] =>
		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JE, BIcr);
	| bge(BIcr, reladdr) [name] =>
		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSGE, BIcr);
	| bgt(BIcr, reladdr) [name] =>
		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSG, BIcr);
	| bnl(BIcr, reladdr) [name] =>
		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSGE, BIcr);
	| bne(BIcr, reladdr) [name] =>
		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JNE, BIcr);
	| bng(BIcr, reladdr) [name] =>
		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSLE, BIcr);
	| bso(BIcr, reladdr) [name] =>								// Branch on summary overflow
		PPC_COND_JUMP(name, 4, reladdr, (BRANCH_TYPE)0, BIcr);	// MVE: Don't know these last 4 yet
	| bns(BIcr, reladdr) [name] =>
		PPC_COND_JUMP(name, 4, reladdr, (BRANCH_TYPE)0, BIcr);
	| bun(BIcr, reladdr) [name] =>
		PPC_COND_JUMP(name, 4, reladdr, (BRANCH_TYPE)0, BIcr);
	| bnu(BIcr, reladdr) [name] =>
		PPC_COND_JUMP(name, 4, reladdr, (BRANCH_TYPE)0, BIcr);
    | bal(BIcr, reladdr) =>
		unconditionalJump("bal", 4, reladdr, delta, pc, stmts, result);

	// bcc_ is blt | ble | beq | bge | bgt | bnl | bne | bng | bso | bns | bun | bnu | bal
	| bltctr(BIcr) [name] =>
		std::cerr << "HACK " << name << "\n";
//		PPC_COND_JUMP(name, 4, new Unary(opMachFtr, new Const("%CTR")), BRANCH_JSL, BIcr);

	| blectr(BIcr) [name] =>
		std::cerr << "HACK " << name << "\n";
//		PPC_COND_JUMP(name, 4, new Unary(opMachFtr, new Const("%CTR")), BRANCH_JSLE, BIcr);

	| beqctr(BIcr) [name] =>
		std::cerr << "HACK " << name << "\n";
//		PPC_COND_JUMP(name, 4, new Unary(opMachFtr, new Const("%CTR")), BRANCH_JE, BIcr);

	| bgectr(BIcr) [name] =>
		std::cerr << "HACK " << name << "\n";
//		PPC_COND_JUMP(name, 4, new Unary(opMachFtr, new Const("%CTR")), BRANCH_JSGE, BIcr);

	| bgtctr(BIcr) [name] =>
		std::cerr << "HACK " << name << "\n";
//		PPC_COND_JUMP(name, 4, new Unary(opMachFtr, new Const("%CTR")), BRANCH_JSG, BIcr);

	| bnlctr(BIcr) [name] =>
		std::cerr << "HACK " << name << "\n";
//		PPC_COND_JUMP(name, 4, new Unary(opMachFtr, new Const("%CTR")), BRANCH_JSGE, BIcr);

	| bnectr(BIcr) [name] =>
		std::cerr << "HACK " << name << "\n";
//		PPC_COND_JUMP(name, 4, new Unary(opMachFtr, new Const("%CTR")), BRANCH_JNE, BIcr);

	| bngctr(BIcr) [name] =>
		std::cerr << "HACK " << name << "\n";
//		PPC_COND_JUMP(name, 4, new Unary(opMachFtr, new Const("%CTR")), BRANCH_JSLE, BIcr);

	| bsoctr(BIcr) [name] =>
		std::cerr << "HACK " << name << "\n";
//		PPC_COND_JUMP(name, 4, new Unary(opMachFtr, new Const("%CTR")), (BRANCH_TYPE)0, BIcr);	// MVE: Don't know these last 4 yet

	| bnsctr(BIcr) [name] =>
		std::cerr << "HACK " << name << "\n";
//		PPC_COND_JUMP(name, 4, new Unary(opMachFtr, new Const("%CTR")), (BRANCH_TYPE)0, BIcr);

	| bunctr(BIcr) [name] =>
		std::cerr << "HACK " << name << "\n";
//		PPC_COND_JUMP(name, 4, new Unary(opMachFtr, new Const("%CTR")), (BRANCH_TYPE)0, BIcr);

	| bnuctr(BIcr) [name] =>
		std::cerr << "HACK " << name << "\n";
//		PPC_COND_JUMP(name, 4, new Unary(opMachFtr, new Const("%CTR")), (BRANCH_TYPE)0, BIcr);

	| balctr(BIcr) [name] =>
		computedJump(name, 4, new Unary(opMachFtr, new Const("%CTR")), pc, stmts, result);
		
	// b<cond>lr: Branch conditionally to the link register. Model this as a conditional branch around a return
	// statement.
	| bltlr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JSGE, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| blelr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JSG, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| beqlr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JNE, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| bgelr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JSL, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| bgtlr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JSLE, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| bnllr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JSL, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| bnelr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JE, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| bnglr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JSG, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| bsolr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, (BRANCH_TYPE)0, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| bnslr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, (BRANCH_TYPE)0, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| bunlr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, (BRANCH_TYPE)0, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| bnulr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, (BRANCH_TYPE)0, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| ballr(BIcr) [name] =>
		result.rtl = new RTL(pc, stmts);
		result.rtl->appendStmt(new ReturnStatement);
		SHOW_ASM(name<<"\n");

	// Link versions of the above. For now, only handle unconditional case
	| ballrl(BIcr) [name] =>
		std::cerr << "HACK " << name << "\n";

	// Conditional calls (bcl)
	// bcc_^LI is bltl | blel | beql | bgel | bgtl | bnll | bnel | bngl | bsol | bnsl | bunl | bnul | ball
	| bltl(BIcr, reladdr) [name] =>
		std::cerr << "HACK " << name << "\n";
	| blel(BIcr, reladdr) [name] =>
		std::cerr << "HACK " << name << "\n";
	| beql(BIcr, reladdr) [name] =>
		std::cerr << "HACK " << name << "\n";
	| bgel(BIcr, reladdr) [name] =>
		std::cerr << "HACK " << name << "\n";
	| bgtl(BIcr, reladdr) [name] =>
		std::cerr << "HACK " << name << "\n";
	| bnll(BIcr, reladdr) [name] =>
		std::cerr << "HACK " << name << "\n";
	| bnel(BIcr, reladdr) [name] =>
		std::cerr << "HACK " << name << "\n";
	| bngl(BIcr, reladdr) [name] =>
		std::cerr << "HACK " << name << "\n";
	| bsol(BIcr, reladdr) [name] =>
		std::cerr << "HACK " << name << "\n";
	| bnsl(BIcr, reladdr) [name] =>
		std::cerr << "HACK " << name << "\n";
	| bunl(BIcr, reladdr) [name] =>
		std::cerr << "HACK " << name << "\n";
	| bnul(BIcr, reladdr) [name] =>
		std::cerr << "HACK " << name << "\n";
	else
		stmts = NULL;
		result.valid = false;
		result.numBytes = 4;	  
	endmatch

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

