#pragma once

/****************************************************************
 *
 * FILENAME
 *
 *   \file mipsfrontend.h
 *
 * PURPOSE
 *
 *   Skeleton for MIPS disassembly.
 *
 * AUTHOR
 *
 *   \author Markus Gothe, nietzsche@lysator.liu.se
 *
 * REVISION
 *
 *   $Id$
 *
 *****************************************************************/

#include "njmcDecoder.h"
#include <cstddef>

class Prog;
struct DecodeResult;

class MIPSDecoder : public NJMCDecoder
{
public:

	/* Default constructor
	 */
	MIPSDecoder(Prog *prog);

	/*
	 * Decodes the machine instruction at pc and returns an RTL instance for
	 * the instruction.
	 */
	virtual DecodeResult& decodeInstruction(ADDRESS pc, ptrdiff_t delta);

	/*
	 * Disassembles the machine instruction at pc and returns the number of
	 * bytes disassembled. Assembler output goes to global _assembly
	 */
	virtual int decodeAssemblyInstruction(ADDRESS pc, ptrdiff_t delta);

private:

/*
 * Various functions to decode the operands of an instruction into an Exp* representation.
 */
#if 0
	Exp *dis_Eaddr(ADDRESS pc, int size = 0);
	Exp *dis_RegImm(ADDRESS pc);
	Exp *dis_Reg(unsigned r);
	Exp *dis_RAmbz(unsigned r);               // Special for rA of certain instructions
#endif
	void unused(int);

#if 0
	RTL *createBranchRtl(ADDRESS pc, std::list<Statement *> *stmts, const char *name);
	bool isFuncPrologue(ADDRESS hostPC);
	DWord getDword(ADDRESS lc);
#endif
};
