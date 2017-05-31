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

#pragma once

#include "include/decoder.h"
#include "include/types.h"
#include "include/rtl.h"

/***************************************************************************/ /**
 * The NJMCDecoder class is a class that contains NJMC generated decoding methods.
 ******************************************************************************/
class NJMCDecoder : public IInstructionTranslator
{
protected:
	Prog *prog;
	class IBinaryImage *Image;

public:
	NJMCDecoder(Prog *prog);
	virtual ~NJMCDecoder() = default;

	RTLInstDict& getRTLDict() { return RTLDict; }
	void computedJump(const char *name, int size, SharedExp dest, ADDRESS pc, std::list<Instruction *> *stmts,
					  DecodeResult& result);
	void computedCall(const char *name, int size, SharedExp dest, ADDRESS pc, std::list<Instruction *> *stmts,
					  DecodeResult& result);
	QString getRegName(int idx) const override;
	int getRegSize(int idx) const override;
	int getRegIdx(const QString& name) const override;

protected:
	std::list<Instruction *> *instantiate(ADDRESS pc, const char *name, const std::initializer_list<SharedExp>& args = {});

	SharedExp instantiateNamedParam(char *name, const std::initializer_list<SharedExp>& args);
	void substituteCallArgs(char *name, SharedExp *exp, const std::initializer_list<SharedExp>& args);
	void unconditionalJump(const char *name, int size, ADDRESS relocd, ptrdiff_t delta, ADDRESS pc,
						   std::list<Instruction *> *stmts, DecodeResult& result);

	SharedExp dis_Num(unsigned num);
	SharedExp dis_Reg(int regNum);

	// Dictionary of instruction patterns, and other information summarised from the SSL file
	// (e.g. source machine's endianness)
	RTLInstDict RTLDict;
};

// Function used to guess whether a given pc-relative address is the start of a function

/*
 * Does the instruction at the given offset correspond to a caller prologue?
 * \note Implemented in the decoder.m files
 */
bool isFuncPrologue(ADDRESS hostPC);

/***************************************************************************/ /**
 * These are the macros that each of the .m files depend upon.
 ******************************************************************************/
#define DEBUG_DECODER    (Boomerang::get()->debugDecoder)
#define SHOW_ASM(output) \
	if (DEBUG_DECODER) { \
		LOG_STREAM() << pc << ": " << output << '\n'; }

/*
 * addresstoPC returns the raw number as the address.  PC could be an
 * abstract type, in our case, PC is the raw address.
 */
#define addressToPC(pc)    pc

// Macros for branches. Note: don't put inside a "match" statement, since
// the ordering is changed and multiple copies may be made

#define COND_JUMP(name, size, relocd, cond)				  \
	result.rtl = new RTL(pc, stmts);					  \
	BranchStatement *jump = new BranchStatement;		  \
	result.rtl->appendStmt(jump);						  \
	result.numBytes = size;								  \
	jump->setDest((relocd - ADDRESS::g(delta)).native()); \
	jump->setCondType(cond);							  \
	SHOW_ASM(name << " " << relocd.native())

// This one is X86 specific
#define SETS(name, dest, cond)			 \
	BoolAssign * bs = new BoolAssign(8); \
	bs->setLeftFromList(stmts);			 \
	stmts->clear();						 \
	result.rtl = new RTL(pc, stmts);	 \
	result.rtl->appendStmt(bs);			 \
	bs->setCondType(cond);				 \
	result.numBytes = 3;				 \
	SHOW_ASM(name << " " << dest)
