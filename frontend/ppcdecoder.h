/*
 * Copyright (C) 1996-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   ppcdecoder.h
 * OVERVIEW:   The implementation of the instruction decoder for PPC.
 *============================================================================*/

/* 
 * $Revision$
 * 23 Nov 04 - Jay Sweeney and Alejandro Dubrovsky: Created.
 */

#ifndef PPCDECODER
#define PPCDECODER

class Prog;
class NJMCDecoder;
struct DecodeResult;

class PPCDecoder : public NJMCDecoder
{
public:
	/* Default constructor
	 */
	PPCDecoder();

	/*
	 * Decodes the machine instruction at pc and returns an RTL instance for
	 * the instruction.
	 */
virtual DecodeResult& decodeInstruction (ADDRESS pc, int delta);

	/*
	 * Disassembles the machine instruction at pc and returns the number of
	 * bytes disassembled. Assembler output goes to global _assembly
	 */
virtual int decodeAssemblyInstruction (ADDRESS pc, int delta);


private:
	/*
	 * Various functions to decode the operands of an instruction into
	 * a SemStr representation.
	 */
	Exp*	dis_Eaddr(ADDRESS pc, int size = 0);
	Exp*	dis_RegImm(ADDRESS pc);
	Exp*	dis_Reg(unsigned r);

	void	unused(int x);
	RTL*	createBranchRtl(ADDRESS pc, std::list<Statement*>* stmts,
			  const char* name);
	bool	isFuncPrologue(ADDRESS hostPC);
	DWord	getDword(ADDRESS lc);

};

#endif
