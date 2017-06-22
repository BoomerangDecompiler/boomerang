/*
 * Copyright (C) 1996-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       decoder.h
 * OVERVIEW:   The interface to the instruction decoder.
 ******************************************************************************/

#pragma once

#include "boomerang/include/types.h"

#include <list>
#include <cstddef>
#include <QtCore/QString>

class Exp;
class RTL;
class Prog;

// These are the instruction classes defined in "A Transformational Approach to
// Binary Translation of Delayed Branches" for SPARC instructions.
// Extended for HPPA. Ignored by machines with no delay slots
enum ICLASS
{
	NCT,   // Non Control Transfer
	SD,    // Static Delayed
	DD,    // Dynamic Delayed
	SCD,   // Static Conditional Delayed
	SCDAN, // Static Conditional Delayed, Anulled if Not taken
	SCDAT, // Static Conditional Delayed, Anulled if Taken
	SU,    // Static Unconditional (not delayed)
	SKIP,  // Skip successor
	//    TRAP,            // Trap
	NOP,   // No operation (e.g. sparc BN,A)
	// HPPA only
	DU,    // Dynamic Unconditional (not delayed)
	NCTA   // Non Control Transfer, with following instr Anulled
};

/***************************************************************************/ /**
 * The DecodeResult struct contains all the information that results from
 * calling the decoder. This prevents excessive use of confusing
 * reference parameters.
 ******************************************************************************/
struct DecodeResult
{
public:
	/// Resets all the fields to their default values.
	void    reset()
	{
		numBytes     = 0;
		type         = NCT;
		valid        = true;
		rtl          = nullptr;
		reDecode     = false;
		forceOutEdge = ADDRESS::g(0L);
	}

public:
	/// The number of bytes decoded in the main instruction
	int     numBytes;

	/// The RTL constructed (if any).
	RTL     *rtl;

	/// Indicates whether or not a valid instruction was decoded.
	bool    valid;

	/**
	 * The class of the instruction decoded. Will be one of the classes described in "A Transformational Approach
	 * to Binary Translation of Delayed Branches" (plus two more HPPA specific entries).
	 * Ignored by machines with no delay slots
	 */
	ICLASS  type;

	/**
	 * If true, don't add numBytes and decode there; instead, re-decode the current instruction. Needed for
	 * instructions like the Pentium BSF/BSR, which emit branches (so numBytes needs to be carefully set for the
	 * fall through out edge after the branch)
	 */
	bool    reDecode;

	/**
	 * If non zero, this field represents a new native address to be used as the out-edge for this instruction's BB.
	 * At present, only used for the SPARC call/add caller prologue
	 */
	ADDRESS forceOutEdge;
};


/**
 * @brief The IInstructionTranslator class - responsible for translating raw bytes to Instruction lists
 */
class IInstructionTranslator
{
public:
	virtual ~IInstructionTranslator() = default;
	
	/// Decodes the machine instruction at pc and returns an RTL instance for the instruction.
	virtual DecodeResult& decodeInstruction(ADDRESS pc, ptrdiff_t delta) = 0;

	/// Returns machine-specific register name given it's index
	virtual QString getRegName(int idx) const = 0;

	/// Returns index of the named register
	virtual int getRegIdx(const QString& name) const = 0;

	/// Returns size of register in bits
	virtual int getRegSize(int idx) const = 0;

	/**
	 * Disassembles the machine instruction at pc and returns the number of bytes disassembled.
	 * Assembler output goes to global _assembly
	 */
	virtual int decodeAssemblyInstruction(ADDRESS pc, ptrdiff_t delta) = 0;
};
