#pragma once

/*
 * Copyright (C) 1996-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       ppcdecoder.h
 * \brief   The implementation of the instruction decoder for PPC.
 ******************************************************************************/

#include "boomerang/frontend/njmcDecoder.h"

#include <cstdlib>

class Prog;

struct DecodeResult;


class PPCDecoder : public NJMCDecoder
{
public:
	/// \copydoc NJMCDecoder::NJMCDecoder
	PPCDecoder(Prog *prog);

	/// \copydoc NJMCDecoder::decodeInstruction
	DecodeResult& decodeInstruction(Address pc, ptrdiff_t delta) override;

private:
	/// Various functions to decode the operands of an instruction into an Exp* representation.
	Exp *dis_Eaddr(Address pc, int size = 0);
	Exp *dis_RegImm(Address pc);

	/***************************************************************************/ /**
	* \brief        Decode the register
	* \param        r - register (0-31)
	* \returns             the expression representing the register
	******************************************************************************/
	SharedExp dis_Reg(unsigned r);

	/***************************************************************************/ /**
	* \brief        Decode the register rA when rA represents constant 0 if r == 0
	* \param        r - register (0-31)
	* \returns             the expression representing the register
	******************************************************************************/
	SharedExp dis_RAmbz(unsigned r); // Special for rA of certain instructions

	RTL *createBranchRtl(Address pc, std::list<Instruction *> *stmts, const char *name);

	/***************************************************************************/ /**
	* \brief      Check to see if the instructions at the given offset match
	*             any callee prologue, i.e. does it look like this offset
	*             is a pointer to a function?
	* \param      hostPC - pointer to the code in question (host address)
	* \returns           True if a match found
	******************************************************************************/
	bool isFuncPrologue(Address hostPC);
	DWord getDword(HostAddress lc);
};
