/*
 * Copyright (C) 1996-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       sparcdecoder.h
 * OVERVIEW:   The implementation of the instruction decoder for Sparc.
 *============================================================================*/

/* 
 * $Revision$
 * 06 Jun 02 - Trent: Created.
 *
 */

#ifndef SPARCDECODER
#define SPARCDECODER

class Prog;
class NJMCDecoder;
struct DecodeResult;

class SparcDecoder : public NJMCDecoder
{
public:
    /* Default constructor
     */
	SparcDecoder();


    /*
     * Decodes the machine instruction at pc and returns an RTL instance for
     * the instruction.
     */
virtual DecodeResult& decodeInstruction (ADDRESS pc, int delta);

    /*
     * Disassembles the machine instruction at pc and returns the number of
     * bytes disassembled. Assembler output goes to global _assembly
     */
virtual int decodeAssemblyInstruction (ADDRESS pc, int delta);

    /*
     * Indicates whether the instruction at the given address is a restore instruction.
     */
    bool    isRestore(ADDRESS hostPC);
private:
    /*
     * Various functions to decode the operands of an instruction into
     * a SemStr representation.
     */
    Exp*    dis_Eaddr(ADDRESS pc, int size = 0);
    Exp*    dis_RegImm(ADDRESS pc);

    void    unused(int x);
    HLJcond* createJcond(ADDRESS pc, std::list<Exp*>* exps, const char* name);
    bool    isFuncPrologue(ADDRESS hostPC);
    DWord   getDword(ADDRESS lc);

};

#endif
