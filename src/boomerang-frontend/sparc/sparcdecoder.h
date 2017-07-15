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
 * \file       sparcdecoder.h
 * \brief   The implementation of the instruction decoder for Sparc.
 ******************************************************************************/

#include "boomerang/frontend/njmcDecoder.h"

#include <cstddef>

class Prog;
struct DecodeResult;

class SparcMachine
{
public:
	SharedExp dis_RegRhs(uint8_t reg_no);
};

class SparcDecoder : public NJMCDecoder
{
	SparcMachine *machine;

public:
	/// @copydoc NJMCDecoder::NJMCDecoder
	SparcDecoder(Prog *prog);

	/// @copydoc NJMCDecoder::decodeInstruction
	/***************************************************************************/ /**
	 * \fn     SparcDecoder::decodeInstruction
	 * \brief  Attempt to decode the high level instruction at a given address.
	 *
	 * Return the corresponding HL type (e.g. CallStatement, GotoStatement etc). If no high level instruction exists at
	 * the given address,then simply return the RTL for the low level instruction at this address. There is an option
	 * to also include the low level statements for a HL instruction.
	 *
	 * \param pc - the native address of the pc
	 * \param delta - the difference between the above address and the host address of the pc (i.e. the address
	 *        that the pc is at in the loaded object file)
	 * \returns a DecodeResult structure containing all the information gathered during decoding
	 ******************************************************************************/
	DecodeResult& decodeInstruction(Address pc, ptrdiff_t delta) override;

	/// Indicates whether the instruction at the given address is a restore instruction.
	bool isRestore(HostAddress hostPC);

private:

	/*
	 * Various functions to decode the operands of an instruction into
	 * a SemStr representation.
	 */
	SharedExp dis_Eaddr(HostAddress pc, int size = 0);
	SharedExp dis_RegImm(HostAddress pc);
	SharedExp dis_RegLhs(unsigned r);

	/***************************************************************************/ /**
	 * \fn    SparcDecoder::createBranchRtl
	 * \brief Create an RTL for a Bx instruction
	 * \param pc - the location counter
	 * \param stmts - ptr to list of Statement pointers
	 * \param name - instruction name (e.g. "BNE,a", or "BPNE")
	 * \returns            Pointer to newly created RTL, or nullptr if invalid
	 ******************************************************************************/
	RTL *createBranchRtl(Address pc, std::list<Instruction *> *stmts, const char *name);
	bool isFuncPrologue(HostAddress hostPC);
	DWord getDword(HostAddress lc);
};
