#pragma once

/*
 * Copyright (C) 2005, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       st20decoder.h
 * \brief   The definition of the instruction decoder for ST20.
 ******************************************************************************/

#include "boomerang/frontend/njmcDecoder.h"

#include "boomerang/db/rtl.h"

#include <cstddef>
#include <list>

class Prog;
class NJMCDecoder;
class Instruction;
struct DecodeResult;


class ST20Decoder : public NJMCDecoder
{
public:
	/// @copydoc NJMCDecoder::NJMCDecoder
	ST20Decoder(Prog *prog);

	/// @copydoc NJMCDecoder::decodeInstruction
	DecodeResult& decodeInstruction(ADDRESS pc, ptrdiff_t delta) override;

	/// @copydoc NJMCDecoder::decodeAssemblyInstruction
	int decodeAssemblyInstruction(ADDRESS pc, ptrdiff_t delta)  override;

private:

	/*
	 * Various functions to decode the operands of an instruction into
	 * a SemStr representation.
	 */
	// Exp*    dis_Eaddr(ADDRESS pc, int size = 0);
	// Exp*    dis_RegImm(ADDRESS pc);
	// Exp*    dis_Reg(unsigned r);
	// Exp*    dis_RAmbz(unsigned r);        // Special for rA of certain instructions

	RTL *createBranchRtl(ADDRESS pc, std::list<Instruction *> *stmts, const char *name);
	bool isFuncPrologue(ADDRESS hostPC);

	DWord getDword(intptr_t lc); // TODO: switch back to using ADDRESS objects
	SWord getWord(intptr_t lc);
	Byte getByte(intptr_t lc);
};
