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
	/***************************************************************************/ /**
	* \brief   Decodes a machine instruction and returns an RTL instance. In most cases a single instruction is
	*              decoded. However, if a higher level construct that may consist of multiple instructions is matched,
	*              then there may be a need to return more than one RTL. The caller_prologue2 is an example of such
	*              a construct which encloses an abritary instruction that must be decoded into its own RTL.
	* \param   pc - the native address of the pc
	* \param   delta - the difference between the above address and the host address of the pc (i.e. the address
	*              that the pc is at in the loaded object file)
	* \returns a DecodeResult structure containing all the information gathered during decoding
	******************************************************************************/
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
