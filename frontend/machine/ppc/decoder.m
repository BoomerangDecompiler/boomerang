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
#define DIS_RS		(dis_RegLhs(rs))
#define DIS_RD		(dis_RegLhs(rd))
#define DIS_CRFD	(dis_RegLhs(64/* condition registers start*/ + crfd))
#define DIS_CRFA	(dis_RegLhs(64/* condition registers start*/ + crfa))
#define DIS_CRFB	(dis_RegLhs(64/* condition registers start*/ + crfb))
#define DIS_RDR		(dis_RegRhs(rd))
#define DIS_RA		(dis_RegRhs(ra))
#define DIS_RB		(dis_RegRhs(rb))
#define DIS_D		(new Const(d))
#define DIS_NZRA	(dis_RegLhs(ra))
#define DIS_NZRB	(dis_RegLhs(rb))
#define DIS_ADDR	(new Const(addr))
#define DIS_RELADDR (new Const(reladdr - delta))
#define DIS_CRBD	(crBit(crbD))
#define DIS_CRBA	(crBit(crbA))
#define DIS_CRBB	(crBit(crbB))

// MVE: Used any more?
#define DIS_INDEX   (new Binary(opPlus, \
					dis_Reg(ra), \
					  new Const(d)))

#define DIS_DISP    (new Binary(opPlus, DIS_RA, DIS_NZRB))

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
	| Dsad_ (rs, d, ra) [name] =>
		stmts = instantiate(pc, name, DIS_RS, DIS_D, DIS_RA);
	| Dsaui_ (rd, ra, uimm) [name] =>
		stmts = instantiate(pc, name, DIS_RD, DIS_RA, DIS_UIMM);
	| Ddasi_ (rd, ra, simm) [name] =>
		stmts = instantiate(pc, name, DIS_RD, DIS_RA, DIS_SIMM);
	| Xsabx_ (rd, ra, rb) [name] =>
		stmts = instantiate(pc, name, DIS_RD, DIS_RA, DIS_RB);
	| Xdab_ (rd, ra, rb) [name] =>
		stmts = instantiate(pc, name, DIS_RD, DIS_DISP);
	| Ddad_ (rd, d, ra) [name] =>
		stmts = instantiate(pc, name, DIS_RD, DIS_INDEX);
	| XLb_ (b0, b1) [name] =>
		/*FIXME: since this is used for returns, do a jump to LR instead (ie ignoring control registers) */
		stmts = instantiate(pc,	 name);
		result.rtl = new RTL(pc, stmts);
		result.rtl->appendStmt(new ReturnStatement);
		unused(b0);
		unused(b1);
	| XLc_ (crbD, crbA, crbB) [name] =>
		stmts = instantiate(pc, name, DIS_CRBD, DIS_CRBA, DIS_CRBB);
		
	| mfspr (rd, uimm) [name] =>
		stmts = instantiate(pc, name, DIS_RD, DIS_UIMM);
	| mtspr (uimm, rs) [name] =>
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

	| Xcmp_ (crfd, l, ra, rb) [name] =>
		stmts = instantiate(pc, name, DIS_CRFD, DIS_NZRA, DIS_NZRB);
		unused(l);
	| cmpi (crfd, l, ra, simm) [name] =>
		stmts = instantiate(pc, name, DIS_CRFD, DIS_NZRA, DIS_SIMM);
		unused(l);
	| cmpli (crfd, l, ra, uimm) [name] =>
		stmts = instantiate(pc, name, DIS_CRFD, DIS_NZRA, DIS_UIMM);
		unused(l);

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
 * FUNCTION:		PPCDecoder::dis_RegLhs
 * OVERVIEW:		Decode the register on the LHS
 * PARAMETERS:		r - register (0-31)
 * RETURNS:			the expression representing the register
 *============================================================================*/
Exp* PPCDecoder::dis_RegLhs(unsigned r)
{
	return Location::regOf(r);
}

/*==============================================================================
 * FUNCTION:		PPCDecoder::dis_RegRhs
 * OVERVIEW:		Decode the register on the RHS
 * NOTE:			Replaces r[0] with const 0
 * NOTE:			Not used by DIS_RD since don't want 0 on LHS
 * PARAMETERS:		r - register (0-31)
 * RETURNS:			the expression representing the register
 *============================================================================*/
Exp* PPCDecoder::dis_RegRhs(unsigned r)
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

