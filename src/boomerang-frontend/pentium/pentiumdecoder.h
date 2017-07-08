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
 * \file       pentiumdecoder.h
 * \brief   The implementation of the instruction decoder for Pentium.
 ******************************************************************************/

#include <cstddef>

#include "boomerang/frontend/njmcDecoder.h"

class Prog;
struct DecodeResult;

class PentiumDecoder : public NJMCDecoder
{
public:
	/// @copydoc NJMCDecoder::NJMCDecoder
	PentiumDecoder(Prog *prog);

	/// @copydoc NJMCDecoder::~NJMCDecoder
	virtual ~PentiumDecoder() = default;

	/// @copydoc NJMCDecoder::decodeInstruction
	DecodeResult& decodeInstruction(Address pc, ptrdiff_t delta) override;

	/// @copydoc NJMCDecode::decodeAssemblyInstruction
	int decodeAssemblyInstruction(Address pc, ptrdiff_t delta) override;

private:

	/*
	 * Various functions to decode the operands of an instruction into
	 * a SemStr representation.
	 */
	SharedExp dis_Eaddr(Address pc, int size = 0);
	SharedExp dis_Mem(Address ps);
	SharedExp addReloc(const SharedExp& e);

	bool isFuncPrologue(Address hostPC);

	Byte getByte(intptr_t lc); // TODO: switch to using ADDRESS objects
	SWord getWord(intptr_t lc);
	DWord getDword(intptr_t lc);

	Byte getByte(Address lc)   { return getByte(lc.m_value); }
	SWord getWord(Address lc)  { return getWord(lc.m_value); }
	DWord getDword(Address lc) { return getDword(lc.m_value); }

private:
	   Address lastDwordLc;
};
