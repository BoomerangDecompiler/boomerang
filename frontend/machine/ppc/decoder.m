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
	| Xsax_^Rc (rd, ra) [name] =>
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
	| Xsabx_^Rc (rd, ra, rb) [name] =>
		stmts = instantiate(pc, name, DIS_RD, DIS_RA, DIS_RB);
	| Xdab_ (rd, ra, rb) [name] =>
		stmts = instantiate(pc, name, DIS_RD, DIS_INDEX);
	| Xsab_ (rd, ra, rb) [name] =>
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

	| Xd_ (rd) [name] =>
		stmts = instantiate(pc, name, DIS_RD);

   	| M_^Rc(ra, rs, uimm, beg, end) [name] =>
		stmts = instantiate(pc, name, DIS_RA, DIS_RS, DIS_UIMM, DIS_BEG, DIS_END);


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
		Proc* destProc = prog->setNewProc(reladdr-delta);
		if (destProc == (Proc*)-1) destProc = NULL;
		newCall->setDestProc(destProc);

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

	| Ddaf_(fd, d, ra) [name] =>									// Floating point loads (non indexed)
		stmts = instantiate(pc, name, DIS_FD, DIS_DISP, DIS_RA);	// Pass RA twice (needed for update)

	| Xdaf_(fd, ra, rb) [name] =>									// Floating point loads (indexed)
		stmts = instantiate(pc, name, DIS_FD, DIS_INDEX, DIS_RA);	// Pass RA twice (needed for update)

	| Dsaf_(fs, d, ra) [name] =>									// Floating point stores (non indexed)
		stmts = instantiate(pc, name, DIS_FS, DIS_DISP, DIS_RA);	// Pass RA twice (needed for update)

	| Xsaf_(fs, ra, rb) [name] =>									// Floating point stores (indexed)
		stmts = instantiate(pc, name, DIS_FS, DIS_INDEX, DIS_RA);	// Pass RA twice (needed for update)


	| Xcab_(crfd, fa, fb) [name] =>									// Floating point compare
		stmts = instantiate(pc, name, DIS_CRFD, DIS_FA, DIS_FB);

	| Xdbx_^Rc(fd, fb) [name] =>									// Floating point unary
		stmts = instantiate(pc, name, DIS_FD, DIS_FB);

	| Ac_^Rc(fd, fa, fb) [name] =>									// Floating point binary
		stmts = instantiate(pc, name, DIS_FD, DIS_FA, DIS_FB);


		

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
//	| bnl(BIcr, reladdr) [name] =>								// bnl same as bge
//		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSGE, BIcr);
	| bne(BIcr, reladdr) [name] =>
		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JNE, BIcr);
//	| bng(BIcr, reladdr) [name] =>								// bng same as blt
//		PPC_COND_JUMP(name, 4, reladdr, BRANCH_JSLE, BIcr);
	| bso(BIcr, reladdr) [name] =>								// Branch on summary overflow
		PPC_COND_JUMP(name, 4, reladdr, (BRANCH_TYPE)0, BIcr);	// MVE: Don't know these last 4 yet
	| bns(BIcr, reladdr) [name] =>
		PPC_COND_JUMP(name, 4, reladdr, (BRANCH_TYPE)0, BIcr);
//	| bun(BIcr, reladdr) [name] =>
//		PPC_COND_JUMP(name, 4, reladdr, (BRANCH_TYPE)0, BIcr);
//	| bnu(BIcr, reladdr) [name] =>
//		PPC_COND_JUMP(name, 4, reladdr, (BRANCH_TYPE)0, BIcr);

	| balctr(BIcr) [name] =>
		computedJump(name, 4, new Unary(opMachFtr, new Const("%CTR")), pc, stmts, result);
		unused(BIcr);
		
	| balctrl(BIcr) [name] =>
		computedCall(name, 4, new Unary(opMachFtr, new Const("%CTR")), pc, stmts, result);
		unused(BIcr);
		
    | bal(BIcr, reladdr) =>
		unconditionalJump("bal", 4, reladdr, delta, pc, stmts, result);
		unused(BIcr);

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

	| bnelr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, BRANCH_JE, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| bsolr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, (BRANCH_TYPE)0, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| bnslr(BIcr) [name] =>
		PPC_COND_JUMP(name, 4, hostPC+4, (BRANCH_TYPE)0, BIcr);
		result.rtl->appendStmt(new ReturnStatement);

	| ballr(BIcr) [name] =>
		result.rtl = new RTL(pc, stmts);
		result.rtl->appendStmt(new ReturnStatement);
		SHOW_ASM(name<<"\n");
		unused(BIcr);

	// Shift right arithmetic
	| srawi(ra, rs, uimm) [name] =>
		stmts = instantiate(pc,	 name, DIS_RA, DIS_RS, DIS_UIMM);
	| srawiq(ra, rs, uimm) [name] =>
		stmts = instantiate(pc,	 name, DIS_RA, DIS_RS, DIS_UIMM);
		
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

