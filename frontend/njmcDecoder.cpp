/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   njmcDecoder.cc
 * OVERVIEW:   This file contains the machine independent
 *			   decoding functionality.
 *
 * $Revision$
 *============================================================================*/ 
/*
 * 27 Apr 02 - Mike: Mods for boomerang
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include <stdarg.h>			// For varargs
#include "rtl.h"
#include "decoder.h"
#include "exp.h"
#include "register.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"
#include "BinaryFile.h"
#include "boomerang.h"
// For some reason, MSVC 5.00 complains about use of undefined types a lot
#if defined(_MSC_VER) && _MSC_VER <= 1100
#include "signature.h"		// For MSVC 5.00
#endif

/**********************************
 * NJMCDecoder methods.
 **********************************/   

/*==============================================================================
 * FUNCTION:	   NJMCDecoder::NJMCDecoder
 * OVERVIEW:	   
 * PARAMETERS:	   None
 * RETURNS:		   N/A
 *============================================================================*/
NJMCDecoder::NJMCDecoder()
{}

/*==============================================================================
 * FUNCTION:	   NJMCDecoder::instantiate
 * OVERVIEW:	   Given an instruction name and a variable list of expressions
 *				   representing the actual operands of the instruction, use the
 *				   RTL template dictionary to return the instantiated RTL
 *				   representing the semantics of the instruction.
 * PARAMETERS:	   pc: native PC
 *				   name - instruction name
 *				   ... - Semantic String ptrs representing actual operands
 * RETURNS:		   an instantiated list of Exps
 *============================================================================*/
std::list<Statement*>* NJMCDecoder::instantiate(ADDRESS pc, const char* name, ...) {
	// Get the signature of the instruction and extract its parts
	std::pair<std::string,unsigned> sig = RTLDict.getSignature(name);
	std::string opcode = sig.first;
	unsigned numOperands = sig.second;

	// Put the operands into a vector
	std::vector<Exp*> actuals(numOperands);
	va_list args;
	va_start(args,name);
	for (unsigned i = 0; i < numOperands; i++)
		actuals[i] = va_arg(args,Exp*);
	va_end(args);

	if (DEBUG_DECODER) {
		// Display a disassembly of this instruction if requested
		std::cout << std::hex << pc << std::dec << ": " << name << " ";
		for (std::vector<Exp*>::iterator itd = actuals.begin();
		  itd != actuals.end(); itd++) {
			(*itd)->print(std::cout);
			if (itd != actuals.end()-1)
				std::cout << ", ";
		}
		std::cout << std::endl;
	}

	std::list<Statement*>* instance = RTLDict.instantiateRTL(opcode, pc,
	  actuals);

	return instance;
}

/*==============================================================================
 * FUNCTION:	   NJMCDecoder::instantiateNamedParam
 * OVERVIEW:	   Similarly to the above, given a parameter name
 *					and a list of Exp*'s representing sub-parameters,
 *					return a fully substituted Exp for the whole expression
 * NOTE:		   Caller must delete result
 * PARAMETERS:	   name - parameter name
 *				   ... - Exp* representing actual operands
 * RETURNS:		   an instantiated list of Exps
 *============================================================================*/
Exp* NJMCDecoder::instantiateNamedParam(char* name, ...) {
	if (RTLDict.ParamSet.find(name) == RTLDict.ParamSet.end()) {
		std::cerr << "No entry for named parameter '" << name << "'\n";
		return 0;
	}
	assert(RTLDict.DetParamMap.find(name) != RTLDict.DetParamMap.end());
	ParamEntry &ent = RTLDict.DetParamMap[name];
	if (ent.kind != PARAM_ASGN && ent.kind != PARAM_LAMBDA ) {
		std::cerr << "Attempt to instantiate expressionless parameter '" << name
		  << "'\n";
		return 0;
	}
	// Start with the RHS
	assert(ent.asgn->getKind() == STMT_ASSIGN);
	Exp* result = ent.asgn->getRight()->clone();

	va_list args;
	va_start(args,name);
	for( std::list<std::string>::iterator it = ent.params.begin();
	  it != ent.params.end(); it++ ) {
		Exp* formal = new Location(opParam, new Const((char*)it->c_str()),
		  NULL);
		Exp* actual = va_arg(args, Exp*);
		bool change;
		result = result->searchReplaceAll(formal, actual, change);
		delete formal;
	}
	return result;
}

/*==============================================================================
 * FUNCTION:	   NJMCDecoder::substituteCallArgs
 * OVERVIEW:	   In the event that it's necessary to synthesize the call of
 *				   a named parameter generated with instantiateNamedParam(),
 *				   this substituteCallArgs() will substitute the arguments that
 *				   follow into the expression.
 * NOTE:		   Should only be used after instantiateNamedParam(name, ..);
 * NOTE:		   exp (the pointer) could be changed
 * PARAMETERS:	   name - parameter name
 *				   exp - expression to instantiate into
 *				   ... - Exp* representing actual operands
 * RETURNS:		   an instantiated list of Exps
 *============================================================================*/
void NJMCDecoder::substituteCallArgs(char *name, Exp*& exp, ...)
{
	if (RTLDict.ParamSet.find(name) == RTLDict.ParamSet.end()) {
		std::cerr << "No entry for named parameter '" << name << "'\n";
		return;
	}
	ParamEntry &ent = RTLDict.DetParamMap[name];
	/*if (ent.kind != PARAM_ASGN && ent.kind != PARAM_LAMBDA) {
		std::cerr << "Attempt to instantiate expressionless parameter '" << name << "'\n";
		return;
	}*/
	
	va_list args;
	va_start(args, exp);
	for (std::list<std::string>::iterator it = ent.funcParams.begin();
		 it != ent.funcParams.end(); it++) {
		Exp* formal = new Location(opParam, new Const((char*)it->c_str()),
		  NULL);
		Exp* actual = va_arg(args, Exp*);
		bool change;
		exp = exp->searchReplaceAll(formal, actual, change);
		delete formal;
	}
}

/*==============================================================================
 * FUNCTION:	   DecodeResult::reset
 * OVERVIEW:	   Resets the fields of a DecodeResult to their default values.
 * PARAMETERS:	   <none>
 * RETURNS:		   <nothing>
 *============================================================================*/
void DecodeResult::reset()
{
	numBytes = 0;
	type = NCT;
	valid = true;
	rtl = NULL;
	reDecode = false;
	forceOutEdge = 0;	
}

/*==============================================================================
 * These are functions used to decode instruction operands into
 * Exp*s.
 *============================================================================*/

/*==============================================================================
 * FUNCTION:		NJMCDecoder::dis_Reg
 * OVERVIEW:		Converts a numbered register to a suitable expression.
 * PARAMETERS:		reg - the register number, e.g. 0 for eax
 * RETURNS:			the Exp* for the register NUMBER (e.g. "int 36" for %f4)
 *============================================================================*/
Exp* NJMCDecoder::dis_Reg(int regNum)
{
	  Exp* expr = Location::regOf(regNum);
	  return expr;
}

/*==============================================================================
 * FUNCTION:		NJMCDecoder::dis_Num
 * OVERVIEW:		Converts a number to a Exp* expression.
 * PARAMETERS:		num - a number
 * RETURNS:			the Exp* representation of the given number
 *============================================================================*/
Exp* NJMCDecoder::dis_Num(unsigned num)
{
	Exp* expr = new Const((int)num);
	return expr;
}

/*==============================================================================
 * FUNCTION:		NJMCDecoder::unconditionalJump
 * OVERVIEW:		Process an unconditional jump instruction
 *					Also check if the destination is a label
 * PARAMETERS:		<none>
 * RETURNS:			the reference to the RTLInstDict object
 *============================================================================*/
void NJMCDecoder::unconditionalJump(const char* name, int size,
  ADDRESS relocd, /*UserProc* proc,*/ int delta, ADDRESS pc, std::list<Statement*>* stmts,
  DecodeResult& result) {
	result.rtl = new RTL(pc, stmts);
	result.numBytes = size;
	GotoStatement* jump = new GotoStatement();
	jump->setDest(relocd-delta);
	result.rtl->appendStmt(jump);
	SHOW_ASM(name<<" "<<relocd)
}

