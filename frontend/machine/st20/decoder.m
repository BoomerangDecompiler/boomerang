/*
 * Copyright (C) 2005 Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   decoder.m
 * OVERVIEW:   This file contains the high level decoding functionality, for matching ST-20 instructions.
 *============================================================================*/ 
/*
 * $Revision$
 *
 * 10 Mar 05 - Mike: Created.
*/

#include <assert.h>

#include "rtl.h"
#include "decoder.h"
#include "st20decoder.h"
#include "exp.h"
#include "proc.h"
#include "boomerang.h"
#include "statement.h"

/**********************************
 * ST20Decoder methods.
 **********************************/   

/*==============================================================================
 * FUNCTION:	   unused
 * OVERVIEW:	   A dummy function to suppress "unused local variable" messages
 * PARAMETERS:	   x: integer variable to be "used"
 * RETURNS:		   Nothing
 *============================================================================*/
void ST20Decoder::unused(int x)
{}

/*==============================================================================
 * FUNCTION:	   ST20Decoder::decodeInstruction
 * OVERVIEW:	   Decodes a machine instruction and returns an RTL instance. In all cases a single instruction is decoded.
 * PARAMETERS:	   pc - the native address of the pc
 *				   delta - the difference between the above address and the host address of the pc (i.e. the address that
					the pc is at in the loaded object file)
 *				   RTLDict - the dictionary of RTL templates used to instantiate the RTL for the instruction being decoded
 *				   proc - the enclosing procedure
 * RETURNS:		   a DecodeResult structure containing all the information gathered during decoding
 *============================================================================*/
static DecodeResult result;
DecodeResult& ST20Decoder::decodeInstruction (ADDRESS pc, int delta) {
	result.reset();							// Clear the result structure (numBytes = 0 etc)
	ADDRESS hostPC = pc + delta;
	std::list<Statement*>* stmts = NULL; 	// The actual list of instantiated Statements
	unsigned total = 0;						// Total value from all prefixes

	while (1) {
		match hostPC+result.numBytes++ to

		| pfix(oper) =>
			total = (total << 4) + oper;
			continue;
					
		| nfix(oper) =>
			total = (total << 4) + ~oper;
			continue;

		| primary(oper) [name] =>
			stmts = instantiate(pc,	name, new Const(total+oper));

		| j (oper) =>
			unconditionalJump("j", result.numBytes, pc+result.numBytes+oper, delta, pc, stmts, result);

		| call (oper) =>
			stmts = instantiate(pc, "call" , new Const(oper));
			CallStatement* newCall = new CallStatement;
			newCall->setIsComputed(false);
			newCall->setDest(pc+result.numBytes+oper);
			result.rtl = new RTL(pc, stmts);
			result.rtl->appendStmt(newCall);

		| cj (oper) =>
			BranchStatement* br = new BranchStatement();
			//br->setCondType(BRANCH_JE);
			br->setDest(pc+result.numBytes+oper);
			//br->setCondExpr(dis_Reg(0));
			br->setCondExpr(new Binary(opEquals,dis_Reg(0),new Const(0)));
			result.rtl = new RTL(pc, stmts);
			result.rtl->appendStmt(br);
					
		| testpranal() =>
			stmts = instantiate(pc,	"testpranal");

		| saveh() =>
			stmts = instantiate(pc,	"saveh");


		| savel() =>
			stmts = instantiate(pc, "savel");
		| sthf() =>
			stmts = instantiate(pc, "sthf");
		| sthb() =>
			stmts = instantiate(pc, "sthb");
		| stlf() =>
			stmts = instantiate(pc, "stlf");
		| stlb() =>
			stmts = instantiate(pc, "stlb");
		| sttimer() =>
			stmts = instantiate(pc, "sttimer");
		| lddevid() =>
			stmts = instantiate(pc, "lddevid");
		| ldmemstartval() =>
			stmts = instantiate(pc, "ldmemstartval");

		| andq() =>
			stmts = instantiate(pc, "andq");
		| orq() =>
			stmts = instantiate(pc, "orq");
		| xor() =>
			stmts = instantiate(pc, "xor");
		| not() =>
			stmts = instantiate(pc, "not");
		| shl() =>
			stmts = instantiate(pc, "shl");
		| shr() =>
			stmts = instantiate(pc, "shr");

		| add() =>
			stmts = instantiate(pc, "add");
		| sub() =>
			stmts = instantiate(pc, "sub");
		| mul() =>
			stmts = instantiate(pc, "mul");
		| fmul() =>
			stmts = instantiate(pc, "fmul");
		| div() =>
			stmts = instantiate(pc, "div");
		| rem() =>
			stmts = instantiate(pc, "rem");
		| gt() =>
			stmts = instantiate(pc, "gt");
		| gtu() =>
			stmts = instantiate(pc, "gtu");
		| diff() =>
			stmts = instantiate(pc, "diff");
		| sum() =>
			stmts = instantiate(pc, "sum");
		| prod() =>
			stmts = instantiate(pc, "proc");
		| satadd() =>
			stmts = instantiate(pc, "satadd");
		| satsub() =>
			stmts = instantiate(pc, "satsub");
		| satmul() =>
			stmts = instantiate(pc, "satmul");

		| ladd() =>
			stmts = instantiate(pc, "ladd");
		| lsub() =>
			stmts = instantiate(pc, "lsub");
		| lsum() =>
			stmts = instantiate(pc, "lsum");
		| ldiff() =>
			stmts = instantiate(pc, "ldiff");
		| lmul() =>
			stmts = instantiate(pc, "lmul");
		| ldiv() =>
			stmts = instantiate(pc, "ldiv");
		| lshl() =>
			stmts = instantiate(pc, "lshl");
		| lshr() =>
			stmts = instantiate(pc, "lshr");
		| norm() =>
			stmts = instantiate(pc, "norm");
		| slmul() =>
			stmts = instantiate(pc, "slmul");
		| sulmul() =>
			stmts = instantiate(pc, "sulmul");

		| rev() =>
			stmts = instantiate(pc, "rev");
		| xword() =>
			stmts = instantiate(pc, "xword");
		| cword() =>
			stmts = instantiate(pc, "cword");
		| xdble() =>
			stmts = instantiate(pc, "xdble");
		| csngl() =>
			stmts = instantiate(pc, "csngl");
		| mint() =>
			stmts = instantiate(pc, "mint");
		| dup() =>
			stmts = instantiate(pc, "dup");
		| pop() =>
			stmts = instantiate(pc, "pop");
		| reboot() =>
			stmts = instantiate(pc, "reboot");

		| bsub() =>
			stmts = instantiate(pc, "bsub");
		| wsub() =>
			stmts = instantiate(pc, "wsub");
		| wsubdb() =>
			stmts = instantiate(pc, "wsubdb");
		| bcnt() =>
			stmts = instantiate(pc, "bcnt");
		| wcnt() =>
			stmts = instantiate(pc, "wcnt");
		| lb() =>
			stmts = instantiate(pc, "lb");
		| sb() =>
			stmts = instantiate(pc, "sb");

		| in() =>
			stmts = instantiate(pc, "in");
		| out() =>
			stmts = instantiate(pc, "out");
		| outword() =>
			stmts = instantiate(pc, "outword");
		| outbyte() =>
			stmts = instantiate(pc, "outbyte");

		| ret() =>
			stmts = instantiate(pc, "ret");
		| ldpi() =>
			stmts = instantiate(pc, "ldpi");
		| gcall() =>
			stmts = instantiate(pc, "gcall");
		| lend() =>
			stmts = instantiate(pc, "lend");

		| startp() =>
			stmts = instantiate(pc, "startp");
		| endp() =>
			stmts = instantiate(pc, "endp");

		| fptesterr() =>
			stmts = instantiate(pc, "fptesterr");

		| nop() =>
			stmts = instantiate(pc, "nop");


		else
			result.valid = false;		// Invalid instruction
			result.rtl = NULL;
			result.numBytes = 0;
			return result;
		endmatch
		break;
	}

	if (result.rtl == 0)
		result.rtl = new RTL(pc, stmts);
	return result;
}

/*==============================================================================
 * These are machine specific functions used to decode instruction operands into Exp*s.
 *============================================================================*/

/**********************************
 * These are the fetch routines.
 **********************************/   

/*==============================================================================
 * FUNCTION:		getWord
 * OVERVIEW:		Returns the word starting at the given address.
 * PARAMETERS:		lc - address at which to decode the double
 * RETURNS:			the decoded double
 *============================================================================*/
Byte ST20Decoder::getByte (unsigned lc)
/* getByte - returns next byte from image pointed to by lc.	 */
{
	return *(Byte *)lc;
}

/*==============================================================================
 * FUNCTION:		getWord
 * OVERVIEW:		Returns the word starting at the given address.
 * PARAMETERS:		lc - address at which to decode the double
 * RETURNS:			the decoded double
 *============================================================================*/
SWord ST20Decoder::getWord (unsigned lc)
/* get2Bytes - returns next 2-Byte from image pointed to by lc.	 */
{
	return (SWord)(*(Byte *)lc + (*(Byte *)(lc+1) << 8));
}

/*==============================================================================
 * FUNCTION:		getDword
 * OVERVIEW:		Returns the double starting at the given address.
 * PARAMETERS:		lc - address at which to decode the double
 * RETURNS:			the decoded double
 *============================================================================*/
DWord ST20Decoder::getDword (unsigned lc)
/* get4Bytes - returns the next 4-Byte word from image pointed to by lc. */
{
	return (DWord)(*(Byte *)lc + (*(Byte *)(lc+1) << 8) +
		(*(Byte *)(lc+2) << 16) + (*(Byte *)(lc+3) << 24));
}


/*==============================================================================
 * FUNCTION:	   ST20Decoder::ST20Decoder
 * OVERVIEW:	   Constructor. The code won't work without this (not sure why the default constructor won't do...)
 * PARAMETERS:	   None
 * RETURNS:		   N/A
 *============================================================================*/
ST20Decoder::ST20Decoder() : NJMCDecoder()
{
  std::string file = Boomerang::get()->getProgPath() + "frontend/machine/st20/st20.ssl";
  RTLDict.readSSLFile(file.c_str());
}

// For now...
int ST20Decoder::decodeAssemblyInstruction(unsigned, int)
{ return 0; }

