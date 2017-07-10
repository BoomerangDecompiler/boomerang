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
	SharedExp dis_Eaddr(HostAddress hostPC, int size = 0);
	SharedExp dis_Mem(HostAddress ps);
	SharedExp addReloc(const SharedExp& e);

	bool isFuncPrologue(Address hostPC);

	/// Read bytes, words or dwords from the memory at address @p addr
	Byte  getByte(HostAddress addr)  { return *(Byte*)addr.value(); }
	SWord getWord(HostAddress addr)  { return *(SWord*)addr.value(); }
	DWord getDword(HostAddress addr) { return *(DWord*)addr.value(); }

private:
	Address lastDwordLc;
};
