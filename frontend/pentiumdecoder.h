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
 * FILE:	   pentdecoder.h
 * OVERVIEW:   The implementation of the instruction decoder for Pentium.
 *============================================================================*/

/* 
 * $Revision$
 * 06 Jun 02 - Trent: Created.
 *
 */

#ifndef PENTDECODER
#define PENTDECODER

class Prog;
class NJMCDecoder;
struct DecodeResult;

class PentiumDecoder : public NJMCDecoder
{
public:
	/* Default constructor
	 */
	PentiumDecoder(Prog* prog);

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
	Exp*	dis_Mem(ADDRESS ps);
	Exp*	addReloc(Exp *e);

	void	unused(int x);
	bool	isFuncPrologue(ADDRESS hostPC);

	Byte	getByte(ADDRESS lc);
	SWord	getWord(ADDRESS lc);
	DWord	getDword(ADDRESS lc);

	unsigned lastDwordLc;
};

#endif
