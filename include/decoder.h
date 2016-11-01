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

#ifndef _DECODER_H_
#define _DECODER_H_

#include <list>
#include <cstddef>
#include "types.h"
#include "rtl.h"

class Exp;
class RTL;
class Prog;

// These are the instruction classes defined in "A Transformational Approach to
// Binary Translation of Delayed Branches" for SPARC instructions.
// Extended for HPPA. Ignored by machines with no delay slots
enum ICLASS {
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
    DU,  // Dynamic Unconditional (not delayed)
    NCTA // Non Control Transfer, with following instr Anulled
};
/***************************************************************************/ /**
  * The DecodeResult struct contains all the information that results from
  * calling the decoder. This prevents excessive use of confusing
  * reference parameters.
  ******************************************************************************/
struct DecodeResult {
    //! Resets all the fields to their default values.
    void reset();

    //! The number of bytes decoded in the main instruction
    int numBytes;

    //! The RTL constructed (if any).
    RTL *rtl;

    //! Indicates whether or not a valid instruction was decoded.
    bool valid;

    /**
     * The class of the instruction decoded. Will be one of the classes described in "A Transformational Approach
     * to Binary Translation of Delayed Branches" (plus two more HPPA specific entries).
     * Ignored by machines with no delay slots
     */
    ICLASS type;

    /**
     * If true, don't add numBytes and decode there; instead, re-decode the current instruction. Needed for
     * instructions like the Pentium BSF/BSR, which emit branches (so numBytes needs to be carefully set for the
     * fall through out edge after the branch)
     */
    bool reDecode;

    /**
     * If non zero, this field represents a new native address to be used as the out-edge for this instruction's BB.
     * At present, only used for the SPARC call/add caller prologue
     */
    ADDRESS forceOutEdge;
};

/***************************************************************************/ /**
  * The NJMCDecoder class is a class that contains NJMC generated decoding methods.
  ******************************************************************************/
class NJMCDecoder {
protected:
    Prog *prog;
    class IBinaryImage *Image;
public:
    NJMCDecoder(Prog *prog);
    virtual ~NJMCDecoder() {}

    //! Decodes the machine instruction at pc and returns an RTL instance for the instruction.
    virtual DecodeResult &decodeInstruction(ADDRESS pc, ptrdiff_t delta) = 0;

    /**
     * Disassembles the machine instruction at pc and returns the number of bytes disassembled.
     * Assembler output goes to global _assembly
     */
    virtual int decodeAssemblyInstruction(ADDRESS pc, ptrdiff_t delta) = 0;
    RTLInstDict &getRTLDict() { return RTLDict; }
    void computedJump(const char *name, int size, SharedExp dest, ADDRESS pc, std::list<Instruction *> *stmts,
                      DecodeResult &result);
    void computedCall(const char *name, int size, SharedExp dest, ADDRESS pc, std::list<Instruction *> *stmts,
                      DecodeResult &result);
    Prog *getProg() { return prog; }

protected:
    std::list<Instruction *> *instantiate(ADDRESS pc, const char *name, const std::initializer_list<SharedExp> &args={});

    SharedExp instantiateNamedParam(char *name, const std::initializer_list<SharedExp> &args);
    void substituteCallArgs(char *name, SharedExp * exp, const std::initializer_list<SharedExp> &args);
    void unconditionalJump(const char *name, int size, ADDRESS relocd, ptrdiff_t delta, ADDRESS pc,
                           std::list<Instruction *> *stmts, DecodeResult &result);

    SharedExp dis_Num(unsigned num);
    SharedExp dis_Reg(int regNum);

    // Public dictionary of instruction patterns, and other information summarised from the SSL file
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
#define DEBUG_DECODER (Boomerang::get()->debugDecoder)
#define SHOW_ASM(output)                                                                                               \
    if (DEBUG_DECODER)                                                                                                 \
    LOG_STREAM() << pc << ": " << output << '\n';

/*
 * addresstoPC returns the raw number as the address.  PC could be an
 * abstract type, in our case, PC is the raw address.
 */
#define addressToPC(pc) pc

// Macros for branches. Note: don't put inside a "match" statement, since
// the ordering is changed and multiple copies may be made

#define COND_JUMP(name, size, relocd, cond)                                                                            \
    result.rtl = new RTL(pc, stmts);                                                                                   \
    BranchStatement *jump = new BranchStatement;                                                                       \
    result.rtl->appendStmt(jump);                                                                                      \
    result.numBytes = size;                                                                                            \
    jump->setDest((relocd - ADDRESS::g(delta)).native());                                                              \
    jump->setCondType(cond);                                                                                           \
    SHOW_ASM(name << " " << relocd.native())

// This one is X86 specific
#define SETS(name, dest, cond)                                                                                         \
    BoolAssign *bs = new BoolAssign(8);                                                                                \
    bs->setLeftFromList(stmts);                                                                                        \
    stmts->clear();                                                                                                    \
    result.rtl = new RTL(pc, stmts);                                                                                   \
    result.rtl->appendStmt(bs);                                                                                        \
    bs->setCondType(cond);                                                                                             \
    result.numBytes = 3;                                                                                               \
    SHOW_ASM(name << " " << dest)

#endif
