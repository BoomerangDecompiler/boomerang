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

/*==============================================================================
 * FILE:       decoder.h
 * OVERVIEW:   The interface to the instruction decoder.
 *============================================================================*/

/* 
 * $Revision$
 * 08 Apr 02 - Mike: Mods for boomerang
 */

#ifndef _DECODER_H_
#define _DECODER_H_

#include <list>
#include "types.h"

class Exp;
class RTL;
class BinaryFile;
class Prog;
class RTLInstDict;

// These are the instruction classes defined in "A Transformational Approach to
// Binary Translation of Delayed Branches" for SPARC instructions.
// Extended for HPPA. Ignored by machines with no delay slots
enum ICLASS {
    NCT,            // Non Control Transfer
    SD,             // Static Delayed
    DD,             // Dynamic Delayed
    SCD,            // Static Conditional Delayed
    SCDAN,          // Static Conditional Delayed, Anulled if Not taken
    SCDAT,          // Static Conditional Delayed, Anulled if Taken
    SU,             // Static Unconditional (not delayed)
    SKIP,           // Skip successor
//  TRAP,           // Trap
    NOP,            // No operation (e.g. sparc BN,A)
    // HPPA only
    DU,             // Dynamic Unconditional (not delayed)
    NCTA            // Non Control Transfer, with following instr Anulled
};
/*==============================================================================
 * The DecodeResult struct contains all the information that results from
 * calling the decoder. This prevents excessive use of confusing
 * reference parameters.
 *============================================================================*/
struct DecodeResult {
    /*
     * Resets all the fields to their default values.
     */
    void reset();

    /*
     * The number of bytes decoded in the main instruction
     */
    int numBytes;

    /*
     * The RTL constructed (if any).
     */
    RTL* rtl;

    /*
     * Indicates whether or not a valid instruction was decoded.
     */
    bool valid;

    /*
     * The class of the instruction decoded. Will be one of the classes
     * described in "A Transformational Approach to Binary Translation of
     * Delayed Branches" (plus two more HPPA specific entries).
     * Ignored by machines with no delay slots
     */
    ICLASS type;

    /*
     * If non zero, this field represents a new native address to be used as
     * the out-edge for this instruction's BB. At present, only used for
     * the SPARC call/add caller prologue
     */
    ADDRESS forceOutEdge;

};

/*==============================================================================
 * The NJMCDecoder class is a class that contains NJMC generated decoding
 * methods.
 *============================================================================*/
class NJMCDecoder {
public:
    /*
     * Constructor and destructor
     */
    NJMCDecoder();
virtual ~NJMCDecoder() {};

    /*
     * Decodes the machine instruction at pc and returns an RTL instance for
     * the instruction.
     */
virtual DecodeResult& decodeInstruction (ADDRESS pc, int delta) = 0;

    /*
     * Disassembles the machine instruction at pc and returns the number of
     * bytes disassembled. Assembler output goes to global _assembly
     */
virtual int decodeAssemblyInstruction (ADDRESS pc, int delta) = 0;

    RTLInstDict& getRTLDict() { return RTLDict; }

protected:

    /*
     * Given an instruction name and a variable list of Exps
     * representing the actual operands of the instruction, use the
     * RTL template dictionary to return the instantiated RTL
     * representing the semantics of the instruction. This method also
     * displays a disassembly of the instruction if the relevant
     * compilation flag has been set.
     */
    std::list<Exp*>* instantiate(ADDRESS pc, const char* name, ...);

    /*
     * Similarly, given a parameter name and a list of Exp*'s
     * representing sub-parameters, return a fully substituted
     * Exp for the whole expression
     */
    Exp* instantiateNamedParam(char *name, ...);

    /*
     * In the event that it's necessary to synthesize the call of
     * a named parameter generated with instantiateNamedParam(),
     * this substituteCallArgs() will substitute the arguments that
     * follow into the expression.
     * Should only be used after e = instantiateNamedParam(name, ..);
     */
    void substituteCallArgs(char *name, Exp*& exp, ...);

    /*
     * This used to be the UNCOND_JUMP macro; it's extended to handle jumps to
     * other procedures
     */
    void unconditionalJump(const char* name, int size, ADDRESS relocd,
        int delta, ADDRESS pc, std::list<Exp*>* exps, DecodeResult& result);

    /*
     * String for the constructor names (displayed with use "-c")
     */
    char    constrName[84];

	/* decodes a number */
	Exp* dis_Num(unsigned num);
	/* decodes a register */
	Exp* dis_Reg(int regNum);

    // Public dictionary of instruction patterns, and other information
    // summarised from the SSL file (e.g. source machine's endianness)
    RTLInstDict RTLDict;
};

// Function used to guess whether a given
// pc-relative address is the start of a function

/*
 * Does the instruction at the given offset correspond to a caller prologue?
 * NOTE: Implemented in the decoder.m files
 */
bool isFuncPrologue(ADDRESS hostPC);


/*==============================================================================
 * These are the macros that each of the .m files depend upon.
 *============================================================================*/
#ifdef  DEBUG_DECODER
#define SHOW_ASM(output) cout<< hex << pc << dec << ": " << output << endl;
#else
#define SHOW_ASM(output)
#endif

/*
 * addresstoPC returns the raw number as the address.  PC could be an
 * abstract type, in our case, PC is the raw address.
 */
#define addressToPC(pc)  pc

// Macros for branches. Note: don't put inside a "match" statement, since
// the ordering is changed and multiple copies may be made

#define COND_JUMP(name, size, relocd, cond) \
    HLJcond* jump = new HLJcond(pc, Exps); \
    result.rtl = jump; \
    result.numBytes = size; \
    jump->setDest(relocd-delta); \
    jump->setCondType(cond); \
    SHOW_ASM(name<<" "<<relocd)

// This one is X86 specific
#define SETS(name, dest, cond) \
    HLScond* scond = new HLScond(pc, Exps); \
    scond->setCondType(cond); \
    result.numBytes = 3; \
    result.rtl = scond; \
    SHOW_ASM(name<<" "<<dest)

/*==============================================================================
 * These are arrays used to map register numbers to their names.
 *============================================================================*/
extern char *r32_names[];
extern char *sr16_names[];
extern char *r8_names[];
extern char *r16_names[];
extern char *fp_names[];

/*==============================================================================
 * This array decodes scale field values in an index memory expression
 * to the scale factor they represent.
 *============================================================================*/
extern int  scale[];


// General purpose
void not_used(int unwanted);

/**********************************
 * These are the fetch routines.
 **********************************/

/*
 * Returns the byte (8 bits) starting at the given address.
 */
Byte getByte(ADDRESS lc);

/*
 * Returns the word (16 bits) starting at the given address.
 */
SWord getWord(ADDRESS lc);

/*
 * Returns the double (32 bits) starting at the given address.
 */
DWord getDword(ADDRESS lc);


#endif
